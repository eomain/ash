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

#include "ash/io.h"
#include "ash/lang/ast.h"
#include "ash/lang/lang.h"
#include "ash/lang/parser.h"

struct parser {
    struct ash_tk_set *set;
    struct ash_tk *token;
    const char *source;
    size_t line;
    size_t offset;
    bool error;
    bool interactive;
    size_t block;
    size_t fblock;
    size_t lblock;
    struct ast_path *path;
};

static void parser_init(struct parser *p, struct parser_meta *meta)
{
    const char *source;
    struct ash_tk *token;
    source = input_get_name(meta->input);
    token = ash_tk_set_front(meta->set);

    p->set = meta->set;
    p->token = token;
    p->source = source;
    p->line = (token) ? token->line: 1;
    p->offset = (token) ? token->offset: 1;
    p->error = false;
    p->interactive = (meta->input->interactive) ? true: false;
    p->block = 0;
    p->fblock = 0;
    p->lblock = 0;
    p->path = NULL;
}

static struct ast_stm *parser_main(struct parser *);
static struct ast_stm *parser_stm(struct parser *);
static struct ast_call *parser_call(struct parser *);
static struct ast_return *parser_return(struct parser *);
static struct ast_expr *parser_path_expr(struct parser *);
static struct ast_ternary *parser_ternary(struct parser *);
static struct ast_match *parser_match(struct parser *);
static struct ast_stm *parser_body_block(struct parser *);
static struct ast_expr *parser_expr(struct parser *);
static struct ast_expr *parser_expr_main(struct parser *);
static struct ast_bool_expr *parser_bool_expr(struct parser *);
static struct ast_composite *parser_args(struct parser *);
static void parser_error_expec(struct parser *, enum ash_tk_type);
static inline enum ash_tk_type parser_check_next(struct parser *);

static inline const char *
parser_get_source(struct parser *p)
{
    return (p->source) ? p->source: "<stdin>";
}

static void
parser_prompt(struct parser *p, enum input_prompt_type type)
{
    if (ash_lang_prompt(p->set, type))
        p->error = true;
}

static inline void
parser_assert_prompt(struct parser *p, enum input_prompt_type type)
{
    if (!p->interactive)
        return;

    if (!parser_check_next(p))
        parser_prompt(p, type);
}

static inline struct ast_path *
parser_get_path(struct parser *p)
{
    struct ast_path *path;
    path = p->path;
    p->path = NULL;
    return path;
}

static inline void
parser_set_path(struct parser *p, struct ast_path *path)
{
    p->path = path;
}

static inline struct ash_tk *
parser_get_token(struct parser *p)
{
    return p->token;
}

static inline enum ash_tk_type
parser_get_type(struct parser *p)
{
    return ash_tk_get(&p->token);
}

static inline void
parser_block_inc(struct parser *p)
{
    p->block++;
}

static inline void
parser_block_dec(struct parser *p)
{
    if (p->block > 0)
        p->block--;
}

static inline void
parser_fblock_inc(struct parser *p)
{
    p->fblock++;
}

static inline void
parser_fblock_dec(struct parser *p)
{
    if (p->fblock > 0)
        p->fblock--;
}

static inline void
parser_lblock_inc(struct parser *p)
{
    parser_block_inc(p);
    p->lblock++;
}

static inline void
parser_lblock_dec(struct parser *p)
{
    parser_block_dec(p);
    if (p->lblock > 0)
        p->lblock--;
}


static inline size_t parser_get_block(struct parser *p)
{
    return p->block;
}

static inline size_t parser_get_fblock(struct parser *p)
{
    return p->fblock;
}

static inline size_t parser_get_lblock(struct parser *p)
{
    return p->lblock;
}

static inline bool parser_block_eq(struct parser *p, size_t block)
{
    return (block == p->block);
}

static inline struct ash_tk *
parser_get_next(struct parser *p)
{
    struct ash_tk *token;
    if ((token = ash_tk_next(&p->token))) {
        p->line = token->line;
        p->offset = token->offset;
    }
    return token;
}

static inline enum ash_tk_type
parser_get_next_type(struct parser *p)
{
    parser_get_next(p);
    return parser_get_type(p);
}

static inline enum ash_tk_type
parser_check_next(struct parser *p)
{
    return ash_tk_cknext(&p->token);
}

static inline bool
parser_check_end(struct parser *p)
{
    return ash_tk_eos(&p->token);
}

static inline bool
parser_end_of_statement(struct parser *p)
{
    return ash_tk_get_eos(&p->token);
}

static inline bool
parser_has_error(struct parser *p)
{
    return p->error;
}

static inline int parser_assert(struct parser *p, enum ash_tk_type type)
{
    int assert = ash_tk_assert_type(&p->token, type);
    if (assert)
        parser_error_expec(p, type);
    return assert;
}

static inline int parser_assert_next(struct parser *p, enum ash_tk_type type)
{
    parser_get_next(p);
    return parser_assert(p, type);
}

