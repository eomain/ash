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
#include <editline/readline.h>
#include <stdlib.h>
#include <string.h>

#include "ash/ash.h"
#include "ash/mem.h"
#include "ash/ops.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/term/term.h"

#define ASH_HISTORY_NAME "~.ash_history"
#define ASH_HISTORY_SIZE 500

void ash_term_clear(void)
{
    clear_history();
}

static inline const char *history(void)
{
    const char *name;
    name = ash_ops_tilde(ASH_HISTORY_NAME);
    assert(name);
    return name;
}

static void ash_term_clean(void)
{
    const char *name;
    name = history();
    write_history(name);
}

static inline bool
ash_term_hist(const char *line)
{
    if (line && (*line)) {
        add_history(line);
        return true;
    }
    return false;
}

static inline char *
newline(const char *input)
{
    size_t len;
    char *s;

    len = strlen(input);
    s = ash_zalloc((len + 2));
    strcpy(s, input);
    s[len] = '\n';
    return s;
}

const char *ash_term_get(const char *prompt)
{
    char *input = NULL, *s;

    if ((input = readline(prompt))) {
        ash_term_hist(input);

        if ((s = newline(input))) {
            free(input);
            input = s;
        }
    }
    return input;
}

const char *ash_term_get_raw(const char *prompt)
{
    char *input = NULL;

    if ((input = readline(prompt)))
        ash_term_hist(input);
    return input;
}

const char *ash_term_get_default(void)
{
    return ash_term_get(NULL);
}

static void init(void)
{
    rl_initialize();

    const char *name;
    name = history();
    read_history(name);
}

static void destroy(void)
{
    ash_term_clean();
}

const struct ash_unit_module ash_module_term = {
    .init = init,
    .destroy = destroy
};
