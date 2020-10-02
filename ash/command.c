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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ash/alias.h"
#include "ash/ash.h"
#include "ash/command.h"
#include "ash/cd.h"
#include "ash/defined.h"
#include "ash/echo.h"
#include "ash/env.h"
#include "ash/exec.h"
#include "ash/exit.h"
#include "ash/export.h"
#include "ash/help.h"
#include "ash/history.h"
#include "ash/io.h"
#include "ash/list.h"
#include "ash/ops.h"
#include "ash/rand.h"
#include "ash/read.h"
#include "ash/sleep.h"
#include "ash/source.h"
#include "ash/type.h"
#include "ash/typeof.h"
#include "ash/unset.h"
#include "ash/var.h"
#include "ash/lang/runtime.h"

struct ash_command {
    enum ash_command_name command;
    const char *name;
    int (*main) (int, const char * const *);
    int (*main_env) (int, const char * const *, struct ash_command_env *);
    const char *(*usage) (void);
};

static struct ash_command commands[ ASH_COMMAND_NO ] = {

    [ ASH_COMMAND_ALIAS ] = {
        .command = ASH_COMMAND_ALIAS,
        .name    = "alias",
        .main    = ash_alias,
        .usage   = ash_alias_usage
    },

    /*[ ASH_COMMAND_ASSERT ] = {
        .command = ASH_COMMAND_ASSERT,
        .name    = "assert",
        .main    = NULL,
        .usage   = NULL
    },*/

    [ ASH_COMMAND_CD ] = {
        .command = ASH_COMMAND_CD,
        .name    = "cd",
        .main    = ash_cd,
        .usage   = ash_cd_usage
    },

    [ ASH_COMMAND_DEFINED ] = {
        .command = ASH_COMMAND_DEFINED,
        .name    = "defined",
        .main    = NULL,
        .main_env = ash_defined_env,
        .usage   = ash_defined_usage
    },

    [ ASH_COMMAND_ECHO ] = {
        .command = ASH_COMMAND_ECHO,
        .name    = "echo",
        .main    = ash_echo,
        .usage   = ash_echo_usage
    },

    [ ASH_COMMAND_EXEC ] = {
        .command = ASH_COMMAND_EXEC,
        .name    = "exec",
        .main    = ash_exec,
        .usage   = ash_exec_usage
    },

    [ ASH_COMMAND_EXIT ] = {
        .command = ASH_COMMAND_EXIT,
        .name    = "exit",
        .main    = ash_exit,
        .usage   = ash_exit_usage
    },

    [ ASH_COMMAND_EXPORT ] = {
        .command = ASH_COMMAND_EXPORT,
        .name    = "export",
        .main    = NULL,
        .main_env = ash_export_env,
        .usage   = ash_export_usage
    },

    [ ASH_COMMAND_HELP ] = {
        .command = ASH_COMMAND_HELP,
        .name    = "help",
        .main    = ash_help,
        .usage   = ash_help_usage
    },

    [ ASH_COMMAND_HISTORY ] = {
        .command = ASH_COMMAND_HISTORY,
        .name    = "history",
        .main    = ash_history,
        .usage   = ash_history_usage
    },

    [ ASH_COMMAND_LIST ] = {
        .command = ASH_COMMAND_LIST,
        .name    = "list",
        .main    = ash_list,
        .usage   = ash_list_usage
    },

    [ ASH_COMMAND_RAND ] = {
        .command = ASH_COMMAND_RAND,
        .name    = "rand",
        .main    = NULL,
        .main_env = ash_rand_env,
        .usage   = ash_rand_usage
    },

    [ ASH_COMMAND_READ ] = {
        .command = ASH_COMMAND_READ,
        .name    = "read",
        .main    = NULL,
        .main_env = ash_read_env,
        .usage   = ash_read_usage
    },

    [ ASH_COMMAND_SLEEP ] = {
        .command = ASH_COMMAND_SLEEP,
        .name    = "sleep",
        .main    = ash_sleep,
        .usage   = ash_sleep_usage
    },

    [ ASH_COMMAND_SOURCE ] = {
        .command = ASH_COMMAND_SOURCE,
        .name    = "source",
        .main    = NULL,
        .main_env = ash_source_env,
        .usage   = ash_source_usage
    },

    [ ASH_COMMAND_TYPEOF ] = {
        .command = ASH_COMMAND_TYPEOF,
        .name    = "typeof",
        .main    = NULL,
        .main_env    = ash_typeof_env,
        .usage   = ash_typeof_usage
    },

