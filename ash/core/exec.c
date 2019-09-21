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
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ash/alias.h"
#include "ash/ash.h"
#include "ash/command.h"
#include "ash/env.h"
#include "ash/exec.h"
#include "ash/int.h"
#include "ash/io.h"
#include "ash/macro.h"
#include "ash/obj.h"
#include "ash/signal.h"
#include "ash/str.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/var.h"
#include "ash/lang/runtime.h"
#include "ash/util/vec.h"

#ifdef ASH_PLATFORM_POSIX
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#define ASH_FD_STDIN  0
#define ASH_FD_STDOUT 1

#define ASH_EXIT_SUCCESS EXIT_SUCCESS
#define ASH_EXIT_FAILURE EXIT_FAILURE
#define ASH_EXIT_DEFAULT ASH_EXIT_SUCCESS

struct ash_exec_env {
    struct ash_obj *exit;
    struct ash_var *vexit;
    struct ash_var *result;
};

static struct ash_exec_env env;

static void
ash_exec_env_exit(struct ash_exec_env *env, int status)
{
    ash_int_set(env->exit, status);
    ash_var_bind(env->vexit, env->exit);
}

static void
ash_exec_env_result(struct ash_exec_env *env, struct ash_obj *obj)
{
    if (!obj) {
        if (env->result)
            ash_var_unbind(env->result);
        return;
    }

    if (env->result)
        ash_var_bind_override(env->result, obj);
    else
        env->result = ash_var_set(ASH_SYMBOL_RESULT, obj);
}

static void
ash_exec_env_init(struct ash_exec_env *env)
{
    env->exit = ash_int_from(ASH_EXIT_DEFAULT);
    env->vexit = ash_var_set(ASH_SYMBOL_EXIT, env->exit);
    env->result = NULL;
}

static void ash_print_err_command(const char *command, const char *msg)
{
    ash_print(PNAME ": %s: %s \n", command, msg);
}

static inline void
ash_exec_child(int input, int output,
               const char *prog, char *const argv[])
{
    if (input != ASH_FD_STDIN) {
        dup2(input, ASH_FD_STDIN);
        close(input);
    }

    if (output != ASH_FD_STDOUT) {
        dup2(output, ASH_FD_STDOUT);
        close(output);
    }

    if (execvp(prog, argv) == -1) {
        const char *msg = (errno == ENOENT) ?
            "Unrecognized command": strerror(errno);
        ash_print_err_command(prog, msg);
    }
    _exit(0);
}

static inline int
ash_exec_wait(const char *prog)
{
    int status;
    wait(&status);
    if (WIFSIGNALED(status))
        ash_print("%s %s\n", ash_signal_get(WTERMSIG(status)), prog);

    status = WEXITSTATUS(status);
    ash_exec_env_result(&env, NULL);
    ash_exec_env_exit(&env, status);
    return status;
}

static int
ash_exec_process(int input, int output,
               const char *prog, char *const argv[])
{
    pid_t pid;
    int status = ASH_EXIT_DEFAULT;

    pid = fork();
    if (pid == -1) {
        ash_print_err("unable to fork process!");
        return -1;
    } else if (pid == 0) {
        ash_exec_child(input, output, prog, argv);
    } else {
        status = ash_exec_wait(argv[0]);
    }

    return status;
}

enum ash_exec_io {
    ASH_STDIO = 0,
    ASH_IN    = 1 << 1,
    ASH_OUT   = 1 << 2
};

static int
ash_exec_builtin(int input, int output,
                 enum ash_command_name command,
                 int argc, char *const *argv)
{
    int status;
    int in, out;
    enum ash_exec_io io = ASH_STDIO;
    struct ash_command_env env;

    if (input != ASH_FD_STDIN) {
        in = dup(ASH_FD_STDIN);
        io |= ASH_IN;
        dup2(input, ASH_FD_STDIN);
        close(input);
    }

    if (output != ASH_FD_STDOUT) {
        out = dup(ASH_FD_STDOUT);
        io |= ASH_OUT;
        dup2(output, ASH_FD_STDOUT);
        close(output);
    }

    ash_command_env_init(&env, NULL);
    status = ash_command_exec(
        command, argc, (const char * const *)argv, &env
    );

    if (io & ASH_IN)
        dup2(in, ASH_FD_STDIN);

    if (io & ASH_OUT)
        dup2(out, ASH_FD_STDOUT);

    return status;
}

int ash_exec_pipeline(int len, struct ash_exec **progs)
{
    int fd[2];
    int input = ASH_FD_STDIN;
    int argc;
    const char *prog = NULL;
    char *const *argv = NULL;

    enum ash_command_name command;

    if (len > 1) {
        for (int i = 0; i < len - 1; ++i) {
            if (pipe(fd) == -1) {
                ash_print_err("unable to open pipe!");
                return -1;
            }

            prog = progs[i]->argv[0];
            argv = progs[i]->argv;

            command = ash_command_find(prog);

            if (ash_command_valid(command)) {
                argc = progs[i]->argc;
                ash_exec_builtin(input, fd[1], command, argc, argv);
            } else {
                ash_exec_process(input, fd[1], prog, argv);
            }

            close(fd[1]);

            input = fd[0];
        }
    }

    const int index = len -1;

    prog = progs[index]->argv[0];
    argv = progs[index]->argv;

    if ((command = ash_command_find(prog)) != ASH_ERR_COMMAND) {
        argc = progs[index]->argc;
        return ash_exec_builtin(input, ASH_FD_STDOUT, command, argc, argv);
    } else
        return ash_exec_process(input, ASH_FD_STDOUT, prog, argv);
}

void ash_exec_command_status(int status, struct ash_command_env *cenv)
{
    struct ash_obj *result;
    result = ash_command_env_get_result(cenv);
    ash_exec_env_result(&env, result);
    ash_exec_env_exit(&env, status);
}

int ash_exec_command(struct vec *vec, struct ash_runtime_env *renv)
{
    int argc, status;
    const char *name, *alias;
    const char **argv;
    enum ash_command_name command;
    struct ash_command_env env;
    ash_command_env_init(&env, renv);

    name = vec_get(vec, 0);
    if ((alias = ash_alias_get(name))) {
        name = alias;
        alias = vec_set(vec, 0, (char *)name);
    }
    command = ash_command_find(name);

    if (ash_command_valid(command)) {
        argc = vec_len(vec);
        argv = (const char **) vec_get_ref(vec);
        status = ash_command_exec(command, argc, argv, &env);
        ash_exec_command_status(status, &env);
    } else {
        vec_push(vec, NULL);
        argv = (const char **) vec_get_ref(vec);
        status = ash_exec_process(ASH_FD_STDIN, ASH_FD_STDOUT,
                                name, (char *const*)argv);
    }

    if (alias)
        vec_set(vec, 0, (char *)alias);

    return status;
}

static void init(void)
{
    ash_exec_env_init(&env);
}

const struct ash_unit_module ash_module_exec = {
    .init = init,
    .destroy = NULL
};
