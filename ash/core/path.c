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

#include <dirent.h>

#include "ash/mem.h"
#include "ash/unit.h"
#include "ash/util/map.h"
#include "ash/util/queue.h"
#include "ash/util/vec.h"

#define PATH_INIT_LENGTH 256
#define PATH_MAX_LENGTH 4096
#define PATH_MAP_SIZE PATH_INIT_LENGTH
#define PATH_QUEUE_SIZE 4096

struct path {
    struct map *map;
    struct vec *paths;
    struct queue *recent;
};

static struct path path = {
    .map = NULL,
    .paths = NULL,
    .recent = NULL
};

void ash_path_cache(const char *command, const char *directory)
{
    if ((map_get(path.map, (key_t *)command)))
        return;

    if (queue_len(path.recent) == PATH_MAX_LENGTH) {
        const char *cmd;
        cmd = queue_dequeue(path.recent);
        map_remove(path.map, (key_t *) cmd);
    }

    queue_enqueue(path.recent, (key_t *)command);
    map_insert(path.map, (key_t *)command, (char *)directory);
}

const char *ash_path_find(const char *command)
{
    return map_get(path.map, (key_t *)command);
}

static void path_set(const char *directory)
{
    vec_push(path.paths, (char *)directory);
}

static void init(void)
{
    struct hashmeta meta;
    hash_meta_string_init(&meta, PATH_MAP_SIZE);
    path.map = map_new(meta);
    path.paths = vec_from(PATH_INIT_LENGTH);
    path.recent = queue_from(PATH_QUEUE_SIZE);
}