static void parser_error_msg(struct parser *p, const char *fmt, const char *msg)
{
    const char *src = parser_get_source(p);
    size_t line = p->line;
    size_t offset = p->offset;
    p->error = true;
    if (msg)
        ash_print(fmt, src, line, offset, msg);
}

static void parser_error_found_msg(struct parser *p, const char *msg)
{
    parser_error_msg(p, "%s:%ld:%ld: syntax: found %s.\n", msg);
}

static void parser_error_found(struct parser *p, enum ash_tk_type type)
{
    const char *token;
    token = ash_tk_name(type);
    parser_error_msg(p,
        "%s:%ld:%ld: syntax: found unexpected '%s'.\n", token
    );
}

static void parser_error_expec_msg(struct parser *p, const char *msg)
{
    parser_error_msg(p, "%s:%ld:%ld: syntax: expected %s.\n", msg);
}

static void parser_error_expec(struct parser *p, enum ash_tk_type type)
{
    const char *token;
    token = ash_tk_name(type);
    parser_error_msg(p,
        "%s:%ld:%ld: syntax: expected '%s'.\n", token
    );
}

static void parser_error_found_current(struct parser *p)
{
    parser_error_found(p, parser_get_type(p));
}

static inline const char *parser_get_str(struct parser *p)
{
    return ash_tk_strcpy(&p->token);
}

static inline isize parser_get_num(struct parser *p)
{
    return ash_tk_num(&p->token);
}

static isize parser_value_num(struct parser *p)
{
    bool negate = parser_get_type(p) == SB_TK;

    if (negate) {
        parser_assert_next(p, NUM_TK);
        return (- parser_get_num(p));
    } else {
        parser_assert(p, NUM_TK);
        return parser_get_num(p);
    }
}

static struct ast_composite *parser_tuple(struct parser *p)
{
    struct ast_composite *tuple = NULL;
    parser_assert(p, LP_TK);

    if (parser_get_next_type(p) != RP_TK)
        tuple = parser_args(p);
    parser_assert(p, RP_TK);

    return tuple;
}

static struct ast_range *parser_range(struct parser *p, isize start)
{
    enum ash_tk_type type;
    struct ast_range *range;
    bool inclusive;
    isize end;

    type = parser_get_type(p);
    if (type == TO_TK)
        inclusive = true;
    else if (type == UT_TK)
        inclusive = false;
    else {
        parser_error_found_current(p);
        return NULL;
    }

    parser_get_next(p);
    end = parser_value_num(p);

    range = ast_range_new(start, end, inclusive);
    return range;
}

static struct ast_entry *parser_entry(struct parser *p)
{
    struct ast_entry *entry = NULL, *next = NULL;
    struct ast_expr *expr = NULL;
    const char *key;

    for (;;) {
        parser_assert(p, VAR_TK);
        key = parser_get_str(p);
        parser_assert_next(p, CN_TK);
        parser_get_next(p);
        expr = parser_expr_main(p);

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

        if (next) {
            next->next = ast_entry_new(key, expr);
            next = next->next;
        } else if (!entry) {
            if ((entry = ast_entry_new(key, expr)))
                next = entry;
        }

        if (parser_get_next_type(p) != CO_TK)
            break;

        parser_get_next(p);
    }

    return entry;
}

static struct ast_map *parser_map(struct parser *p)
{
    struct ast_map *map;
    struct ast_entry *entry = NULL;
    parser_assert(p, LB_TK);

    if (parser_get_next_type(p) != RB_TK)
        entry = parser_entry(p);
    parser_assert(p, RB_TK);
    map = ast_map_new(entry);
    return map;
}

static struct ast_param *parser_param(struct parser *p)
{
    struct ast_param *param = NULL, *next = NULL;
    const char *id;

    do {
        parser_assert(p, VAR_TK);
        id = parser_get_str(p);

        if (next) {
            next->next = ast_param_new(id);
            next = next->next;
        } else if (!param) {
            param = ast_param_new(id);
            next = param;
        }

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

    } while ((parser_get_next_type(p) == CO_TK) &&
              parser_get_next(p));

    return param;
}

static struct ast_function *parser_closure(struct parser *p)
{
    struct ast_function *function = NULL;
    struct ast_param *param = NULL;
    struct ast_stm *stm = NULL;

    parser_assert(p, PIP_TK);

    if (parser_get_next_type(p) != PIP_TK)
        param = parser_param(p);
    parser_assert(p, PIP_TK);

    if (parser_get_next_type(p) != END_TK)
        stm = parser_body_block(p);
    parser_assert(p, END_TK);

    return ast_function_new(NULL, param, stm);
}

static struct ast_var *parser_var(struct parser *p)
{
    struct ast_var *var;
    struct ast_path *path;
    const char *id;

    id = parser_get_str(p);
    path = parser_get_path(p);
    var = ast_var_new(id, true, path);
    return var;
}

