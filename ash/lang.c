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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ash.h"
#include "env.h"
#include "exec.h"
#include "io.h"
#include "mem.h"
#include "ops.h"
#include "var.h"

#define ASH_LANG_TRUE  "1"
#define ASH_LANG_FALSE "0"

#define ASH_LANG_NIL "(nil)"

/* list of runtime exception codes */
#define ASH_EXCEPTION -1
#define ASH_EXCEPTION_STACK_OVERFLOW 0
#define ASH_EXCEPTION_ZERO_DIV 1
#define ASH_EXCEPTION_NIL 2
#define ASH_EXCEPTION_ILLEGAL_OP 3
#define ASH_EXCEPTION_EMPTY_STACK 4

#define ASH_FMT_SIZE 32

/* amount of tokens to pre-allocate for */
#define DEFAULT_TOKEN_BUFFER 300
/* init size of the eval stack */
#define DEFAULT_STACK_SIZE 255

/*
indicates an error has occurred while
scanning the input or parsing the token stream
*/
static void ash_lang_err(void);

/*
displays the exception encountered and
sets the error check
*/
static void ash_runtime_exception(int excep)
{
    const char *e_type;
    if (excep == ASH_EXCEPTION_STACK_OVERFLOW)
        e_type = "(stack overflow!)";
    else if (excep == ASH_EXCEPTION_ZERO_DIV)
        e_type = "(division by zero!)";
    else if (excep == ASH_EXCEPTION_NIL)
        e_type = ASH_LANG_NIL;
    else if (excep == ASH_EXCEPTION_ILLEGAL_OP)
        e_type = "(illegal operation)";
    else if (excep == ASH_EXCEPTION_EMPTY_STACK)
        e_type = "(empty stack!)";
    else
        e_type = "(runtime!)";

    ash_print(PNAME ": exception: %s\n", e_type);
    ash_lang_err();
}

static const char *ash_reserved(int);
static inline int ash_lang_hasnext(void);
static inline char ash_lang_next(void);
static inline char ash_lang_readnext(void);
static inline int ash_lang_fetch(char);
static inline int ash_lang_advance(char);
static inline const char *ash_lang_get(void);
static inline void ash_lang_rm_quote(void);
static int ash_lang_reserved(const char *);
static void ash_lang_err_expec(int);
static void ash_lang_err_expec_msg(const char *);
static void ash_lang_err_found(int);
static void ash_lang_err_found_msg(const char *);
static void ash_lang_err_msg(const char *, ...);
static inline int ash_lang_ckerr(void);
static void ash_lang_prompt(void);
static int ash_lang_assert_prompt(void);

/*
contains a list of symbols representing all
recognised tokens in ash
*/
enum {
    /* no token */
    NO_TK  = 0x00,
    /* comment token */
    CM_TK,
    /* whitespace token */
    WS_TK,
    /* comma token */
    CO_TK,
    /* ash var token */
    AV_TK,
    /* if token */
    IF_TK,
    /* else token */
    EL_TK,
    /* loop (while) token */
    DO_TK,
    /* break token */
    BK_TK,
    /* negate token */
    NT_TK,
    /* equal token */
    EQ_TK,
    /* not equal token */
    NE_TK,
    /* less than token */
    LN_TK,
    /* greater than token */
    GN_TK,
    /* plus token */
    PS_TK,
    /* subtract token */
    SB_TK,
    /* star token */
    ST_TK,
    /* div token */
    DV_TK,
    /* mod token */
    MD_TK,
    /* assign token */
    AS_TK,
    /* colon token */
    CN_TK,
    /* left square token */
    LS_TK,
    /* right square token */
    RS_TK,
    /* left paren */
    LP_TK,
    /* right paren */
    RP_TK,
    /* true */
    TR_TK,
    /* false */
    FS_TK,
    /* and */
    AN_TK,
    /* or */
    OR_TK,
    /* in */
    IN_TK,
    /* to */
    TO_TK,
    /* until */
    UT_TK,
    /* for */
    FOR_TK,
    /* pipe */
    PIP_TK,
    /* end */
    END_TK,

    NUM_TK,
    VAR_TK,
    DQT_TK,
    ELF_TK,
    /* semi-colon or newline */
    EOS_TK,
    RET_TK
};

struct ash_tk {
    int res;
    long num;
    const char *str;
    struct ash_tk *next;
};

static struct ash_tk ash_tk_buf[DEFAULT_TOKEN_BUFFER];

static struct ash_tk *
ash_tk_new(int res, const char *str)
{
    struct ash_tk *tk = ash_alloc(sizeof ( *tk ));

    tk->str = str;
    tk->res = res;
    tk->next = NULL;
    return tk;
}

static inline struct ash_tk *
ash_tk_next(struct ash_tk **tk)
{
    if (*tk == NULL)
        return NULL;
    assert(*tk);
    *tk = (*tk)->next;
    return *tk;
}

static inline int
ash_tk_get(struct ash_tk **tk)
{
    if (tk && *tk)
        return (*tk)->res;
    else
        return NO_TK;
}

static inline int
ash_tk_valid(struct ash_tk **tk)
{
    return ash_tk_get(tk) != NO_TK;
}

static inline int
ash_tk_getnext(struct ash_tk **tk)
{
    ash_tk_next(tk);
    return ash_tk_get(tk);
}

static inline int
ash_tk_cknext(struct ash_tk **tk)
{
    if (tk && *tk)
        return ash_tk_get( &(*tk)->next );
    else
        return NO_TK;
}

/* move string out of token */
static inline const char *
ash_tk_str_take(struct ash_tk **tk)
{
    if (tk && *tk){
        const char *s = (*tk)->str;
        (*tk)->str = NULL;
        return s;
    }
    return NULL;
}

static inline int
ash_tk_assert_type(struct ash_tk **tk, int res)
{
    if (ash_tk_get(tk) != res && !ash_lang_ckerr())
        ash_lang_err_expec(res);
    return ash_lang_ckerr();
}

static inline int
ash_lang_ws(char c)
{
    return (c == ' ' || c == '\0' || c == '\t' || c == '\v' || c == '\r' || c == '\f');
}

static inline void
ash_lang_comment(void)
{
    while (!ash_lang_fetch('\n'));
}

static inline void
ash_lang_quote(char c)
{
    while (!ash_lang_fetch(c));
    ash_lang_rm_quote();
}

