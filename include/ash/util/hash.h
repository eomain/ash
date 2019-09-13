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

#ifndef ASH_UTIL_HASH_H
#define ASH_UTIL_HASH_H

#include "ash/type.h"

typedef unsigned int hash_t;
typedef void key_t;

struct hashmeta {
    size_t size;
    bool (*eq)(key_t *, key_t *);
    hash_t (*hash)(key_t *, size_t);
};

static inline void
hash_meta_init(struct hashmeta *meta, size_t size,
               bool (*eq)(key_t *, key_t *),
               hash_t (*hash)(key_t *, size_t))
{
    meta->size = size;
    meta->eq = eq;
    meta->hash = hash;
}

extern bool ash_eq_string(key_t *a, key_t *b);
extern hash_t ash_hash_string(key_t *k, size_t size);

static inline void
hash_meta_string_init(struct hashmeta *meta, size_t size)
{
    hash_meta_init(meta, size, ash_eq_string, ash_hash_string);
}

#endif
