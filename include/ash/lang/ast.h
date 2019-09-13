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

#ifndef ASH_LANG_AST_H
#define ASH_LANG_AST_H

#include "ash/bool.h"
#include "ash/exec.h"
#include "ash/int.h"
#include "ash/module.h"
#include "ash/obj.h"
#include "ash/var.h"
#include "ash/lang/lang.h"

struct ast_scope {
    const char *id;
    struct ast_scope *next;
};

extern struct ast_scope *ast_scope_new(const char *);

struct ast_path {
    enum ash_module_path_type type;
    size_t length;
    struct ast_scope *path;
};

extern struct ast_path *ast_path_new(enum ash_module_path_type, size_t,
                                     struct ast_scope *);

struct ast_var {
    const char *id;
    bool ref;
    struct ast_path *path;
};

extern struct ast_var *ast_var_new(const char *, bool, struct ast_path *);

struct ast_function;
struct ast_composite;

struct ast_range {
    isize start;
    isize end;
    bool inclusive;
};

extern struct ast_range* ast_range_new(isize, isize, bool);

struct ast_literal {
    enum ast_literal_type {
        AST_LITERAL_BOOL,
        AST_LITERAL_NUM,
        AST_LITERAL_STR,
        AST_LITERAL_TUPLE,
        AST_LITERAL_RANGE,
        AST_LITERAL_CLOSURE
    } type;

    union {
        bool boolean;
        isize numeric;
        const char *string;
        struct ast_composite *tuple;
        struct ast_range *range;
        struct ast_function *closure;
    } value;
};

extern struct ast_literal *ast_literal_bool(bool);
extern struct ast_literal *ast_literal_num(isize);
extern struct ast_literal *ast_literal_str(const char *);
extern struct ast_literal *ast_literal_tuple(struct ast_composite *);
extern struct ast_literal *ast_literal_range(struct ast_range *);
extern struct ast_literal *ast_literal_closure(struct ast_function *);

struct ast_value {
    enum ast_value_type {
        AST_VALUE_VAR,
        AST_VALUE_LITERAL
    } type;

    union {
        struct ast_var *var;
        struct ast_literal *literal;
    } value;
};

extern struct ast_value *ast_value_var(struct ast_var *);
extern struct ast_value *ast_value_literal(struct ast_literal *);

struct ast_composite {
    struct ast_expr *expr;
    size_t length;
};

extern struct ast_composite *ast_composite_new(struct ast_expr *, size_t);

struct ast_unary {
    enum ast_unary_op {
        AST_UNARY_MINUS,
        AST_UNARY_NOT
    } op;

    struct ast_expr *expr;
};

extern struct ast_unary *
ast_unary_new(enum ast_unary_op, struct ast_expr *);

struct ast_binary {
    enum ast_binary_op {
        AST_BINARY_ADD,
        AST_BINARY_SUB,
        AST_BINARY_MUL,
        AST_BINARY_DIV,
        AST_BINARY_MOD
    } op;

    struct ast_expr *e1;
    struct ast_expr *e2;
};

extern struct ast_binary *
ast_binary_new(enum ast_binary_op, struct ast_expr *, struct ast_expr *);

struct ast_cmp {
    enum ast_cmp_op {
        AST_CMP_EQ,
        AST_CMP_NE,
        AST_CMP_LN,
        AST_CMP_GN,
        AST_CMP_LE,
        AST_CMP_GE
    } op;

    struct ast_expr *e1;
    struct ast_expr *e2;
};

extern struct ast_cmp *
ast_cmp_new(enum ast_cmp_op, struct ast_expr *, struct ast_expr *);

struct ast_logical {
    enum ast_logical_op {
        AST_LOGICAL_AND,
        AST_LOGICAL_OR
    } op;

    struct ast_expr *e1;
    struct ast_expr *e2;
};

extern struct ast_logical *
ast_logical_new(enum ast_logical_op, struct ast_expr *, struct ast_expr *);

struct ast_assign {
    bool local;
    struct ast_var *var;
    struct ast_expr *expr;
};

extern struct ast_assign *
ast_assign_new(bool, struct ast_var *, struct ast_expr *);

struct ast_ternary {
    struct ast_bool_expr *cond;
    struct ast_expr *e1;
    struct ast_expr *e2;
};

extern struct ast_ternary *
ast_ternary_new(struct ast_bool_expr *, struct ast_expr *, struct ast_expr *);

struct ast_case {
    struct ast_expr *expr;
    struct ast_expr *eval;
    struct ast_case *next;
};