static struct ast_literal *parser_literal(struct parser *p)
{
    struct ast_literal *literal = NULL;
    enum ash_tk_type type, next;
    type = parser_get_type(p);

    switch (type) {
        case TR_TK:
            literal = ast_literal_bool(true);
            break;

        case FS_TK:
            literal = ast_literal_bool(false);
            break;

        case SB_TK:
        case NUM_TK: {
                isize num;
                num = parser_value_num(p);
                next = parser_check_next(p);
                if (next == TO_TK || next == UT_TK) {
                    parser_get_next(p);
                    literal = ast_literal_range(parser_range(p, num));
                } else {
                    literal = ast_literal_num(num);
                }
            }
            break;

        case VAR_TK:
        case DQT_TK:
            literal = ast_literal_str(parser_get_str(p));
            break;

        case LP_TK:
            literal = ast_literal_tuple(parser_tuple(p));
            break;

        case LB_TK:
            literal = ast_literal_map(parser_map(p));
            break;

        case PIP_TK:
            literal = ast_literal_closure(parser_closure(p));
            break;

        default:
            if (type)
                parser_error_found_current(p);
            else
                parser_error_expec_msg(p, "<value>");
    }

    return literal;
}

static struct ast_value *parser_value(struct parser *p)
{
    struct ast_value *value = NULL;
    enum ash_tk_type type;
    type = parser_get_type(p);

    if (type == AV_TK) {
        struct ast_var *var;
        if ((var = parser_var(p)))
            value = ast_value_var(var);
    } else {
        struct ast_literal *literal;
        if ((literal = parser_literal(p)))
            value = ast_value_literal(literal);
    }

    return value;
}

static struct ast_bool_expr *parser_expr_block(struct parser *p)
{
    struct ast_bool_expr *expr = NULL;
    if (parser_assert_next(p, LS_TK))
        return NULL;
    if (!parser_check_next(p))
        parser_error_expec_msg(p, "<expression>");
    else if ((parser_get_next_type(p) == RS_TK))
        return NULL;
    if ((expr = parser_bool_expr(p)))
        parser_assert_next(p, RS_TK);
    return expr;
}

static struct ast_stm *parser_body_block(struct parser *p)
{
    enum ash_tk_type type;
    struct ast_stm *stm = NULL, *next = NULL;

    while ((type = parser_get_type(p)) && type != END_TK) {
        if (next) {
            next->next = parser_stm(p);
            next = next->next;
        } else if (!stm) {
            stm = parser_stm(p);
            next = stm;
        }

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

        parser_get_next(p);
    }

    return stm;
}

static inline bool parser_is_else(enum ash_tk_type type)
{
    return (type == EL_TK || type == ELF_TK);
}

static struct ast_stm *parser_if_block(struct parser *p)
{
    enum ash_tk_type type;
    struct ast_stm *stm = NULL, *next = NULL;

    while ((type = parser_get_type(p)) &&
           (type != END_TK && !parser_is_else(type))) {
        if (next) {
            next->next = parser_stm(p);
            next = next->next;
        } else if (!stm) {
            stm = parser_stm(p);
            next = stm;
        }

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

        parser_get_next(p);
    }

    return stm;
}

static struct ast_else *parser_else(struct parser *);

static struct ast_if *parser_if(struct parser *p)
{
    enum ash_tk_type type;
    struct ast_if *ast_if = NULL;
    struct ast_bool_expr *expr = NULL;
    struct ast_stm *stm = NULL;
    struct ast_else *ast_else = NULL;
    size_t block;

    if (parser_check_next(p) == IF_TK)
        parser_block_inc(p);

    expr = parser_expr_block(p);
    if (parser_has_error(p)) {
        /* TODO: free */
        return NULL;
    }
    parser_assert_prompt(p, INPUT_PROMPT_BLOCK);

    block = parser_get_block(p);
    if (parser_get_next_type(p) != END_TK)
        stm = parser_if_block(p);

    if (parser_has_error(p)) {
        /* TODO: free */
        return NULL;
    }

    type = parser_get_type(p);
    if ((type == EL_TK || type == ELF_TK)) {
        if (parser_block_eq(p, block)) {
            ast_else = parser_else(p);
        } else {
            parser_error_found_current(p);
            return NULL;
        }
    } else {
        parser_assert(p, END_TK);
        parser_block_dec(p);
    }

    ast_if = ast_if_new(expr, stm, ast_else);
    return ast_if;
}

static struct ast_else *parser_else(struct parser *p)
{
    struct ast_else *ast_else = NULL;
    enum ash_tk_type type;
    type = parser_get_type(p);

    if (type == EL_TK) {
        struct ast_stm *stm = NULL;
        if (parser_get_next_type(p) != END_TK)
            stm = parser_body_block(p);
        parser_assert(p, END_TK);
        ast_else = ast_else_new(stm);
    } else if (type == ELF_TK) {
        struct ast_if *ast_if;
        if ((ast_if = parser_if(p)))
            ast_else = ast_else_if_new(ast_if);
    }