static int
ash_lang_type(char c)
{
    if (ash_lang_ws(c))
        return WS_TK;

    switch (c){
        case '\n':
        case ';':   return EOS_TK;

        case '$':   return AV_TK;
        case '#':   return CM_TK;
        case '\"':  return DQT_TK;

        case '|':   return PIP_TK;

        case ':':
            if (ash_lang_advance('='))
                return AS_TK;
            return CN_TK;
        case '[':   return LS_TK;
        case ']':   return RS_TK;
        case '(':   return LP_TK;
        case ')':   return RP_TK;

        case '<':   return LN_TK;
        case '>':   return GN_TK;

        case ',':   return CO_TK;

        default:    return VAR_TK;
    }
    return NO_TK;
}

struct {
    size_t len;
    int err;
    const char *string;
    const char *script;
}

static ash_lang = {
    .len = 0,
    .err = 0,
    .string = NULL,
    .script = NULL
};

static inline int ash_lang_ckerr(void)
{
    return ash_lang.err;
}

static const char *ash_lang_fmt(void)
{
    const char *s = ash_lang_get();
    if (*s == '~'){
        size_t len = ash_lang.len + 1;
        char buf[len];
        buf[len -1] = '\0';
        memcpy(buf, s, len - 1);
        return ash_ops_tilde(buf);
    }
    return NULL;
}

static const char *ash_lang_string(int res)
{
    if (res == DQT_TK || res == VAR_TK){
        const char *fmt = ash_lang_fmt();
        if (fmt)
            return fmt;
    }
    char *str = ash_alloc(ash_lang.len + 1);
    memset(str, 0, (ash_lang.len + 1));
    memcpy(str, ash_lang_get(), ash_lang.len);
    ash_lang.len = 0;
    return str;
}

static int ash_lang_reserved(const char *s)
{
    switch (s[0]){
        case 'a':
            if (s[1] == 'n' &&
                s[2] == 'd' &&
                !s[3])
                return AN_TK;
            break;
        case 'b':
            if (s[1] == 'r' &&
                s[2] == 'e' &&
                s[3] == 'a' &&
                s[4] == 'k' &&
                !s[5])
                return BK_TK;
            break;
        case 'e':
            if (s[1] == 'l' &&
                s[2] == 's' &&
                s[3] == 'e' &&
                !s[4])
                return EL_TK;

            if (s[1] == 'n' &&
                s[2] == 'd' &&
                !s[3])
                return END_TK;
            break;
        case 'f':
            if (s[1] == 'a' &&
                s[2] == 'l' &&
                s[3] == 's' &&
                s[4] == 'e' &&
                !s[5])
                return FS_TK;
            else if (s[1] == 'o' &&
                     s[2] == 'r' &&
                     !s[3])
                     return FOR_TK;
            break;
        case 'i':
            if (s[1] == 'f' &&
                !s[2])
                return IF_TK;
            else if (s[1] == 'n' &&
                     !s[2])
                     return IN_TK;
            break;
        case 'o':
            if (s[1] == 'r' &&
                !s[2])
                return OR_TK;
            break;
        case 'r':
            if (s[1] == 'e' &&
                s[2] == 't' &&
                s[3] == 'u' &&
                s[4] == 'r' &&
                s[5] == 'n' &&
                !s[6])
                return RET_TK;
            break;
        case 't':
            if (s[1] == 'o' &&
                !s[2])
                return TO_TK;
            else if (s[1] == 'r' &&
                     s[2] == 'u' &&
                     s[3] == 'e' &&
                     !s[4])
                return TR_TK;
            break;
        case 'u':
            if (s[1] == 'n' &&
                s[2] == 't' &&
                s[3] == 'i' &&
                s[4] == 'l' &&
                !s[5])
                return UT_TK;
            break;
        case 'w':
            if (s[1] == 'h' &&
                s[2] == 'i' &&
                s[3] == 'l' &&
                s[4] == 'e' &&
                !s[5])
                return DO_TK;
            break;
    }
    return NO_TK;
}

struct ash_tk_set {
    struct ash_tk *front;
    struct ash_tk *rear;
};

static void ash_tk_set_add(struct ash_tk_set *set,
                           int res, const char *str)
{
    if (set->rear){
        set->rear->next = ash_tk_new(res, str);
        set->rear = set->rear->next;
    } else {
        set->front = set->rear = ash_tk_new(res, str);
    }
}

static void ash_tk_set_free(struct ash_tk_set *set)
{
    if (set->front){
        struct ash_tk *tk = set->front, *n;
        do {
            n = tk->next;
            if (tk->str)
                ash_free((void *)tk->str);
            ash_free(tk);
        } while((tk = n));

        set->front = NULL;
    }
    set->rear = NULL;
}

static int ash_lang_match_num(void)
{
    while (ash_lang_type(ash_lang_next()) == NUM_TK)
        ash_lang_readnext();
    return NUM_TK;
}

static void
ash_lang_match_var(struct ash_tk_set *set, int c)
{
    int type;
    for (;;){
        type = ash_lang_type(ash_lang_next());
        if (type == VAR_TK ||
            type == NUM_TK)
                ash_lang_readnext();
        else
            break;
    }

    if (c == AV_TK)
        return ash_tk_set_add(set, AV_TK, ash_lang_string(AV_TK));

    char res[ash_lang.len + 1];
    memset(res, 0, ash_lang.len + 1);
    memcpy(res, ash_lang_get(), ash_lang.len);

    type = ash_lang_reserved(res);

    if (type == NO_TK)
        ash_tk_set_add(set, VAR_TK, ash_lang_string(VAR_TK));
    else if (type == TR_TK)
        ash_tk_set_add(set, TR_TK, ASH_LANG_TRUE);
    else if (type == FS_TK)
        ash_tk_set_add(set, FS_TK, ASH_LANG_FALSE);
    else
        ash_tk_set_add(set, type, NULL);
}

static inline int ash_lang_hasnext(void)
{
    return *ash_lang.script ? 1: 0;
}

static void ash_lang_reset(void)
{
    ash_lang.len = 0;
    ash_lang.string = ash_lang.script;
}

static inline char ash_lang_readnext(void)
{
    ++ash_lang.len;
    return *(ash_lang.script++);
}

static inline char ash_lang_next(void)
{
    return *ash_lang.script;
}

/* check for escape sequence */
static int esc = 0;

