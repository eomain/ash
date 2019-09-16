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

#include <assert.h>
#include <string.h>

#include "ash/exec.h"
#include "ash/func.h"
#include "ash/io.h"
#include "ash/iter.h"
#include "ash/macro.h"
#include "ash/mem.h"
#include "ash/module.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/range.h"
#include "ash/str.h"
#include "ash/tuple.h"
#include "ash/type.h"
#include "ash/var.h"
#include "ash/lang/ast.h"
#include "ash/lang/parser.h"
#include "ash/lang/runtime.h"
#include "ash/type/array.h"
#include "ash/type/map.h"
#include "ash/util/vec.h"

struct ash_runtime_context;

static void runtime_exec_stm(struct ash_runtime_context *, struct ast_stm *);
static struct ash_obj *runtime_call(struct ash_runtime_context *, struct ast_call *);
static bool runtime_bool_expr(struct ash_runtime_context *, struct ast_bool_expr *);
static struct ash_obj *runtime_eval_expr(struct ash_runtime_context *, struct ast_expr *);
static struct ash_obj *runtime_eval_bool_expr(struct ash_runtime_context *, struct ast_bool_expr *);

struct ash_runtime_manage {
    int count;
    struct ash_runtime **rts;
};

static struct ash_runtime_manage manage = {
    .count = 1,
    .rts = NULL
};

static struct ash_runtime *runtime_new(void)
{
    size_t size, index;
    struct ash_runtime *rt;

    index = manage.count;
    manage.count++;
    size = (manage.count * sizeof *manage.rts);
    manage.rts = ash_realloc(manage.rts, size);
    manage.rts[index] = ash_alloc(sizeof (struct ash_runtime));
    return manage.rts[index];
}

void runtime_init(struct ash_runtime *rt, int id)
{
    rt->id = id;
    rt->ssize = 0;
    rt->csize = 0;
    rt->aux = NULL;
    memset(rt->command, 0, sizeof rt->command);
    memset(rt->aux_cmd, 0, sizeof rt->aux_cmd);
    memset(rt->stack, 0, sizeof rt->stack);
}

void runtime_command_push(struct ash_runtime *rt, struct ash_obj *obj)
{
    if (rt->csize < RUNTIME_COMMAND_SIZE) {
        rt->aux_cmd[rt->csize] = obj;
        rt->command[rt->csize] = ash_str_get(ash_obj_str(obj));
        rt->csize++;
    }
}

static size_t runtime_command_argc(struct ash_runtime *rt)
{
    return rt->csize;
}

static const char **runtime_command_args(struct ash_runtime *rt)
{
    return rt->command;
}

static void runtime_command_clear(struct ash_runtime *rt)
{
    if (rt->csize > 0) {
        for (size_t i = 0; i < rt->csize; ++i) {
            if (rt->aux_cmd[i]) {
                ash_obj_dec_rc(rt->aux_cmd[i]);
                rt->aux_cmd[i] = NULL;
            }
        }
    }

    rt->csize = 0;
}

struct ash_runtime_context {
    struct ash_runtime_env env;
    enum ash_runtime_state {
        ASH_RUNTIME_STATE_RUN,
        ASH_RUNTIME_STATE_BRK,
        ASH_RUNTIME_STATE_NXT,
        ASH_RUNTIME_STATE_RET
    } state;
    struct ash_obj *ret;
};

static void
runtime_context_init(struct ash_runtime_context *context, struct ash_runtime_env env)
{
    context->env = env;
    context->state = ASH_RUNTIME_STATE_RUN;
    context->ret = NULL;
}

static inline struct ash_module *
runtime_context_module(struct ash_runtime_context *context)
{
    return context->env.module;
}

static inline struct ash_env *
runtime_context_env(struct ash_runtime_context *context)
{
    return context->env.env;
}

static void
runtime_context_env_new(struct ash_runtime_context *context)
{
    struct ash_env *env;
    struct ash_module *mod;
    mod = runtime_context_module(context);
    env = ash_env_new_from(mod, runtime_context_env(context));
    context->env.env = env;
}

static void
runtime_context_env_destroy(struct ash_runtime_context *context)
{
    struct ash_env *env;
    if (!(env = runtime_context_env(context)))
        return;
    context->env.env = ash_env_parent(env);
    ash_env_destroy(env);
}