    return ast_else;
}

static void *parser_break(struct parser *p)
{
    parser_assert(p, BK_TK);
    if (!(parser_get_lblock(p) > 0))
        parser_error_found_current(p);
    return NULL;
}

static void *parser_next(struct parser *p)
{
    parser_assert(p, NXT_TK);
    if (!(parser_get_lblock(p) > 0))
        parser_error_found_current(p);
    return NULL;
}

static struct ast_while *parser_while(struct parser *p)
{
    struct ast_while *ast_while = NULL;
    struct ast_bool_expr *expr = NULL;
    struct ast_stm *stm = NULL;

    parser_assert(p, DO_TK);
    parser_lblock_inc(p);
    expr = parser_expr_block(p);
    if (parser_has_error(p)) {
        /* TODO: free */
        return NULL;
    }
    parser_assert_prompt(p, INPUT_PROMPT_BLOCK);

    if (parser_get_next_type(p) != END_TK)
        stm = parser_body_block(p);

    if (parser_has_error(p)) {
        /* TODO: free */
        return NULL;
    }

    parser_assert(p, END_TK);
    parser_lblock_dec(p);

    ast_while = ast_while_new(expr, stm);

    return ast_while;
}

static struct ast_for *parser_for(struct parser *p)
{
    struct ast_for *ast_for = NULL;
    struct ast_expr *expr;
    struct ast_stm *stm = NULL;
    struct ast_var *av;
    const char *id;

    parser_assert(p, FOR_TK);
    parser_lblock_inc(p);

    if (parser_assert_next(p, VAR_TK))
        return NULL;
    id = parser_get_str(p);
    av = ast_var_new(id, false, NULL);
    if (parser_assert_next(p, IN_TK))
        return NULL;

    parser_get_next(p);
    if (!(expr = parser_expr_main(p))) {
        parser_error_expec_msg(p, "<iterable>");
        return NULL;
    }
    parser_assert_prompt(p, INPUT_PROMPT_BLOCK);

    if (parser_check_next(p) != END_TK) {
        parser_get_next(p);
        stm = parser_body_block(p);
    } else
        parser_assert_next(p, END_TK);

    parser_lblock_dec(p);
    ast_for = ast_for_new(av, expr, stm);
    return ast_for;
}

static inline bool parser_is_call(struct parser *p, enum ash_tk_type type)
{
    return ((type == VAR_TK || type == AV_TK) && parser_check_next(p) == LP_TK);
}

static inline bool parser_is_bin_ops(enum ash_tk_type type)
{
    return (type == PS_TK || type == SB_TK ||
            type == ST_TK || type == DV_TK ||
            type == MD_TK);
}

static struct ast_expr *parser_bin_expr(struct parser *p, struct ast_expr *expr)
{
    struct ast_binary *binary = NULL;
    enum ast_binary_op op;
    struct ast_expr *nexpr;

    switch (parser_get_next_type(p)) {
        case PS_TK:
            op = AST_BINARY_ADD;
            break;
        case SB_TK:
            op = AST_BINARY_SUB;
            break;
        case ST_TK:
            op = AST_BINARY_MUL;
            break;
        case DV_TK:
            op = AST_BINARY_DIV;
            break;
        case MD_TK:
            op = AST_BINARY_MOD;
            break;
    }

    parser_get_next(p);
    nexpr = parser_expr(p);
    if (parser_has_error(p))
        return NULL;
    binary = ast_binary_new(op, expr, nexpr);
    expr = ast_expr_new(AST_EXPR_BINARY, binary);
    return expr;
}

static inline bool parser_is_scope(struct parser *p, enum ash_tk_type type)
{
    return (type == SCP_TK ||
           (type == VAR_TK && parser_check_next(p) == SCP_TK));
}

static struct ast_expr *parser_hash_expr(struct parser *p, struct ast_expr *expr)
{
    const char *key;
    struct ast_hash *hash = NULL;

    parser_assert_next(p, LS_TK);
    parser_assert_next(p, VAR_TK);
    key = parser_get_str(p);
    parser_assert_next(p, RS_TK);

    hash = ast_hash_new(key, expr);

    return ast_expr_new(AST_EXPR_HASH, hash);
}

static struct ast_expr *parser_expr(struct parser *p)
{
    struct ast_expr *expr = NULL;
    enum ash_tk_type type;
    type = parser_get_type(p);


