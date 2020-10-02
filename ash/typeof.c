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
#include "ash/command.h"
#include "ash/io.h"
#include "ash/iter.h"
#include "ash/str.h"
#include "ash/type.h"
#include "ash/typeof.h"
#include "ash/var.h"
#include "ash/lang/runtime.h"

static const char *USAGE =
    "typeof:\n"
    "    display the type of a variable\n"
    "usage:\n"
    "    typeof [VARIABLE]...\n";

const char *ash_typeof_usage(void)
{
    return USAGE;
}

static int ash_typeof_match(struct ash_obj *);

static void ash_typeof_iterable(struct ash_obj *iterable)
{
    ash_putchar('(');

    struct ash_iter iter;
    ash_iter_init(&iter, iterable);
    struct ash_obj *value;

    while ((value = ash_iter_next(&iter))) {
        ash_typeof_match(value);
        if (ash_iter_hasnext(&iter)) {
            ash_putchar(',');
            ash_putchar(' ');
        }
    }

    ash_putchar(')');
}

static int ash_typeof_match(struct ash_obj *obj)
{
    if (!obj)
        return -1;

    const char *name;
    if ((name = ash_obj_name(obj)))
        ash_print(name);

    return 0;
}

int ash_typeof_env(int argc, const char * const *argv,
                   struct ash_command_env *env)
{
    int status = ASH_STATUS_OK;
    struct ash_obj *obj;
    struct ash_var *var;

    for (int i = 1; i < argc; ++i) {
        if ((var = runtime_get_var(env->env, argv[i]))) {
            obj = ash_var_obj(var);
            ash_typeof_match(obj);
            ash_putchar('\n');
        } else {
            status = ASH_STATUS_ERR;
        }
    }

    return status;
}
