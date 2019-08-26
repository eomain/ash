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
#include <string.h>

#include "ash/mem.h"
#include "ash/type.h"
#include "ash/lang/lang.h"
#include "ash/lang/lex.h"

static inline bool lex_is_ws(char c)
{
    return (c == ' '  ||
            c == '\0' ||
            c == '\t' ||
            c == '\v' ||
            c == '\r' ||
            c == '\f');
}

static inline bool lex_is_numeric(char c)
{
    return (c >= '0' && c <= '9');
}

static enum ash_tk_type lex_token_type(char c)
{
    if (lex_is_ws(c))
        return WS_TK;

    switch (c) {
        case '\n':  return EOS_TK;
        case ';':   return SEM_TK;

        case '$':   return AV_TK;
        case '#':   return CM_TK;
        case '`':   return BQ_TK;
        case '\'':  return SQT_TK;
        case '\"':  return DQT_TK;

        case '\\':  return BS_TK;
        case '|':   return PIP_TK;
        case '?':   return QMK_TK;

        case ':':   return CN_TK;
        case '[':   return LS_TK;
        case ']':   return RS_TK;
        case '(':   return LP_TK;
        case ')':   return RP_TK;

        case '<':   return LN_TK;
        case '>':   return GN_TK;
        case '=':   return EQ_TK;

        case ',':   return CO_TK;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':   return NUM_TK;

        default:    return VAR_TK;
    }

    return NO_TK;
}