    if (type == QMK_TK) {
        struct ast_ternary *ternary;
        if ((ternary = parser_ternary(p)))
            expr = ast_expr_new(AST_EXPR_TERNARY,  ternary);
    } else if (type == MAT_TK) {
        struct ast_match *match;
        if ((match = parser_match(p)))
            expr = ast_expr_new(AST_EXPR_MATCH, match);
    } else if (parser_is_call(p, type)) {
        struct ast_call *call;
        if ((call = parser_call(p)))
            expr = ast_expr_new(AST_EXPR_CALL, call);
    } else if (parser_is_scope(p, type)) {
        expr = parser_path_expr(p);
    } else {
        struct ast_value *value;
        if ((value = parser_value(p)))
            expr = ast_expr_new(AST_EXPR_VALUE, value);
    }

    if (parser_is_bin_ops(parser_check_next(p)))
        expr = parser_bin_expr(p, expr);
    else if (parser_check_next(p) == LS_TK) {
        while ((parser_check_next(p) == LS_TK))
            expr = parser_hash_expr(p, expr);
    }

    return expr;
}

static inline bool parser_is_cmp_ops(enum ash_tk_type type)
{
    return (type == EQ_TK || type == NE_TK ||
            type == LN_TK || type == GN_TK ||
            type == LE_TK || type == GE_TK);
}

static struct ast_expr *parser_cmp_expr(struct parser *p, struct ast_expr *expr)
{
    enum ast_cmp_op op;
    struct ast_cmp *cmp;
    struct ast_expr *nexpr;

    switch (parser_get_next_type(p)) {
        case EQ_TK:
            op = AST_CMP_EQ;
            break;
        case NE_TK:
            op = AST_CMP_NE;
            break;
        case LN_TK:
            op = AST_CMP_LN;
            break;
        case GN_TK:
            op = AST_CMP_GN;
            break;
        case LE_TK:
            op = AST_CMP_LE;
            break;
        case GE_TK:
            op = AST_CMP_GE;
            break;
    }

    parser_get_next(p);
    nexpr = parser_expr(p);
    if (parser_has_error(p))
        return NULL;
    cmp = ast_cmp_new(op, expr, nexpr);
    expr = ast_expr_new(AST_EXPR_CMP, cmp);
    return expr;
}

static struct ast_expr *parser_expr_cmp(struct parser *p)
{
    struct ast_expr *expr;
    expr = parser_expr(p);

    if (parser_is_cmp_ops(parser_check_next(p)))
        return parser_cmp_expr(p, expr);
    return expr;
}

static inline bool parser_is_logical_ops(enum ash_tk_type type)
{
    return (type == AN_TK || type == OR_TK);
}

static struct ast_expr *parser_expr_logical(struct parser *);

static struct ast_expr *parser_logical_expr(struct parser *p, struct ast_expr *expr)
{
    enum ast_logical_op op;
    struct ast_logical *logical;
    struct ast_expr *nexpr;

    switch (parser_get_next_type(p)) {
        case AN_TK:
            op = AST_LOGICAL_AND;
            break;
        case OR_TK:
            op = AST_LOGICAL_OR;
            break;
    }

    parser_get_next(p);
    nexpr = parser_expr_logical(p);
    if (parser_has_error(p))
        return NULL;
    logical = ast_logical_new(op, expr, nexpr);
    expr = ast_expr_new(AST_EXPR_LOGICAL, logical);
    return expr;
}

static struct ast_expr *parser_expr_logical(struct parser *p)
{
    bool negate = false;
    struct ast_expr *expr;

    if (parser_get_type(p) == NT_TK) {
        negate = true;
        parser_get_next(p);
    }
    expr = parser_expr_cmp(p);

    if (negate) {
        struct ast_unary *unary;
        unary = ast_unary_new(AST_UNARY_NOT, expr);
        expr = ast_expr_new(AST_EXPR_UNARY, unary);
    }

    if (parser_is_logical_ops(parser_check_next(p)))
        expr = parser_logical_expr(p, expr);

    return expr;
}

static struct ast_expr *parser_bq_expr(struct parser *p)
{
    struct ast_expr *expr = NULL;

    parser_assert(p, BQ_TK);
    if (parser_check_next(p) != BQ_TK) {
        parser_get_next(p);
        expr = parser_expr_logical(p);
    }
    parser_assert_next(p, BQ_TK);
    return expr;
}

static struct ast_expr *parser_expr_main(struct parser *p)
{
    struct ast_expr *expr = NULL;
    enum ash_tk_type type;
    type = parser_get_type(p);

    if (type == BQ_TK)
        expr = parser_bq_expr(p);
    else
        expr = parser_expr(p);

    return expr;
}

static struct ast_bool_expr *parser_bool_expr(struct parser *p)
{
    struct ast_bool_expr *bexpr;
    struct ast_expr *expr;
    expr = parser_expr_logical(p);
    return ast_bool_expr_new(expr);
}

static struct ast_assign *parser_assign(struct parser *p)
{
    struct ast_assign *assign = NULL;
    struct ast_var *av;
    const char *id;
    struct ast_expr *expr;
    bool local = false;

    if (parser_get_type(p) == LET_TK) {
        local = true;
        parser_get_next(p);
    }