static inline int ash_lang_fetch(char c)
{
    int next = ash_lang_hasnext();
    int seq = ash_lang_next();
    int o = (c == seq);
    if (esc == 1){
        o = 0;
        esc = 0;
    }
    if (seq == '\\')
        esc = 1;
    if (next)
        ash_lang_readnext();
    else
        o = 1;
    return o;
}

static inline int ash_lang_advance(char c)
{
    int o = (c == ash_lang_next());
    if (o)
        ash_lang_readnext();
    return o;
}

static inline void ash_lang_rm_quote(void)
{
    ash_lang.string++;
    ash_lang.len -= 2;
}

static inline const char *ash_lang_get(void)
{
    return ash_lang.string;
}

static inline int
ash_lang_num(char c)
{
    return (c >= '0' && c <= '9');
}

static void
ash_lang_scan_default(struct ash_tk_set *set, int type)
{
    if (type == WS_TK || type == NO_TK)
            return;
    else if (type == CM_TK)
        ash_lang_comment();
    else if (type ==  AV_TK || type == VAR_TK){
        ash_lang_match_var(set, type);
    }
    else if (type == DQT_TK){
        ash_lang_quote('\"');
        ash_tk_set_add(set, type, ash_lang_string(DQT_TK));
    } else
        ash_tk_set_add(set, type, NULL);
}

static void
ash_lang_scan_symbol(struct ash_tk_set *set, int c)
{
    do {

        if (c == ']'){
            ash_tk_set_add(set, RS_TK, NULL);
            return;
        }
        else if (c == '[')
            ash_tk_set_add(set, LS_TK, NULL);
        else if (c == '=')
            ash_tk_set_add(set, EQ_TK, NULL);
        else if (c == '!'){
            if (ash_lang_advance('='))
                ash_tk_set_add(set, NE_TK, NULL);
            else
                ash_tk_set_add(set, NT_TK, NULL);
        }
        else if (c == '<')
            ash_tk_set_add(set, LN_TK, NULL);
        else if (c == '>')
            ash_tk_set_add(set, GN_TK, NULL);
        else if (c == '+')
            ash_tk_set_add(set, PS_TK, NULL);
        else if (c == '-')
            ash_tk_set_add(set, SB_TK, NULL);
        else if (c == '*')
            ash_tk_set_add(set, ST_TK, NULL);
        else if (c == '/')
            ash_tk_set_add(set, DV_TK, NULL);
        else if (c == '%')
            ash_tk_set_add(set, MD_TK, NULL);
        else if (ash_lang_num(c)){
            while (ash_lang_num(ash_lang_next()))
                ash_lang_readnext();
            ash_tk_set_add(set, NUM_TK, ash_lang_string(NUM_TK));

        } else
            ash_lang_scan_default(set, ash_lang_type(c));

    ash_lang_reset();
    c = ash_lang_readnext();

    } while (ash_lang_next() && !ash_lang_ckerr());
}

static struct ash_tk_set
ash_lang_scan(const char *init)
{
    ash_lang.script = init;
    struct ash_tk_set set = { NULL, NULL };

    char c;

    while (ash_lang_hasnext() && !ash_lang_ckerr()){
        ash_lang_reset();
        c = ash_lang_readnext();

        int type = ash_lang_type(c);

        if (type == LS_TK)
            ash_lang_scan_symbol(&set, c);
        else
            ash_lang_scan_default(&set, type);
    }

    if (ash_lang_ckerr())
        ash_tk_set_free(&set);

    return set;
}

static struct ash_tk *tk;

static long eval_base_stack[DEFAULT_STACK_SIZE];

struct ash_eval_stack {
    long *stack;
    int size;
    int index;
} static ash_stack = {
    .stack = eval_base_stack,
    .index = 0,
    .size = DEFAULT_STACK_SIZE
};

static int ash_stack_assert(struct ash_eval_stack *stack)
{
    return stack->index > 0;
}

static void ash_stack_reset(struct ash_eval_stack *stack)
{
    stack->index = 0;
    memset(stack->stack, 0, sizeof *stack->stack * stack->size);
}

static void ash_stack_push(struct ash_eval_stack *stack, long val)
{
    if (stack->index < stack->size){
        *(++stack->stack) = val;
        stack->index++;
    } else {
        ash_runtime_exception(ASH_EXCEPTION_STACK_OVERFLOW);
        ash_stack_reset(stack);
    }
}

static long ash_stack_pop(struct ash_eval_stack *stack)
{
    if (stack->index > 0){
        stack->index--;
        return *stack->stack--;
    } else
        return 0;
}

static inline long ash_stack_peek(struct ash_eval_stack *stack)
{
    assert(stack->stack);
    return *stack->stack;
}

static struct ash_tk none = {
    .res = NO_TK,
    .str = NULL,
    .next = NULL
};

static void ash_lang_err(void)
{
    ash_lang.err = 1;
    tk = &none;
}

static void ash_lang_err_clear(void)
{
    ash_lang.err = 0;
}

static void ash_lang_err_msg(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    ash_print(PNAME ": error: ");
    ash_vprint(fmt, ap);
    va_end(ap);
    ash_lang_err();
}

static void ash_lang_err_expec(int type)
{
    if (type != NO_TK && type != EOS_TK){
        const char *msg = ash_reserved(type);
        ash_print(PNAME ": syntax: expected '%s'\n", msg);
    }
    ash_lang_err();
}

static void ash_lang_err_found(int type)
{
    if (type != NO_TK && type != EOS_TK){
        const char *msg = ash_reserved(type);
        ash_print(PNAME ": syntax: found unexpected '%s'\n", msg);
    }
    ash_lang_err();
}

static void ash_lang_err_expec_msg(const char *msg)
{
    ash_print(PNAME ": syntax: expected '%s'\n", msg);
    ash_lang_err();
}

static void ash_lang_err_found_msg(const char *msg)
{
    ash_print(PNAME ": syntax: found unexpected '%s'\n", msg);
    ash_lang_err();
}

static inline long
ash_stack_result(struct ash_eval_stack *stack);

static void ash_lang_print_result(void)
{
    if (!ash_lang_ckerr()){
        if (ash_stack_assert(&ash_stack))
            ash_print("%ld\n", ash_stack_result(&ash_stack));
        else
            ash_puts(ASH_LANG_NIL);
    }
}

static int ash_eval_expr(int);
static int ash_eval_expr_until(int);
static void ash_lang_eval_main(void);

static inline const char *
ash_lang_var(const char *key)
{
    return key && *key ? key + 1: NULL;
}

