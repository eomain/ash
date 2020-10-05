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

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "ash/bool.h"
#include "ash/func.h"
#include "ash/int.h"
#include "ash/iter.h"
#include "ash/macro.h"
#include "ash/obj.h"
#include "ash/script.h"
#include "ash/str.h"
#include "ash/tuple.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/var.h"
#include "ash/ffi/ffi.h"

struct ash_ffi_function {
    const char *name;
    ash_ffi function;
    bool anonymous;
};

static void ash_ffi_set(struct ash_ffi_function *ffi)
{
    assert(ffi->name);
    const char *name;
    struct ash_obj *obj;

    name = (ffi->anonymous) ? NULL: ffi->name;
    obj = ash_func_from_ffi(name, ffi->function);

    ash_var_func_set(name, obj);
}

static inline size_t
ffi_args_len(struct ash_obj *args)
{
    if (args)
        return ash_tuple_len(args);
    return 0;
}

static inline struct ash_obj *
ffi_args_get(struct ash_obj *args, size_t pos)
{
    return ash_tuple_get(args, pos);
}

static struct ash_obj *env(struct ash_obj *args)
{
    if (ffi_args_len(args) == 0)
        return ash_str_from("");
    const char *name;
    if (!(name = ash_str_get(ffi_args_get(args, 0))))
        return ash_str_from("");
    if (!(name = getenv(name)))
        return ash_str_from("");
    return ash_str_from(name);
}

static struct ash_obj *len(struct ash_obj *args)
{
    size_t argc;
    isize len = 0;
    struct ash_obj *ret = NULL;

    if ((argc = ffi_args_len(args)) > 0) {
        struct ash_obj *obj;
        struct ash_iter iter;

        obj = ffi_args_get(args, 0);
        ash_iter_init(&iter, obj);

        while ((ash_iter_hasnext(&iter))) {
            ash_iter_next(&iter);
            len++;
        }
    }

    ret = ash_int_from(len);
    return ret;
}

static struct ash_obj *load(struct ash_obj *args)
{
    if (ffi_args_len(args) > 0) {
        const char *script;
        struct ash_obj *obj;
        struct ash_iter iter;
        ash_iter_init(&iter, args);

        while ((ash_iter_hasnext(&iter))) {
            obj = ash_iter_next(&iter);
            if (!(script = ash_str_get(obj))) {
                ash_obj_inc_rc(obj);
                return obj;
            }
            if (ash_script_load(script, false) == -1)
                return ash_str_from(script);
        }
    }

    return ash_bool_from(true);
}

static struct ash_ffi_function functions[] = {
    {
        .name = "env",
        .function = env,
        .anonymous = false,
    },

    {
        .name = "len",
        .function = len,
        .anonymous = false
    },

    {
        .name = "load",
        .function = load,
        .anonymous = false
    }
};

static void init(void)
{
    for (size_t i = 0; i < array_length(functions); ++i)
        ash_ffi_set(&functions[i]);
}

const struct ash_unit_module ash_module_ffi = {
    .init = init,
    .destroy = NULL
};
