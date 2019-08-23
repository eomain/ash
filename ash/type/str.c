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

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "ash/bool.h"
#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/str.h"
#include "ash/type.h"
#include "ash/util.h"
#include "ash/var.h"

#define ASH_STR_TYPENAME "string"

struct ash_string {
    struct ash_obj obj;
    const char *data;
    size_t len;
    struct ash_obj *character;
};

static inline const char *
get(struct ash_string *as)
{
    return as->data;
}

static struct ash_obj *
string_eq(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_string *as, *bs;
    as = (struct ash_string *) a;
    bs = (struct ash_string *) b;
    bool eq = (strcmp(get(as), get(bs)) == 0);
    obj = ash_bool_from(eq);
    return obj;
}

static struct ash_obj *
string_ne(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_string *as, *bs;
    as = (struct ash_string *) a;
    bs = (struct ash_string *) b;
    bool eq = (strcmp(get(as), get(bs)) != 0);
    obj = ash_bool_from(eq);
    return obj;
}

static struct ash_obj *
string_add(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_string *as, *bs;
    as = (struct ash_string *) a;
    bs = (struct ash_string *) b;
    obj = ash_str_from(ash_strcat(get(as), get(bs)));
    return obj;
}

static void dealloc(struct ash_obj *obj)
{
    struct ash_string *as;
    as = (struct ash_string *) obj;
    if (as->data) {
        ash_free((char *)as->data);
        as->data = NULL;
    }

    ash_obj_dec_rc(as->character);
    as->len = 0;
}

static struct ash_obj *
boolean(struct ash_obj *a)
{
    struct ash_obj *obj;
    struct ash_string *as;
    as = (struct ash_string *) a;
    bool value = (get(as) && (ash_strlen(get(as)) > 0)) ? true: false;
    obj = ash_bool_from(value);
    return obj;
}

static const char *name()
{
    return ASH_STR_TYPENAME;
}

static struct ash_obj *string(struct ash_obj *obj)
{
    return obj;
}

static struct option iter(struct ash_obj *obj, size_t pos)
{
    struct option opt;

    struct ash_string *string;
    string = (struct ash_string *) obj;
    if (pos < string->len) {
        if (!string->character)
            string->character = ash_str_new();
        char c[] = { string->data[pos], '\0' };
        ash_str_set(string->character, ash_strcpy(c));
        ash_obj_inc_rc(string->character);
        option_some(&opt, string->character);
        return opt;
    }

    option_none(&opt);
    return opt;
}

static bool match(struct ash_obj *obj, struct ash_obj *m)
{
    if (!ash_obj_type_eq(obj, m))
        return false;
    return (strcmp(ash_str_get(obj), ash_str_get(m)) == 0);
}

static struct ash_base base = {
    .ops = {
        .eq = string_eq,
        .ne = string_ne,
        .gt = NULL,
        .lt = NULL,

        .add = string_add,
        .sub = NULL,
        .mul = NULL,
        .div = NULL,
        .mod = NULL
    },

    .into = {
        .boolean = boolean,
        .string  = string
    },

    .util = {
        .match = match
    },

    .iter = iter,
    .dealloc = dealloc,
    .name = name
};

struct ash_obj *ash_str_new(void)
{
    struct ash_string *as;
    as = ash_alloc(sizeof *as);
    as->data = NULL;
    as->len = 0;
    as->character = NULL;

    struct ash_obj *obj;
    obj = (struct ash_obj *) as;
    ash_obj_init(obj, &base);
    return obj;
}

void ash_str_set(struct ash_obj *obj, const char *value)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_string *as;
        as = (struct ash_string *) obj;
        if (as->data)
            ash_free((char *)as->data);
        as->data = value;
        as->len = ash_strlen(value);
    }
}

const char *ash_str_get(struct ash_obj *obj)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_string *as;
        as = (struct ash_string *) obj;
        return as->data;
    }

    return NULL;
}

struct ash_obj *ash_str_from(const char *value)
{
    struct ash_obj *obj;
    obj = ash_str_new();
    ash_str_set(obj, value);
    return obj;
}
