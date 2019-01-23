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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ash.h"
#include "mem.h"
#include "io.h"
#include "ops.h"
#include "var.h"

/* ash atomic variable */
#define ASH_ATOMIC 0
/* ash composite variable - array  */
#define ASH_COMPOSITE 1

/* max number of namespaces */
#define ASH_NS_LIMIT 255

#define GLOBAL_SIZE 255
#define GLOBAL_RATE 2
#define BUCKET_SIZE 937

/* ash nil value - may denote empty string */
static char nil = ASH_NIL;

struct ash_array {
    /* number of data elements in the array  */
    size_t len;
    const char **data;
    /* size of the buffer */
    size_t size;
};

static void
ash_array_resize(struct ash_array *array, size_t len)
{
    size_t size = sizeof *array->data;
    if (array->len == 0){
        array->data = ash_alloc(size * len);
        array->size = len;
        array->len = 0;
    } else {
        array->size += len;
        array->data = ash_realloc((void *)array->data, size * array->size);
    }
}

static const char *
ash_array_get_value(struct ash_array *array, size_t index)
{
    assert(array);
    if (array->len > index)
        return array->data[index];
    else
        return &nil;
}

static inline int
ash_array_set_value(struct ash_array *array, int argc, const char **argv)
{
    int space = (int) array->size - (int) array->len - argc;
    if (space <= 0)
        ash_array_resize(array, argc);
    size_t i = array->len > 0 ? array->len -1: 0;
    for (; i < argc; ++i){
        assert( i < array->size );
        array->data[i] = argv[i];
    }
    array->len += argc;
}

static void
ash_array_unset(struct ash_array *array)
{
    for (size_t i = 0; i < array->len; ++i){
        if (array->data[i])
            ash_free((void *)array->data[i]);
    }

    if (array->data){
        ash_free(array->data);
        array->data = NULL;
    }

    array->len = 0;
    array->size = 0;
}

struct ash_var {
    int id;
    int type;
    int nil;
    const char *name;
    union {
        const char *atom;
        struct ash_array comp;
    } value;
    int rodata;
};

static struct ash_var *
ash_var_assert(struct ash_var *var)
{
    if (!var)
        return NULL;

    if (var->rodata == ASH_RODATA){
        ash_print_err(perr(RODATA_ERR));
        return NULL;
    }

    return var;
}

int ash_var_check_composite(struct ash_var *var)
{
    return var && var->type == ASH_COMPOSITE? 0: -1;
}

static void ash_var_init(struct ash_var *var, int type,
                         const char *name, const char *value, int ro)
{
    assert(var != NULL);
    var->type = type;
    var->nil = 0;
    if (type == ASH_ATOMIC && !value)
        var->nil = 1;
    var->name = name;
    var->value.atom = value;
    var->rodata = ro;
}

struct {
    size_t len;
    size_t pos;
    struct ash_var *vars;
} static ash_global = {
    .len = GLOBAL_SIZE,
    .pos = 0,
    .vars = NULL
};

static void ash_global_resize(void)
{
    ash_global.len *= GLOBAL_RATE;
    size_t size = ash_global.len * sizeof (*ash_global.vars);
    ash_global.vars = ash_realloc(ash_global.vars, size);
}

static struct ash_var *ash_global_new(void)
{
    assert(ash_global.pos <= ash_global.len);
    if (ash_global.pos == (ash_global.len -2))
        ash_global_resize();
    size_t pos = ash_global.pos++;
    return &ash_global.vars[pos];
}

static struct ash_var ash_vars[] = {
    [ ASH_VERSION_ ] = {
        .id = ASH_VERSION_,
        .name = "VERSION",
        .value = ASH_VERSION,
        .rodata = 1
    },

    [ ASH_HOST ] = {
        .id = ASH_HOST,
        .name = "HOST",
        .value = NULL,
        .rodata = 1
    },

    [ ASH_PATH ] = {
        .id = ASH_PATH,
        .name = "PATH",
        .value = NULL,
        .rodata = 1
    },

