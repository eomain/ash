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

#include "ash/bool.h"
#include "ash/int.h"
#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/range.h"
#include "ash/type.h"

#define ASH_RANGE_TYPENAME "range"

struct ash_range {
    struct ash_obj obj;
    struct ash_obj *index;
    isize start;
    isize end;
    bool inclusive;
};

static void dealloc(struct ash_obj *obj)
{
    struct ash_range *range;
    range = (struct ash_range *) obj;
    ash_obj_dec_rc(range->index);
}

static const char *name()
{
    return ASH_RANGE_TYPENAME;
}

static inline bool in_range(struct ash_range *range, isize pos)
{
    isize start, end;
    start = range->start;
    end = range->end;

    if (!range->inclusive) {
        end = (end - 1);
    }

    return ((pos >= start) && (pos <= end)) ||
           ((pos <= start) && (pos >= end)) ? true: false;
}

static inline isize position(struct ash_range *range, size_t n)
{
    return (range->start < range->end) ?
           (n + range->start): (n - range->start);
}

static struct option iter(struct ash_obj *obj, size_t pos)
{
    struct option opt;
    isize index;
    struct ash_range *range;

    range = (struct ash_range *) obj;
    index = position(range, pos);

    if (!in_range(range, index)) {
        option_none(&opt);
        return opt;
    }

    ash_int_set(range->index, index);
    option_some(&opt, range->index);
    return opt;
}

static bool match(struct ash_obj *obj, struct ash_obj *m)
{
    struct ash_range *range;
    range = (struct ash_range *) obj;

    if (!ash_obj_type_eq(range->index, m))
        return false;
    return in_range(range, ash_int_get(m));
}

static struct ash_base base = {
    .util = {
        .match = match
    },

    .iter = iter,
    .dealloc = dealloc,
    .name = name
};

struct ash_obj *ash_range_new(void)
{
    struct ash_range *range;
    range = ash_alloc(sizeof *range);
    range->index = ash_int_new();
    range->start = 0;
    range->end = 0;
    range->inclusive = false;

    struct ash_obj *obj;
    obj = (struct ash_obj *) range;
    ash_obj_init(obj, &base);
    return obj;
}

void ash_range_set(struct ash_obj *obj, isize start, isize end, bool inclusive)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_range *range;
        range = (struct ash_range *) obj;
        range->start = start;
        range->end = end;
        range->inclusive = inclusive;
    }
}

struct ash_obj *ash_range_from(isize start, isize end, bool inclusive)
{
    struct ash_obj *obj;
    obj = ash_range_new();
    ash_range_set(obj, start, end, inclusive);
    return obj;
}
