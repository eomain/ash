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
#include <stdlib.h>
#include <string.h>

#include "ash/ash.h"
#include "ash/mem.h"
#include "ash/io.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/tuple.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/var.h"

#define ASH_GLOBAL_BUCKET 937
#define ASH_LOCAL_BUCKET   37
#define ASH_MODULE_BUCKET  37
#define ASH_LIST_LENGTH    30

static size_t ash_compute_hash(struct ash_cache *, const char *);
static struct ash_var *ash_var_access(struct ash_cache *, size_t, const char *);
static void ash_var_insert(struct ash_cache *, size_t, const char *, struct ash_var *);
static struct ash_var *ash_var_remove(struct ash_cache *, size_t, const char *);

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
    struct ash_var *next;
};

static inline void
ash_var_init(struct ash_var *var, const char *id)
{
    var->obj = NULL;
    var->id = id;
    var->mutable = var_is_mutable(id);
}

static struct ash_var *
ash_var_new(const char *id)
{
    struct ash_var *var;
    var = ash_alloc(sizeof *var);
    var->next = NULL;
    ash_var_init(var, id);
    return var;
}

static void
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

bool ash_var_mutable(struct ash_var *var)
{
    return (var) ? var->mutable: false;
}

void ash_var_bind(struct ash_var *var, struct ash_obj *obj)
{
    if (var->obj) {
        if (!var->mutable)
            return;
        ash_var_unbind(var);
    }
    var->obj = obj;

    if (!ash_obj_nil(obj)) {
        obj->bound = true;
        ash_obj_inc_rc(obj);
    }
}

void ash_var_bind_override(struct ash_var *var, struct ash_obj *obj)
{
    if (var->obj)
        ash_var_unbind(var);
    var->obj = obj;

    if (!ash_obj_nil(obj)) {
        obj->bound = true;
        ash_obj_inc_rc(obj);
    }
}

void ash_var_unbind(struct ash_var *var)
{
    if (var->obj)
        ash_obj_dec_rc(var->obj);
    var->obj = NULL;
}

struct ash_cache {
    size_t size;
    void **bucket;
    struct ash_cache *next;
};

void ash_cache_init(struct ash_cache *cache, size_t size)
{
    cache->size = size;
    size_t bytes = (sizeof *cache->bucket) * size;
    cache->bucket = ash_zalloc(bytes);
    cache->next = NULL;
}

void ash_cache_clear(struct ash_cache *cache)
{
    assert(cache != NULL);

    if (cache->bucket) {
        struct ash_var *av, *next;
        for (size_t i = 0; i < cache->size; ++i) {
            if ((av = cache->bucket[i])) {
                do {
                    next = av->next;
                    ash_var_destroy(av);
                } while ((av = next));
            }
        }

        ash_free(cache->bucket);
        cache->bucket = NULL;
        cache->size = 0;
    }

    if (cache->next) {
        ash_cache_clear(cache->next);
        ash_free(cache->next);
        cache->next = NULL;
    }
}

static struct ash_cache *
ash_cache_next(struct ash_cache *cache)
{
    if (!cache->next) {
        cache->next = ash_alloc(sizeof *cache->next);
        ash_cache_init(cache->next, cache->size);
    }

    return cache->next;
}

struct ash_var *ash_var_cache_get(struct ash_cache *, const char *);

struct ash_var *
ash_var_cache_set(struct ash_cache *cache,
                const char *id, struct ash_obj *obj)
{
    struct ash_var *var;
    if ((var = ash_var_cache_get(cache, id))) {
        if (ash_var_mutable(var))
            ash_var_bind(var, obj);
        return var;
    }

    var = ash_var_new(id);
    ash_var_bind(var, obj);

    size_t hash = ash_compute_hash(cache, id);
    ash_var_insert(cache, hash, id, var);
    return var;
}

static struct ash_var *
ash_var_cache_set_override(struct ash_cache *cache,
                const char *id, struct ash_obj *obj)
{
    struct ash_var *var;
    if ((var = ash_var_cache_get(cache, id))) {
        ash_var_bind(var, obj);
        return var;
    }

    var = ash_var_new(id);
    ash_var_bind(var, obj);

    size_t hash = ash_compute_hash(cache, id);
    ash_var_insert(cache, hash, id, var);
    return var;
}

