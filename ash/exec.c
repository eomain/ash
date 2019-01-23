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

#include "ash.h"
#include "builtin.h"
#include "env.h"
#include "io.h"
#include "var.h"

#ifdef ASH_UNIX
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#define ASH_EXIT_STATUS 5

char e_status[ASH_EXIT_STATUS] = { 0 };

void ash_exec_set_exit(int exit)
{
    memset(e_status, 0, ASH_EXIT_STATUS);
    sprintf(e_status, "%d", exit);
    ash_var_set("?", e_status, ASH_STATIC);
}

#define ASH_DEFAULT_DIR_MAX 100

size_t n_paths = 0;
const char *sysdir[ ASH_DEFAULT_DIR_MAX ];

static inline void ash_exec_clear_path(void);

static void ash_exec_path(int len, const char **path)
{
    ash_exec_clear_path();

    for (int i = 0; i < len && i < ASH_DEFAULT_DIR_MAX; ++i)
        sysdir[n_paths++] = path[i];
}

static inline void ash_exec_clear_path(void)
{
    if (n_paths > 0){
        memset((void *) sysdir, 0, ASH_DEFAULT_DIR_MAX);
        n_paths = 0;
    }
}

static int str_get_occurance(const char *s, char c)
{
    int n = 0;
    while ((*s++) != '\0'){
        if (*s == c)
            n++;
    }
    return n;
}

int ash_exec_set_path(void)
{
    char *path = ash_var_clone_value(ash_var_find_builtin(ASH_PATH));
    if (!path)
        return -1;

    char tok = ':';
    int n = str_get_occurance(path, tok);

    if (n > 0){
        char *paths[n];
        paths[0] = path;
        int index = 1;
        for (char *s = path; *s != '\0'; ++s){
            if (*s == tok){
                paths[index++] = (s + 1);
                *s = '\0';
            }
        }

        ash_exec_path(n, (const char **)paths);
    } else
        ash_exec_path(1, (const char **)&path);

    return 0;
}

static void ash_print_err_command(const char *command, const char *msg)
{
    ash_print(PNAME ": %s: %s \n", command, msg);
}

static const char *ash_exec_signal(int e_status);

static int ash_exec(const char *p, char *const argv[])
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == -1)
        ash_print_errno(argv[0]);
    else if (pid == 0){

        if (execvp(p, argv) == -1)
            ash_print_err_command(argv[0], "no such command!");
        _exit(0);
    }
    else {

        wait(&status);
        if (WIFSIGNALED(status))
            ash_print("%s %s\n", ash_exec_signal(WTERMSIG(status)), argv[0]);
    }

    ash_exec_set_exit(WEXITSTATUS(status));
    return status;
}

int ash_exec_command(int argc, const char **argv)
{
    if (!argv[0])
        return -1;
    assert(*argv[0]);

    int status;
    int o;
    if (( o = ash_builtin_find(argv[0])) != -1)
        status = ash_builtin_exec(o, argc, argv);
    else
        status = ash_exec(argv[0], (char *const*)argv);

    return status;
}

static const char *e_msg[] = {
    [ SIGABRT ] = "(abort)",
    [ SIGFPE  ] = "(floating point error)",
    [ SIGILL  ] = "(illegal operation)",
    [ SIGINT  ] = "(interrupt)",
    [ SIGSEGV ] = "(segfault)",
    [ SIGTERM ] = "(terminated)"
};

static const char *ash_exec_signal(int e_status)
{
    if (e_status < sizeof(e_msg) / sizeof(e_msg[0]))
        return e_msg[e_status];
    return e_msg[SIGTERM];
}
