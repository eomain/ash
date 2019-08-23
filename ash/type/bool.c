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
#include <string.h>

#include "ash/bool.h"
#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/str.h"
#include "ash/type.h"

#define ASH_BOOL_TYPENAME "bool"
#define ASH_BOOL_TRUE "true"
#define ASH_BOOL_FALSE "false"
#define ASH_BOOL_STR_LEN 6

#define ASH_BOOL_DEFAULT false

struct ash_bool {
    struct ash_obj obj;
    bool value;
};

static struct ash_obj *
bool_eq(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_bool *ab, *bb;
    ab = (struct ash_bool *) a;
    bb = (struct ash_bool *) b;
    bool eq = (ab->value == bb->value);
    obj = ash_bool_new();
    ash_bool_set(obj, eq);
    return obj;
}

static struct ash_obj *
bool_ne(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_bool *ab, *bb;
    ab = (struct ash_bool *) a;
    bb = (struct ash_bool *) b;
    bool eq = (ab->value != bb->value);
    obj = ash_bool_new();
    ash_bool_set(obj, eq);
    return obj;
}

static struct ash_obj *
bool_and(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_bool *ab, *bb;
    ab = (struct ash_bool *) a;
    bb = (struct ash_bool *) b;
    bool eq = (ab->value && bb->value);
    obj = ash_bool_new();
    ash_bool_set(obj, eq);
    return obj;
}

static struct ash_obj *
bool_or(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_bool *ab, *bb;
    ab = (struct ash_bool *) a;
    bb = (struct ash_bool *) b;
    bool eq = (ab->value || bb->value);
    obj = ash_bool_new();
    ash_bool_set(obj, eq);
    return obj;
}

static struct ash_obj *
boolean(struct ash_obj *obj)
{
    return obj;
}

static const char *name()
{
    return ASH_BOOL_TYPENAME;
}

struct ash_obj *string(struct ash_obj *obj)
{
    if (ash_bool_get(obj))
        return ash_str_from(ASH_BOOL_TRUE);
    return ash_str_from(ASH_BOOL_FALSE);
}

static bool match(struct ash_obj *obj, struct ash_obj *m)
{
    if (!ash_obj_type_eq(obj, m))
        return false;
    return (ash_bool_get(obj) == ash_bool_get(m));
}

static struct ash_base base = {
    .ops = {
        .eq = bool_eq,
        .ne = bool_ne,
        .gt = NULL,
        .lt = NULL,

        .and = bool_and,
        .or  = bool_or,

        .add = NULL,
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

    .iter = ash_base_iter_default,
    .dealloc = NULL,
    .name = name
};

struct ash_obj *ash_bool_new(void)
{
    struct ash_bool *ab;
    ab = ash_alloc(sizeof *ab);
    ab->value = ASH_BOOL_DEFAULT;

    struct ash_obj *obj;
    obj = (struct ash_obj *) ab;
    ash_obj_init(obj, &base);
    return obj;
}

bool ash_bool_get(struct ash_obj *obj)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_bool *ab;
        ab = (struct ash_bool *) obj;
        return ab->value;
    }
    return false;
}

void ash_bool_set(struct ash_obj *obj, bool value)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_bool *ab;
        ab = (struct ash_bool *) obj;
        ab->value = value;
    }
}

struct ash_obj *ash_bool_from(bool value)
{
    struct ash_obj *obj;
    obj = ash_bool_new();
    ash_bool_set(obj, value);
    return obj;
}
