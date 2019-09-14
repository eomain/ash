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

void vec_append(struct vec *vec, struct vec *v)
{
    size_t len = vec_len(v);

    for (size_t i = 0; i < len; ++i)
        vec_push(vec, vec_get(v, i));
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

void *vec_set(struct vec *vec, size_t index, void *v)
{
    void *n = NULL;
    if (index < vec->length) {
        n = vec->data[index];
        vec->data[index] = v;
    }
    return n;
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

void vec_for_each(struct vec *vec, void (*func)(void *))
{
    size_t len = vec_len(vec);

    for (size_t i = 0; i < len; ++i)
        func(vec->data[i]);
}

struct vec *vec_map(struct vec *vec, void *(*func)(void *))
{
    size_t len;
    struct vec *v;

    len = vec_len(vec);
    v = vec_from(len);

    for (size_t i = 0; i < len; ++i)
        vec_push(v, func(vec->data[i]));

    return v;
}
