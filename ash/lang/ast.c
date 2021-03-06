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

#include <stddef.h>

#include "ash/bool.h"
#include "ash/int.h"
#include "ash/mem.h"
#include "ash/var.h"
#include "ash/lang/ast.h"

void ast_expr_destroy(struct ast_expr *);
void ast_else_destroy(struct ast_else *);

struct ast_scope *ast_scope_new(const char *id)
{
    struct ast_scope *scope;
    scope = ash_alloc(sizeof *scope);
    scope->id = id;
    scope->next = NULL;
    return scope;
}

void ast_scope_destroy(struct ast_scope *scope)
{
    if (scope->id)
        ash_free((char *)scope->id);
    if (scope->next)
        ast_scope_destroy(scope->next);
    ash_free(scope);
}

struct ast_path *ast_path_new(enum ash_module_path_type type,
                              size_t length, struct ast_scope *scope)
{
    struct ast_path *path;
    path = ash_alloc(sizeof *path);
    path->type = type;
    path->length = length;
    path->path = scope;
    return path;
}

void ast_path_destroy(struct ast_path *path)
{
    if (path->path)
        ast_scope_destroy(path->path);
    ash_free(path);
}

struct ast_var *ast_var_new(const char *id, bool ref, struct ast_path *path)
{
    struct ast_var *av;
    av = ash_alloc(sizeof *av);
    av->id = id;
    av->ref = ref;
    av->path = path;
    return av;
}

void ast_var_destroy(struct ast_var *av)
{
    if (av->id)
        ash_free((char *)av->id);
    if (av->path)
        ast_path_destroy(av->path);
    ash_free(av);
}

struct ast_range *ast_range_new(isize start, isize end, bool inclusive)
{
    struct ast_range *range;
    range = ash_alloc(sizeof *range);
    range->start = start;
    range->end = end;
    range->inclusive = inclusive;
    return range;
}

void ast_range_destroy(struct ast_range *range)
{
    ash_free(range);
}

struct ast_entry *ast_entry_new(const char *key, struct ast_expr *expr)
{
    struct ast_entry *entry;
    entry = ash_alloc(sizeof *entry);
    entry->key = key;
    entry->expr = expr;
    entry->next = NULL;
    return entry;
}

void ast_entry_destroy(struct ast_entry *entry)
{
    if (entry->key)
        ash_free((char *)entry->key);
    if (entry->expr)
        ast_expr_destroy(entry->expr);
    if (entry->next)
        ast_entry_destroy(entry->next);
    ash_free(entry);
}

struct ast_map *ast_map_new(struct ast_entry *entry)
{
    struct ast_map *map;
    map = ash_alloc(sizeof *map);
    map->entry = entry;
    return map;
}

void ast_map_destroy(struct ast_map *map)
{
    if (map->entry)
        ast_entry_destroy(map->entry);
    ash_free(map);
}

static inline struct ast_literal *ash_literal_new(void)
{
    return ash_zalloc(sizeof (struct ast_literal));
}

struct ast_literal *ast_literal_bool(bool value)
{
    struct ast_literal *literal;
    literal = ash_literal_new();
    literal->type = AST_LITERAL_BOOL;
    literal->value.boolean = value;
    return literal;
}

struct ast_literal *ast_literal_num(isize value)
{
    struct ast_literal *literal;
    literal = ash_literal_new();
    literal->type = AST_LITERAL_NUM;
    literal->value.numeric = value;
    return literal;
}

struct ast_literal *ast_literal_str(const char *value)
{
    struct ast_literal *literal;
    literal = ash_literal_new();
    literal->type = AST_LITERAL_STR;
    literal->value.string = value;
    return literal;
}

struct ast_literal *ast_literal_array(struct ast_composite *value)
{
    struct ast_literal *literal;
    literal = ash_literal_new();
    literal->type = AST_LITERAL_ARRAY;
    literal->value.array = value;
    return literal;
}

struct ast_literal *ast_literal_tuple(struct ast_composite *value)
{
    struct ast_literal *literal;
    literal = ash_literal_new();
    literal->type = AST_LITERAL_TUPLE;
    literal->value.tuple = value;
    return literal;
}

struct ast_literal *ast_literal_range(struct ast_range *value)
{
    struct ast_literal *literal;
    literal = ash_literal_new();
    literal->type = AST_LITERAL_RANGE;
    literal->value.range = value;
    return literal;
}

struct ast_literal *ast_literal_map(struct ast_map *value)
{
    struct ast_literal *literal;
    literal = ash_literal_new();
    literal->type = AST_LITERAL_MAP;
    literal->value.map = value;
    return literal;
}

