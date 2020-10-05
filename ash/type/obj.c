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
#include "ash/mem.h"
#include "ash/obj.h"

#define ASH_OBJ_NIL "(nil)"

#define ASH_OBJ_REF_ZERO (0)
#define ASH_OBJ_REF_INIT (1)
#define ASH_OBJ_REF_MAX  (255)

struct option ash_base_iter_default(struct ash_obj *obj, size_t pos)
{
    struct option opt;
    if (pos == 0)
        option_some(&opt, obj);
    else
        option_none(&opt);

    return opt;
}

bool ash_base_derived(struct ash_base *base, struct ash_obj *obj)
{
    return (ash_obj_get_base(obj) == base);
}

void
ash_obj_init(struct ash_obj *obj, struct ash_base *base)
{
    obj->base = base;
    obj->ref = NULL;
    obj->bound = false;
    obj->mutable = true;
    obj->rc = ASH_OBJ_REF_INIT;
    obj->string = NULL;
}

void ash_obj_destroy(struct ash_obj *obj)
{
    assert(obj != NULL);
    if (obj->string)
        ash_free(obj->string);
    ash_free(obj);
}

const struct ash_base *
ash_obj_get_base(struct ash_obj *obj)
{
    return (obj) ? obj->base: NULL;
}

const struct ash_base_ops *
ash_obj_get_ops(struct ash_obj *obj)
{
    const struct ash_base *base;
    base = ash_obj_get_base(obj);
    return (base) ? &base->ops: NULL;
}

const struct ash_base_into *
ash_obj_get_into(struct ash_obj *obj)
{
    const struct ash_base *base;
    base = ash_obj_get_base(obj);
    return (base) ? &base->into: NULL;
}

const char *
ash_obj_name(struct ash_obj *obj)
{
    if (!obj || !obj->base)
        return ASH_OBJ_NIL;
    return obj->base->name();
}

struct ash_obj *
ash_obj_str(struct ash_obj *obj)
{
    if (obj && obj->base) {
        if (obj->base->into.string)
            return obj->base->into.string(obj);
    }
    return NULL;
}

struct ash_obj *
ash_obj_bool(struct ash_obj *obj)
{
    if (obj && obj->base) {
        if (obj->base->into.boolean)
            return obj->base->into.boolean(obj);
    }
    return NULL;
}

static bool
ash_obj_match_default(struct ash_obj *obj, struct ash_obj *m)
{
    if (!ash_obj_type_eq(obj, m))
        return false;

    const struct ash_base_ops *ops;
    if ((ops = ash_obj_get_ops(obj))) {
        bool value;
        struct ash_obj *o;
        if (ops->eq) {
            o = ops->eq(obj, m);
            value = ash_bool_get(o);
            ash_obj_dec_rc(o);
            return value;
        }
    }
    return false;
}

bool
ash_obj_match(struct ash_obj *obj, struct ash_obj *m)
{
    if (obj && obj->base) {
        if (obj->base->util.match)
            return obj->base->util.match(obj, m);
        else
            return ash_obj_match_default(obj, m);
    }
    return false;
}

inline bool
ash_obj_nil(struct ash_obj *obj)
{
    return (obj == NULL) ? true: false;
}

bool ash_obj_mutable(struct ash_obj *obj)
{
    return (obj == NULL) ? false: obj->mutable;
}

inline bool
ash_obj_eq(struct ash_obj *a, struct ash_obj *b)
{
    return (a == b) || (a->ref == b->ref) ? true: false;
}

inline bool
ash_obj_type_eq(struct ash_obj *a, struct ash_obj *b)
{
    return (a->base == b->base) ? true: false;
}

static inline bool
ash_obj_zero_rc(struct ash_obj *obj)
{
    return (obj->rc == ASH_OBJ_REF_ZERO && !ash_obj_has_bind(obj)) ?
        true: false;
}

static inline bool
ash_obj_max_rc(struct ash_obj *obj)
{
    return (obj->rc == ASH_OBJ_REF_MAX) ? true: false;
}

void
ash_obj_inc_rc(struct ash_obj *obj)
{
    if (!ash_obj_nil(obj)) {
        if (!ash_obj_max_rc(obj))
            obj->rc++;
    }
}

void
ash_obj_dec_rc(struct ash_obj *obj)
{
    if (!ash_obj_nil(obj)) {
        if (obj->rc > 0) {
            obj->rc--;

            if (ash_obj_zero_rc(obj)) {
                if (obj->base->dealloc)
                    obj->base->dealloc(obj);
                ash_obj_destroy(obj);
            }
        }
    }
}

bool ash_obj_has_bind(struct ash_obj *obj)
{
    if (ash_obj_nil(obj))
        return false;

    return obj->bound;
}

usize
ash_obj_get_rc(struct ash_obj *obj)
{
    if (!ash_obj_nil(obj))
        return obj->rc;

    return 0;
}

static bool
ash_obj_has_rc(struct ash_obj *obj)
{
    if (!ash_obj_nil(obj))
        return obj->rc == 0 ? false: true;

    return false;
}

bool
ash_obj_is_ref(struct ash_obj *obj)
{
    if (!ash_obj_nil(obj))
        return obj->ref == NULL ? false: true;

    return false;
}

struct ash_obj *
ash_obj_ref(struct ash_obj *obj)
{
    ash_obj_inc_rc(obj);
    return obj;
}
