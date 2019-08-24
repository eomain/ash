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

#include "ash/ash.h"
#include "ash/echo.h"
#include "ash/io.h"

enum echo_flag_option {
    /* no trailing newline */
    LINE   = 1 << 1,
    /* remove whitespace */
    SPACE  = 1 << 2,
    /* format string */
    FORMAT = 1 << 3
};

const char *ash_echo_usage(void)
{
    return "print formatted string to standard out";
}

static ash_flag ash_echo_option(const char *s)
{
    char c;
    ash_flag options = ASH_FLAG_RESET;

    while ((c = *(s++))) {
        switch (c){
            case 'n':
                options |= LINE;
                break;

            case 's':
                options |= SPACE;
                break;

            case 'f':
                options |= FORMAT;
                break;

            default:
                return ASH_FLAG_RESET;
        }
    }

    return options;
}

static void ash_echo_format(const char *fmt)
{
    char c;

    while ((c = *fmt++)) {
        if (c == '\\') {
            c = *fmt;
            switch (c) {
                case 'a':
                    ash_putchar('\a');
                    break;

                case 'b':
                    ash_putchar('\b');
                    break;

                case 'n':
                    ash_putchar('\n');
                    break;

                case 'r':
                    ash_putchar('\r');
                    break;

                case 'f':
                    ash_putchar('\f');
                    break;

                case 't':
                    ash_putchar('\t');
                    break;

                case 'v':
                    ash_putchar('\v');
                    break;

                case '\\':
                    ash_putchar('\\');
                    break;

                default:
                    ash_putchar('\\');
                    continue;
            }
            fmt++;
        } else
            ash_putchar(c);
    }
}

int ash_echo(int argc, const char * const *argv)
{
    int start = 1;
    ash_flag options = ASH_FLAG_RESET;

    if (argc > 1) {
        const char *opt;
        for (int i = 1; i < argc; ++i) {
            opt = argv[i];

            if (opt[0] == '-') {
                int n;
                if ((n = ash_echo_option(&opt[1]))) {
                    options |= n;
                    start++;
                }
            } else
                break;
        }

        const char *fmt = options & SPACE ? "%s": "%s ";

        if (start < argc) {

            if (options & FORMAT) {
                for (; start < argc -1; ++start) {
                    ash_echo_format(argv[start]);
                    if (!(options & SPACE))
                        ash_putchar(' ');
                }
                ash_echo_format(argv[argc - 1]);

            } else {
                for (; start < argc -1; ++start)
                    ash_print(fmt, argv[start]);
                ash_print(argv[argc - 1]);
            }
        }

    }

    if (!(options & LINE))
        ash_putchar('\n');

    return ASH_STATUS_OK;
}


void ash_echo_help(void);
