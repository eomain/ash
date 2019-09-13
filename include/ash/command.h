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

#ifndef ASH_COMMAND_H
#define ASH_COMMAND_H

#include "ash/obj.h"
#include "ash/lang/runtime.h"

struct ash_command_env {
    struct ash_runtime_env *env;
    struct ash_obj *result;
};

static inline void
ash_command_env_init(struct ash_command_env *env,
                     struct ash_runtime_env *renv)
{
    env->env = renv;
    env->result = NULL;
}

static inline void
ash_command_env_set_result(struct ash_command_env *env,
                    struct ash_obj *obj)
{
    env->result = obj;
}

static inline struct ash_obj *
ash_command_env_get_result(struct ash_command_env *env)
{
    return env->result;
}

/* All ash built-in commands */
enum ash_command_name {
    ASH_ERR_COMMAND = -1,
    ASH_COMMAND_ALIAS,
    ASH_COMMAND_ASSERT,
    ASH_COMMAND_BUILTIN,
    ASH_COMMAND_CD,
    ASH_COMMAND_DEFINED,
    ASH_COMMAND_ECHO,
    ASH_COMMAND_EXEC,
    ASH_COMMAND_EXIT,
    ASH_COMMAND_EXPORT,
    ASH_COMMAND_HELP,
    ASH_COMMAND_HISTORY,
    ASH_COMMAND_RAND,
    ASH_COMMAND_READ,
    ASH_COMMAND_SLEEP,
    ASH_COMMAND_SOURCE,
    ASH_COMMAND_TYPEOF,
    ASH_COMMAND_UNSET,

    /* number of ash builtin commands */
    ASH_COMMAND_NO
};

static inline bool
ash_command_valid(enum ash_command_name command)
{
    return (command != ASH_ERR_COMMAND);
}

extern int
ash_command_exec(enum ash_command_name,
                 int, const char *const *,
                 struct ash_command_env *);

extern void ash_command_usage(enum ash_command_name);

extern enum ash_command_name ash_command_find(const char *);

#endif
