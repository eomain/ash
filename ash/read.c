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

#include <stdlib.h>
#include <string.h>

#include "builtin.h"
#include "env.h"
#include "io.h"
#include "mem.h"
#include "read.h"
#include "var.h"

#define MAX_INPUT_SIZE 255

const char *ash_read_usage(void)
{
    return "read input from standard input";
}

static int ash_read_input(const char *var)
{
    if (var){
        char *buf = ash_alloc(MAX_INPUT_SIZE);
        if (buf){
            if (ash_scan_buffer(buf, MAX_INPUT_SIZE) == 0){
                size_t len = strlen(buf) + 1;
                if (len < MAX_INPUT_SIZE){
                    char *n;
                    if ((n = strchr(buf, '\n'))){
                        *n = '\0';
                        len--;
                    }
                    buf = ash_realloc(buf, len);
                }
                ash_var_set(var, buf, ASH_DATA);
            } else
                return -1;
        } else
            return -1;
    } else
        ash_scan();

    return 0;
}

int ash_read(int argc, const char * const *argv)
{
    int status = 0;
    int i = 1;
    const char *prompt = NULL;
    const char *var = NULL;

    for (; i < argc; ++i){
        const char *opt = argv[i];

        if (opt[0] == '-' && strlen(opt) == 2){
            char c = opt[1];

            if (c == 'p'){
                if (i < argc)
                    prompt = argv[++i];
            }

        } else {
            var = argv[i];
            break;
        }
    }

    if (prompt)
        ash_print(prompt);

    status = ash_read_input(var);

    return status ? 1: 0;
}