static inline bool
runtime_context_is_run(struct ash_runtime_context *context)
{
    if (!context)
        return true;
    return context->state == ASH_RUNTIME_STATE_RUN;
}

static enum ash_runtime_state
runtime_context_get_state(struct ash_runtime_context *context)
{
    assert(context != NULL);
    return context->state;
}

static inline void
runtime_context_set_state(struct ash_runtime_context *context,
                          enum ash_runtime_state state)
{
    assert(context != NULL);
    context->state = state;
}

static inline void
runtime_context_set_ret(struct ash_runtime_context *context, struct ash_obj *obj)
{
    assert(context != NULL);
    context->ret = obj;
}

static struct ash_runtime rt = {
    .ssize = 0,
    .aux = NULL,
    .stack = { NULL }
};

static void runtime_push(struct ash_runtime *rt, struct ash_obj *obj)
{
    if (rt->ssize < RUNTIME_STACK_SIZE)
        rt->stack[rt->ssize] = obj;
}

static struct ash_obj *runtime_pop(struct ash_runtime *rt)
{
    return (rt->ssize > 0) ? rt->stack[rt->ssize]: NULL;
}

static inline struct ash_var *
runtime_var_set(struct ash_runtime_context *context, const char *id,
                struct ash_obj *obj)
{
    struct ash_env *env;

    if ((env = runtime_context_env(context)))
        return ash_var_env_set(env, id, obj);

    struct ash_module *mod;
    mod = runtime_context_module(context);
    return ash_module_var_set(mod, id, obj);
}

static inline struct ash_var *
runtime_func_set(struct ash_runtime_context *context, const char *id,
                 struct ash_obj *obj)
{
    struct ash_env *env;

    if ((env = runtime_context_env(context)))
        return ash_var_env_func_set(env, id, obj);

    struct ash_module *mod;
    mod = runtime_context_module(context);
    return ash_module_func_set(mod, id, obj);
}

static struct ash_module *
runtime_eval_path(struct ash_runtime_context *context, struct ast_var *av,
                  const char *id)
{
    struct ash_module *module;
    struct ast_path *path;
    struct ast_scope *scope;
    struct vec *vec;
    const char **paths;

    path = av->path;
    scope = path->path;
    vec = vec_from(path->length);

    for (size_t i = 0; i < path->length; ++i) {
        vec_push(vec, (char *)scope->id);
        scope = scope->next;
    }

    paths = (const char **) vec_get_ref(vec);

    struct ash_module_path mpath = {
        .type = path->type,
        .length = path->length,
        .path = paths
    };

    module = ash_module_path(runtime_context_module(context), &mpath);
    vec_destroy(vec);
    return module;
}

static inline const char *
runtime_eval_ref(struct ast_var *av)
{
    const char *id;
    id = av->id;
    if (av->ref)
        id++;
    return id;
}

static struct ash_obj *
runtime_eval_var(struct ash_runtime_context *context, struct ast_var *av)
{
    const char *id;
    struct ash_var *var = NULL;
    struct ash_env *env;
    struct ash_obj *obj = NULL;
    struct ash_module *module;

    id = runtime_eval_ref(av);
    if (av->path)
        module = runtime_eval_path(context, av, id);
    else
        module = runtime_context_module(context);

    if (!av->path && (env = runtime_context_env(context)))
        var = ash_var_env_get(env, id);

    if (!var && module) {
        if (!(var = ash_module_var_get(module, id)))
            var = ash_module_var_get(ash_module_root(), id);
    }

    if (var)
        obj = ash_var_obj(var);
    return obj;
}

static struct ash_obj *
runtime_eval_func(struct ash_runtime_context *context, struct ast_var *av,
                  struct ash_obj *args)
{
    const char *id;
    struct ash_var *var = NULL;
    struct ash_env *env;
    struct ash_obj *obj;
    struct ash_module *module;
    struct ash_runtime_env renv;

    id = runtime_eval_ref(av);
    if (av->path)
        module = runtime_eval_path(context, av, id);
    else
        module = runtime_context_module(context);

    if (!av->path && (env = runtime_context_env(context))) {
        if ((var = ash_var_env_func_get(env, id))) {
            runtime_env_init(&renv, module, env);
        }
    }

    if (!var && module) {
        if (!(var = ash_module_func_get(module, id)))
            var = ash_module_func_get(ash_module_root(), id);
        runtime_env_init(&renv, module, NULL);
    }