struct ash_var *
ash_var_cache_get(struct ash_cache *cache, const char *id)
{
    size_t hash = ash_compute_hash(cache, id);
    return ash_var_access(cache, hash, id);
}

void
ash_var_cache_unset(struct ash_cache *cache, struct ash_var *var)
{
    if (!var || !ash_var_mutable(var))
        return;

    const char *id;
    id = var->id;
    size_t hash = ash_compute_hash(cache, id);
    ash_var_remove(cache, hash, id);
    ash_var_destroy(var);
}

static inline bool
ash_var_check(struct ash_var *var, const char *id)
{
    return (var && strcmp(var->id, id) == 0) ? true: false;
}

static struct ash_var *
ash_var_access(struct ash_cache *cache, size_t k, const char *id)
{
    assert(cache && cache->bucket);
    assert(k < cache->size);

    struct ash_var *var;
    if ((var =  cache->bucket[k])) {
        do {
            if (ash_var_check(var, id))
                return var;

            var = var->next;
        } while (var);
    }

    if (cache->next)
        return ash_var_access(cache->next, k, id);

    return NULL;
}

static void
ash_var_insert(struct ash_cache *cache, size_t k,
               const char *id, struct ash_var *v)
{
    assert(k < cache->size);

    struct ash_var *var;
    if (!(var = cache->bucket[k])) {
        cache->bucket[k] = v;
        return;
    }

    int limit = 0;

    do {
        var = var->next;

        if (ash_var_check(var, id) ||
            limit == ASH_LIST_LENGTH)
            break;

        limit++;

    } while (var);

    if (limit == 0)
        cache->bucket[k] = v;
    else if (var && limit < ASH_LIST_LENGTH)
        var->next = v;
    else
        ash_var_insert(ash_cache_next(cache), k, id, v);
}

static struct ash_var *
ash_var_remove(struct ash_cache *cache, size_t k, const char *id)
{
    assert(k < cache->size);

    struct ash_var *var, *v = NULL;
    if ((var = cache->bucket[k])) {
        do {
            if (ash_var_check(var, id)) {
                if (v)
                    v->next = var->next;
                else
                    cache->bucket[k] = NULL;
                return var;
            }

            v = var;
            var = var->next;
        } while (var);
    }

    if (cache->next)
        return ash_var_remove(cache->next, k, id);

    return NULL;
}

struct ash_module {
    const char *name;
    struct ash_cache variable;
    struct ash_cache function;
    struct ash_cache module;
    struct ash_module *parent;
};

static void ash_module_init(struct ash_module *);

struct ash_module *ash_module_new(const char *name)
{
    struct ash_module *module;
    module = ash_alloc(sizeof *module);
    module->name = name;
    ash_module_init(module);
    module->parent = NULL;
    return module;
}

struct ash_module *ash_module_new_within(const char *name, struct ash_module *parent)
{
    struct ash_module *module;
    module = ash_module_new(name);
    module->parent = parent;
    return module;
}

static void ash_module_init(struct ash_module *mod)
{
    ash_cache_init(&mod->variable, ASH_GLOBAL_BUCKET);
    ash_cache_init(&mod->function, ASH_GLOBAL_BUCKET);
    ash_cache_init(&mod->module,   ASH_MODULE_BUCKET);
}

struct ash_mod {
    const char *id;
    struct ash_module *module;
    struct ash_mod *next;
};

static struct ash_module module = {
    .name = "",

    .variable = {
        .size = ASH_GLOBAL_BUCKET,
        .bucket = NULL,
        .next = NULL
    },

    .function = {
        .size = ASH_GLOBAL_BUCKET,
        .bucket = NULL,
        .next = NULL
    },

    .module = {
        .size = ASH_MODULE_BUCKET,
        .bucket = NULL,
        .next = NULL
    },

    .parent = NULL
};

struct ash_module *
ash_module_root(void)
{
    return &module;
}


static inline void
ash_mod_init(struct ash_mod *mod, const char *id)
{
    mod->id = id;
    mod->module = NULL;
}

static struct ash_mod *
ash_mod_new(const char *id)
{
    struct ash_mod *mod;
    mod = ash_alloc(sizeof *mod);
    ash_mod_init(mod, id);
    mod->next = NULL;
    return mod;
}

static inline void
ash_mod_set(struct ash_mod *mod, struct ash_module *module)
{
    mod->module = module;
}

