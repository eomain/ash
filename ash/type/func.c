/* Copyright 2019 eomain
   this program is licensed under the 2-clause BSD license
   see COPYING for the full license info

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stddef.h>

#include "ash/func.h"
#include "ash/iter.h"
#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/str.h"
#include "ash/var.h"
#include "ash/lang/ast.h"
#include "ash/lang/runtime.h"

#define ASH_FUNC_TYPENAME "function"
#define ASH_FUNC_ANONYMOUS "(?())"
#define ASH_SYMBOL_ARGS "@"

struct ash_func {
    struct ash_obj obj;
    struct ast_prog prog;
    const char *name;
    struct ast_param *param;
};

static void dealloc(struct ash_obj *obj)
{

}

static struct ash_obj *
boolean(struct ash_obj *o)
{
    return ash_bool_from((o) ? true: false);
}

static const char *name()
{
    return ASH_FUNC_TYPENAME;
}

static struct ash_obj *string(struct ash_obj *obj)
{
    struct ash_func *func;
    func = (struct ash_func *) obj;
    return ash_str_from(func->name ? func->name: ASH_FUNC_ANONYMOUS);
}

static struct ash_base base = {
    .ops = {
        .nt = NULL,
        .sb = NULL,

        .eq = NULL,
        .ne = NULL,
        .gt = NULL,
        .lt = NULL,

        .and = NULL,
        .or  = NULL,

        .add = NULL,
        .sub = NULL,
        .mul = NULL,
        .div = NULL,
        .mod = NULL
    },

    .into = {
        .boolean = boolean,
        .string  = string
    },

    .iter = ash_base_iter_default,
    .dealloc = dealloc,
    .name = name
};

struct ash_obj *ash_func_new(void)
{
    struct ash_func *func;
    func = ash_zalloc(sizeof *func);
    func->name = NULL;
    func->param = NULL;

    struct ash_obj *obj;
    obj = (struct ash_obj *) func;
    ash_obj_init(obj, &base);
    return obj;
}

void ash_func_set(struct ash_obj *obj, const char *name, struct ast_prog prog,
                  struct ast_param *param)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_func *func;
        func = (struct ash_func *) obj;
        func->prog = prog;
        func->param = param;
        func->name = name;
    }
}

struct ash_obj *ash_func_from(const char *name, struct ast_prog prog,
                              struct ast_param *param)
{
    struct ash_obj *obj;
    obj = ash_func_new();
    ash_func_set(obj, name, prog, param);
    return obj;
}

static void ash_func_set_args(struct ash_env *env, struct ash_obj *args,
                              struct ast_param *param)
{
    const char *id;
    struct ash_obj *argv;
    struct ash_iter iter;
    ash_var_env_set(env, ash_strcpy(ASH_SYMBOL_ARGS), args);

    ash_iter_init(&iter, args);
    while (param) {
        argv = ash_iter_next(&iter);
        id = ash_strcpy(param->id);
        ash_var_env_set(env, id, argv);
        param = param->next;
    }
}

struct ash_obj *
ash_func_exec(struct ash_obj *obj, struct ash_runtime_env *renv,
              struct ash_obj *args)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_func *func;
        func = (struct ash_func *) obj;

        struct ash_runtime_prog prog;
        struct ash_runtime_env nenv;
        struct ash_obj *ret;
        struct ash_env *env;
        env = ash_env_new_from(renv->module, renv->env);

        if (args)
            ash_func_set_args(env, args, func->param);

        runtime_env_init(&nenv, renv->module, env);
        runtime_prog_init(&prog, func->prog, nenv);
        ret = runtime_exec_func(&prog);
        ash_env_destroy(env);
        return ret;
    }

    return NULL;
}