static enum ash_tk_type lex_token_key_type(const char *s)
{
    switch (s[0]) {
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

        case 'd':
            if (s[1] == 'e' &&
                s[2] == 'f' &&
                !s[3])
                return DEF_TK;
            break;

        case 'e':
            if (s[1] == 'l') {
                if (s[2] == 's' &&
                    s[3] == 'e' &&
                    !s[4])
                    return EL_TK;
                else if (s[2] == 'i' &&
                         s[3] == 'f' &&
                         !s[4])
                    return ELF_TK;
            }

            else if (s[1] == 'n' &&
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

        case 'l':
            if (s[1] == 'e' &&
                s[2] == 't' &&
                !s[3])
                return LET_TK;
            break;
        case 'm':
            if (s[1] == 'a' &&
                s[2] == 't' &&
                s[3] == 'c' &&
                s[4] == 'h' &&
                !s[5])
                return MAT_TK;
            else if (s[1] == 'o' &&
                    s[2] == 'd' &&
                    !s[3])
                return MOD_TK;
            break;

        case 'n':
            if (s[1] == 'e' &&
                s[2] == 'x' &&
                s[3] == 't' &&
                !s[4])
                return NXT_TK;
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
            else if (s[1] == 's' &&
                     s[2] == 'e' &&
                     !s[3])
                return USE_TK;
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

struct lexer {
    size_t len;
    bool err;
    size_t line;
    size_t offset;
    const char *string;
    const char *cursor;
    const char *input;
    struct ash_tk_set *set;
};

static void lexer_init(struct lexer *lexer,
                       const char *input, struct ash_tk_set *set)
{
    lexer->len = 0;
    lexer->err = false;
    lexer->line = 1;
    lexer->offset = 1;
    lexer->string = NULL;
    lexer->cursor = input;
    lexer->input = input;
    lexer->set = set;
}

static void lexer_reset(struct lexer *lexer)
{
    lexer->len = 0;
    lexer->string = lexer->cursor;
}

static inline struct ash_tk_set *lexer_token_set(struct lexer *lexer)
{
    return lexer->set;
}

static inline bool lexer_get_error(struct lexer *lexer)
{
    return lexer->err;
}

static inline bool lexer_hasnext(struct lexer *lexer)
{
    return ((*lexer->cursor) != '\0') ? true: false;
}

static char lexer_readnext(struct lexer *lexer)
{
    char next;

    next = *(lexer->cursor++);
    if (next)
        lexer->len++;

    if (next == '\n') {
        lexer->line++;
        lexer->offset = 1;
    } else {
        lexer->offset++;
    }

    return next;
}

static inline char lexer_read(struct lexer *lexer)
{
    return (*lexer->cursor);
}

static inline bool lexer_assert_next(struct lexer *lexer, char c)
{
    if ((c == lexer_read(lexer))) {
        lexer_readnext(lexer);
        return true;
    }
    return false;
}

static inline void lexer_token_add_string(struct lexer *lexer, enum ash_tk_type type,
                                   const char *string)
{
    size_t offset;
    struct ash_tk_set *set;
    struct ash_tk_meta meta;

    offset = (lexer->offset - lexer->len);
    set = lexer_token_set(lexer);
    ash_tk_meta_init(&meta, lexer->line, offset);

    ash_tk_set_add(set, type, string, &meta);
}

static inline void lexer_token_add(struct lexer *lexer, enum ash_tk_type type)
{
    lexer_token_add_string(lexer, type, NULL);
}

static inline bool lexer_find_char(struct lexer *lexer, char c)
{
    do {
        if ((c == lexer_read(lexer)))
            return true;
    } while (lexer_readnext(lexer));

    return false;
}

static const char *lexer_get_string(struct lexer *lexer)
{
    char *s;
    size_t len;

    len = lexer->len;
    assert((len + 1) > 0);
    s = ash_zalloc((len + 1));
    memcpy(s, lexer->string, len);
    return s;
}

static const char *lexer_get_qstring(struct lexer *lexer)
{
    if (!lexer_find_char(lexer, '"')) {
        /* TODO: error */
        return NULL;
    }
    lexer_readnext(lexer);

    lexer->string++;
    lexer->len -= 2;
    return lexer_get_string(lexer);
}

static inline const char *lexer_get_cursor(struct lexer *lexer)
{
    return lexer->cursor;
}

static void lexer_skip_comment(struct lexer *lexer)
{
    lexer_find_char(lexer, '\n');
}

static inline void lexer_end_of_statement(struct lexer *lexer)
{
    ash_tk_set_eos(lexer_token_set(lexer));
}

static inline bool lex_is_expr_type(enum ash_tk_type type)
{
    return (type == LS_TK || type == BQ_TK);
}

static void lex_symbol_var(struct lexer *lexer, enum ash_tk_type type)
{
    enum ash_tk_type next;
    const char *string;

    for (;;) {
        next = lex_token_type(lexer_read(lexer));
        if (!(next == VAR_TK || next == NUM_TK || next == EQ_TK))
            break;
        lexer_readnext(lexer);
    }

    if (type == AV_TK)
        return lexer_token_add_string(lexer, AV_TK, lexer_get_string(lexer));

    string = lexer_get_string(lexer);
    type = lex_token_key_type(string);

    if (type == NO_TK)
        return lexer_token_add_string(lexer, VAR_TK, string);
    ash_free((char *) string);
    lexer_token_add(lexer, type);
}

static void lex_symbol_default(struct lexer *lexer, enum ash_tk_type type)
{
    if (type == WS_TK || type == NO_TK)
            return;
    else if (type == CM_TK)
        lexer_skip_comment(lexer);
    else if (type == AV_TK || type == VAR_TK)
        lex_symbol_var(lexer, type);
    else if (type == EQ_TK) {
        if (lexer_assert_next(lexer, '>'))
            lexer_token_add(lexer, ARW_TK);
        else
            lex_symbol_var(lexer, VAR_TK);
    }
    else if (type == EOS_TK) {
        lexer_end_of_statement(lexer);
    }
    else if (type == CN_TK) {
        if (lexer_assert_next(lexer, '='))
            type = AS_TK;
        else if (lexer_assert_next(lexer, ':'))
            type = SCP_TK;
        lexer_token_add(lexer, type);
    }
    else if (type == NUM_TK) {
        while (lex_is_numeric(lexer_read(lexer)))
            lexer_readnext(lexer);
        lexer_token_add_string(lexer, NUM_TK, lexer_get_string(lexer));
    }
    else if (type == DQT_TK) {
        lexer_token_add_string(lexer, type, lexer_get_qstring(lexer));
    } else
        lexer_token_add(lexer, type);
}

static inline char lex_expr_exit(char c)
{
    if (c == '[')
        return ']';
    else
        return c;
}

static void lex_symbol_expr(struct lexer *lexer, char exit)
{
    char c;
    exit = lex_expr_exit(exit);

    do {
        lexer_reset(lexer);
        c = lexer_readnext(lexer);

        if (c == exit) {
            lexer_token_add(lexer, lex_token_type(exit));
            return;
        }
        else if (c == '[')
            lexer_token_add(lexer, LS_TK);
        else if (c == '=')
            lexer_token_add(lexer, EQ_TK);
        else if (c == '!') {
            if (lexer_assert_next(lexer, '='))
                lexer_token_add(lexer, NE_TK);
            else
                lexer_token_add(lexer, NT_TK);
        }
        else if (c == '<') {
            if (lexer_assert_next(lexer, '='))
                lexer_token_add(lexer, LE_TK);
            else
                lexer_token_add(lexer, LN_TK);
        }
        else if (c == '>') {
            if (lexer_assert_next(lexer, '='))
                lexer_token_add(lexer, GE_TK);
            else
                lexer_token_add(lexer, GN_TK);
        }
        else if (c == '+')
            lexer_token_add(lexer, PS_TK);
        else if (c == '-')
            lexer_token_add(lexer, SB_TK);
        else if (c == '*')
            lexer_token_add(lexer, ST_TK);
        else if (c == '/')
            lexer_token_add(lexer, DV_TK);
        else if (c == '%')
            lexer_token_add(lexer, MD_TK);
        else
            lex_symbol_default(lexer, lex_token_type(c));

    } while (lexer_hasnext(lexer));
}

static void lex_main(struct lexer *lexer)
{
    char c;
    enum ash_tk_type type;

    while (lexer_hasnext(lexer)) {
        lexer_reset(lexer);
        c = lexer_readnext(lexer);
        type = lex_token_type(c);

        if (lex_is_expr_type(type)) {
            lexer_token_add(lexer, type);
            lex_symbol_expr(lexer, c);
        } else {
            lex_symbol_default(lexer, type);
        }
    }
}

int lex_scan_input(struct ash_tk_set *set, const char *input)
{
    struct lexer lexer;
    lexer_init(&lexer, input, set);
    lex_main(&lexer);

    return 0;
}
