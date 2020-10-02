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

#include "ash/ash.h"
#include "ash/command.h"
#include "ash/export.h"
#include "ash/obj.h"
#include "ash/str.h"
#include "ash/type.h"
#include "ash/var.h"
#include "ash/lang/runtime.h"

enum export_flag_option {
    /* do not override */
    OVERRIDE = 1 << 0,
    /* search for global variable */
    GLOBAL   = 1 << 1
};

static const char *USAGE =
    "export:\n"
    "    export an enviornment variable\n"
    "usage:\n"
    "    export [FLAGS]... [NAME=VALUE]...\n"
    "\n"
    "FLAGS:\n"
    "    -d                 Override existing variable if present\n"
    "    -g                 Set as a shell global variable\n";

const char *ash_export_usage(void)
{
    return USAGE;
}

static ash_flag ash_export_option(const char *s)
{
    char c;
    ash_flag options = ASH_FLAG_RESET;

    while ((c = *(s++))) {
        switch (c) {
            case 'd':
                options |= OVERRIDE;
                break;

            case 'g':
                options |= GLOBAL;
                break;

            default:
                return ASH_FLAG_RESET;
        }
    }

    return options;
}

int ash_export_env(int argc, const char * const *argv, struct ash_command_env *env)
{
    int status = ASH_STATUS_OK;
    ash_flag option = ASH_FLAG_RESET;

    int start = 1;
    for (int i = start; i < argc; ++i) {
        if (*argv[i] != '-')
            break;

        option |= ash_export_option(&argv[i][1]);
        start++;
    }


    int override = (option & OVERRIDE) ? ASH_FLAG_RESET: ASH_FLAG_SET;
    const char *name = NULL;
    struct ash_obj *obj;
    struct ash_var *var;

    for (int i = start; i < argc; ++i) {
        name = argv[i];

        if (!(option & GLOBAL))
            var = runtime_get_var(env->env, name);
        else
            var = ash_var_get(name);

        if (var && (obj = ash_var_obj(var))) {
            const char *str;
            str = ash_str_get(ash_obj_str(obj));
            if (!setenv(name, str, override))
                status = ASH_STATUS_ERR;
        } else
            status = ASH_STATUS_ERR;
    }

    return status;
}
