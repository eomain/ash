
#include <stddef.h>

#include "ash/mem.h"
#include "ash/util/vec.h"

#define VEC_MAX_ALLOC 4096
#define VEC_DEFAULT_SIZE 8

struct vec {
    void **data;
    size_t length;
    size_t capacity;
};

struct vec *vec_from(size_t len)
{
    struct vec *vec;
    vec = ash_alloc(sizeof *vec);
    vec->data = ash_zalloc(len * sizeof *vec->data);
    vec->length = 0;
    vec->capacity = len;
    return vec;
}

struct vec *vec_new(void)
{
    return vec_from(VEC_DEFAULT_SIZE);
}

void vec_destroy(struct vec *vec)
{
    if (vec->data)
        ash_free(vec->data);
    ash_free(vec);
}

void *vec_get(struct vec *vec, size_t index)
{
    void *v = NULL;
    if (index < vec->length)
        v = vec->data[index];
    return v;
}

void **vec_get_ref(struct vec *vec)
{
    return vec->data;
}

void *vec_pop(struct vec *vec)
{
    size_t index;
    void *v = NULL;

    if (vec->length > 0) {
        index = (vec->length - 1);
        v = vec->data[index];
        vec->length--;
    }
    return v;
}

void vec_push(struct vec *vec, void *v)
{
    size_t index;
    index = vec->length;

    if (index >= vec->capacity) {
        size_t len;
        if ((vec->capacity * 2) < (VEC_MAX_ALLOC / 2))
            len = (vec->capacity * 2);
        else
            len = (vec->capacity + VEC_MAX_ALLOC);

        vec->data = ash_realloc(vec->data, len * sizeof *vec->data);
        vec->capacity = len;
    }

    vec->data[index] = v;
    vec->length++;
}

size_t vec_len(struct vec *vec)
{
    return vec->length;
}
