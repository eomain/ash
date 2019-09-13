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

#include <assert.h>
#include <stddef.h>

#include "ash/mem.h"
#include "ash/type.h"
#include "ash/util/hash.h"
#include "ash/util/map.h"

#define MAP_LIST_LENGTH 8

struct entry {
    key_t *key;
    void *value;
    struct entry *next;
};

static struct entry *
entry_new(key_t *key, void *value)
{
    struct entry *entry;
    entry = ash_alloc(sizeof *entry);
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static void *
entry_destroy(struct entry *entry)
{
    void *v;
    v = entry->value;
    assert(entry->next == NULL);
    ash_free(entry);
    return v;
}

struct hashmap {
    size_t size;
    struct entry **entries;
    struct hashmap *next;
};

static struct hashmap *
hash_map_new(size_t size)
{
    struct hashmap *hashmap;
    hashmap = ash_alloc(sizeof *hashmap);
    hashmap->size = size;
    hashmap->entries = ash_zalloc(size * sizeof *hashmap->entries);
    hashmap->next = NULL;
    return hashmap;
}

static inline struct hashmap *
hash_map_next(struct hashmap *hashmap)
{
    if (!hashmap->next)
        hashmap->next = hash_map_new(hashmap->size);
    return hashmap->next;
}

static inline size_t
hash_map_size(struct hashmap *hashmap)
{
    return hashmap->size;
}

static bool
hash_map_insert(struct hashmap *hashmap, hash_t hash,
                struct entry *entry, bool (*eq)(key_t *, key_t *))
{
    struct entry *e;
    if (!(e = hashmap->entries[hash])) {
        hashmap->entries[hash] = entry;
        return true;
    }

    size_t limit = 0;

    do {
        if (eq(e->key, entry->key))
            break;
        limit++;
    } while ((e = e->next));

    if (limit == 0)
        hashmap->entries[hash] = entry;
    else if (e && limit <= MAP_LIST_LENGTH)
        e->next = entry;
    else
        return false;
    return true;
}

static struct entry *
hash_map_get(struct hashmap *hashmap, hash_t hash, key_t *key,
             bool (*eq)(key_t *, key_t *))
{
    struct entry *e;
    if ((e = hashmap->entries[hash])) {
        do {
            if (eq(e->key, key))
                return e;
        } while ((e = e->next));
    }
    return NULL;
}

static struct entry *
hash_map_remove(struct hashmap *hashmap, hash_t hash, key_t *key,
                bool (*eq)(key_t *, key_t *))
{
    struct entry *e, *p = NULL;
    if (!(e = hashmap->entries[hash]))
        return NULL;

    size_t limit = 0;

    do {
        if (eq(e->key, key))
            break;
        limit++;
        p = e;
    } while ((e = e->next));

    if (limit == 0)
        hashmap->entries[hash] = NULL;
    else if (e && e->next && p)
        p->next = e->next;
    return e;
}

static void
hash_map_destroy(struct hashmap *hashmap)
{
    ash_free(hashmap);
}

struct map {
    struct hashmap *map;
    struct hashmeta meta;
};

struct map *map_new(struct hashmeta meta)
{
    struct map *map;
    map = ash_alloc(sizeof *map);
    map->map = hash_map_new(meta.size);
    map->meta = meta;
    return map;
}

void map_destroy(struct map *map)
{
    hash_map_destroy(map->map);
    ash_free(map);
}

void map_insert(struct map *map, key_t *k, void *v)
{
    hash_t hash;
    struct hashmap *hashmap;
    struct entry *entry;

    hash = map->meta.hash(k, map->meta.size);
    hashmap = map->map;
    entry = entry_new(k, v);

    while (!hash_map_insert(hashmap, hash, entry, map->meta.eq))
        hashmap = hash_map_next(hashmap);
}

void *map_get(struct map *map, key_t *k)
{
    hash_t hash;
    void *v = NULL;
    struct hashmap *hashmap;
    struct entry *entry;

    hash = map->meta.hash(k, map->meta.size);
    hashmap = map->map;

    while (!(entry = hash_map_get(hashmap, hash, k, map->meta.eq))) {
        if (!(hashmap = hashmap->next))
            break;
    }

    if (entry)
        v = entry->value;

    return v;
}

void *map_remove(struct map *map, key_t *k)
{
    hash_t hash;
    void *v = NULL;
    struct hashmap *hashmap;
    struct entry *entry;

    hash = map->meta.hash(k, map->meta.size);
    hashmap = map->map;

    while (!(entry = hash_map_remove(hashmap, hash, k, map->meta.eq))) {
        if (!(hashmap = hashmap->next))
            break;
    }

    if (entry)
        v = entry_destroy(entry);

    return v;
}
