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
#include <stdio.h>

#include "ash/bool.h"
#include "ash/int.h"
#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/str.h"
#include "ash/type.h"

#define ASH_INT_TYPENAME "int"
#define ASH_INT_DEFAULT 0
#define ASH_FMT_SIZE 20

struct ash_int {
    struct ash_obj obj;
    isize value;
};

static struct ash_obj *
int_eq(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    bool eq = (ai->value == bi->value);
    obj = ash_bool_from(eq);
    return obj;
}

static struct ash_obj *
int_ne(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    bool eq = (ai->value != bi->value);
    obj = ash_bool_from(eq);
    return obj;
}

static struct ash_obj *
int_gt(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    bool eq = (ai->value > bi->value);
    obj = ash_bool_from(eq);
    return obj;
}

static struct ash_obj *
int_lt(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    bool eq = (ai->value < bi->value);
    obj = ash_bool_from(eq);
    return obj;
}

static struct ash_obj *
int_add(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    isize value = (ai->value + bi->value);
    obj = ash_int_from(value);
    return obj;
}

static struct ash_obj *
int_sub(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    isize value = (ai->value - bi->value);
    obj = ash_int_from(value);
    return obj;
}

static struct ash_obj *
int_mul(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    isize value = (ai->value * bi->value);
    obj = ash_int_from(value);
    return obj;
}

static struct ash_obj *
int_div(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    if (bi->value == 0)
        return NULL;
    isize value = (ai->value / bi->value);
    obj = ash_int_from(value);
    return obj;
}

static struct ash_obj *
int_mod(struct ash_obj *a, struct ash_obj *b)
{
    struct ash_obj *obj;
    struct ash_int *ai, *bi;
    ai = (struct ash_int *) a;
    bi = (struct ash_int *) b;
    if (bi->value == 0)
        return NULL;
    isize value = (ai->value % bi->value);
    obj = ash_int_from(value);
    return obj;
}

static struct ash_obj *
boolean(struct ash_obj *a)
{
    struct ash_obj *obj;
    struct ash_int *ai;
    ai = (struct ash_int *) a;
    bool value = (ai->value == 0) ? false: true;
    obj = ash_bool_from(value);
    return obj;
}

static const char *name()
{
    return ASH_INT_TYPENAME;
}

static struct ash_obj *
string(struct ash_obj *obj)
{
    struct ash_int *ai;
    ai = (struct ash_int *) obj;
    char *fmt = ash_zalloc(ASH_FMT_SIZE * sizeof *fmt);
    ash_ops_fmt_num(fmt, ai->value, ASH_FMT_SIZE);
    return ash_str_from(fmt);
}

static bool match(struct ash_obj *obj, struct ash_obj *m)
{
    if (!ash_obj_type_eq(obj, m))
        return false;
    return (ash_int_get(obj) == ash_int_get(m));
}

static struct ash_base base = {
    .ops = {
        .eq = int_eq,
        .ne = int_ne,
        .gt = int_gt,
        .lt = int_lt,

        .add = int_add,
        .sub = int_sub,
        .mul = int_mul,
        .div = int_div,
        .mod = int_mod
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

struct ash_base *ash_int_base(void)
{
    return &base;
}

struct ash_obj *ash_int_new(void)
{
    struct ash_int *ai;
    ai = ash_alloc(sizeof *ai);
    ai->value = ASH_INT_DEFAULT;

    struct ash_obj *obj;
    obj = (struct ash_obj *) ai;
    ash_obj_init(obj, &base);
    return obj;
}

isize ash_int_get(struct ash_obj *obj)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_int *ai;
        ai = (struct ash_int *) obj;
        return ai->value;
    }
    return 0;
}

void ash_int_set(struct ash_obj *obj, isize value)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_int *ai;
        ai = (struct ash_int *) obj;
        ai->value = value;
    }
}

struct ash_obj *ash_int_from(isize value)
{
    struct ash_obj *obj;
    obj = ash_int_new();
    ash_int_set(obj, value);
    return obj;
}