    if (var && (obj = ash_var_obj(var)))
        return ash_func_exec(obj, &renv, args);
    return NULL;
}

static struct ash_obj *runtime_eval_closure(struct ast_function *function)
{
    struct ash_obj *obj;
    const char *id;
    struct ast_prog prog;
    ast_prog_init(&prog, function->stm);

    id = function->id;
    obj = ash_func_from(id, prog, function->param);
    return obj;
}

static struct ash_obj *
runtime_eval_array(struct ash_runtime_context *context,
                   struct ast_composite *comp)
{
    if (!comp)
        return NULL;

    struct vec *vec;
    size_t argc = comp->length;
    struct ash_obj *obj = NULL;

    if (argc > 0) {
        struct ast_expr *expr = comp->expr;
        vec = vec_from(argc);

        for (size_t i = 0; i < argc; ++i) {
            vec_push(vec, runtime_eval_expr(context, expr));
            expr = expr->next;
        }

        obj = ash_array_from(vec);
    }

    return obj;
}

static struct ash_obj *
runtime_eval_tuple(struct ash_runtime_context *context,
                   struct ast_composite *tuple)
{
    if (!tuple)
        return NULL;

    size_t argc = tuple->length;
    struct ash_obj *obj = NULL;

    if (argc > 0) {
        struct ast_expr *expr = tuple->expr;
        struct ash_obj *argv[argc];

        for (size_t i = 0; i < argc; ++i) {
            argv[i] = runtime_eval_expr(context, expr);
            expr = expr->next;
        }

        obj = ash_tuple_from(argc, argv);
    }

    return obj;
}

static struct ash_obj *
runtime_eval_range(struct ash_runtime_context *context,
                   struct ast_range *range)
{
    struct ash_obj *obj;
    obj = ash_range_from(range->start, range->end, range->inclusive);
    return obj;
}

static struct ash_obj *
runtime_eval_map(struct ash_runtime_context *context,
                 struct ast_map *map)
{
    struct ash_obj *obj, *value;
    struct ast_entry *entry;

    obj = ash_map_new();
    entry = map->entry;

    while (entry) {
        if ((value = runtime_eval_expr(context, entry->expr)))
            ash_map_insert(obj, entry->key, value);
        entry = entry->next;
    }

    return obj;
}

static struct ash_obj *
runtime_eval_string(struct ash_runtime_context *context,
                    const char *str)
{
    const char *fmt;
    struct ash_obj *obj;

    if ((fmt = ash_ops_format(str, &context->env)))
        str = fmt;
    obj = ash_str_from(str);
    return obj;
}

static struct ash_obj *
runtime_eval_literal(struct ash_runtime_context *context,
                     struct ast_literal *literal)
{
    enum ast_literal_type type;
    type = literal->type;

    if (type == AST_LITERAL_BOOL)
        return ash_bool_from(literal->value.boolean);
    else if (type == AST_LITERAL_NUM)
        return ash_int_from(literal->value.numeric);
    else if (type == AST_LITERAL_STR)
        return runtime_eval_string(context, literal->value.string);
    else if (type == AST_LITERAL_ARRAY)
        return runtime_eval_array(context, literal->value.array);
    else if (type == AST_LITERAL_TUPLE)
        return runtime_eval_tuple(context, literal->value.tuple);
    else if (type == AST_LITERAL_RANGE)
        return runtime_eval_range(context, literal->value.range);
    else if (type == AST_LITERAL_MAP)
        return runtime_eval_map(context, literal->value.map);
    else if (type == AST_LITERAL_CLOSURE)
        return runtime_eval_closure(literal->value.closure);
    else
        return NULL;
}

static struct ash_obj *
runtime_eval_value(struct ash_runtime_context *context,
                   struct ast_value *value)
{
    assert(value != NULL);

    if (value->type == AST_VALUE_VAR)
        return runtime_eval_var(context, value->value.var);
    else if (value->type == AST_VALUE_LITERAL)
        return runtime_eval_literal(context, value->value.literal);

    return NULL;
}

static struct ash_obj *
unary_op(enum ast_binary_op op, struct ash_obj *a)
{
    struct ash_obj *obj = NULL;
    struct ash_obj *(*unary) (struct ash_obj *) = NULL;
    const struct ash_base_ops *ops;
    ops = ash_obj_get_ops(a);

    if (op == AST_UNARY_MINUS) {
        unary = ops->sb;
    } else if (op == AST_UNARY_NOT) {
        struct ash_obj *o;
        if ((o = ash_obj_bool(a)))
            ash_bool_negate(o);
        return o;
    }

    if (unary)
        obj = unary(a);

    return obj;
}

