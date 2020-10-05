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

#ifndef ASH_OBJ_H
#define ASH_OBJ_H

#include "ash/macro.h"
#include "ash/type.h"

#define ash_obj_set_dec_rc(objs) \
    for (size_t i = 0; i < array_length(objs); ++i) { \
        if (objs[i]) \
            ash_obj_dec_rc(objs[i]); \
    }

struct ash_obj;

struct ash_base_ops {
    struct ash_obj * (*nt) (struct ash_obj *);
    struct ash_obj * (*sb) (struct ash_obj *);

    struct ash_obj * (*eq) (struct ash_obj *, struct ash_obj *);
    struct ash_obj * (*ne) (struct ash_obj *, struct ash_obj *);
    struct ash_obj * (*gt) (struct ash_obj *, struct ash_obj *);
    struct ash_obj * (*lt) (struct ash_obj *, struct ash_obj *);

    struct ash_obj * (*and)(struct ash_obj *, struct ash_obj *);
    struct ash_obj * (*or) (struct ash_obj *, struct ash_obj *);

    struct ash_obj * (*add) (struct ash_obj *, struct ash_obj *);
    struct ash_obj * (*sub) (struct ash_obj *, struct ash_obj *);
    struct ash_obj * (*mul) (struct ash_obj *, struct ash_obj *);
    struct ash_obj * (*div) (struct ash_obj *, struct ash_obj *);
    struct ash_obj * (*mod) (struct ash_obj *, struct ash_obj *);
};

struct ash_base_into {
    struct ash_obj * (*boolean) (struct ash_obj *);
    struct ash_obj * (*string)  (struct ash_obj *);
};

struct ash_base_util {
    bool (*match) (struct ash_obj *, struct ash_obj *);
};

struct ash_base {
    struct ash_base_ops ops;
    struct ash_base_into into;
    struct ash_base_util util;
    struct option (*iter) (struct ash_obj *, size_t);
    void (*dealloc) (struct ash_obj *);
    const char * (*name) ();
    struct ash_obj * (*clone) (struct ash_obj *);
};

extern struct option ash_base_iter_default(struct ash_obj *, size_t);

extern bool ash_base_derived(struct ash_base *, struct ash_obj *);

struct ash_obj {
    struct ash_base *base;
    struct ash_obj *ref;
    bool bound;
    bool mutable;
    usize rc;
    struct ash_obj *string;
};

extern void ash_obj_init(struct ash_obj *, struct ash_base *);
extern void ash_obj_destroy(struct ash_obj *);
extern struct ash_obj *ash_obj_ref(struct ash_obj *);
extern const struct ash_base *ash_obj_get_base(struct ash_obj *);
extern const struct ash_base_ops *ash_obj_get_ops(struct ash_obj *);
extern const struct ash_base_into *ash_obj_get_into(struct ash_obj *);

extern const char *ash_obj_name(struct ash_obj *);
extern struct ash_obj *ash_obj_str(struct ash_obj *);
extern struct ash_obj *ash_obj_bool(struct ash_obj *);
extern bool ash_obj_match(struct ash_obj *, struct ash_obj *);

extern bool ash_obj_iterable(struct ash_obj *);
extern bool ash_obj_nil(struct ash_obj *);
extern bool ash_obj_eq(struct ash_obj *, struct ash_obj *);
extern bool ash_obj_type_eq(struct ash_obj *, struct ash_obj *);
extern bool ash_obj_has_bind(struct ash_obj *);
extern void ash_obj_inc_rc(struct ash_obj *);
extern void ash_obj_dec_rc(struct ash_obj *);
extern usize ash_obj_get_rc(struct ash_obj *);
extern bool ash_obj_is_ref(struct ash_obj *);

#endif