    [ ASH_HOME ] = {
        .id = ASH_HOME,
        .name = "HOME",
        .value = NULL,
        .rodata = 1
    },

    [ ASH_PWD ] = {
        .id = ASH_PWD,
        .name = "PWD",
        .value = NULL,
        .rodata = 1
    },

    [ ASH_LOGNAME ] = {
        .id = ASH_LOGNAME,
        .name = "LOGNAME",
        .value = NULL,
        .rodata = 1
    }
};

struct ash_var *ash_var_find_builtin(int o)
{
    if (o < sizeof (ash_vars) / sizeof (ash_vars[0]))
        return &ash_vars[o];
    return NULL;
}

struct ash_var *ash_var_find(const char *s)
{
    const char *v = s;
    switch(v[0]){
        case 'V':
            if (v[1] == 'E' &&
                v[2] == 'R' &&
                v[3] == 'S' &&
                v[4] == 'I' &&
                v[5] == 'O' &&
                v[6] == 'N' &&
                !(v[7]))
                return ash_var_find_builtin(ASH_VERSION_);
            break;

        case 'H':
            if (v[1] == 'O'){
                if (v[2] == 'S' &&
                    v[3] == 'T' &&
                    !(v[4]))
                    return ash_var_find_builtin(ASH_HOST);
                else if (v[2] == 'M' &&
                         v[3] == 'E' &&
                         !(v[4]))
                    return ash_var_find_builtin(ASH_HOME);
            }
            break;

        case 'P':
            if (v[1] == 'A' &&
                v[2] == 'T' &&
                v[3] == 'H' &&
                !(v[4]))
                return ash_var_find_builtin(ASH_PATH);
            else if (v[1] == 'W' &&
                     v[2] == 'D' &&
                     !(v[3]))
                return ash_var_find_builtin(ASH_PWD);
            break;

        case 'L':
            if (v[1] == 'O' &&
                v[2] == 'G' &&
                v[3] == 'N' &&
                v[4] == 'A' &&
                v[5] == 'M' &&
                v[6] == 'E' &&
                !(v[7]))
                return ash_var_find_builtin(ASH_LOGNAME);
            break;
    }

    return ash_var_get(s);
}

static struct ash_var *default_bucket[BUCKET_SIZE] = { 0 };
static void *default_func[BUCKET_SIZE] = { 0 };

struct ash_var_set {
    size_t size;
    struct ash_var **bucket;
    void **func;

} static ash_bucket = {
    .size = BUCKET_SIZE,
    .bucket = default_bucket,
    .func = default_func
};

static inline void ash_var_set_init(struct ash_var_set *set)
{
    set->bucket = ash_alloc( sizeof (*set->bucket) * BUCKET_SIZE);
    set->func = ash_alloc( sizeof (void *) * BUCKET_SIZE );
    memset(set->bucket, 0, sizeof (*set->bucket) * BUCKET_SIZE);
    memset(set->func, 0, sizeof (void *) * BUCKET_SIZE);
    set->size = BUCKET_SIZE;
}

static void ash_var_set_clear(struct ash_var_set *set)
{
    if (set->bucket)
        ash_free(set->bucket);
    if (set->func)
        ash_free(set->func);
    set->size = 0;
}

static int ash_var_insert(struct ash_var_set *set, size_t k, struct ash_var *var)
{
    assert( k < set->size );
    set->bucket[k] = var;
    return 0;
}

static void ash_var_remove(struct ash_var_set *set, size_t k)
{
    assert( k < set->size );
    set->bucket[k] = NULL;
}

static inline struct ash_var *ash_var_access(struct ash_var_set *set, size_t k)
{
    assert( k < set->size );
    return set->bucket[k];
}

static int ash_func_insert(struct ash_var_set *set, size_t k, void *func)
{
    assert( k < set->size );
    set->func[k] = func;
    return 0;
}

static void ash_func_remove(struct ash_var_set *set, size_t k)
{
    assert( k < set->size );
    set->func[k] = NULL;
}

static inline void *ash_func_access(struct ash_var_set *set, size_t k)
{
    assert( k < set->size );
    return set->func[k];
}