static inline const char *
ash_lang_var_get(const char *key)
{
    key = ash_lang_var(key);
    return ash_var_get_value(ash_var_env_get(key));
}

static inline long
ash_stack_result(struct ash_eval_stack *stack)
{
    if (ash_stack_assert(stack)){
        struct ash_tk *tk = (struct ash_tk *) ash_stack_pop(stack);
        ash_stack_reset(stack);
        return tk->num;
    }
    return 0;
}

static int tk_size = 0;
static struct ash_tk tk_res[DEFAULT_STACK_SIZE];

static inline struct ash_tk*
ash_res_get(void)
{
    if (tk_size == DEFAULT_STACK_SIZE)
        tk_size = 0;
    return &tk_res[tk_size++];
}

static struct ash_tk *
ash_num(int num)
{
    struct ash_tk *tk_res = ash_res_get();
    tk_res->res = NUM_TK;
    tk_res->num = num;
    tk_res->str = NULL;
    return tk_res;
}

static struct ash_tk *
ash_bool(int bool)
{
    struct ash_tk *tk_res = ash_res_get();
    tk_res->res = bool ? TR_TK: FS_TK;
    tk_res->num = bool;
    tk_res->str = NULL;
    return tk_res;
}

static int
ash_eval_unary_expr(int op)
{
    struct ash_tk *a_tk = (struct ash_tk *) ash_stack_pop(&ash_stack);
    int a_type = ash_tk_get(&a_tk);
    long res;

    if (a_type == NUM_TK || a_type == TR_TK || a_type == FS_TK){
        long a = a_tk->str? atol(a_tk->str): a_tk->num;

        if (op == NT_TK)
            res = a? 0: 1;
        else
            return -1;
    } else {
        const char *sa = ash_tk_str_take(&a_tk);
        if (a_type == AV_TK)
            sa = ash_lang_var_get(sa);

        if (op == NT_TK)
            res = sa == NULL ? 1: 0;
        else
            return -1;
    }

    ash_stack_push(&ash_stack, (long) ash_num(res));
    return 0;
}

static inline int
ash_lang_bool(int res)
{
    return (res == TR_TK || res == FS_TK);
}

static int
ash_eval_binary_num(int op, long a, long b)
{
    long res;

    switch (op){
        default:    return -1;

        case PS_TK:
            res = a + b;
            break;

        case SB_TK:
            res = a - b;
            break;

        case ST_TK:
            res = a * b;
            break;

        case DV_TK:
            if (b == 0){
                ash_runtime_exception(ASH_EXCEPTION_ZERO_DIV);
                return -1;
            }
            res = a / b;
            break;

        case MD_TK:
            res = a % b;
            break;

        case EQ_TK:
            res = a == b;
            break;

        case NE_TK:
            res = a != b;
            break;

        case LN_TK:
            res = a < b;
            break;

        case GN_TK:
            res = a > b;
            break;
    }
    ash_stack_push(&ash_stack, (long) ash_num(res));

    return 0;
}

static int
ash_eval_binary_expr(int op)
{
    struct ash_tk *b_tk = (struct ash_tk *) ash_stack_pop(&ash_stack);
    struct ash_tk *a_tk = (struct ash_tk *) ash_stack_pop(&ash_stack);
    assert(b_tk && a_tk);
    long res;

    int a_type = ash_tk_get(&a_tk);
    int b_type = ash_tk_get(&b_tk);

    if (a_type == NUM_TK && b_type == NUM_TK)
        return ash_eval_binary_num(op, a_tk->num, b_tk->num);

    if (a_type == NUM_TK || b_type == NUM_TK){
        ash_lang_err_msg("cannot evaluate expression of type '%s' and '%s'\n",
        ash_reserved(a_type), ash_reserved(b_type));
        return -1;
    }

    const char *sb = ash_tk_str_take(&b_tk);
    const char *sa = ash_tk_str_take(&a_tk);

    if (b_type == AV_TK)
        sb = ash_lang_var_get(sb);
    if (a_type == AV_TK)
        sa = ash_lang_var_get(sa);

    if (op == EQ_TK){
        if (!sa || !sb)
            res = sa == sb ? 1: 0;
        else
            res = (strcmp(sa, sb) == 0);
    }
    else if (op == NE_TK){
        if (!sa || !sb)
            res = sa != sb ? 1: 0;
        else
            res = (strcmp(sa, sb) != 0);
    }
    else {
        ash_lang_err_found(op);
        return -1;
    }

    ash_stack_push(&ash_stack, (long) ash_num(res));
    return 0;
}

static int
ash_eval_logical_expr(int op)
{
    struct ash_tk *b_tk = (struct ash_tk *) ash_stack_pop(&ash_stack);
    struct ash_tk *a_tk = (struct ash_tk *) ash_stack_pop(&ash_stack);
    assert(b_tk && a_tk);
    long res;

    long b = b_tk->num;
    long a = a_tk->num;

    if (op == AN_TK)
        res = a && b;
    else if (op == OR_TK)
        res = a || b;

    ash_stack_push(&ash_stack, (long) ash_num(res));
    return 0;
}

static const char *ash_lang_get_value(struct ash_tk **);

static void
ash_eval_lang_value(void)
{
    struct ash_tk *a_tk = (struct ash_tk *) ash_stack_pop(&ash_stack);
    int a_type = ash_tk_get(&a_tk);
    long res;

    if (a_type == AV_TK)
        res = ash_var_check_nil(ash_var_env_get(ash_lang_var(tk->str))) == 0;
    else
        res = tk->str == NULL ? 0: 1;
    ash_stack_push(&ash_stack, (long) ash_num(res));
}

static int
ash_lang_value(void)
{
    int res = ash_tk_get(&tk);

    if(res == NO_TK){
        ash_lang_err_expec_msg("<expression>");
        return -1;
    } else if (res == NUM_TK)
        ash_stack_push(&ash_stack, (long) ash_num(atol(tk->str)));
    else if (res == AV_TK){
        ash_stack_push(&ash_stack, (long) tk);
        //ash_lang_get_value(&tk);
    }
    else if (res == DQT_TK)
        ash_stack_push(&ash_stack, (long) tk);
    else if (res == LP_TK)
        ash_eval_expr_until(RP_TK);
    else {
        ash_lang_err_found(res);
        return -1;
    }

    return 0;
}

static inline int
ash_lang_bin_ops(int res)
{
    return (res == PS_TK || res == SB_TK ||
            res == ST_TK || res == DV_TK ||
            res == MD_TK);
}

