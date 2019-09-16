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
#include "ash/util/rc.h"

struct pointer {
    void *object;
    size_t refs;
    void (*destroy)(void *);
};

static struct pointer *
pointer_new(void *object, void (*destroy)(void *))
{
    struct pointer *pointer;
    pointer = ash_alloc(sizeof *pointer);
    pointer->object = object;
    pointer->refs = 0;
    pointer->destroy = destroy;
    return pointer;
}

static void
pointer_destroy(struct pointer *pointer)
{
    if (pointer->destroy)
        pointer->destroy(pointer->object);
    ash_free(pointer);
}

static inline void *
pointer_get(struct pointer *pointer)
{
    return pointer->object;
}

static inline void
pointer_ref_inc(struct pointer *pointer)
{
    pointer->refs++;
}

static inline void
pointer_ref_dec(struct pointer *pointer)
{
    pointer->refs--;

    if (pointer->refs == 0)
        pointer_destroy(pointer);
}

static inline const size_t
pointer_refs(struct pointer *pointer)
{
    return pointer->refs;
}

struct rc {
    struct pointer *pointer;
};

struct rc *
rc_new(void *object, void (*destroy)(void *))
{
    struct pointer *pointer;
    pointer = pointer_new(object, destroy);
    pointer_ref_inc(pointer);

    struct rc *rc;
    rc = ash_alloc(sizeof *rc);
    rc->pointer = pointer;
    return rc;
}

void
rc_destroy(struct rc *rc)
{
    pointer_ref_dec(rc->pointer);
    ash_free(rc);
}

struct rc *
rc_clone(struct rc *r)
{
    struct rc *rc;
    rc = ash_alloc(sizeof *rc);
    rc->pointer = r->pointer;
    pointer_ref_inc(rc->pointer);
    return rc;
}

void *
rc_get(struct rc *rc)
{
    return pointer_get(rc->pointer);
}

const size_t
rc_count(struct rc *rc)
{
    return pointer_refs(rc->pointer);
}