    parser_assert(p, VAR_TK);
    id = parser_get_str(p);
    parser_assert_next(p, AS_TK);
    parser_get_next(p);

    if ((expr = parser_expr_main(p))) {
        av = ast_var_new(id, false, NULL);
        assign = ast_assign_new(local, av, expr);
    }
    return assign;
}

static struct ast_ternary *parser_ternary(struct parser *p)
{
    struct ast_ternary *ternary = NULL;
    struct ast_bool_expr *cond;
    struct ast_expr *e1, *e2;

    parser_assert(p, QMK_TK);
    cond = parser_expr_block(p);
    parser_get_next(p);

    if (!(e1 = parser_expr_main(p)))
        return NULL;

    parser_assert_next(p, CN_TK);
    parser_get_next(p);

    if (!(e2 = parser_expr_main(p)))
        return NULL;

    ternary = ast_ternary_new(cond, e1, e2);

    return ternary;
}

static struct ast_expr *parser_expr_list(struct parser *p)
{
    struct ast_expr *expr = NULL, *next = NULL;

    for (;;) {
        if (next) {
            next->next = parser_expr_main(p);
            next = next->next;
        } else if (!expr) {
            if ((expr = parser_expr_main(p)))
                next = expr;
        }

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

        if (parser_get_next_type(p) != CO_TK)
            break;

        parser_get_next(p);
    }

    return expr;
}

static inline bool parser_is_otherwise(struct parser *p)
{
    if (parser_get_type(p) != VAR_TK)
        return false;

    const char *s = parser_get_str(p);
    return (s && s[0] == '_' && !s[1]) ? true: false;
}

static struct ast_case *parser_case(struct parser *p)
{
    struct ast_case *ecase, *next = NULL;
    struct ast_expr *expr, *eval;

    expr = parser_expr_list(p);
    parser_assert(p, ARW_TK);
    parser_get_next(p);
    eval = parser_expr_main(p);

    if (parser_check_next(p) == CO_TK) {
        parser_get_next(p);
        parser_get_next(p);
        if (!(parser_is_otherwise(p)))
            next = parser_case(p);
    }

    ecase = ast_case_new(expr, eval, next);
    return ecase;
}

static struct ast_expr *parser_otherwise(struct parser *p)
{
    struct ast_expr *otherwise = NULL;

    parser_assert_next(p, ARW_TK);
    parser_get_next(p);
    otherwise = parser_expr_main(p);

    return otherwise;
}

static struct ast_match *parser_match(struct parser *p)
{
    struct ast_match *match = NULL;
    struct ast_expr *expr = NULL, *otherwise = NULL;
    struct ast_case *ecase = NULL;

    parser_assert(p, MAT_TK);
    parser_assert_next(p, LS_TK);
    if (parser_get_next_type(p) != RS_TK) {
        expr = parser_expr_logical(p);
        parser_get_next(p);
    }
    parser_assert(p, RS_TK);

    if (parser_has_error(p)) {
        /* TODO: free */
        return NULL;
    }
    parser_assert_prompt(p, INPUT_PROMPT_BLOCK);

    if (parser_get_next_type(p) != END_TK) {
        if (!(parser_is_otherwise(p))) {
            ecase = parser_case(p);
            if (parser_is_otherwise(p))
                otherwise = parser_otherwise(p);
        } else {
            otherwise = parser_otherwise(p);
        }
        parser_assert_next(p, END_TK);
    }

    match = ast_match_new(expr, ecase, otherwise);
    return match;
}

static struct ast_command *parser_command(struct parser *p)
{
    enum ash_tk_type type;
    struct ast_command *command = NULL;
    struct ast_expr *expr = NULL, *next = NULL;
    size_t length = 0;

    for (;;) {
        if (next) {
            next->next = parser_expr_main(p);
            if ((next = next->next))
                length++;
            else
                parser_error_expec_msg(p, "<expression>");
        } else if (!expr) {
            if ((expr = parser_expr_main(p))) {
                next = expr;
                length++;
            } else
                parser_error_expec_msg(p, "<expression>");
        }

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

        if (parser_end_of_statement(p))
            break;

        if ((type = parser_get_next_type(p)) == NO_TK)
            break;

        if (type == CO_TK) {
            if (!parser_check_end(p)) {
                parser_error_expec_msg(p,
                    "'newline' following ','"
                );
                return NULL;
            }
            parser_assert_prompt(p, INPUT_PROMPT_COMMAND);
            parser_get_next(p);
        }
    }

    command = ast_command_new(expr, length);
    return command;
}

static struct ast_composite *parser_args(struct parser *p)
{
    struct ast_composite *args = NULL;
    size_t length = 0;
    struct ast_expr *expr = NULL, *next = NULL;

    for (;;) {
        if (next) {
            next->next = parser_expr_main(p);
            if ((next = next->next))
                length++;
        } else if (!expr) {
            if ((expr = parser_expr_main(p))) {
                next = expr;
                length++;
            }
        }

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

        if (parser_get_next_type(p) != CO_TK)
            break;

        parser_get_next(p);
    }