static inline int
ash_lang_comp_ops(int res)
{
    return (res == EQ_TK || res == NE_TK ||
            res == LN_TK || res == GN_TK);
}

static int
ash_ops_order(int op)
{
    if (op == LP_TK)
        return 0;
    else if (op == MD_TK)
        return 1;
    else if (op == DV_TK)
        return 2;
    else if (op == ST_TK)
        return 3;
    else if (op == PS_TK)
        return 4;
    else if (op == SB_TK)
        return 5;
    else
        return -1;
}

static void
ash_lang_ops_match(int op1, int op2)
{
    int ord1 = ash_ops_order(op1);
    int ord2 = ash_ops_order(op2);

    if (ord1 < 0 || ord2 < 0)
        return;

    if (ord1 > ord2){
        ash_tk_next(&tk);
        ash_eval_expr(op2);
    }
}

static int
ash_eval_expr(int expr)
{
    int res = ash_tk_get(&tk);

    if (ash_lang_bin_ops(res)){
        if (!ash_stack_assert(&ash_stack)){
            ash_lang_err_found(res);
            return -1;
        }
        ash_tk_next(&tk);
        if (ash_lang_value())
            return -1;

        int next = ash_tk_cknext(&tk);
        ash_lang_ops_match(res, next);
        ash_eval_binary_expr(res);
        next = ash_tk_cknext(&tk);

        if (ash_lang_bin_ops(next)){
            ash_tk_next(&tk);
            return ash_eval_expr(next);
        }
    }
    else {
        if (!expr && ash_stack_assert(&ash_stack)){
            ash_lang_err_found(res);
            return -1;
        }

        if (ash_lang_value())
            return -1;

        int next = ash_tk_cknext(&tk);
        if (ash_lang_bin_ops(next)){
            ash_tk_next(&tk);
            return ash_eval_expr(next);
        }
        else if (res != NUM_TK && !ash_lang_comp_ops(next) && !expr)
            ash_eval_lang_value();
    }
    return 0;
}

static int
ash_eval_expr_until(int o)
{
    if (ash_lang_ckerr())
        return -1;

    while (ash_tk_valid(&tk) && ash_tk_getnext(&tk) != o)
        ash_eval_expr(NO_TK);

    return ash_lang_ckerr();
}

static void
ash_eval_bool_expr_until(int);

static inline int
ash_lang_bool_value(int expr)
{
    int res = ash_tk_get(&tk);
    if (res == TR_TK)
        ash_stack_push(&ash_stack, (long) ash_bool(1));
    else if (res == FS_TK)
        ash_stack_push(&ash_stack, (long) ash_bool(0));
    else
        return ash_eval_expr(expr);

    return 0;
}

static int
ash_eval_bool_expr(void)
{
    int res = ash_tk_get(&tk);

    if (res == NT_TK){
        ash_tk_next(&tk);
        if (ash_lang_bool_value(NO_TK))
            return -1;
        ash_eval_unary_expr(res);
    }
    else if (ash_lang_comp_ops(res)){
        if (!ash_stack_assert(&ash_stack)){
            ash_lang_err_found(res);
            return -1;
        }
        ash_tk_next(&tk);
        if (ash_lang_bool_value(res))
            return -1;
        ash_eval_binary_expr(res);

        int next = ash_tk_cknext(&tk);
        if (ash_lang_comp_ops(next)){
            ash_lang_err_found(next);
            return -1;
        }
    }
    else if (res == LP_TK)
        ash_eval_bool_expr_until(RP_TK);
    else {
        ash_lang_bool_value(NO_TK);

        int next = ash_tk_cknext(&tk);
        if (ash_lang_comp_ops(next)){
            ash_tk_next(&tk);
            return ash_eval_bool_expr();
        }
    }
    return 0;
}

static int
ash_eval_logical(void)
{
    int res = ash_tk_get(&tk);

    if (res == AN_TK || res == OR_TK){
        if (!ash_stack_assert(&ash_stack)){
            ash_lang_err_found(res);
            return -1;
        }
        ash_tk_next(&tk);
        if (ash_eval_bool_expr())
            return -1;
        ash_eval_logical_expr(res);
    }
    else if (res == LP_TK)
        ash_eval_bool_expr_until(RP_TK);
    else
        return ash_eval_bool_expr();
    return 0;
}

static void
ash_eval_bool_expr_until(int o)
{
    if (ash_lang_ckerr())
        return;

    while (ash_tk_valid(&tk) && ash_tk_getnext(&tk) != o)
        ash_eval_bool_expr();
}

static void
ash_eval_logical_until(int o)
{
    if (ash_lang_ckerr())
        return;

    while (ash_tk_valid(&tk) && ash_tk_getnext(&tk) != o)
        ash_eval_logical();
}

static struct ash_eval_stack ash_block_stack = {
    .stack = NULL,
    .index = 0
};

/* block state */
static int b_state;
/* loop block */
static int b_iter;

static void ash_block_push(struct ash_tk *block)
{
    assert(block);
    int type = block->res;

    if (type == IF_TK ||
        type == EL_TK ||
        type == DO_TK ||
        type == FOR_TK){

        ash_stack_push(&ash_block_stack, (long)block);
        if (type == DO_TK ||
            type == FOR_TK)
            b_iter = 1;
    }
}

static struct ash_tk *ash_block_pop(void)
{
    if (!ash_stack_assert(&ash_block_stack))
        return NULL;

    long val = ash_stack_pop(&ash_block_stack);
    struct ash_tk *block = val != 0 ? (struct ash_tk *) val: NULL;
    if (block){
        int type = block->res;
        if (type == DO_TK || type == FOR_TK)
            b_iter = 0;
    }
    return block;
}

static int ash_block_peek(void)
{
    if (ash_stack_assert(&ash_block_stack)){
        struct ash_tk *tk = (struct ash_tk *) ash_stack_peek(&ash_block_stack);
        return ash_tk_get(&tk);
    }
    return NO_TK;
}

static int ash_eval_block(void)
{
    if (ash_lang_ckerr())
        return NO_TK;

    while (ash_tk_valid(&tk) && ash_tk_getnext(&tk) != END_TK){
        ash_lang_eval_main();
        int next = ash_tk_get(&tk);
        if (next == END_TK)
            break;
        else if (next == RET_TK)
            return RET_TK;
    }

    if (b_state){
        int state = b_state;
        b_state = NO_TK;
        return state;
    }

    ash_tk_assert_type(&tk, END_TK);
    return ash_tk_get(&tk);
}

