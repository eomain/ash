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
#include <string.h>

#include "ash/alias.h"
#include "ash/ash.h"
#include "ash/ops.h"
#include "ash/unit.h"
#include "ash/util/hash.h"
#include "ash/util/map.h"

#define ALIAS_SIZE 1024

static const char *USAGE =
    "alias:\n"
    "    assign a new name to a command\n"
    "usage:\n"
    "    alias <COMMAND> <NAME>\n";

const char *ash_alias_usage(void)
{
    return USAGE;
}

int ash_alias(int argc, const char * const *argv)
{
    if (argc < 3)
        return ASH_STATUS_ERR;

    if (strcmp(argv[1], argv[2]) == 0) {
        ash_alias_unset(argv[1]);
        return ASH_STATUS_OK;
    }

    const char *name, *alias;
    name = ash_strcpy(argv[1]);
    alias = ash_strcpy(argv[2]);

    ash_alias_set(alias, name);

    return ASH_STATUS_OK;
}

struct ash_alias {
    struct map *map;
};

static struct ash_alias alias = {
    .map = NULL
};

const char *ash_alias_get(const char *a)
{
    const char *name;
    name = map_get(alias.map, (key_t *)a);
    return name;
}

void ash_alias_set(const char *a, const char *name)
{
    return map_insert(alias.map, (key_t *)a, (void *)name);
}

void ash_alias_unset(const char *a)
{
    map_remove(alias.map, (key_t *)a);
}

static void init(void)
{
    struct hashmeta meta;
    hash_meta_string_init(&meta, ALIAS_SIZE);
    alias.map = map_new(meta);
}

const struct ash_unit_module ash_module_alias = {
    .init = init,
    .destroy = NULL
};