static struct ash_obj *
runtime_eval_unary(struct ash_runtime_context *context,
                   struct ast_unary *unary)
{
    struct ash_obj *a;

    if (!(a = runtime_eval_expr(context, unary->expr)))
        return NULL;
    return unary_op(unary->op, a);
}

static struct ash_obj *
binary_op(enum ast_binary_op op, struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj = NULL;
    struct ash_obj *(*bin) (struct ash_obj *, struct ash_obj *) = NULL;
    const struct ash_base_ops *ops;
    ops = ash_obj_get_ops(a);

    switch (op) {
        case AST_BINARY_ADD:
            bin = ops->add;
            break;

        case AST_BINARY_SUB:
            bin = ops->sub;
            break;

        case AST_BINARY_MUL:
            bin = ops->mul;
            break;

        case AST_BINARY_DIV:
            bin = ops->div;
            break;

        case AST_BINARY_MOD:
            bin = ops->mod;
            break;
    }

    if (bin)
        obj = bin(a, b);

    return obj;
}

static struct ash_obj *
runtime_eval_binary(struct ash_runtime_context *context, struct ast_binary *bin)
{
    struct ash_obj *a, *b;
    a = runtime_eval_expr(context, bin->e1);
    b = runtime_eval_expr(context, bin->e2);

    if (!a || !b)
        return NULL;

    if (!ash_obj_type_eq(a, b))
        return NULL;

    struct ash_obj *obj;
    obj = binary_op(bin->op, a, b);

    struct ash_obj *objs[] = {
        a, b
    };

    ash_obj_set_dec_rc(objs);
    return obj;
}

static struct ash_obj *
cmp_op(enum ast_cmp_op op, struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj = NULL;
    struct ash_obj *(*cmp)(struct ash_obj *, struct ash_obj *) = NULL;
    const struct ash_base_ops *ops;
    ops = ash_obj_get_ops(a);

    switch (op) {
        case AST_CMP_EQ:
            cmp = ops->eq;
            break;

        case AST_CMP_NE:
            cmp = ops->ne;
            break;

        case AST_CMP_LN:
            cmp = ops->lt;
            break;

        case AST_CMP_GN:
            cmp = ops->gt;
            break;

        /*case AST_CMP_LE:
            cmp = ops->le;
            break;

        case AST_CMP_GE:
            cmp = ops->ge;
            break;*/
    }

    if (cmp)
        obj = cmp(a, b);

    return obj;
}

static struct ash_obj *
runtime_eval_cmp(struct ash_runtime_context *context, struct ast_cmp *cmp)
{
    struct ash_obj *a, *b;
    a = runtime_eval_expr(context, cmp->e1);
    b = runtime_eval_expr(context, cmp->e2);

    if (!a || !b || !ash_obj_type_eq(a, b))
        return NULL;

    return cmp_op(cmp->op, a, b);
}

static struct ash_obj *
logical_op(enum ast_logical_op op, struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj = NULL;
    struct ash_obj *(*logical)(struct ash_obj *, struct ash_obj *) = NULL;
    const struct ash_base_ops *ops;
    if (!(ops = ash_obj_get_ops(a)))
        return NULL;

    switch (op) {
        case AST_LOGICAL_AND:
            logical = ops->and;
            break;

        case AST_LOGICAL_OR:
            logical = ops->or;
            break;
    }

    if (logical)
        obj = logical(a, b);

    return obj;
}

static struct ash_obj *
runtime_to_bool(struct ash_obj *obj)
{
    struct ash_obj *o = NULL;
    o = ash_obj_bool(obj);
    ash_obj_dec_rc(obj);
    return o;
}

static struct ash_obj *
runtime_eval_logical(struct ash_runtime_context *context, struct ast_logical *logical)
{
    struct ash_obj *a, *b;
    a = runtime_eval_expr(context, logical->e1);
    b = runtime_eval_expr(context, logical->e2);

    if (!a || !b)
        return NULL;

    a = runtime_to_bool(a);
    b = runtime_to_bool(b);

    return (a && b) ? logical_op(logical->op, a, b): NULL;
}