static int ash_eval_block_end(int b_start, int b_end)
{
    int level = 0;
    int type;

    while ((type = ash_tk_getnext(&tk))){

        if (type == IF_TK ||
            type == DO_TK ||
            type == FOR_TK){
            ++level;

        } else if (type == EL_TK &&
                   b_start == IF_TK &&
                   b_end == EL_TK){
            if (level == 0)
                return EL_TK;

        } else if (type == VAR_TK){
            if (ash_tk_cknext(&tk) == CN_TK)
                ++level;

        } else if (type == END_TK){
            if (level == 0)
                return END_TK;
            else
                --level;
        } else if (type == NO_TK)
            break;
    }
    return NO_TK;
}

static int ash_eval_expr_block(void)
{
    ash_tk_assert_type(&tk, LS_TK);
    ash_eval_logical_until(RS_TK);
    ash_tk_assert_type(&tk, RS_TK);
    return ash_lang_ckerr();
}

static void ash_eval_condition(void)
{
    ash_tk_assert_type(&tk, IF_TK);
    ash_block_push(tk);
    ash_tk_next(&tk);
    if (ash_eval_expr_block()){
        ash_stack_reset(&ash_stack);
        return;
    }

    ash_lang_assert_prompt();

    int exit;

    if (ash_stack_result(&ash_stack))
        exit = ash_eval_block();
    else if (ash_eval_block_end(IF_TK, EL_TK) == EL_TK)
        ash_eval_block();

    if (exit == END_TK)
        ash_block_pop();
}

static void ash_eval_else(void)
{
    if (ash_block_peek() == IF_TK){
        b_state = EL_TK;
        tk = ash_block_pop();
        assert(ash_tk_get(&tk) == IF_TK);
        ash_eval_block_end(IF_TK, END_TK);
        assert(ash_tk_get(&tk) == END_TK);
    } else
        ash_lang_err_found(EL_TK);
}

static void ash_eval_iterate(void)
{
    ash_tk_assert_type(&tk, DO_TK);
    ash_block_push(tk);
    int cond, exit;
    struct ash_tk *iter = tk;

    do {
        ash_tk_next(&tk);
        if (ash_eval_expr_block()){
            ash_stack_reset(&ash_stack);
            return;
        }

        ash_lang_assert_prompt();

        if ((cond = ash_stack_result(&ash_stack))){
            exit = ash_eval_block();
            tk = iter;

            if (exit == BK_TK)
                break;
            else if (exit == RET_TK)
                return;

        } else
            break;

    } while (cond);

    if (exit == END_TK)
        ash_block_pop();
    ash_eval_block_end(DO_TK, END_TK);
}

static int ash_range_value(void)
{
    char res = ash_tk_getnext(&tk);
    if (res == NO_TK)
        ash_lang_err_expec_msg("<range>");
    else if (res == NUM_TK)
        ash_stack_push(&ash_stack, (long) tk);

    return res;
}

static int ash_eval_expr_range(int op)
{
    struct ash_tk *b_tk = (struct ash_tk *) ash_stack_pop(&ash_stack);
    struct ash_tk *a_tk = (struct ash_tk *) ash_stack_pop(&ash_stack);
    assert(b_tk && a_tk);
    long b;
    long a;

    int a_type = ash_tk_get(&a_tk);
    int b_type = ash_tk_get(&b_tk);

    if (a_type != NUM_TK || b_type != NUM_TK){
        ash_lang_err_expec(NUM_TK);
        return -1;
    }

    b = atol(b_tk->str);
    a = atol(a_tk->str);

    if (op == UT_TK){
        if (b > a)
            b -= 1;
        else if (a > b)
            b += 1;
    }

    ash_stack_push(&ash_stack, b);
    ash_stack_push(&ash_stack, a);
    return 0;
}

static int ash_eval_range(void)
{
    int res = ash_range_value();
    if (res == NUM_TK){
        res = ash_tk_getnext(&tk);
        if (res != TO_TK && res != UT_TK){
            ash_lang_err_msg("expected '%s' or '%s'\n",
            ash_reserved(TO_TK), ash_reserved(UT_TK));
            return -1;
        }
        if (ash_range_value() != NUM_TK){
            ash_lang_err_expec(NUM_TK);
            return -1;
        }
        ash_eval_expr_range(res);
    } else {
        ash_lang_err_found(res);
        return -1;
    }

    return 0;
}

static int ash_eval_range_block(void)
{
    ash_tk_assert_type(&tk, LS_TK);
    if (ash_tk_valid(&tk)){
        if (ash_eval_range())
            return -1;
        ash_tk_next(&tk);
        ash_tk_assert_type(&tk, RS_TK);
    }
    return 0;
}

static void ash_eval_iterate_for(void)
{
    ash_tk_assert_type(&tk, FOR_TK);
    ash_block_push(tk);
    ash_tk_next(&tk);
    if(ash_tk_assert_type(&tk, VAR_TK))
        return;
    const char *var = tk->str;
    ash_tk_next(&tk);
    if (ash_tk_assert_type(&tk, IN_TK))
        return;
    ash_tk_next(&tk);
    if (ash_eval_range_block()){
        ash_stack_reset(&ash_stack);
        return;
    }

    ash_lang_assert_prompt();

    struct ash_tk *iter = tk;
    long end = ash_stack_pop(&ash_stack);
    long start = ash_stack_pop(&ash_stack);

    int exit;

    if (start > end){
        int a = start;
        start = end;
        end = a;
    }

    ash_var_env_new(0, NULL);
    char index[ASH_FMT_SIZE];

    do {
        sprintf(index, "%ld", start);
        ash_var_env_set(var, index, ASH_STATIC);

        exit = ash_eval_block();
        tk = iter;
        if (exit == BK_TK)
            break;
        else if (exit == RET_TK)
            return;
        ++start;
    } while (start <= end);

    ash_var_env_destroy();

    if (exit == END_TK)
        tk = ash_block_pop();
    ash_eval_block_end(FOR_TK, END_TK);
}

static void ash_eval_break(void)
{
    if (b_iter){
        struct ash_tk *b_tk;
        while ((b_tk = ash_block_pop())){
            int block = ash_tk_get(&b_tk);
            if (block == NO_TK)
                break;

            if (block == DO_TK || block == FOR_TK){
                b_state = BK_TK;
                tk = b_tk;
                assert(ash_tk_get(&tk) == block);
                ash_eval_block_end(block, END_TK);
                break;
            }
        }
        assert(b_iter == 0);
    } else
        ash_lang_err_found(BK_TK);
}

