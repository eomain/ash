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

#include "ash/ash.h"
#include "ash/env.h"
#include "ash/macro.h"
#include "ash/mem.h"
#include "ash/ops.h"

static struct ash_tk *
ash_tk_new(int res, const char *str)
{
    struct ash_tk *tk;
    tk = ash_alloc(sizeof ( *tk ));

    tk->str = str;
    tk->res = res;
    tk->eos = 0;
    tk->next = NULL;
    return tk;
}

struct ash_tk *
ash_tk_next(struct ash_tk **tk)
{
    if (*tk == NULL)
        return NULL;
    assert(*tk);
    *tk = (*tk)->next;
    return *tk;
}

int
ash_tk_get(struct ash_tk **tk)
{
    if (tk && *tk)
        return (*tk)->res;
    else
        return NO_TK;
}

int
ash_tk_valid(struct ash_tk **tk)
{
    return ash_tk_get(tk) != NO_TK;
}

int
ash_tk_getnext(struct ash_tk **tk)
{
    ash_tk_next(tk);
    return ash_tk_get(tk);
}

int
ash_tk_cknext(struct ash_tk **tk)
{
    if (tk && *tk)
        return ash_tk_get( &(*tk)->next );
    else
        return NO_TK;
}

int
ash_tk_eos(struct ash_tk **tk)
{
    if (tk && *tk)
        return (*tk)->eos;
    return 0;
}

bool ash_tk_get_eos(struct ash_tk **tk)
{
    enum ash_tk_type type;
    type = ash_tk_get(tk);
    if (ash_tk_cknext(tk) == SEM_TK) {
        ash_tk_next(tk);
        return true;
    }
    else if (ash_tk_eos(tk))
        return true;

    return false;
}

const char *
ash_tk_strcpy(struct ash_tk **tk)
{
    if (tk && *tk && (*tk)->str)
        return ash_strcpy((*tk)->str);
    return NULL;
}

isize
ash_tk_num(struct ash_tk **tk)
{
    if (tk && *tk && (*tk) ->str)
        return atol((*tk)->str);
    return 0;
}

int
ash_tk_assert_type(struct ash_tk **tk, enum ash_tk_type type)
{
    if (ash_tk_get(tk) != type)
        return -1;
    return 0;
}

struct ash_tk *
ash_tk_set_front(struct ash_tk_set *set)
{
    return set->front;
}

void
ash_tk_set_add(struct ash_tk_set *set, enum ash_tk_type type,
               const char *string, struct ash_tk_meta *meta)
{
    struct ash_tk *tk;
    tk = ash_tk_new(type, string);

    tk->line = meta->line;
    tk->offset = meta->offset;

    if (set->rear) {
        set->rear->next = tk;
        set->rear = set->rear->next;
    } else {
        set->front = set->rear = tk;
    }
}

static void
ash_tk_set_free(struct ash_tk_set *set)
{
    if (set->front) {
        struct ash_tk *tk = set->front, *n;
        do {
            n = tk->next;
            if (tk->str)
                ash_free((void *)tk->str);
            tk->str = NULL;
            tk->next = NULL;
            ash_free(tk);
        } while((tk = n));

        set->front = NULL;
    }
    set->rear = NULL;
}

static const char *ash_token_names[] = {
    [ AN_TK  ]   = "and",
    [ BK_TK  ]   = "break",
    [ DEF_TK ]   = "def",
    [ EL_TK  ]   = "else",
    [ ELF_TK ]   = "elif",
    [ END_TK ]   = "end",
    [ FS_TK  ]   = "false",
    [ FOR_TK ]   = "for",
    [ IF_TK  ]   = "if",
    [ IN_TK  ]   = "in",
    [ LET_TK ]   = "let",
    [ MAT_TK ]   = "match",
    [ MOD_TK ]   = "mod",
    [ NXT_TK ]   = "next",
    [ OR_TK  ]   = "or",
    [ RET_TK ]   = "return",
    [ TO_TK  ]   = "to",
    [ TR_TK  ]   = "true",
    [ UT_TK  ]   = "until",
    [ USE_TK ]   = "use",
    [ DO_TK  ]   = "while",
    [ LS_TK  ]   = "[",
    [ RS_TK  ]   = "]",
    [ LP_TK  ]   = "(",
    [ RP_TK  ]   = ")",
    [ BQ_TK  ]   = "`",
    [ SQT_TK ]   = "'",
    [ DQT_TK ]   = "\"",
    [ CO_TK  ]   = ",",
    [ CN_TK  ]   = ":",
    [ SCP_TK ]   = "::",
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
    [ ARW_TK ]   = "=>",
    [ LE_TK  ]   = "<=",
    [ GE_TK  ]   = ">=",
    [ PIP_TK ]   = "|",
    [ QMK_TK ]   = "?",
    [ VAR_TK ]   = "<id>",
    [ NUM_TK ]   = "<num>",
    [ SEM_TK ]   = ";",
    [ NO_TK  ]   = NULL
};

const char *ash_tk_name(enum ash_tk_type type)
{
    assert(type < array_length(ash_token_names));

    const char *token;
    if (!(token = ash_token_names[type]))
        token = "token";
    return token;
}