struct ast_literal *ast_literal_closure(struct ast_function *value)
{
    struct ast_literal *literal;
    literal = ash_literal_new();
    literal->type = AST_LITERAL_CLOSURE;
    literal->value.closure = value;
    return literal;
}

void ast_literal_destroy(struct ast_literal *literal)
{
    switch (literal->type) {
        case AST_LITERAL_STR:
            ash_free((char *)literal->value.string);
            break;
        case AST_LITERAL_RANGE:
            ast_range_destroy(literal->value.range);
            break;
    }

    ash_free(literal);
}

static inline struct ast_value *ast_value_new(void)
{
    return ash_zalloc(sizeof (struct ast_value));
}

struct ast_value *ast_value_var(struct ast_var *var)
{
    struct ast_value *value;
    value = ast_value_new();
    value->type = AST_VALUE_VAR;
    value->value.var = var;
    return value;
}

struct ast_value *ast_value_literal(struct ast_literal *literal)
{
    struct ast_value *value;
    value = ast_value_new();
    value->type = AST_VALUE_LITERAL;
    value->value.literal = literal;
    return value;
}

void ast_value_destroy(struct ast_value *value)
{
    if (value->type == AST_VALUE_VAR)
        ast_var_destroy(value->value.var);
    else if (value->type == AST_VALUE_LITERAL)
        ast_literal_destroy(value->value.literal);

    ash_free(value);
}

struct ast_unary *ast_unary_new(enum ast_unary_op op, struct ast_expr *expr)
{
    struct ast_unary *unary;
    unary = ash_alloc(sizeof *unary);
    unary->op = op;
    unary->expr = expr;
    return unary;
}

void ast_unary_destroy(struct ast_unary *unary)
{
    if (unary->expr)
        ast_expr_destroy(unary->expr);
    ash_free(unary);
}

struct ast_binary *ast_binary_new(enum ast_binary_op op, struct ast_expr *e1,
                                  struct ast_expr *e2)
{
    struct ast_binary *binary;
    binary = ash_alloc(sizeof *binary);
    binary->op = op;
    binary->e1 = e1;
    binary->e2 = e2;
    return binary;
}

void ast_binary_destroy(struct ast_binary *binary)
{
    if (binary->e1)
        ast_expr_destroy(binary->e1);
    if (binary->e2)
        ast_expr_destroy(binary->e2);
    ash_free(binary);
}

struct ast_cmp *ast_cmp_new(enum ast_cmp_op op, struct ast_expr *e1,
                            struct ast_expr *e2)
{
    struct ast_cmp *cmp;
    cmp = ash_alloc(sizeof *cmp);
    cmp->op = op;
    cmp->e1 = e1;
    cmp->e2 = e2;
    return cmp;
}

void ast_cmp_destroy(struct ast_cmp *cmp)
{
    if (cmp->e1)
        ast_expr_destroy(cmp->e1);
    if (cmp->e2)
        ast_expr_destroy(cmp->e2);
    ash_free(cmp);
}

struct ast_logical *
ast_logical_new(enum ast_logical_op op, struct ast_expr *e1, struct ast_expr *e2)
{
    struct ast_logical *logical;
    logical = ash_alloc(sizeof *logical);
    logical->op = op;
    logical->e1 = e1;
    logical->e2 = e2;
    return logical;
}

void ast_logical_destroy(struct ast_logical *logical)
{
    if (logical->e1)
        ast_expr_destroy(logical->e1);
    if (logical->e2)
        ast_expr_destroy(logical->e2);
    ash_free(logical);
}

struct ast_composite *ast_composite_new(struct ast_expr *expr, size_t length)
{
    struct ast_composite *comp;
    comp = ash_alloc(sizeof *comp);
    comp->expr = expr;
    comp->length = length;
    return comp;
}

void ast_composite_destroy(struct ast_composite *comp)
{
    if (comp->expr)
        ast_expr_destroy(comp->expr);
    ash_free(comp);
}

struct ast_expr *ast_expr_new(enum ast_expr_type type, void *value)
{
    struct ast_expr *expr;
    expr = ash_alloc(sizeof *expr);
    expr->type = type;
    expr->expr = value;
    expr->next = NULL;
    return expr;
}

void ast_expr_destroy(struct ast_expr *expr)
{
    switch (expr->type) {

    }

    if (expr->next)
        ast_expr_destroy(expr->next);
    ash_free(expr);
}

struct ast_assign *ast_assign_new(bool local, struct ast_var *var,
                                  struct ast_expr *expr)
{
    struct ast_assign *assign;
    assign = ash_alloc(sizeof *assign);
    assign->local = local;
    assign->var = var;
    assign->expr = expr;
    return assign;
}