static size_t ash_compute_hash(const char *s)
{
    size_t hash = 0;
    size_t bucket = ash_bucket.size;
    int c;
    while ((c = *(s++)))
        hash = (((hash << 3) + c * 5) + hash ) % bucket;
    return hash;
}

struct ash_var *
ash_var_set(const unsigned char *key, const char *value, int ro)
{
    size_t hash = ash_compute_hash(key);
    struct ash_var *var = ash_var_get(key);
    if ((var = ash_var_assert(var)))
        ash_var_unset(key);
    else
        var = ash_global_new();
    ash_var_init(var, ASH_ATOMIC, key, value, ro);
    ash_var_insert(&ash_bucket, hash, var);
    return var;
}

struct ash_var *
ash_var_set_array(const unsigned char *key,
                  int size, const char **values, int ro)
{
    size_t hash = ash_compute_hash(key);
    struct ash_var *var = ash_var_get(key);
    if ((var = ash_var_assert(var)))
        ash_var_unset(key);
    else
        var = ash_global_new();
    ash_var_init(var, ASH_COMPOSITE, key, NULL, ro);
    ash_array_set_value(&var->value.comp, size, values);
    ash_var_insert(&ash_bucket, hash, var);
    return var;
}

void *
ash_func_set(const unsigned char *key, void *func)
{
    size_t hash = ash_compute_hash(key);
    ash_func_insert(&ash_bucket, hash, func);
    return func;
}

int ash_var_insert_array(struct ash_var *var, int index, const char *value)
{
    assert( var );

    if (!ash_var_assert(var))
        return -1;

    if (var->type == ASH_COMPOSITE){
        struct ash_array *array = &var->value.comp;
        if (array->len > (size_t)index){
            array->data[index] = value;
            return 0;
        }
    }
    return -1;
}

static void ash_var_free(struct ash_var *var)
{
    if (var->rodata == ASH_DATA){
        if (var->type == ASH_ATOMIC)
            ash_free((void *)var->value.atom);
        else if (var->type == ASH_COMPOSITE)
            ash_array_unset(&var->value.comp);
    }
}

int ash_var_unset(const unsigned char *key)
{
    size_t hash = ash_compute_hash(key);
    struct ash_var *var = ash_var_access(&ash_bucket, hash);
    if (!var)
        return -1;
    ash_var_free(var);
    ash_var_remove(&ash_bucket, hash);
    return 0;
}

int ash_func_unset(const unsigned char *key)
{
    size_t hash = ash_compute_hash(key);
    void * func = ash_func_access(&ash_bucket, hash);
    if (!func)
        return -1;
    ash_func_remove(&ash_bucket, hash);
    return 0;
}

struct ash_var *ash_var_get(const char *key)
{
    size_t hash = ash_compute_hash(key);
    return ash_var_access(&ash_bucket, hash);
}

void *ash_func_get(const char *key)
{
    size_t hash = ash_compute_hash(key);
    return ash_func_access(&ash_bucket, hash);
}

const char *ash_var_array_value(struct ash_var *var, size_t index)
{
    if (!var)
        return NULL;
    if (var->type != ASH_COMPOSITE){
        ash_print_err("atomic value cannot be indexed");
        return NULL;
    }

    return ash_array_get_value(&var->value.comp, index);
}

void ash_var_set_builtin(int o, const char *value)
{
    struct ash_var *var = ash_var_find_builtin(o);
    if(var && var->id == o){
        if (var->type == ASH_ATOMIC)
            var->value.atom = value;
    }
}

const char *ash_var_get_value(struct ash_var *var)
{
    if (var){
        if (var->nil)
            return &nil;
        else if (var->type == ASH_ATOMIC)
            return var->value.atom;
        else if (var->type == ASH_COMPOSITE){
            struct ash_array *array = &var->value.comp;
            return ash_array_get_value(array, 0);
        }
    }
    return NULL;
}

char *ash_var_clone_value(struct ash_var *var)
{
    const char *value = ash_var_get_value(var);
    if (!value)
        return NULL;
    size_t len = strlen(value) + 1;
    char *s = ash_alloc(len);
    strcpy(s, (const char *) value);
    return s;
}

