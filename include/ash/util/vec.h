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

#ifndef ASH_UTIL_VEC_H
#define ASH_UTIL_VEC_H

#include <stddef.h>

struct vec *vec;

extern struct vec *vec_from(size_t);
extern struct vec *vec_new(void);
extern void vec_destroy(struct vec *);
extern void vec_append(struct vec *, struct vec *);
extern void *vec_get(struct vec *, size_t);
extern void **vec_get_ref(struct vec *);
extern void *vec_set(struct vec *, size_t, void *);
extern void *vec_pop(struct vec *);
extern void vec_push(struct vec *, void *);
extern size_t vec_len(struct vec *);
extern void vec_for_each(struct vec *, void (*)(void *));
extern struct vec *vec_map(struct vec *, void *(*)(void *));

#endif