static const char *ash_lang_get_value(struct ash_tk **start)
{
    ash_tk_assert_type(start, AV_TK);
    const char *var = ash_lang_var((*start)->str);

    if (ash_tk_cknext(start) != LS_TK)
        return ash_var_get_value(ash_var_env_get(var));

    else if (ash_tk_cknext(start) == LS_TK) {
        ash_tk_next(start);
        tk = *start;
        assert( ash_tk_get(start) == LS_TK && ash_tk_get(&tk) == LS_TK );
        if (ash_eval_expr_until(RS_TK))
            return NULL;
        long index = ash_stack_result(&ash_stack);
        return ash_var_array_value(ash_var_env_get(var), (int)index);
    }
    return NULL;
}

static inline int
ash_lang_str(int type)
{
    return (type == VAR_TK || type == DQT_TK);
}

static int ash_lang_stm_len(void)
{
    int len;
    int type = ash_tk_get(&tk);

    for (len = 0; type; len++){
        if (type == EOS_TK)
            break;
        else if (type == AV_TK){
            const char *var = ash_lang_get_value(&tk);
            if (!var || !*var){
                len--;
                if (len <= 0)
                    return len;
            }
        }
        else if (!ash_lang_str(type)){
            ash_lang_err_found(type);
            return 0;
        }

        type = ash_tk_getnext(&tk);
    }
    return len;
}

static void ash_eval_stm(void)
{
    int type = ash_tk_get(&tk);
    struct ash_tk *stm = tk;
    int argc = ash_lang_stm_len();
    if (argc <= 0)
        return;
    tk = stm;

    const char *argv[argc + 1];
    argv[argc] = NULL;

    for (int i = 0; i < argc; ++i){
        type = ash_tk_get(&tk);
        if (type == AV_TK){
            const char *var;
            if ((var = ash_lang_get_value(&tk)))
                argv[i] = var;
        }
        else if (ash_lang_str(type))
            argv[i] = tk->str;
        ash_tk_next(&tk);
    }

    ash_exec_command(argc, (const char **)argv);
}

static int ash_eval_list(void)
{
    ash_tk_assert_type(&tk, LP_TK);
    int len = 0;

    while (ash_tk_next(&tk)){
        int type = ash_tk_get(&tk);

        if (type == RP_TK)
            return len;
        else if (ash_lang_str(type)){
            ++len;
            if (ash_tk_cknext(&tk) != RP_TK){
                ash_tk_next(&tk);
                if (ash_tk_assert_type(&tk, CO_TK))
                    return -1;
                else if (ash_tk_cknext(&tk) == RP_TK) {
                    ash_lang_err_expec(RP_TK);
                    return -1;
                }
            }
        }
        else {
            ash_lang_err_found(type);
            return -1;
        }
    }
}

static void ash_eval_value(void)
{
    ash_tk_assert_type(&tk, VAR_TK);

    const char *var = tk->str;
    int var_type = *var == '*'? ASH_RODATA: ASH_DATA;
    if (var_type == ASH_RODATA)
        var++;

    ash_tk_next(&tk);
    ash_tk_assert_type(&tk, AS_TK);

    int next = ash_tk_getnext(&tk);

    if (ash_lang_str(next))
        ash_var_set(var, ash_tk_str_take(&tk), var_type);

    else if (next == LS_TK){
        if (ash_eval_expr_until(RS_TK))
            return;
        char fmt[ASH_FMT_SIZE];
        sprintf(fmt, "%ld", ash_stack_result(&ash_stack));
        char *value = ash_alloc(strlen(fmt) + 1);
        strcpy(value, fmt);
        ash_var_set(var, value, var_type);
    }

    else if (next == LP_TK){
        struct ash_tk *stm = tk;
        int len = ash_eval_list();

        if (len <= 0 || ash_tk_assert_type(&tk, RP_TK))
            return;

        int i = 0;
        const char *array[len];
        while (stm != tk){
            int type = ash_tk_get(&stm);
            if (ash_lang_str(type))
                array[i++] = ash_tk_str_take(&stm);
            ash_tk_next(&stm);
        }
        ash_var_set_array(var, len, array, var_type);
    }
}

static void ash_eval_array(void)
{
    struct ash_tk *start = tk;
    const char *var = ash_lang_var(tk->str);
    ash_tk_next(&tk);
    ash_eval_expr_until(RS_TK);
    long index = ash_stack_result(&ash_stack);

    if (ash_tk_cknext(&tk) == AS_TK){
        ash_tk_next(&tk);
        int type = ash_tk_getnext(&tk);
        if (ash_lang_str(type)){
            const char *value = ash_tk_str_take(&tk);
            ash_var_insert_array(ash_var_get(var), (int)index, value);
        }
    } else {
        tk = start;
        ash_eval_stm();
    }
}

static struct ash_eval_stack ash_func_stack = {
    .stack = NULL,
    .index = 0
};

static struct ash_tk *ash_lang_get_func(const char *name)
{
    if (*name == '.' && ash_stack_assert(&ash_func_stack)){
        const char *fname = (const char *) ash_stack_peek(&ash_func_stack);
        if (fname && strcmp(fname, name) != 0){
            size_t pos = strlen(fname);
            size_t len = pos + strlen(name) + 1;
            char fns[len];
            strcpy(fns, fname);
            strcpy(&fns[pos], name);
            return ash_func_get(fns);
        }
    }
    return ash_func_get(name);
}

static void ash_eval_call_ret(void)
{
    ash_stack_pop(&ash_func_stack);
    /* return to save point */
    tk = (struct ash_tk *) ash_stack_pop( &ash_func_stack );
    /*assert(tk);*/
    ash_var_env_destroy();
}

