
#ifndef ASH_UTIL_RC_H
#define ASH_UTIL_RC_H

#include <stddef.h>

#include "ash/obj.h"

struct rc *rc;

extern struct rc *rc_new(void *, void (*)(void *));
extern void rc_destroy(struct rc *);
extern struct rc *rc_clone(struct rc *);
extern void *rc_get(struct rc *);
extern const size_t rc_count(struct rc *);

static inline struct rc *
rc_from_obj(struct ash_obj *obj)
{
    struct rc *rc;
    rc = rc_new(obj, (void (*)(void *))ash_obj_destroy);
    return rc;
}

#endif
