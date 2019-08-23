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

#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/tuple.h"
#include "ash/type.h"

#define ASH_TUPLE_TYPENAME "tuple"


struct ash_tuple {
    struct ash_obj obj;
    /* number of data elements in the tuple */
    size_t len;
    /* size of the buffer */
    size_t buf;
    struct ash_obj **tup;
};

static void dealloc(struct ash_obj *);
static size_t ash_tuple_length(struct ash_tuple *);
static struct ash_obj *get(struct ash_tuple *, size_t);
static struct option iter(struct ash_obj *, size_t);

static const char *name()
{
    return ASH_TUPLE_TYPENAME;
}

static struct ash_obj *string(struct ash_obj *value)
{
    return NULL;
}

static struct ash_base base = {
    .iter = iter,
    .dealloc = dealloc,
    .name = name
};

struct ash_obj *ash_tuple_new(void)
{
    struct ash_tuple *tuple;
    tuple = ash_alloc(sizeof *tuple);
    tuple->len = 0;
    tuple->buf = 0;
    tuple->tup = NULL;

    struct ash_obj *obj;
    obj = (struct ash_obj *) tuple;
    ash_obj_init(obj, &base);
    return obj;
}

static struct option iter(struct ash_obj *value, size_t pos)
{
    struct option opt;
    struct ash_tuple *tuple;
    struct ash_obj *obj;
    tuple = (struct ash_tuple *) value;

    if (pos < ash_tuple_length(tuple))
        option_some(&opt, get(tuple, pos));
    else
        option_none(&opt);
    return opt;
}

static void ash_tuple_clear(struct ash_tuple *tuple)
{
    for (size_t i = 0; i < tuple->len; ++i) {
        struct ash_obj *value;
        value = tuple->tup[i];
        if (value != NULL) {
            ash_obj_dec_rc(value);
            tuple->tup[i] = NULL;
        }
    }
}

static void dealloc(struct ash_obj *obj)
{
    struct ash_tuple *tuple;
    tuple = (struct ash_tuple *) obj;

    if (tuple->tup) {
        ash_tuple_clear(tuple);
        ash_free(tuple->tup);
        tuple->tup = NULL;
    }

    tuple->len = 0;
    tuple->buf = 0;
    ash_free(tuple);
}

static void ash_tuple_resize(struct ash_tuple *tuple, size_t n)
{
    if (!(n > 0))
        return;

    const size_t size =
        (sizeof (struct ash_obj *)) * (tuple->buf + n);

    if (tuple->tup) {
        tuple->tup = ash_realloc(tuple->tup, size);
        const size_t offset = tuple->buf;
        for (size_t i = offset; i < offset + n; i++)
            tuple->tup[i] = NULL;
    } else {
        tuple->tup = ash_alloc(size);
        for (size_t i = 0; i < n; i++)
            tuple->tup[i] = NULL;
    }

    tuple->buf += n;
}

static void append(struct ash_tuple *tuple, struct ash_obj *value)
{
    if (tuple->buf == tuple->len)
        ash_tuple_resize(tuple, 1);

    const size_t pos = tuple->len;
    tuple->tup[pos] = value;
    tuple->len++;
}

static size_t ash_tuple_length(struct ash_tuple *tuple)
{
    return tuple->len;
}

static struct ash_obj *get(struct ash_tuple *tuple, size_t pos)
{
    if (pos < tuple->len)
        return tuple->tup[pos];
    return NULL;
}

static int set(struct ash_tuple *tuple, size_t pos,
                         struct ash_obj *value)
{
    if (pos > tuple->len)
        return -1;

    struct ash_obj *obj;
    if ((obj = tuple->tup[pos]))
        ash_obj_dec_rc(obj);

    tuple->tup[pos] = value;
    return 0;
}

struct ash_obj *
ash_tuple_from(size_t argc, struct ash_obj **argv)
{
    struct ash_obj *obj;
    struct ash_tuple *tuple;
    obj = ash_tuple_new();
    tuple = (struct ash_tuple *) obj;

    ash_tuple_resize(tuple, argc);

    for (int i = 0; i < argc; ++i)
        append(tuple, argv[i]);

    return obj;
}

void
ash_tuple_append(struct ash_obj *obj, struct ash_obj *value)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_tuple *tuple;
        tuple = (struct ash_tuple *) obj;
        append(tuple, value);
    }
}

void
ash_tuple_set(struct ash_obj *obj, size_t pos, struct ash_obj *value)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_tuple *tuple;
        tuple = (struct ash_tuple *) obj;
        set(tuple, pos, value);
    }
}

struct ash_obj *
ash_tuple_get(struct ash_obj *obj, size_t pos)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_tuple *tuple;
        tuple = (struct ash_tuple *) obj;
        return get(tuple, pos);
    }
    return NULL;
}
