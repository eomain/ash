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
#include <histedit.h>

#include "ash/ash.h"
#include "ash/mem.h"
#include "ash/ops.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/term/term.h"

#define ASH_HISTORY_NAME "~.ash_history"
#define ASH_HISTORY_SIZE 500

struct ash_term_hist {
    History *hist;
    HistEvent event;
};

static void ash_term_hist_init(struct ash_term_hist *hist)
{
    if ((hist->hist = history_init()))
        history(hist->hist, &hist->event, H_SETSIZE, ASH_HISTORY_SIZE);
}

static void ash_term_hist_load(struct ash_term_hist *hist)
{
    assert(hist != NULL);

    const char *name;
    name = ash_ops_tilde(ASH_HISTORY_NAME);
    assert(name);
    history(hist->hist, &hist->event, H_LOAD, name);
    ash_free((char *) name);
}

static void ash_term_hist_save(struct ash_term_hist *hist)
{
    assert(hist != NULL);

    const char *name;
    name = ash_ops_tilde(ASH_HISTORY_NAME);
    assert(name);
    history(hist->hist, &hist->event, H_SAVE, name);
    ash_free((char *) name);
}

static inline History *ash_term_hist_get(struct ash_term_hist *hist)
{
    return hist->hist;
}

static inline bool hist_entry_valid(const char *input)
{
    return (input && (*input != '\n'));
}

static void ash_term_hist_append(struct ash_term_hist *hist, const char *input)
{
    if (hist_entry_valid(input))
        history(hist->hist, &hist->event, H_ENTER, input);
}

struct ash_term {
    EditLine *edit;
    struct ash_term_hist hist;
};

struct ash_term term;
struct ash_term dterm;

static void ash_term_init(struct ash_term *term, bool use_hist,
                          const char *(*prompt)(EditLine *))
{
    term->edit = el_init(PNAME, stdin, stdout, stderr);
    ash_term_hist_init(&term->hist);

    if (use_hist) {
        History *hist;
        if ((hist = ash_term_hist_get(&term->hist))) {
            ash_term_hist_load(&term->hist);
            el_set(term->edit, EL_HIST, history, hist);
        }
    }

    el_set(term->edit, EL_PROMPT, prompt);
    el_set(term->edit, EL_EDITOR, "emacs");
}

static void ash_term_clean(struct ash_term *term)
{
    ash_term_hist_save(&term->hist);
}

const char *ash_term_get(struct ash_term *term)
{
    int read = 0;
    const char *input = NULL;
    assert(term->edit);

    if ((input = el_gets(term->edit, &read)))
        ash_term_hist_append(&term->hist, input);
    return input;
}

const char *ash_term_get_default(void)
{
    int read = 0;
    const char *input = NULL;
    assert(dterm.edit);
    input = el_gets(dterm.edit, &read);
    return input;
}

static const char *prompt(EditLine *e)
{
    return "";
}

static void init(void)
{
    ash_term_init(&term, true, prompt);
    ash_term_init(&dterm, false, prompt);
}

static void destroy(void)
{
    ash_term_clean(&term);
}

const struct ash_unit_module ash_module_term = {
    .init = init,
    .destroy = destroy
};
