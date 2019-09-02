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

#include "ash/ash.h"
#include "ash/history.h"
#include "ash/term/term.h"

enum history_flag_option {
    CLEAR = 1 << 0
};

const char *ash_history_usage(void)
{
    return "manipulate the shell history";
}

static ash_flag ash_history_option(const char *s)
{
    char c;
    ash_flag options = ASH_FLAG_RESET;

    while ((c = *(s++))) {
        switch (c) {
            case 'c':
                options |= CLEAR;
                break;

            default:
                return ASH_FLAG_RESET;
        }
    }

    return options;
}

int ash_history(int argc, const char * const *argv)
{
    if (argc == 1)
        return ASH_STATUS_OK;

    ash_flag options = ASH_FLAG_RESET;
    const char *opt;

    for (int i = 1; i < argc; ++i) {
        opt = argv[i];

        if (opt[0] == '-') {
            ash_flag n;
            if ((n = ash_history_option(&opt[1])))
                options |= n;
        } else
            break;
    }

    if (options & CLEAR)
        ash_term_clear();

    return ASH_STATUS_OK;
}
