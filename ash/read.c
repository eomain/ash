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

#include <string.h>

#include "ash/command.h"
#include "ash/env.h"
#include "ash/io.h"
#include "ash/mem.h"
#include "ash/ops.h"
#include "ash/read.h"
#include "ash/str.h"
#include "ash/var.h"
#include "ash/lang/runtime.h"

#define MAX_INPUT_SIZE 2048

static const char *USAGE =
    "read:\n"
    "    read from standard input\n"
    "usage:\n"
    "    read [OPTIONS] [VARIABLE]\n"
    "\n"
    "OPTIONS:\n"
    "    -p <PROMPT>         Display as prompt\n";

const char *ash_read_usage(void)
{
    return USAGE;
}

static int ash_read_input(const char *var, const char *prompt,
                          struct ash_runtime_env *renv)
{
    const char *input;

    if ((input = ash_scan_prompt(prompt))) {
        if (!var) {
            ash_free((char *)input);
            return -1;
        }

        struct ash_obj *obj;
        obj = ash_str_from(ash_strcpy(input));
        runtime_set_var(renv, var, obj);
        return 0;
    }

    return -1;
}

int ash_read_env(int argc, const char * const *argv,
                 struct ash_command_env *env)
{
    const char *opt;
    const char *prompt = NULL;
    const char *var = NULL;

    for (int i = 1; i < argc; ++i) {
        opt = argv[i];

        if (opt[0] == '-' && strlen(opt) == 2) {
            char c = opt[1];

            if (c == 'p') {
                if (i < argc)
                    prompt = argv[++i];
            }

        } else {
            var = argv[i];
            break;
        }
    }

    return ash_read_input(var, prompt, env->env);
}
