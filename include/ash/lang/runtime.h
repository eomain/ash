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

#ifndef ASH_LANG_RUNTIME_H
#define ASH_LANG_RUNTIME_H

#include "ash/module.h"
#include "ash/obj.h"
#include "ash/var.h"
#include "ash/lang/ast.h"

#define RUNTIME_ID_DEFAULT 0
#define RUNTIME_COMMAND_SIZE 512
#define RUNTIME_STACK_SIZE 16384

struct ash_runtime {
    int id;
    size_t ssize;
    size_t csize;
    struct ash_obj *aux;
    const char *command[RUNTIME_COMMAND_SIZE];
    struct ash_obj *aux_cmd[RUNTIME_COMMAND_SIZE];
    struct ash_obj *stack[RUNTIME_STACK_SIZE];

    struct ash_runtime_swap {
        size_t size;
        const char **command;
        struct ash_obj **objs;
    } swap;
};

extern void runtime_init(struct ash_runtime *, int);

struct ash_runtime_env {
    struct ash_runtime *rt;
    struct ash_module *module;
    struct ash_env *env;
};

static inline void
runtime_env_init(struct ash_runtime_env *renv, struct ash_module *module,
                 struct ash_env *env)
{
    renv->rt = NULL;
    renv->module = (module) ? module: ash_module_root();
    renv->env = env;
}

static inline void
runtime_env_rt_init(struct ash_runtime_env *renv, struct ash_runtime *rt,
                    struct ash_module *module, struct ash_env *env)
{
    renv->rt = rt;
    renv->module = (module) ? module: ash_module_root();
    renv->env = env;
}

static inline struct ash_var *
runtime_set_var(struct ash_runtime_env *renv, const char *id, struct ash_obj *obj)
{
    struct ash_var *av = NULL;
    struct ash_env *env;
    struct ash_module *mod;

    if ((env = renv->env)) {
        if ((av = ash_var_env_set(env, id, obj)))
            return av;
    }

    mod = renv->module;
    return ash_module_var_set(mod, id, obj);
}

static inline struct ash_var *
runtime_get_var(struct ash_runtime_env *renv, const char *id)
{
    struct ash_var *av = NULL;
    struct ash_env *env;
    struct ash_module *mod;

    if ((env = renv->env)) {
        if ((av = ash_var_env_get(env, id)))
            return av;
    }

    mod = renv->module;
    return ash_module_var_get(mod, id);
}

static inline void
runtime_unset_var(struct ash_runtime_env *renv, const char *id)
{
    struct ash_var *av = NULL;
    struct ash_env *env;
    struct ash_module *mod;

    if ((env = renv->env)) {
        if ((av = ash_var_env_get(env, id)))
            ash_var_env_unset(env, av);
    }

    if (!av) {
        mod = renv->module;
        if ((av = ash_module_var_get(mod, id)))
            ash_module_var_unset(mod, av);
    }
}

static inline struct ash_var *
runtime_get_func(struct ash_runtime_env *renv, const char *id)
{
    struct ash_var *av = NULL;
    struct ash_env *env;
    struct ash_module *mod;

    if ((env = renv->env)) {
        if ((av = ash_var_env_func_get(env, id)))
            return av;
    }

    mod = renv->module;
    return ash_module_func_get(mod, id);
}

struct ash_runtime_prog {
    struct ast_prog prog;
    struct ash_runtime_env env;
};

static inline void
runtime_prog_init(struct ash_runtime_prog *rt, struct ast_prog prog,
                  struct ash_runtime_env env)
{
    rt->prog = prog;
    rt->env = env;
}

extern int runtime_exec(struct ash_runtime_prog *);
struct ash_obj *runtime_exec_func(struct ash_runtime_prog *);

#endif
