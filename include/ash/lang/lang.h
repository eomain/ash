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

#ifndef ASH_LANG_LANG_H
#define ASH_LANG_LANG_H

#include <stddef.h>

#include "ash/ash.h"
#include "ash/script.h"
#include "ash/type.h"

enum input_prompt_type {
    INPUT_PROMPT_BLOCK,
    INPUT_PROMPT_COMMAND
};

struct input {
    union {
        const char *text;
        struct script *script;
    } method;

    enum {
        ASH_INPUT_TEXT,
        ASH_INPUT_SCRIPT
    } type;

    ash_flag interactive;
};

static inline void input_script_init(struct input *input, struct script *script)
{
    input->method.script = script;
    input->type = ASH_INPUT_SCRIPT;
    input->interactive = ASH_FLAG_RESET;
}

static inline void input_text_init(struct input *input, const char *text)
{
    input->method.text = text;
    input->type = ASH_INPUT_TEXT;
    input->interactive = ASH_FLAG_SET;
}

static inline const char *input_text_content(struct input *input)
{
    const char *content = NULL;

    if (input->type == ASH_INPUT_SCRIPT)
        content = ash_script_content(input->method.script);
    else if (input->type == ASH_INPUT_TEXT)
        content = input->method.text;

    return content;
}

static inline const char *input_get_name(struct input *input)
{
    if (input->type == ASH_INPUT_SCRIPT)
        return ash_script_name(input->method.script);
    return NULL;
}

/*
contains a list of symbols representing all
recognised tokens in ash
*/
enum ash_tk_type {
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
    /* else if token */
    ELF_TK,
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
    /* less than equal to token */
    LE_TK,
    /* greater than equal to token */
    GE_TK,
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
    /* left brace */
    LB_TK,
    /* right brace */
    RB_TK,
    /* backquote */
    BQ_TK,
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
    /* def */
    DEF_TK,
    /* module */
    MOD_TK,
    /* match */
    MAT_TK,
    ARW_TK,
    SCP_TK,

    BS_TK,

    QMK_TK,

    LET_TK,
    USE_TK,

    NUM_TK,
    VAR_TK,
    SQT_TK,
    DQT_TK,
    NXT_TK,
    /* end of statement or newline */
    EOS_TK,
    /* semi-colon */
    SEM_TK,
    RET_TK
};

struct ash_tk {
    enum ash_tk_type type;
    bool eos;
    size_t line;
    size_t offset;
    const char *str;
    struct ash_tk *next;
};

struct ash_tk_meta {
    size_t line;
    size_t offset;
};

static inline void ash_tk_meta_init(struct ash_tk_meta *meta,
                                    size_t line, size_t offset)
{
    meta->line = line;
    meta->offset = offset;
}

extern enum ash_tk_type ash_tk_get(struct ash_tk **);
extern struct ash_tk *ash_tk_next(struct ash_tk **);
extern enum ash_tk_type ash_tk_cknext(struct ash_tk **);
extern bool ash_tk_eos(struct ash_tk **);
extern bool ash_tk_get_eos(struct ash_tk **);
extern const char *ash_tk_strcpy(struct ash_tk **);
extern isize ash_tk_num(struct ash_tk **);
extern const char *ash_tk_name(enum ash_tk_type);
extern int ash_tk_assert_type(struct ash_tk **, enum ash_tk_type);

struct ash_tk_set {
    struct ash_tk *front;
    struct ash_tk *rear;
};

static inline void ash_tk_set_init(struct ash_tk_set *set)
{
    set->front = NULL;
    set->rear = NULL;
}

static inline bool ash_tk_set_empty(struct ash_tk_set *set)
{
    return (!set->front) ? true: false;
}

static inline void ash_tk_set_eos(struct ash_tk_set *set)
{
    if (set->rear && !set->rear->eos)
        set->rear->eos = true;
}

extern void ash_tk_set_add(struct ash_tk_set *, enum ash_tk_type,
                           const char *, struct ash_tk_meta *);

extern struct ash_tk *ash_tk_set_front(struct ash_tk_set *);

extern int ash_lang_prompt(struct ash_tk_set *, enum input_prompt_type);

#endif
