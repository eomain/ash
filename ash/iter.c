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


#include <assert.h>
#include <stddef.h>

#include "ash/iter.h"
#include "ash/obj.h"
#include "ash/type.h"

void ash_iter_init(struct ash_iter *iter, struct ash_obj *value)
{
    iter->pos = 0;
    iter->value = value;
}

struct ash_obj *ash_iter_next(struct ash_iter *iter)
{
    struct ash_obj *obj;
    if (!(obj = iter->value))
        return NULL;

    size_t pos;
    const struct ash_base *base;
    pos = iter->pos++;
    base = ash_obj_get_base(obj);
    assert(base != NULL);

    if (!base->iter)
        return NULL;

    iter->opt = base->iter(obj, pos);
    return option_get_value(&iter->opt);
}

bool ash_iter_hasnext(struct ash_iter *iter)
{
    struct ash_obj *obj;
    if (!(obj = iter->value))
        return false;

    size_t pos;
    const struct ash_base *base;
    pos = iter->pos;
    base = ash_obj_get_base(obj);
    assert(base != NULL);

    if (!base->iter)
        return false;

    iter->opt = base->iter(obj, pos);
    return option_is_some(&iter->opt);
}