    [ ASH_COMMAND_UNSET ] = {
        .command = ASH_COMMAND_UNSET,
        .name    = "unset",
        .main    = NULL,
        .main_env = ash_unset_env,
        .usage   = ash_unset_usage
    }

};

int ash_command_exec(enum ash_command_name command,
                     int argc, const char *const *argv,
                     struct ash_command_env *env)
{
    int status;
    assert(command < ASH_COMMAND_NO);
    if (commands[command].main)
        status = commands[command].main(argc, argv);
    else if (commands[command].main_env)
        status = commands[command].main_env(argc, argv, env);
    else
        status = ASH_STATUS_ERR;
    return status;
}

const char *ash_command_name(enum ash_command_name command)
{
    return commands[command].name;
}

void ash_command_usage(enum ash_command_name command)
{
    assert(command < ASH_COMMAND_NO);
    if (commands[command].usage) {
        ash_print("%s",commands[command].usage());
    }
}

enum ash_command_name ash_command_find(const char *v)
{
    switch (v[0]) {
        case 'a':
            if (v[1] == 'l' &&
                v[2] == 'i' &&
                v[3] == 'a' &&
                v[4] == 's' &&
                !v[5])
                return ASH_COMMAND_ALIAS;
            break;

        case 'c':
            if (v[1] == 'd' &&
                !(v[2]))
                return ASH_COMMAND_CD;
            break;

        case 'd':
            if (v[1] == 'e' &&
                v[2] == 'f' &&
                v[3] == 'i' &&
                v[4] == 'n' &&
                v[5] == 'e' &&
                v[6] == 'd' &&
                !v[7])
                return ASH_COMMAND_DEFINED;
            break;

        case 'e':
            if (v[1] == 'x') {
                if (v[2] == 'e' &&
                    v[3] == 'c' &&
                    !v[4])
                    return ASH_COMMAND_EXEC;

                else if (v[2] == 'i' &&
                         v[3] == 't' &&
                         !(v[4]))
                    return ASH_COMMAND_EXIT;

                else if (v[2] == 'p' &&
                         v[3] == 'o' &&
                         v[4] == 'r' &&
                         v[5] == 't' &&
                         !v[6])
                    return ASH_COMMAND_EXPORT;
            }
            else if (v[1] == 'c' &&
                     v[2] == 'h' &&
                     v[3] == 'o' &&
                     !(v[4]))
                return ASH_COMMAND_ECHO;
            break;

        case 'h':
            if (v[1] == 'e' &&
                v[2] == 'l' &&
                v[3] == 'p' &&
                !(v[4]))
                return ASH_COMMAND_HELP;
            else if (v[1] == 'i' &&
                     v[2] == 's' &&
                     v[3] == 't' &&
                     v[4] == 'o' &&
                     v[5] == 'r' &&
                     v[6] == 'y' &&
                     !v[7])
                return ASH_COMMAND_HISTORY;
            break;

        case 'l':
            if (v[1] == 'i' &&
                v[2] == 's' &&
                v[3] == 't')
                return ASH_COMMAND_LIST;
            break;

        case 'r':
            if (v[1] == 'a' &&
                v[2] == 'n' &&
                v[3] == 'd' &&
                !v[4])
                return ASH_COMMAND_RAND;
            else if (v[1] == 'e' &&
                     v[2] == 'a' &&
                     v[3] == 'd' &&
                     !v[4])
                return ASH_COMMAND_READ;
            break;

        case 's':
            if (v[1] == 'l' &&
                v[2] == 'e' &&
                v[3] == 'e' &&
                v[4] == 'p' &&
                !(v[5]))
                return ASH_COMMAND_SLEEP;
            else if (v[1] == 'o' &&
                     v[2] == 'u' &&
                     v[3] == 'r' &&
                     v[4] == 'c' &&
                     v[5] == 'e' &&
                     !v[6])
                return ASH_COMMAND_SOURCE;
            break;

        case 't':
            if (v[1] == 'y' &&
                v[2] == 'p' &&
                v[3] == 'e' &&
                v[4] == 'o' &&
                v[5] == 'f' &&
                !v[6])
                return ASH_COMMAND_TYPEOF;
            break;

        case 'u':
            if (v[1] == 'n' &&
                v[2] == 's' &&
                v[3] == 'e' &&
                v[4] == 't' &&
                !v[5])
                return ASH_COMMAND_UNSET;
    }

    return ASH_ERR_COMMAND;
}