void ast_assign_destroy(struct ast_assign *assign)
{
    if (assign->var)
        ast_var_destroy(assign->var);
    if (assign->expr)
        ast_expr_destroy(assign->expr);
    ash_free(assign);
}

struct ast_bool_expr *ast_bool_expr_new(struct ast_expr *expr)
{
    struct ast_bool_expr *bexpr;
    bexpr = ash_alloc(sizeof *bexpr);
    bexpr->expr = expr;
    return bexpr;
}

void ast_bool_expr_destroy(struct ast_bool_expr *bexpr)
{
    if (bexpr->expr)
        ast_expr_destroy(bexpr->expr);
    ash_free(bexpr);
}

struct ast_ternary *ast_ternary_new(struct ast_bool_expr *cond,
                                    struct ast_expr *e1, struct ast_expr *e2)
{
    struct ast_ternary *ternary;
    ternary = ash_alloc(sizeof *ternary);
    ternary->cond = cond;
    ternary->e1 = e1;
    ternary->e2 = e2;
    return ternary;
}

void ast_ternary_destroy(struct ast_ternary *ternary)
{
    if (ternary->cond)
        ast_bool_expr_destroy(ternary->cond);
    if (ternary->e1)
        ast_expr_destroy(ternary->e1);
    if (ternary->e2)
        ast_expr_destroy(ternary->e2);
    ash_free(ternary);
}

struct ast_case *
ast_case_new(struct ast_expr *expr, struct ast_expr *eval,
             struct ast_case *next)
{
    struct ast_case *ecase;
    ecase = ash_alloc(sizeof *ecase);
    ecase->expr = expr;
    ecase->eval = eval;
    ecase->next = next;
    return ecase;
}

void ast_case_destroy(struct ast_case *ecase)
{
    if (ecase->expr)
        ast_expr_destroy(ecase->expr);
    if (ecase->eval)
        ast_expr_destroy(ecase->eval);
    if (ecase->next)
        ast_case_destroy(ecase->next);
    ash_free(ecase);
}

struct ast_match *
ast_match_new(struct ast_expr *expr, struct ast_case *ecase,
              struct ast_expr *otherwise)
{
    struct ast_match *match;
    match = ash_alloc(sizeof *match);
    match->expr = expr;
    match->ecase = ecase;
    match->otherwise = otherwise;
    return match;
}

void ast_match_destroy(struct ast_match *match)
{
    if (match->expr)
        ast_expr_destroy(match->expr);
    if (match->ecase)
        ast_case_destroy(match->ecase);
    if (match->otherwise)
        ast_expr_destroy(match->otherwise);
    ash_free(match);
}

struct ast_hash *
ast_hash_new(const char *key, struct ast_expr *expr)
{
    struct ast_hash *hash;
    hash = ash_alloc(sizeof *hash);
    hash->key = key;
    hash->expr = expr;
    return hash;
}

void ast_hash_destroy(struct ast_hash *hash)
{
    if (hash->key)
        ash_free((char *)hash->key);
    if (hash->expr)
        ast_expr_destroy(hash->expr);
    ash_free(hash);
}

struct ast_stm *
ast_stm_new(enum ast_node_type type, void *node)
{
    struct ast_stm *stm;
    stm = ash_alloc(sizeof *stm);
    stm->type = type;
    stm->node = node;
    stm->next = NULL;
    return stm;
}

void ast_stm_destroy(struct ast_stm *stm)
{
    switch (stm->type) {

    }

    if (stm->next)
        ast_stm_destroy(stm->next);
    ash_free(stm);
}

struct ast_command *ast_command_new(struct ast_expr *expr, size_t length)
{
    struct ast_command *command;
    command = ash_alloc(sizeof *command);
    command->expr = expr;
    command->length = length;
    command->redirect = NULL;
    return command;
}

void ast_command_destroy(struct ast_command *command)
{
    if (command->expr)
        ast_expr_destroy(command->expr);
    ash_free(command);
}

struct ast_call *ast_call_new(struct ast_var *var, struct ast_composite *args)
{
    struct ast_call *call;
    call = ash_alloc(sizeof *call);
    call->var = var;
    call->args = args;
    return call;
}

void ast_call_destroy(struct ast_call *call)
{
    if (call->var)
        ast_var_destroy(call->var);
    if (call->args)
        ast_composite_destroy(call->args);
    ash_free(call);
}

struct ast_if *ast_if_new(struct ast_bool_expr *expr, struct ast_stm *stm,
                          struct ast_else *else_t)
{
    struct ast_if *ast_if;
    ast_if = ash_alloc(sizeof *ast_if);
    ast_if->cond = expr;
    ast_if->stm = stm;
    ast_if->else_t = else_t;
    return ast_if;
}