/* ash function call */
static void ash_eval_call(void)
{
    /* get the name of the function */
    const char *name = tk->str;
    struct ash_tk *func = ash_lang_get_func(name);
    if (!func){
        ash_lang_err_msg("undefined symbol '%s'\n", name);
        return;
    }

    struct ash_tk *start = ash_tk_next(&tk);
    ash_tk_assert_type(&tk, LP_TK);
    /* number of function args */
    int argc = ash_eval_list();

    ash_tk_assert_type(&tk, RP_TK);

    const char *argv[argc + 1];
    for (int i = 0; i < argc; ++i){
        if (ash_lang_str(ash_tk_get(&start)))
            argv[i] = ash_tk_str_take(&start);
        else
            --i;
        ash_tk_next(&start);
    }

    /* set up a new env */
    ash_var_env_new(argc, argv);

    /* save return point */
    ash_stack_push(&ash_func_stack, (long) tk);
    ash_stack_push(&ash_func_stack, (long) name);

    /* begin function call */
    tk = func;
    assert(ash_tk_get(&tk) != NO_TK);
    do {
        ash_lang_eval_main();
        if (ash_tk_get(&tk) == RET_TK)
            break;
    } while (ash_tk_valid(&tk) && ash_tk_getnext(&tk) != END_TK);

    ash_eval_call_ret();
}

static void ash_eval_function(void)
{
    struct ash_tk *start = tk;
    const char *name = ash_tk_str_take(&tk);
    ash_tk_next(&tk);
    ash_tk_assert_type(&tk, CN_TK);

    ash_lang_assert_prompt();

    struct ash_tk *func = tk;

    ash_eval_block_end(VAR_TK, END_TK);
    ash_tk_assert_type(&tk, END_TK);
    if (ash_tk_cknext(&tk) != NO_TK){
        start->next = tk->next;
        tk->next = NULL;
        tk = start;
    }
    ash_func_set(name, (void *)func);
}

static void ash_eval_return(void)
{
    if (!ash_stack_assert(&ash_func_stack))
        ash_lang_err_found(RET_TK);
}

static void ash_lang_eval_main(void)
{
    if (ash_lang_ckerr())
        return;

    int type = ash_tk_get(&tk);
    int next = ash_tk_cknext(&tk);

    switch (type){
        case NO_TK:
        case EOS_TK:
            break;

        case RET_TK:
            ash_eval_return();
            break;

        case AV_TK:
            if (next == LS_TK)
                ash_eval_array();
            else
                ash_eval_stm();
            break;

        case IF_TK:
            ash_eval_condition();
            break;

        case DO_TK:
            ash_eval_iterate();
            break;

        case FOR_TK:
            ash_eval_iterate_for();
            break;

        case LS_TK:
            ash_eval_expr_block();
            ash_lang_print_result();
            break;

        case EL_TK:
            ash_eval_else();
            break;

        case BK_TK:
            ash_eval_break();
            break;

        case VAR_TK:
            if (next == AS_TK)
                ash_eval_value();
            else if (next == CN_TK)
                ash_eval_function();
            else if (next == LP_TK)
                ash_eval_call();
            else
                ash_eval_stm();
            break;

        case DQT_TK:
            ash_eval_stm();
            break;

        default:
            ash_lang_err_found(ash_tk_get(&tk));
    }
}

static struct ash_tk_set set;

static void ash_lang_prompt(void)
{
    int level = 1;
    struct ash_tk_set s;

    do {
        ash_prompt_next();
        s = ash_lang_scan(ash_scan());

        struct ash_tk *next = s.front;
        if (next){
            set.rear->next = next;
            set.rear = s.rear;
        }

        for (; next != NULL; next = next->next){
            if (next->res == IF_TK ||
                next->res == DO_TK ||
                next->res == FOR_TK)
                ++level;

            else if (next->res == VAR_TK &&
                     ash_tk_cknext(&next) == CN_TK)
                ++level;

            if (next->res == END_TK)
                --level;
        }
    } while (level != 0);
}

static int ash_lang_assert_prompt(void)
{
    if (ash_tk_cknext(&tk) == EOS_TK){
        ash_tk_getnext(&tk);
        if (ash_tk_cknext(&tk) == NO_TK){
            ash_lang_prompt();
            return 0;
        }
    }
    return -1;
}

/*
   evaluate input string and return exit status
   -1 : empty string
    0 : exit success
    1 : exit failure
*/
int ash_lang_eval(const char *s)
{
    set = ash_lang_scan(s);
    tk = set.front;
    if (!tk)
        return -1;

    do
        ash_lang_eval_main();
    while (ash_tk_next(&tk));

    /* todo: clean up */

    int status = ash_lang_ckerr();
    ash_lang_err_clear();
    return status;
}

static void ash_stack_init(struct ash_eval_stack *stack)
{
    stack->size = DEFAULT_STACK_SIZE;
    size_t size = DEFAULT_STACK_SIZE * sizeof ( *stack->stack );
    stack->stack = ash_alloc(size);
    memset(stack->stack, 0, size);
}

void ash_lang_init(void)
{
    ash_stack_init(&ash_block_stack);
    ash_stack_init(&ash_func_stack);
}

static const char *ash_reserved_tk[] = {
    [ AN_TK  ]   = "and",
    [ BK_TK  ]   = "break",
    [ EL_TK  ]   = "else",
    [ END_TK ]   = "end",
    [ FS_TK  ]   = "false",
    [ FOR_TK ]   = "for",
    [ IF_TK  ]   = "if",
    [ IN_TK  ]   = "in",
    [ OR_TK  ]   = "or",
    [ RET_TK ]   = "return",
    [ TO_TK  ]   = "to",
    [ TR_TK  ]   = "true",
    [ UT_TK  ]   = "until",
    [ DO_TK  ]   = "while",
    [ LS_TK  ]   = "[",
    [ RS_TK  ]   = "]",
    [ LP_TK  ]   = "(",
    [ RP_TK  ]   = ")",
    [ CO_TK  ]   = ",",
    [ CN_TK  ]   = ":",
    [ AS_TK  ]   = ":=",
    [ DQT_TK ]   = "<str>",
    [ PS_TK  ]   = "+",
    [ SB_TK  ]   = "-",
    [ ST_TK  ]   = "*",
    [ DV_TK  ]   = "/",
    [ MD_TK  ]   = "%",
    [ NT_TK  ]   = "!",
    [ EQ_TK  ]   = "=",
    [ NE_TK  ]   = "!=",
    [ LN_TK  ]   = "<",
    [ GN_TK  ]   = ">",
    [ PIP_TK ]   = "|",
    [ VAR_TK ]   = "<id>",
    [ NUM_TK ]   = "<num>"
};

static const char *ash_reserved(int res)
{
    assert(res < sizeof(ash_reserved_tk)/sizeof(ash_reserved_tk[0]));

    if (res == NO_TK)
        return NULL;

    const char *s = ash_reserved_tk[res];
    if (!s)
        s = "token";
    return s;
}
