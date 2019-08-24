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

#ifndef ASH_TYPE_H
#define ASH_TYPE_H

#include <stddef.h>
#include <stdint.h>

#include "ash/ash.h"

typedef unsigned long usize;
typedef signed long isize;
typedef unsigned char ubyte;

typedef ubyte bool;

typedef uint32_t uchar;

#define true  1
#define false 0

struct option {
    enum option_variant {
        SOME,
        NONE
    } variant;

    void *value;
};

static inline void
option_some(struct option *opt, void *value)
{
    opt->variant = SOME;
    opt->value = value;
}

static inline void
option_none(struct option *opt)
{
    opt->variant = NONE;
    opt->value = NULL;
}

static inline void *
option_get_value(struct option *opt)
{
    return opt->value;
}

static inline bool
option_is_some(struct option *opt)
{
    return opt->variant == SOME;
}

#endif