static struct ash_obj *
runtime_eval_ternary(struct ash_runtime_context *context, struct ast_ternary *ternary)
{
    struct ash_obj *obj = NULL;
    bool cond = false;
    struct ast_bool_expr *expr;
    expr = ternary->cond;

    if (expr)
        cond = runtime_bool_expr(context, expr);

    obj = runtime_eval_expr(context, (cond) ? ternary->e1: ternary->e2);
    return obj;
}

static struct ash_obj *
runtime_eval_match(struct ash_runtime_context *context, struct ast_match *match)
{
    struct ash_obj *obj = NULL;
    struct ash_obj *mexpr = NULL, *cexpr = NULL;
    struct ast_case *ecase;
    struct ast_expr *expr;

    if (match->expr)
        mexpr = runtime_eval_expr(context, match->expr);

    if (mexpr && (ecase = match->ecase)) {
        do {
            expr = ecase->expr;
            while (expr) {
                cexpr = runtime_eval_expr(context, expr);
                if (ash_obj_match(cexpr, mexpr)) {
                    obj = runtime_eval_expr(context, ecase->eval);
                    break;
                }
                expr = expr->next;
            }
        } while (!obj && (ecase = ecase->next));
    }

    struct ash_obj *objs[] = {
        mexpr, cexpr
    };

    ash_obj_set_dec_rc(objs);

    if (!obj && match->otherwise)
        return runtime_eval_expr(context, match->otherwise);

    return obj;
}

static struct ash_obj *
runtime_eval_hash(struct ash_runtime_context *context, struct ast_hash *hash)
{
    struct ash_obj *obj = NULL, *map;
    if ((map = runtime_eval_expr(context, hash->expr)))
        obj = ash_map_get(map, hash->key);
    return obj;
}

static struct ash_obj *
runtime_eval_expr(struct ash_runtime_context *context, struct ast_expr *expr)
{
    enum ast_expr_type type;
    struct ash_obj *obj = NULL;
    type = expr->type;

    if (type == AST_EXPR_VALUE)
        obj = runtime_eval_value(context, expr->expr);
    else if (type == AST_EXPR_CALL)
        obj = runtime_call(context, expr->expr);
    else if (type == AST_EXPR_UNARY)
        obj = runtime_eval_unary(context, expr->expr);
    else if (type == AST_EXPR_BINARY)
        obj = runtime_eval_binary(context, expr->expr);
    else if (type == AST_EXPR_CMP)
        obj = runtime_eval_cmp(context, expr->expr);
    else if (type == AST_EXPR_LOGICAL)
        obj = runtime_eval_logical(context, expr->expr);
    else if (type == AST_EXPR_TERNARY)
        obj = runtime_eval_ternary(context, expr->expr);
    else if (type == AST_EXPR_MATCH)
        obj = runtime_eval_match(context, expr->expr);
    else if (type == AST_EXPR_HASH)
        obj = runtime_eval_hash(context, expr->expr);

    return obj;
}

static struct ash_obj *
runtime_eval_bool_expr(struct ash_runtime_context *context, struct ast_bool_expr *expr)
{
    struct ash_obj *bexpr;
    struct ash_obj *obj;
    const struct ash_base_into *into;

    if (!(obj = runtime_eval_expr(context, expr->expr)))
        return NULL;

    bexpr = ash_obj_bool(obj);
    return bexpr;
}

static bool
runtime_bool_expr(struct ash_runtime_context *context, struct ast_bool_expr *expr)
{
    struct ash_obj *obj;
    obj = runtime_eval_bool_expr(context, expr);
    bool value = (obj) ? ash_bool_get(obj): false;
    ash_obj_dec_rc(obj);
    return value;
}

static void
runtime_assign(struct ash_runtime_context *context, struct ast_assign *assign)
{
    struct ash_obj *obj;
    const char *id;
    bool local;

    id = assign->var->id;
    local = assign->local;
    if (!assign->expr)
        return;
    if (!(obj = runtime_eval_expr(context, assign->expr)))
        return;

    if (local) {
        runtime_var_set(context, id, obj);
        return;
    }

    struct ash_module *module;
    struct ash_env *env;
    struct ash_var *av;
    module = runtime_context_module(context);

    if (env = runtime_context_env(context)) {
        do {
            if ((av = ash_var_env_get(env, id))) {
                ash_var_bind(av, obj);
                return;
            }
        } while ((env = ash_env_parent(env)));
    }

    if ((av = ash_module_var_get(module, id)))
        ash_var_bind(av, obj);
    else
        ash_module_var_set(module, id, obj);
}