static inline bool
ash_mod_check(struct ash_mod *mod, const char *id)
{
    return (mod && strcmp(mod->id, id) == 0) ? true: false;
}

static struct ash_mod *
ash_mod_access(struct ash_cache *cache, size_t k, const char *id)
{
    assert(cache && cache->bucket);
    assert(k < cache->size);

    struct ash_mod *mod;
    if ((mod = cache->bucket[k])) {
        do {
            if (ash_mod_check(mod, id))
                return mod;

            mod = mod->next;
        } while (mod);
    }

    if (cache->next)
        return ash_mod_access(cache->next, k, id);

    return NULL;
}

static void
ash_mod_insert(struct ash_cache *cache, size_t k,
               const char *id, struct ash_mod *v)
{
    assert(k < cache->size);

    struct ash_mod *mod;
    if (!(mod = cache->bucket[k])) {
        cache->bucket[k] = v;
        return;
    }

    int limit = 0;

    do {
        mod = mod->next;

        if (ash_mod_check(mod, id) ||
            limit == ASH_LIST_LENGTH)
            break;

        limit++;

    } while (mod);

    if (limit == 0)
        cache->bucket[k] = v;
    else if (mod && limit < ASH_LIST_LENGTH)
        mod->next = v;
    else
        ash_mod_insert(ash_cache_next(cache), k, id, v);
}

static void
ash_mod_remove(struct ash_cache *cache, size_t k, const char *id)
{
    assert(k < cache->size);

    struct ash_mod *mod, *v = NULL;
    if ((mod = cache->bucket[k])) {
        do {
            if (ash_mod_check(mod, id)) {
                if (v)
                    v->next = mod->next;
                else
                    cache->bucket[k] = NULL;
                return;
            }

            v = mod;
            mod = mod->next;
        } while (mod);
    }

    if (cache->next)
        return ash_mod_remove(cache->next, k, id);
}

struct ash_mod *
ash_mod_cache_get(struct ash_cache *, const char *);

struct ash_mod *
ash_mod_cache_set(struct ash_cache *cache, const char *id, struct ash_module *module)
{
    struct ash_mod *mod;
    mod = ash_mod_new(id);

    ash_mod_set(mod, module);

    size_t hash = ash_compute_hash(cache, id);
    ash_mod_insert(cache, hash, id, mod);
    return mod;
}

struct ash_mod *
ash_mod_cache_get(struct ash_cache *cache, const char *id)
{
    size_t hash = ash_compute_hash(cache, id);
    return ash_mod_access(cache, hash, id);
}

struct ash_module *
ash_sub_module_set(struct ash_module *mod, const char *name)
{
    struct ash_module *module;
    if ((ash_mod_cache_get(&mod->module, name)))
        return NULL;
    module = ash_module_new_within(name, mod);
    ash_mod_cache_set(&mod->module, name, module);
    return module;
}

struct ash_module *
ash_sub_module_get(struct ash_module *mod, const char *name)
{
    struct ash_mod *m;
    m = ash_mod_cache_get(&mod->module, name);
    return (m) ? m->module: NULL;
}

static struct ash_module *
ash_module_search(struct ash_module *mod, const char **path, size_t length)
{
    const char *name;
    size_t pos = 0;

    while (mod && (pos < length)) {
        name = path[pos];
        mod = ash_sub_module_get(mod, name);
        pos++;
    }

    return mod;
}

struct ash_module *
ash_module_path(struct ash_module *mod, struct ash_module_path *path)
{
    struct ash_module *m = NULL;
    enum ash_module_path_type type;
    const char **p = path->path;
    size_t length = path->length;
    type = path->type;

    if (type == ASH_PATH_ABS)
        m = ash_module_root();
    else if (type == ASH_PATH_CUR)
        m = mod;
    else if (type == ASH_PATH_REL)
        m = (mod) ? mod->parent: NULL;

    if (!(m = ash_module_search(m, p, length)))
        m = ash_module_search(ash_module_root(), p, length);

    return m;
}

struct ash_var *
ash_module_set(struct ash_module *mod, const char *id, struct ash_obj *obj)
{
    return ash_var_cache_set(&mod->variable, id, obj);
}

struct ash_var *
ash_module_get(struct ash_module *mod, const char *id)
{
    return ash_var_cache_get(&mod->variable, id);
}

