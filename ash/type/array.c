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

#include <stddef.h>

#include "ash/macro.h"
#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/type.h"
#include "ash/type/array.h"
#include "ash/util/vec.h"

#define ASH_ARRAY_TYPENAME "array"
#define ARRAY_SIZE 16

struct ash_array {
    struct ash_obj obj;
    struct vec *vec;
};

static struct ash_obj *
array_add(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_array *aa, *ba;
    struct vec *vec;

    aa = (struct ash_array *)a;
    ba = (struct ash_array *)b;
    struct vec *array[] = { aa->vec, ba->vec };
    vec = vec_new();

    for (size_t n = 0; n < array_length(array); ++n)
        vec_append(vec, array[n]);

    vec_for_each(vec, (void (*)(void *))ash_obj_inc_rc);

    obj = ash_array_from(vec);
    return obj;
}

static const char *name(void)
{
    return ASH_ARRAY_TYPENAME;
}

static struct option iter(struct ash_obj *value, size_t pos)
{
    struct option opt;

    if (pos < ash_array_len(value))
        option_some(&opt, ash_array_get(value, pos));
    else
        option_none(&opt);
    return opt;
}

static void dealloc(struct ash_obj *obj)
{
    struct vec *vec;
    struct ash_array *array;
    array = (struct ash_array *)obj;
    vec = array->vec;

    vec_for_each(vec, (void (*)(void *))ash_obj_dec_rc);
    vec_destroy(vec);
}

static struct ash_base base = {
    .ops = {
        .add = array_add
    },


    .iter = iter,
    .dealloc = dealloc,
    .name = name
};

struct ash_obj *ash_array_new(struct vec *vec)
{
    struct ash_obj *obj;
    struct ash_array *array;

    array = ash_alloc(sizeof *array);
    array->vec = vec;
    obj = (struct ash_obj *) array;
    ash_obj_init(obj, &base);

    return obj;
}

struct ash_obj *ash_array_from(struct vec *vec)
{
    return ash_array_new(vec);
}

struct ash_obj *ash_array_get(struct ash_obj *obj, size_t index)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_array *array;
        array = (struct ash_array *) obj;
        return vec_get(array->vec, index);
    }
    return NULL;
}

size_t ash_array_len(struct ash_obj *obj)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_array *array;
        array = (struct ash_array *) obj;
        return (isize)vec_len(array->vec);
    }
    return 0;
}