extern struct ast_case *
ast_case_new(struct ast_expr *, struct ast_expr *, struct ast_case *);

struct ast_match {
    struct ast_expr *expr;
    struct ast_case *ecase;
    struct ast_expr *otherwise;
};

extern struct ast_match *
ast_match_new(struct ast_expr *, struct ast_case *, struct ast_expr *);

struct ast_expr {
    enum ast_expr_type {
        AST_EXPR_VALUE,
        AST_EXPR_CALL,
        AST_EXPR_UNARY,
        AST_EXPR_BINARY,
        AST_EXPR_CMP,
        AST_EXPR_LOGICAL,
        AST_EXPR_TERNARY,
        AST_EXPR_MATCH,
    } type;

    void *expr;

    struct ast_expr *next;
};

extern struct ast_expr *ast_expr_new(enum ast_expr_type, void *);

struct ast_bool_expr {
    struct ast_expr *expr;
};

extern struct ast_bool_expr *ast_bool_expr_new(struct ast_expr *);

struct ast_stm {
    /* ast node */
    void *node;

    /* type of the ast node */
    enum ast_node_type {
        AST_NODE_MODULE,
        AST_NODE_COMMAND,
        AST_NODE_EXPR,
        AST_NODE_ASSIGN,
        AST_NODE_IF,
        AST_NODE_WHILE,
        AST_NODE_FOR,
        AST_NODE_BREAK,
        AST_NODE_NEXT,
        AST_NODE_FUNC,
        AST_NODE_RET
    } type;

    struct ast_stm *next;
};

extern struct ast_stm *ast_stm_new(enum ast_node_type, void *);

struct ast_command_redirect {
    enum ash_exec_redirect type;
    struct ast_command *command;
};

extern struct ast_command_redirect
ast_command_redirect_new(enum ash_exec_redirect, struct ast_command *);

struct ast_command {
    struct ast_expr *expr;
    size_t length;
    struct ast_command_redirect *redirect;
};

extern struct ast_command *
ast_command_new(struct ast_expr *, size_t);

struct ast_call {
    struct ast_var *var;
    struct ast_composite *args;
};

extern struct ast_call *
ast_call_new(struct ast_var *, struct ast_composite *);

struct ast_if;

struct ast_else {
    enum ast_else_type {
        AST_ELSE,
        AST_ELSE_IF
    } type;

    union {
        struct ast_stm *stm;
        struct ast_if *if_t;
    } stm;
};

extern struct ast_else *ast_else_new(struct ast_stm *);
extern struct ast_else *ast_else_if_new(struct ast_if *);

struct ast_if {
    struct ast_bool_expr *cond;
    struct ast_stm *stm;
    struct ast_else *else_t;
};

extern struct ast_if *
ast_if_new(struct ast_bool_expr *, struct ast_stm *, struct ast_else *);

struct ast_while {
    struct ast_bool_expr *cond;
    struct ast_stm *stm;
};

extern struct ast_while *
ast_while_new(struct ast_bool_expr *, struct ast_stm *);

struct ast_for {
    struct ast_var *var;
    struct ast_expr *expr;
    struct ast_stm *stm;
};

extern struct ast_for *ast_for_new(struct ast_var *, struct ast_expr *, struct ast_stm *);
extern struct ast_for *ast_for_var(struct ast_var *, struct ast_var *, struct ast_stm *);
extern struct ast_for *ast_for_tuple(struct ast_var *, struct ast_composite *, struct ast_stm *);
extern struct ast_for *ast_for_range(struct ast_var *, struct ast_range *);

struct ast_param {
    const char *id;
    struct ast_param *next;
};

extern struct ast_param *ast_param_new(const char *);

struct ast_function {
    const char *id;
    struct ast_param *param;
    struct ast_stm *stm;
};

extern struct ast_function *
ast_function_new(const char *, struct ast_param *, struct ast_stm *);

struct ast_closure {
    struct ash_env *env;
    struct ast_function *func;
};

struct ast_return {
    struct ast_expr *expr;
};

extern struct ast_return *
ast_return_new(struct ast_expr *);

struct ast_use {
    struct ast_path *path;
};

extern struct ast_use *ast_use_new(struct ast_path *);

struct ast_module {
    const char *name;
    struct ast_stm *stm;
};

extern struct ast_module *
ast_module_new(const char *, struct ast_stm *);

struct ast_prog {
    struct ast_stm *stm;
};

static inline void ast_prog_init(struct ast_prog *prog, struct ast_stm *stm)
{
    prog->stm = stm;
}

#endif
