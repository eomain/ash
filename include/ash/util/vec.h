
#ifndef ASH_UTIL_VEC_H
#define ASH_UTIL_VEC_H

#include <stddef.h>

struct vec *vec;

extern struct vec *vec_from(size_t);
extern struct vec *vec_new(void);
extern void vec_destroy(struct vec *);
extern void *vec_get(struct vec *, size_t);
extern void **vec_get_ref(struct vec *);
extern void *vec_pop(struct vec *);
extern void vec_push(struct vec *, void *);
extern size_t vec_len(struct vec *);

#endif