static void
runtime_command(struct ash_runtime_context *context, struct ast_command *command)
{
    struct ash_runtime_env renv;
    struct ash_module *mod;
    struct ash_env *env;
    struct ash_obj *obj;
    struct ast_expr *expr;

    expr = command->expr;
    mod = runtime_context_module(context);
    env = runtime_context_env(context);
    runtime_env_init(&renv, mod, env);

    struct vec *objs;
    objs = vec_from(command->length);

    do {
        if ((obj = runtime_eval_expr(context, expr))) {
            if ((obj = ash_obj_str(obj)))
                vec_push(objs, obj);
        }
    } while ((expr = expr->next));

    if (vec_len(objs) > 0) {
        struct vec *vec;
        vec = vec_map(objs, (void *(*)(void *))ash_str_get);
        if (vec_len(vec) > 0)
            ash_exec_command(vec, &renv);
        vec_destroy(vec);
    }

    vec_destroy(objs);
}

static struct ash_obj *
runtime_call(struct ash_runtime_context *context, struct ast_call *call)
{
    struct ash_runtime_env *env;
    struct ash_obj *obj, *ret = NULL;
    struct ash_obj *argv = NULL;
    struct ast_composite *args = NULL;
    bool ref = call->var->ref;

    env = &context->env;
    if ((args = call->args))
        argv = runtime_eval_tuple(context, args);

    if (!ref)
        ret = runtime_eval_func(context, call->var, argv);
    else if ((obj = runtime_eval_var(context, call->var)))
        ret = ash_func_exec(obj, env, argv);

    return ret;
}

static void runtime_return(struct ash_runtime_context *context, struct ast_return *ret)
{
    struct ash_obj *obj = NULL;
    assert(context != NULL);
    runtime_context_set_state(context, ASH_RUNTIME_STATE_RET);

    if (ret->expr)
        obj = runtime_eval_expr(context, ret->expr);
    runtime_context_set_ret(context, obj);
}

static void runtime_if(struct ash_runtime_context *, struct ast_if *);

static void runtime_else(struct ash_runtime_context *context, struct ast_else *ast_else)
{
    enum ast_else_type type;
    type = ast_else->type;

    if (type == AST_ELSE) {
        runtime_context_env_new(context);
        runtime_exec_stm(context, ast_else->stm.stm);
        runtime_context_env_destroy(context);
    } else if (type == AST_ELSE_IF) {
        runtime_if(context, ast_else->stm.if_t);
    }
}

static void runtime_if(struct ash_runtime_context *context, struct ast_if *ast_if)
{
    struct ast_stm *stm;
    struct ast_bool_expr *expr;
    bool cond = false;
    struct ast_else *ast_else;

    stm = ast_if->stm;
    expr = ast_if->cond;
    ast_else = ast_if->else_t;

    if (expr)
        cond = runtime_bool_expr(context, expr);

    if (cond) {
        if (stm) {
            runtime_context_env_new(context);
            runtime_exec_stm(context, stm);
            runtime_context_env_destroy(context);
        }
    } else if (ast_else) {
        runtime_else(context, ast_else);
    }
}

static void runtime_break(struct ash_runtime_context *context)
{
    runtime_context_set_state(context, ASH_RUNTIME_STATE_BRK);
}

static void runtime_next(struct ash_runtime_context *context)
{
    runtime_context_set_state(context, ASH_RUNTIME_STATE_NXT);
}

static void runtime_while(struct ash_runtime_context *context,
                          struct ast_while *ast_while)
{
    struct ast_stm *stm;
    struct ast_bool_expr *expr;
    bool cond = false;
    enum ash_runtime_state state;

    stm = ast_while->stm;
    expr = ast_while->cond;

    if (expr) {
        runtime_context_env_new(context);

        while ((cond = runtime_bool_expr(context, expr))) {
            if (stm)
                runtime_exec_stm(context, stm);

            state = runtime_context_get_state(context);
            if (state == ASH_RUNTIME_STATE_RET) {
                break;
            } else if (state == ASH_RUNTIME_STATE_NXT) {
                runtime_context_set_state(context, ASH_RUNTIME_STATE_RUN);
                continue;
            } else if (state == ASH_RUNTIME_STATE_BRK) {
                runtime_context_set_state(context, ASH_RUNTIME_STATE_RUN);
                break;
            }
        }

        runtime_context_env_destroy(context);
    }
}