int ash_var_check_nil(struct ash_var *var)
{
    if (!var)
        return 1;
    return var->nil;
}

/* number of namespaces in use */
static int ns_use = 0;

struct ash_var_env {
    /* env arguments */
    struct ash_array local;
    /* the env namespace */
    struct ash_var_set ns;
    /* the parent env */
    struct ash_var_env *parent;
};

static struct ash_var_env ash_envs[ASH_NS_LIMIT];
static struct ash_var_env *ash_env;

static inline struct ash_var_env *env_new(void)
{
    struct ash_var_env *env = &ash_envs[ns_use];
    if (ns_use < ASH_NS_LIMIT)
        ns_use++;
    return env;
}

static void env_destroy(void)
{
    ash_env = ash_env->parent;
    if (ns_use > 0)
        --ns_use;
}

void ash_var_env_new(int argc, const char **argv)
{
    struct ash_var_env *env = env_new();
    if (argc > 0)
        ash_array_set_value(&env->local, argc, argv);
    ash_var_set_init(&env->ns);
    if (ash_env)
        env->parent = ash_env;
    ash_env = env;
}

void ash_var_env_destroy(void)
{
    if (ash_env){
        ash_array_unset(&ash_env->local);
        ash_var_set_clear(&ash_env->ns);
        env_destroy();
    }
}

static struct ash_var env_var = {
    .type = ASH_ATOMIC,
    .nil = 0,
    .rodata = ASH_STATIC
};

int ash_var_env_unset(const unsigned char *key)
{
    if (!ash_env)
        return -1;

    size_t hash = ash_compute_hash(key);
    struct ash_var *var = ash_var_access(&ash_env->ns, hash);
    if (!var)
        return -1;
    ash_var_free(var);
    ash_var_remove(&ash_env->ns, hash);
    return 0;
}

static struct ash_var_set *ash_var_env_find(size_t hash)
{
    assert(ash_env);
    struct ash_var_env *env = ash_env;
    do {
        if (ash_var_access(&env->ns, hash))
            return &env->ns;
    } while ((env = env->parent));

    return NULL;
}

struct ash_var *ash_var_env_set(const unsigned char *key, const char *value, int ro)
{
    size_t hash = ash_compute_hash(key);
    struct ash_var *var;
    if ((var = ash_var_env_get(key)))
        ash_var_env_unset(key);
    else
        var = ash_global_new();

    ash_var_init(var, ASH_ATOMIC, key, value, ro);

    if (ash_env) {
        struct ash_var_set *set = ash_var_env_find(hash);
        if (set)
            ash_var_insert(set, hash, var);
        else
            ash_var_insert(&ash_env->ns, hash, var);
    }
    else
        ash_var_insert(&ash_bucket, hash, var);
    return var;
}

static struct ash_var *ash_var_env_local(const char *name, int index)
{
    assert(ash_env);
    const char *local = ash_array_get_value(&ash_env->local, index);
    if (!local)
        return NULL;
    env_var.name = name;
    env_var.value.atom = local;
    return &env_var;
}

struct ash_var *ash_var_env_get(const unsigned char *key)
{
    if (!key)
        return NULL;

    if (ash_env){
        if (ash_stoi_ck(key)){
            int index = atoi(key);
            if (index > 0)
                return ash_var_env_local(key, index - 1);
        }

        size_t hash = ash_compute_hash(key);
        struct ash_var *var = NULL;
        struct ash_var_env *env = ash_env;

        do {
            struct ash_var_set *set = &env->ns;
            if ((var = ash_var_access(set, hash)))
                return var;
        } while ((env = env->parent));
    }

    return ash_var_find(key);
}

void ash_vars_init(void)
{
    size_t size = ash_global.len;
    ash_global.vars = ash_alloc(sizeof (*ash_global.vars) * size);
    memset(ash_bucket.bucket, 0, sizeof (*ash_bucket.bucket) * BUCKET_SIZE);
    memset(ash_bucket.func, 0, sizeof (void *) * BUCKET_SIZE);
}