    args = ast_composite_new(expr, length);
    return args;
}

static struct ast_call *parser_call(struct parser *p)
{
    struct ast_var *var;
    struct ast_path *path;
    struct ast_call *call = NULL;
    struct ast_composite *args = NULL;
    const char *id;

    id = parser_get_str(p);
    path = parser_get_path(p);
    var = ast_var_new(id, (*id == '$') ? true: false, path);
    parser_assert_next(p, LP_TK);

    parser_get_next(p);
    if (parser_get_type(p) != RP_TK)
        args = parser_args(p);
    parser_assert(p, RP_TK);

    call = ast_call_new(var, args);

    return call;
}

static struct ast_stm *parser_stm(struct parser *p)
{
    if (parser_has_error(p))
        return NULL;

    enum ash_tk_type type, ntype = NO_TK;
    type = parser_get_type(p);
    void *node = NULL;
    struct ast_stm *stm = NULL;

    switch (type) {

        /* if statement */
        case IF_TK:
            ntype = AST_NODE_IF;
            node = parser_if(p);
            break;

        /* while loop */
        case DO_TK:
            ntype = AST_NODE_WHILE;
            node = parser_while(p);
            break;

        /* for loop */
        case FOR_TK:
            ntype = AST_NODE_FOR;
            node = parser_for(p);
            break;

        case AV_TK: {
                enum ash_tk_type next;
                next = parser_check_next(p);
                 if (next == LP_TK) {
                    ntype = AST_NODE_EXPR;
                    node = parser_expr(p);
                } else {
                    ntype = AST_NODE_COMMAND;
                    node = parser_command(p);
                }

                parser_end_of_statement(p);
            }
            break;

        case BQ_TK:
        case DQT_TK:
        case NUM_TK:
        case QMK_TK:
        case MAT_TK:
            ntype = AST_NODE_COMMAND;
            node = parser_command(p);
            break;

        case LET_TK:
            ntype = AST_NODE_ASSIGN;
            node = parser_assign(p);
            parser_end_of_statement(p);
            break;

        case SCP_TK:
            ntype = AST_NODE_EXPR;
            node = parser_expr(p);
            parser_end_of_statement(p);
            break;

        case VAR_TK: {
                enum ash_tk_type next;
                next = parser_check_next(p);
                if (next == AS_TK) {
                    ntype = AST_NODE_ASSIGN;
                    node = parser_assign(p);
                } else if (next == LP_TK || next == SCP_TK) {
                    ntype = AST_NODE_EXPR;
                    node = parser_expr(p);
                } else {
                    ntype = AST_NODE_COMMAND;
                    node = parser_command(p);
                    break;
                }

                parser_end_of_statement(p);
            }
            break;

        case RET_TK:
            ntype = AST_NODE_RET;
            node = parser_return(p);
            parser_end_of_statement(p);
            break;

        case BK_TK:
            ntype = AST_NODE_BREAK;
            node = parser_break(p);
            parser_end_of_statement(p);
            break;

        case NXT_TK:
            ntype = AST_NODE_NEXT;
            node = parser_next(p);
            parser_end_of_statement(p);
            break;

        default:
            parser_error_found_current(p);
    }

    if (!parser_has_error(p) && ntype != NO_TK)
        stm = ast_stm_new(ntype, node);

    return stm;
}

static struct ast_stm *parser_function_main(struct parser *);

static struct ast_stm *parser_function_block(struct parser *p)
{
    enum ash_tk_type type;
    struct ast_stm *stm = NULL, *next = NULL;

    while ((type = parser_get_type(p)) && type != END_TK) {
        if (next) {
            next->next = parser_function_main(p);
            next = next->next;
        } else if (!stm) {
            stm = parser_function_main(p);
            next = stm;
        }

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

        parser_get_next(p);
    }

    return stm;
}

static struct ast_function *parser_function(struct parser *p)
{
    struct ast_function *function = NULL;
    struct ast_stm *stm = NULL;
    struct ast_param *param = NULL, *next = NULL;
    const char *id;

    parser_assert(p, DEF_TK);
    parser_fblock_inc(p);
    parser_assert_next(p, VAR_TK);
    id = parser_get_str(p);
    parser_assert_next(p, LP_TK);

    if (parser_check_next(p) != RP_TK) {
        parser_get_next(p);
        param = parser_param(p);
        parser_assert(p, RP_TK);
    } else
        parser_assert_next(p, RP_TK);

    if (parser_has_error(p)) {
        /* TODO: free */
        return NULL;
    }
    parser_assert_prompt(p, INPUT_PROMPT_BLOCK);
    parser_get_next(p);
    stm = parser_function_block(p);
    parser_assert(p, END_TK);
    parser_fblock_dec(p);
    function = ast_function_new(id, param, stm);
    return function;
}

