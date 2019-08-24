
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
