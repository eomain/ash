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


#include "ash/ash.h"
#include "ash/bool.h"
#include "ash/command.h"
#include "ash/defined.h"
#include "ash/exec.h"
#include "ash/type.h"
#include "ash/var.h"
#include "ash/lang/runtime.h"

static const char *USAGE =
    "defined:\n"
    "    check if a variable is defined\n"
    "usage:\n"
    "    defined [VARIABLE]...\n";

const char *ash_defined_usage(void)
{
    return USAGE;
}

int ash_defined_env(int argc, const char * const *argv,
                    struct ash_command_env *env)
{
    struct ash_obj *result;
    struct ash_var *var;
    bool defined = true;

    for (int i = 1; i < argc; ++i) {
        if (!(var = runtime_get_var(env->env, argv[i]))) {
            defined = false;
            break;
        }
    }

    result = ash_bool_from(defined);
    ash_command_env_set_result(env, result);

    return ASH_STATUS_OK;
}
