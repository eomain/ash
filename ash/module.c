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

#include "ash/mem.h"
#include "ash/module.h"
#include "ash/obj.h"
#include "ash/type.h"
#include "ash/var.h"
#include "ash/util/hash.h"
#include "ash/util/map.h"

#define ROOT_NAME ""
#define VARIABLE_SIZE 37
#define FUNCTION_SIZE 37
#define SUBMODULE_SIZE 37

struct ash_module {
    const char *name;
    struct map *variable;
    struct map *function;
    struct map *module;
    struct ash_module *parent;
};

struct ash_module *
ash_module_new(const char *name)
{
    struct ash_module *module;
    module = ash_alloc(sizeof *module);
    module->name = name;

    struct hashmeta meta;
    hash_meta_string_init(&meta, VARIABLE_SIZE);
    module->variable = map_new(meta);
    hash_meta_string_init(&meta, FUNCTION_SIZE);
    module->function = map_new(meta);
    module->module = NULL;
    module->parent = NULL;
    return module;
}

struct ash_module *
ash_module_from(const char *name, struct ash_module *parent)
{
    struct ash_module *module;
    module = ash_module_new(name);
    module->parent = parent;
    return module;
}

static inline struct ash_module *
get(struct ash_module *module, const char *name)
{
    if (!module->module)
        return NULL;
    return map_get(module->module, (key_t *)name);
}

static inline struct ash_module *
set(struct ash_module *module, const char *name)
{
    struct ash_module *m;
    m = ash_module_from(name, module);

    if (!module->module) {
        struct hashmeta meta;
        hash_meta_string_init(&meta, SUBMODULE_SIZE);
        module->module = map_new(meta);
    }

    map_insert(module->module, (key_t *)name, m);
    return m;
}

struct ash_module *
ash_module_sub_get(struct ash_module *module, const char *name)
{
    return get(module, name);
}

struct ash_module *
ash_module_sub_set(struct ash_module *module, const char *name)
{
    if (get(module, name))
        return NULL;
    return set(module, name);
}

static struct ash_module *
ash_module_search(struct ash_module *module,
                  const char **path, size_t length)
{
    const char *name;
    size_t pos = 0;

    while (module && (pos < length)) {
        name = path[pos];
        module = get(module, name);
        pos++;
    }

    return module;
}

struct ash_var *
var_get(struct map *map, const char *id)
{
    struct ash_var *var;
    var = map_get(map, (key_t *)id);
    return var;
}

struct ash_var *
var_set(struct map *map, const char *id, struct ash_obj *obj)
{
    struct ash_var *var;
    if ((var = var_get(map, id))) {
        if (ash_var_mutable(var))
            ash_var_bind(var, obj);
        return var;
    }

    var = ash_var_new(id);
    ash_var_bind(var, obj);
    map_insert(map, (key_t *)id, var);
    return var;
}

struct ash_var *
var_set_override(struct map *map, const char *id,
                 struct ash_obj *obj)
{
    struct ash_var *var;
    if ((var = var_get(map, id))) {
        ash_var_bind(var, obj);
        return var;
    }

    var = ash_var_new(id);
    ash_var_bind(var, obj);
    map_insert(map, (key_t *)id, var);
    return var;
}

void
var_unset(struct map *map, const char *id)
{
    struct ash_var *var;
    if (!(var = var_get(map, id)))
        return;
    if (!ash_var_mutable(var))
        return;

    if ((var = map_remove(map, (key_t *)id)))
        ash_var_destroy(var);
}

struct ash_var *
ash_module_var_get(struct ash_module *module, const char *id)
{
    return var_get(module->variable, id);
}

struct ash_var *
ash_module_var_set(struct ash_module *module,
                   const char *id, struct ash_obj *obj)
{
    return var_set(module->variable, id, obj);
}

struct ash_var *
ash_module_var_set_override(struct ash_module *module,
                   const char *id, struct ash_obj *obj)
{
    return var_set_override(module->variable, id, obj);
}

void ash_module_var_unset(struct ash_module *module,
                          struct ash_var *var)
{
    var_unset(module->variable, ash_var_id(var));
}

struct ash_var *
ash_module_func_get(struct ash_module *module, const char *id)
{
    return var_get(module->function, id);
}

struct ash_var *
ash_module_func_set(struct ash_module *module,
                    const char *id, struct ash_obj *obj)
{
    return var_set(module->function, id, obj);
}

struct ash_var *
ash_module_func_unset(struct ash_module *module, const char *id)
{
    return map_remove(module->function, (key_t *)id);
}

struct ash_module *
ash_module_path(struct ash_module *mod,
                 struct ash_module_path *path)
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

    if (!(m = ash_module_search(m, p, length))) {
        if (type != ASH_PATH_ABS)
            m = ash_module_search(ash_module_root(), p, length);
    }

    return m;
}

static struct ash_module *module;

struct ash_module *
ash_module_root(void)
{
    return module;
}

static void init(void)
{
    module = ash_module_new(ROOT_NAME);
}

const struct ash_unit_module ash_module_module = {
    .init = init,
    .destroy = NULL
};