static void runtime_for(struct ash_runtime_context *context,
                        struct ast_for *ast_for)
{
    const char *id;
    struct ash_iter iter;
    struct ash_obj *obj, *ao = NULL;
    struct ash_var *av = NULL;
    struct ast_stm *stm;
    enum ash_runtime_state state;

    id = ast_for->var->id;
    stm = ast_for->stm;

    if (!(ao = runtime_eval_expr(context, ast_for->expr)))
        return;

    ash_iter_init(&iter, ao);
    runtime_context_env_new(context);

    while (ash_iter_hasnext(&iter)) {
        obj = ash_iter_next(&iter);
        if (av)
            ash_var_bind(av, obj);
        else
            av = runtime_var_set(context, id, obj);

        if (stm)
            runtime_exec_stm(context, stm);

        state = runtime_context_get_state(context);
        if (state == ASH_RUNTIME_STATE_RET) {
            break;
        } else if (state == ASH_RUNTIME_STATE_NXT) {
            runtime_context_set_state(context, ASH_RUNTIME_STATE_RUN);
            continue;
        } else if (state == ASH_RUNTIME_STATE_BRK) {
            runtime_context_set_state(context, ASH_RUNTIME_STATE_RUN);
            break;
        }
    }

    runtime_context_env_destroy(context);
}

static void runtime_func(struct ash_runtime_context *context,
                         struct ast_function *function)
{
    struct ash_obj *obj;
    const char *id;
    struct ast_prog prog;
    ast_prog_init(&prog, function->stm);

    id = function->id;
    obj = ash_func_from(id, prog, function->param);
    runtime_func_set(context, id, obj);
}

static void runtime_module(struct ash_runtime_context *context,
                           struct ast_module *module)
{
    struct ash_module *mod = NULL, *pmod;
    const char *name;
    struct ast_stm *stm;
    struct ash_runtime_prog rprog;
    struct ash_runtime_env env;
    struct ast_prog prog;

    pmod = runtime_context_module(context);
    assert(pmod != NULL);

    name = module->name;
    stm = module->stm;

    if ((mod = ash_module_sub_set(pmod, name))) {
        ast_prog_init(&prog, stm);
        runtime_env_init(&env, mod, NULL);
        runtime_prog_init(&rprog, prog, env);
        runtime_exec(&rprog);
    }
}

static int runtime_stm(struct ash_runtime_context *context, struct ast_stm *stm)
{
    void *node = stm->node;

    switch (stm->type) {

        case AST_NODE_MODULE:
            runtime_module(context, node);
            break;

        case AST_NODE_COMMAND:
            runtime_command(context, node);
            break;

        case AST_NODE_EXPR:
            runtime_eval_expr(context, node);
            break;

        case AST_NODE_ASSIGN:
            runtime_assign(context, node);
            break;

        case AST_NODE_IF:
            runtime_if(context, node);
            break;

        case AST_NODE_WHILE:
            runtime_while(context, node);
            break;

        case AST_NODE_FOR:
            runtime_for(context, node);
            break;

        case AST_NODE_BREAK:
            runtime_break(context);
            break;

        case AST_NODE_NEXT:
            runtime_next(context);
            break;

        case AST_NODE_FUNC:
            runtime_func(context, node);
            break;

        case AST_NODE_RET:
            runtime_return(context, node);
            break;

    }

    return 0;
}

void runtime_exec_stm(struct ash_runtime_context *context, struct ast_stm *stm)
{
    while (stm) {
        runtime_stm(context, stm);
        if (!runtime_context_is_run(context))
            break;
        stm = stm->next;
    }
}

struct ash_obj *runtime_exec_func(struct ash_runtime_prog *prog)
{
    struct ast_stm *stm;
    struct ash_runtime_context context;
    runtime_context_init(&context, prog->env);

    stm = prog->prog.stm;

    if (stm)
        runtime_exec_stm(&context, stm);
    return context.ret;
}

int runtime_exec(struct ash_runtime_prog *prog)
{
    assert(prog != NULL);
    struct ast_stm *stm;
    if (!(stm = prog->prog.stm))
        return -1;

    struct ash_runtime_context context;
    runtime_context_init(&context, prog->env);
    runtime_exec_stm(&context, stm);

    return 0;
}