void ash_module_unset(struct ash_module *mod, struct ash_var *var)
{
    ash_var_cache_unset(&mod->variable, var);
}

struct ash_var *
ash_module_func_set(struct ash_module *mod,
                    const char *id, struct ash_obj *obj)
{
    return ash_var_cache_set(&mod->function, id, obj);
}

struct ash_var *
ash_module_func_get(struct ash_module *mod, const char *id)
{
    return ash_var_cache_get(&mod->function, id);
}

void
ash_module_func_unset(struct ash_module *mod, struct ash_var *var)
{
    ash_var_cache_unset(&mod->function, var);
}

struct ash_var *ash_var_set(const char *id, struct ash_obj *obj)
{
    return ash_module_set(&module, id, obj);
}

struct ash_var *ash_var_set_override(const char *id, struct ash_obj *obj)
{
    return ash_var_cache_set_override(&module.variable, id, obj);
}

struct ash_var *ash_var_get(const char *id)
{
    return ash_module_get(&module, id);
}

void ash_var_unset(struct ash_var *var)
{
    ash_module_unset(&module, var);
}

struct ash_var *
ash_var_func_set(const char *id, struct ash_obj *obj)
{
    return ash_module_func_set(&module, id, obj);
}

struct ash_var *
ash_var_func_get(const char *id)
{
    return ash_module_func_get(&module, id);
}

void
ash_var_func_unset(struct ash_var *var)
{
    ash_module_func_unset(&module, var);
}

struct ash_env {
    struct ash_module *module;
    struct ash_cache variable;
    struct ash_cache function;
    struct ash_env *parent;
};

struct ash_env *ash_env_parent(struct ash_env *env)
{
    return env->parent;
}

struct ash_module *ash_env_module(struct ash_env *env)
{
    return env->module;
}

struct ash_env *ash_env_new(struct ash_module *module)
{
    struct ash_env *env;
    env = ash_alloc(sizeof *env);
    env->parent = NULL;
    env->module = module;
    ash_cache_init(&env->variable, ASH_LOCAL_BUCKET);
    ash_cache_init(&env->function, ASH_LOCAL_BUCKET);
    return env;
}

struct ash_env *ash_env_new_from(struct ash_module *module, struct ash_env *parent)
{
    struct ash_env *env;
    env = ash_env_new(module);
    env->parent = parent;
    return env;
}

void ash_env_set_module(struct ash_env *env, struct ash_module *mod)
{
    env->module = mod;
}

struct ash_var *
ash_var_env_set(struct ash_env *env, const char *id, struct ash_obj *obj)
{
     return ash_var_cache_set(&env->variable, id, obj);
}

struct ash_var *
ash_var_env_get(struct ash_env *env, const char *id)
{
    struct ash_var *av = NULL;

    do {
        av = ash_var_cache_get(&env->variable, id);
    } while ((!av) && (env = env->parent));

    return av;
}

void ash_var_env_unset(struct ash_env *env, struct ash_var *var)
{
    ash_var_cache_unset(&env->variable, var);
}

struct ash_var *
ash_var_env_func_set(struct ash_env *env,
                 const char *id, struct ash_obj *obj)
{
    return ash_var_cache_set(&env->function, id, obj);
}

struct ash_var *
ash_var_env_func_get(struct ash_env *env, const char *id)
{
    struct ash_var *av = NULL;

    do {
        av = ash_var_cache_get(&env->function, id);
    } while ((!av) && (env = env->parent));

    return av;
}

void
ash_var_env_func_unset(struct ash_env *env, struct ash_var *var)
{
    ash_var_cache_unset(&env->function, var);
}

void ash_env_destroy(struct ash_env *env)
{
    ash_cache_clear(&env->variable);
    ash_cache_clear(&env->function);
    ash_free(env);
}

static size_t
ash_compute_hash(struct ash_cache *cache, const char *s)
{
    assert(cache != NULL);
    size_t hash = 0;
    size_t bucket = cache->size;
    int c;
    while ((c = *(s++)))
        hash = (((hash << 3) + c * 5) + hash ) % bucket;
    return hash;
}

static void init(void)
{
    ash_module_init(&module);
}

const struct ash_unit_module ash_module_var = {
    .init = init,
    .destroy = NULL
};
