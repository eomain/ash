/* Copyright 2018 eomain
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

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "ash/ash.h"
#include "ash/mem.h"
#include "ash/module.h"
#include "ash/io.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/tuple.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/var.h"
#include "ash/util/map.h"

#define ASH_LOCAL_BUCKET 37

static inline bool alpha(char c)
{
    return (islower(c) || isupper(c)) ? true: false;
}

static inline bool var_is_mutable(const char *id)
{
    char c;
    while ((c = (*id++))) {
        if (alpha(c))
            return isupper(c) ? false: true;
    }
    return true;
}

struct ash_var {
    struct ash_obj *obj;
    const char *id;
    bool mutable;
};

static inline void
ash_var_init(struct ash_var *var, const char *id)
{
    var->obj = NULL;
    var->id = id;
    var->mutable = var_is_mutable(id);
}

struct ash_var *
ash_var_new(const char *id)
{
    struct ash_var *var;
    var = ash_alloc(sizeof *var);
    ash_var_init(var, id);
    return var;
}

void
ash_var_destroy(struct ash_var *var)
{
    assert(var != NULL);
    ash_var_unbind(var);
    ash_free(var);
}

struct ash_obj *
ash_var_obj(struct ash_var *var)
{
    if (!var)
        return NULL;

    struct ash_obj *obj;
    if ((obj = var->obj))
        ash_obj_inc_rc(obj);
    return obj;
}

const char *
ash_var_id(struct ash_var *var)
{
    return var->id;
}

bool ash_var_mutable(struct ash_var *var)
{
    return (var) ? var->mutable: false;
}

void ash_var_bind(struct ash_var *var,
                  struct ash_obj *obj)
{
    if (var->obj) {
        if (!var->mutable)
            return;
        ash_var_unbind(var);
    }
    var->obj = obj;

    if (!ash_obj_nil(obj)) {
        if (obj->bound)
            ash_obj_inc_rc(obj);
        else
            obj->bound = true;
    }
}

void ash_var_bind_override(struct ash_var *var,
                           struct ash_obj *obj)
{
    if (var->obj)
        ash_var_unbind(var);
    var->obj = obj;

    if (!ash_obj_nil(obj)) {
        if (obj->bound)
            ash_obj_inc_rc(obj);
        else
            obj->bound = true;
    }
}

void ash_var_unbind(struct ash_var *var)
{
    if (var->obj)
        ash_obj_dec_rc(var->obj);
    var->obj = NULL;
}

struct ash_var *
ash_var_set(const char *id, struct ash_obj *obj)
{
    return ash_module_var_set(ash_module_root(), id, obj);
}

struct ash_var *
ash_var_set_override(const char *id, struct ash_obj *obj)
{
    return ash_module_var_set_override(ash_module_root(), id, obj);
}

struct ash_var *ash_var_get(const char *id)
{
    return ash_module_var_get(ash_module_root(), id);
}

void ash_var_unset(struct ash_var *var)
{
    ash_module_var_unset(ash_module_root(), var);
}

struct ash_var *
ash_var_func_set(const char *id, struct ash_obj *obj)
{
    return ash_module_func_set(ash_module_root(), id, obj);
}

struct ash_var *
ash_var_func_get(const char *id)
{
    return ash_module_func_get(ash_module_root(), id);
}

void
ash_var_func_unset(const char *id)
{
    ash_module_func_unset(ash_module_root(), id);
}

struct ash_env {
    struct ash_module *module;
    struct map *variable;
    struct map *function;
    struct ash_env *parent;
};

struct ash_env *
ash_env_parent(struct ash_env *env)
{
    return env->parent;
}

struct ash_module *
ash_env_module(struct ash_env *env)
{
    return env->module;
}

struct ash_env *
ash_env_new(struct ash_module *module)
{
    struct ash_env *env;
    env = ash_alloc(sizeof *env);
    env->parent = NULL;
    env->module = module;

    struct hashmeta meta;
    hash_meta_string_init(&meta, ASH_LOCAL_BUCKET);
    env->variable = map_new(meta);
    env->function = map_new(meta);
    return env;
}

struct ash_env *
ash_env_new_from(struct ash_module *module,
                 struct ash_env *parent)
{
    struct ash_env *env;
    env = ash_env_new(module);
    env->parent = parent;
    return env;
}

void
ash_env_set_module(struct ash_env *env, struct ash_module *module)
{
    env->module = module;
}

struct ash_var *
ash_var_env_set(struct ash_env *env, const char *id, struct ash_obj *obj)
{
    return var_set(env->variable, id, obj);
}

struct ash_var *
ash_var_env_get(struct ash_env *env, const char *id)
{
    struct ash_var *av = NULL;

    do {
        av = var_get(env->variable, id);
    } while ((!av) && (env = env->parent));

    return av;
}

void ash_var_env_unset(struct ash_env *env, struct ash_var *var)
{
    var_unset(env->variable, var->id);
}

struct ash_var *
ash_var_env_func_set(struct ash_env *env,
                 const char *id, struct ash_obj *obj)
{
    return var_set(env->function, id, obj);
}

struct ash_var *
ash_var_env_func_get(struct ash_env *env, const char *id)
{
    struct ash_var *av = NULL;

    do {
        av = var_get(env->function, id);
    } while ((!av) && (env = env->parent));

    return av;
}

void
ash_var_env_func_unset(struct ash_env *env, struct ash_var *var)
{
    var_unset(env->function, var->id);
}

void ash_env_destroy(struct ash_env *env)
{
    map_destroy(env->variable);
    map_destroy(env->function);
    ash_free(env);
}
