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
#include "ash/obj.h"
#include "ash/type/map.h"
#include "ash/util/hash.h"
#include "ash/util/map.h"

#define ASH_MAP_TYPENAME "map"
#define MAP_SIZE 32

struct ash_map {
    struct ash_obj obj;
    struct map *map;
};

static const char *name()
{
    return ASH_MAP_TYPENAME;
}

static struct ash_base base = {
    .iter = ash_base_iter_default,
    .name = name
};

struct ash_obj *ash_map_new(void)
{
    struct ash_map *map;
    struct ash_obj *obj;

    map = ash_alloc(sizeof *map);
    struct hashmeta meta;
    hash_meta_string_init(&meta, MAP_SIZE);
    map->map = map_new(meta);

    obj = (struct ash_obj *) map;
    ash_obj_init(obj, &base);
    return obj;
}

struct ash_obj *ash_map_get(struct ash_obj *obj, const char *key)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_map *map;
        map = (struct ash_map *) obj;
        return map_get(map->map, (key_t *)key);
    }
    return NULL;
}

void ash_map_insert(struct ash_obj *obj, const char *key,
                    struct ash_obj *value)
{
    if (ash_base_derived(&base, obj)) {
        struct ash_map *map;
        map = (struct ash_map *) obj;
        return map_insert(map->map, (key_t *)key, value);
    }
}