static struct ast_return *parser_return(struct parser *p)
{
    struct ast_return *ret = NULL;
    struct ast_expr *expr = NULL;

    if (!(parser_get_fblock(p) > 0)) {
        parser_error_found_msg(p,
            "'return' outside of function definition"
        );
        return NULL;
    }

    parser_assert(p, RET_TK);
    if (!parser_end_of_statement(p)) {
        parser_get_next(p);
        expr = parser_expr_main(p);
    }
    ret = ast_return_new(expr);
    return ret;
}

static struct ast_stm *parser_function_main(struct parser *p)
{
    struct ast_stm *stm = NULL;

    if (parser_get_type(p) == DEF_TK) {
        void *node;
        if ((node = parser_function(p)))
            stm = ast_stm_new(AST_NODE_FUNC, node);
    } else
        stm = parser_stm(p);

    return stm;
}

static struct ast_path *parser_path(struct parser *p)
{
    enum ash_tk_type type;
    struct ast_path *path = NULL;
    struct ast_scope *scope = NULL, *next = NULL;
    enum ash_module_path_type ptype;
    size_t length = 0;
    const char *id;

    type = parser_get_type(p);
    if (type == SCP_TK) {
        ptype = ASH_PATH_ABS;
        parser_get_next(p);
    } else if (type == VAR_TK) {
        ptype = ASH_PATH_CUR;
    }

    parser_assert(p, VAR_TK);
    id = parser_get_str(p);
    if (!scope) {
        scope = ast_scope_new(id);
        if ((next = scope))
            length++;
    }
    parser_assert_next(p, SCP_TK);

    for (;;) {
        type = parser_get_next_type(p);

        if (type == AV_TK) {
            break;
        } else if (type == VAR_TK) {
            enum ash_tk_type next;
            next = parser_check_next(p);

            if (next == LP_TK) {
                break;
            } else if (next == SCP_TK) {
                id = parser_get_str(p);
            } else {
                parser_error_expec_msg(p, ":: or (");
                return NULL;
            }

        } else {
            parser_error_found_current(p);
            /* TODO: free */
            return NULL;
        }

        if (next) {
            next->next = ast_scope_new(id);
            if ((next = next->next))
                length++;
        }

        parser_assert_next(p, SCP_TK);
    }

    if (!parser_has_error(p))
        path = ast_path_new(ptype, length, scope);
    return path;
}

static struct ast_expr *parser_path_expr(struct parser *p)
{
    struct ast_expr *expr = NULL;
    struct ast_path *path;

    if ((path = parser_path(p))) {
        parser_set_path(p, path);
        expr = parser_expr(p);
    }

    return expr;
}

static struct ast_use *parser_use(struct parser *p)
{
    struct ast_use *use = NULL;
    struct ast_path *path;

    parser_assert(p, USE_TK);
    parser_get_next(p);
    if ((path = parser_path(p)))
        use = ast_use_new(path);

    return use;
}

static struct ast_stm *parser_module_block(struct parser *p)
{
    enum ash_tk_type type;
    struct ast_stm *stm = NULL, *next = NULL;

    while ((type = parser_get_type(p)) && type != END_TK) {
        if (next) {
            next->next = parser_main(p);
            next = next->next;
        } else if (!stm) {
            stm = parser_main(p);
            next = stm;
        }

        if (parser_has_error(p)) {
            /* TODO: free */
            return NULL;
        }

        parser_get_next(p);
    }

    return stm;
}

static struct ast_module *parser_module(struct parser *p)
{
    struct ast_module *module = NULL;
    struct ast_stm *stm = NULL;
    const char *name;

    parser_assert(p, MOD_TK);
    parser_assert_next(p, VAR_TK);
    name = parser_get_str(p);
    parser_assert_prompt(p, INPUT_PROMPT_BLOCK);
    parser_get_next(p);
    stm = parser_module_block(p);
    module = ast_module_new(name, stm);

    parser_assert(p, END_TK);

    return module;
}

static struct ast_stm *parser_main(struct parser *p)
{
    struct ast_stm *stm = NULL;

    if (parser_get_type(p) == MOD_TK) {
        void *node;
        if ((node = parser_module(p)))
            stm = ast_stm_new(AST_NODE_MODULE, node);
    } else
        stm = parser_function_main(p);

    return stm;
}

int parser_ast_construct(struct ast_prog *prog, struct parser_meta *meta)
{
    struct parser parser;
    struct ast_stm *stm = NULL, *next = NULL;
    parser_init(&parser, meta);

    do {
        if (next) {
            next->next = parser_main(&parser);
            next = next->next;
        } else if (!stm) {
            stm = parser_main(&parser);
            next = stm;
        }
    } while (parser_get_next(&parser));

    if (parser_has_error(&parser)) {

        return -1;
    }

    ast_prog_init(prog, stm);
    return 0;
}
