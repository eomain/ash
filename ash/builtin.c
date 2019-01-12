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

#include "ash.h"
#include "builtin.h"
#include "cd.h"
#include "echo.h"
#include "env.h"
#include "exec.h"
#include "io.h"
#include "sleep.h"
#include "unset.h"
#include "var.h"

static const char *builtin_msg[] = {
    [  ASH_BUILTIN_BUILTIN ]    =   "list built-in commands",
    [  ASH_BUILTIN_EXIT    ]    =   "exit shell session",
    [  ASH_BUILTIN_HELP    ]    =   "display help prompt"
};

static const char *ash_builtin_usage(void)
{
    return builtin_msg[ASH_BUILTIN_BUILTIN];
}

static const char *ash_exit_usage(void)
{
    return builtin_msg[ASH_BUILTIN_EXIT];
}

static const char *ash_help_usage(void)
{
    return builtin_msg[ASH_BUILTIN_HELP];
}

static void ash_print_builtin(void);
static void ash_print_builtin_info(int, const char *);

static int ash_builtin(int argc, const char * const *argv)
{
    int status;

    if (argc == 1)
        ash_print_builtin();
    else {
        if ((status = ash_builtin_find(argv[1])) != -1)
            ash_print_builtin_info(status, argv[1]);
        else
            ash_print_err_builtin(argv[1], perr(UREG_CMD_ERR));
    }
    return 0;
}

static int ash_exit(int argc, const char * const *argv)
{
    ash_puts(argv[0]);
    ash_logout();
    return 0;
}

static int ash_help(int argc, const char * const *argv)
{
    ash_print_help();
    return 0;
}

struct {
    int builtin;
    const char *name;
    int (*main) (int, const char * const *);
    const char *(*usage) (void);
}

static usage[ ASH_BUILTIN_NO ] = {

    [ ASH_BUILTIN_ASSERT ] = {
        .builtin = ASH_BUILTIN_ASSERT,
        .name    = "assert",
        .main    = NULL,
        .usage   = NULL
    },

    [ ASH_BUILTIN_BUILTIN ] = {
        .builtin = ASH_BUILTIN_BUILTIN,
        .name    = "builtin",
        .main    = ash_builtin,
        .usage   = ash_builtin_usage
    },

    [ ASH_BUILTIN_CD ] = {
        .builtin = ASH_BUILTIN_CD,
        .name    = "cd",
        .main    = ash_cd,
        .usage   = ash_cd_usage
    },

    [ ASH_BUILTIN_ECHO ] = {
        .builtin = ASH_BUILTIN_ECHO,
        .name    = "echo",
        .main    = ash_echo,
        .usage   = ash_echo_usage
    },

    [ ASH_BUILTIN_EXIT ] = {
        .builtin = ASH_BUILTIN_EXIT,
        .name    = "exit",
        .main    = ash_exit,
        .usage   = ash_exit_usage
    },

    [ ASH_BUILTIN_EXPORT ] = {
        .builtin = ASH_BUILTIN_EXPORT,
        .name    = "export",
        .main    = NULL,
        .usage   = NULL
    },

    [ ASH_BUILTIN_HELP ] = {
        .builtin = ASH_BUILTIN_HELP,
        .name    = "help",
        .main    = ash_help,
        .usage   = ash_help_usage
    },

    [ ASH_BUILTIN_HISTORY ] = {
        .builtin = ASH_BUILTIN_HISTORY,
        .name    = "history",
        .main    = NULL,
        .usage   = NULL
    },

    [ ASH_BUILTIN_SLEEP ] = {
        .builtin = ASH_BUILTIN_SLEEP,
        .name    = "sleep",
        .main    = ash_sleep,
        .usage   = ash_sleep_usage
    },

    [ ASH_BUILTIN_UNSET ] = {
        .builtin = ASH_BUILTIN_UNSET,
        .name    = "unset",
        .main    = ash_unset,
        .usage   = ash_unset_usage
    }

};

void ash_builtin_exec(int o, int argc, const char * const *argv)
{
    assert( o < ASH_BUILTIN_NO );
    int status = usage[o].main(argc, argv);
    ash_exec_set_exit(status);
}

int ash_builtin_find(const char *v)
{
    switch (v[0]){
        case 'b':
            if (v[1] == 'u' &&
                v[2] == 'i' &&
                v[3] == 'l' &&
                v[4] == 't' &&
                v[5] == 'i' &&
                v[6] == 'n' &&
                !(v[7]))
                return ASH_BUILTIN_BUILTIN;
            break;
        case 'c':
            if (v[1] == 'd' &&
                !(v[2]))
                return ASH_BUILTIN_CD;
            break;
        case 'e':
            if (v[1] == 'x' &&
                v[2] == 'i' &&
                v[3] == 't' &&
                !(v[4]))
                return ASH_BUILTIN_EXIT;
            else if (v[1] == 'c' &&
                     v[2] == 'h' &&
                     v[3] == 'o' &&
                     !(v[4]))
                return ASH_BUILTIN_ECHO;
            break;
        case 'h':
            if (v[1] == 'e' &&
                v[2] == 'l' &&
                v[3] == 'p' &&
                !(v[4]))
                return ASH_BUILTIN_HELP;
            break;
        case 's':
            if (v[1] == 'l' &&
                v[2] == 'e' &&
                v[3] == 'e' &&
                v[4] == 'p' &&
                !(v[5]))
                return ASH_BUILTIN_SLEEP;
            break;

        case 'u':
            if (v[1] == 'n' &&
                v[2] == 's' &&
                v[3] == 'e' &&
                v[4] == 't' &&
                !v[5])
                return ASH_BUILTIN_UNSET;
    }
    return -1;
}

static void ash_print_builtin_info(int o, const char *s)
{
    assert( o < ASH_BUILTIN_NO );
    if (usage[o].usage)
        ash_print("%s :: %s\n", usage[o].name, usage[o].usage());
}

static void ash_print_builtin(void)
{
    ash_puts("built-in commands:");
    ash_puts("type builtin [command] for more info\n");
    for (size_t i = 0; i < ASH_BUILTIN_NO; ++i){
        if (!usage[i].usage)
            continue;
        ash_print("%s     \t\t:: %s\n", usage[i].name, usage[i].usage());
    }
}