void ast_if_destroy(struct ast_if *ast_if)
{
    if (ast_if->cond)
        ast_bool_expr_destroy(ast_if->cond);
    if (ast_if->stm)
        ast_stm_destroy(ast_if->stm);
    if (ast_if->else_t)
        ast_else_destroy(ast_if->else_t);
    ash_free(ast_if);
}

static struct ast_else *
ast_else(void)
{
    return ash_zalloc(sizeof (struct ast_else));
}

void ast_else_destroy(struct ast_else *ae)
{
    if (ae->type == AST_ELSE)
        ast_stm_destroy(ae->stm.stm);
    else if (ae->type == AST_ELSE_IF)
        ast_if_destroy(ae->stm.if_t);
    ash_free(ae);
}

struct ast_else *
ast_else_new(struct ast_stm *stm)
{
    struct ast_else *ae;
    ae = ast_else();
    ae->type = AST_ELSE;
    ae->stm.stm = stm;
    return ae;
}

struct ast_else *
ast_else_if_new(struct ast_if *if_t)
{
    struct ast_else *ae;
    ae = ast_else();
    ae->type = AST_ELSE_IF;
    ae->stm.if_t = if_t;
    return ae;
}

struct ast_while *
ast_while_new(struct ast_bool_expr *cond, struct ast_stm *stm)
{
    struct ast_while *ast_while;
    ast_while = ash_alloc(sizeof *ast_while);
    ast_while->cond = cond;
    ast_while->stm = stm;
    return ast_while;
}

void ast_while_destroy(struct ast_while *ast_while)
{
    if (ast_while->cond)
        ast_bool_expr_destroy(ast_while->cond);
    if (ast_while->stm)
        ast_stm_destroy(ast_while->stm);
    ash_free(ast_while);
}

struct ast_for *
ast_for_new(struct ast_var *var, struct ast_expr *expr, struct ast_stm *stm)
{
    struct ast_for *ast_for;
    ast_for = ash_alloc(sizeof *ast_for);
    ast_for->var = var;
    ast_for->expr = expr;
    ast_for->stm = stm;
    return ast_for;
}

void ast_for_destroy(struct ast_for *ast_for)
{
    if (ast_for->var)
        ast_var_destroy(ast_for->var);
    if (ast_for->expr)
        ast_expr_destroy(ast_for->expr);
    if (ast_for->stm)
        ast_stm_destroy(ast_for->stm);
    ash_free(ast_for);
}

struct ast_param *ast_param_new(const char *id)
{
    struct ast_param *param;
    param = ash_alloc(sizeof *param);
    param->id = id;
    param->next = NULL;
    return param;
}

void ast_param_destroy(struct ast_param *param)
{
    if (param->id)
        ash_free(param);
    if (param->next)
        ast_param_destroy(param->next);
    ash_free(param);
}

struct ast_function *
ast_function_new(const char *id, struct ast_param *param, struct ast_stm *stm)
{
    struct ast_function *function;
    function = ash_alloc(sizeof *function);
    function->id = id;
    function->param = param;
    function->stm = stm;
    return function;
}

void ast_function_destroy(struct ast_function *function)
{
    if (function->id)
        ash_free((char *)function->id);
    if (function->param)
        ast_param_destroy(function->param);
    if (function->stm)
        ast_stm_destroy(function->stm);
    ash_free(function);
}

struct ast_return *
ast_return_new(struct ast_expr *expr)
{
    struct ast_return *ret;
    ret = ash_alloc(sizeof *ret);
    ret->expr = expr;
    return ret;
}

void ast_return_destroy(struct ast_return *ret)
{
    if (ret->expr)
        ast_expr_destroy(ret->expr);
    ash_free(ret);
}

struct ast_use *
ast_use_new(struct ast_path *path)
{
    struct ast_use *use;
    use = ash_alloc(sizeof *use);
    use->path = path;
    return use;
}

void ast_use_destroy(struct ast_use *use)
{
    if (use->path)
        ast_path_destroy(use->path);
    ash_free(use);
}

struct ast_module *
ast_module_new(const char *name, struct ast_stm *stm)
{
    struct ast_module *module;
    module = ash_alloc(sizeof *module);
    module->name = name;
    module->stm = stm;
    return module;
}

void ast_module_destroy(struct ast_module *module)
{
    if (module->name)
        ash_free((char *)module->name);
    if (module->stm)
        ast_stm_destroy(module->stm);
    ash_free(module);
}
