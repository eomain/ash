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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "exec.h"
#include "io.h"
#include "ops.h"
#include "var.h"


#ifdef ASH_UNIX
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <pwd.h>
    #include <unistd.h>
#endif

#define ASH_HOME_DIR "~"

#define ASH_ENV_HOME  "HOME"
#define ASH_ENV_PATH  "PATH"
#define ASH_ENV_SHELL "SHELL"

#define ASH_ENV_P1 "PS1"
#define ASH_ENV_P2 "PS2"

/* default value for prompt 1 */
#define DEFAULT_P1 "\\u::\\h \\W|\\$ "
/* default value for prompt 2 */
#define DEFAULT_P2 "| "

#define DEFAULT_UNAME "[?]"
#define DEFAULT_HOME ""
#define DEFAULT_HOST "[localhost]"
#define DEFAULT_DIR ""
#define DEFAULT_PATH_SIZE 225
#define MAX_HOST_SIZE 64

/* global env variables */

/* full path of the pwd */
static char *pwd = NULL;
/* the default max pwd size */
static size_t pwd_size = DEFAULT_PATH_SIZE;
/* the current directory */
static char *dir = NULL;
/* the user home directory  */
static const char *home = NULL;
/* the username/logname */
static const char *uname = DEFAULT_UNAME;
/* system hostname */
static const char *host = DEFAULT_HOST;
/* the paths of the current shell env */
static const char *path = NULL;
/* the user default shell  */
static const char *shell = NULL;
/* number of commands entered at the prompt */
static size_t count = 0;

/* user id */
static int uid;
/* if user is 'root' */
static int root = 0;


int ash_check_root(void)
{
    if ((uid = getuid()) == 0)
        root = 1;
    return root;
}

static const char *ash_env_set_greeter(void);

const char *ash_env_get_greeter(void)
{
    const char *greeter;
    if (!(greeter = ash_var_get_value(ash_var_get(ASH_GREETER))))
        return ash_env_set_greeter();
    return greeter;
}

void ash_env_prompt_default(void)
{
    ash_var_set(ASH_ENV_P1, DEFAULT_P1, ASH_STATIC);
    ash_var_set(ASH_ENV_P2, DEFAULT_P2, ASH_STATIC);
}

static void ash_env_set_vars(void)
{
    ash_var_set_builtin(ASH_HOST, host);
    ash_var_set_builtin(ASH_PATH, path);
    ash_var_set_builtin(ASH_PWD, pwd);
    ash_var_set_builtin(ASH_LOGNAME, uname);
    ash_var_set_builtin(ASH_HOME, home);
}

/* primary command prompt */
static const char *ps1;

void ash_prompt(void)
{
    ps1 = ash_var_get_value(ash_var_get(ASH_ENV_P1));

    char c;
    const char *fmt = ps1;

    while ((c = *(fmt++))){
        if (c == '\\'){
            c = *fmt;
            switch (c){
                case '#':
                    ash_print("%lu", count);
                    break;

                case 'n':
                    ash_putchar('\n');
                    break;

                case 'r':
                    ash_putchar('\r');
                    break;

                case 'u':
                    ash_print(ash_env_get_uname());
                    break;

                case 'h':
                    ash_print(ash_env_get_host());
                    break;

                case 'w':
                    ash_print(ash_env_get_pwd());
                    break;

                case 'W':
                    ash_print(ash_env_get_dir());
                    break;

                case '$':
                    ash_putchar(root ? '#': c);
                    break;

                default:
                    ash_putchar('\\');
                    continue;
            }
            fmt++;
        } else
            ash_putchar(c);
    }

    ++count;
}

/* secondary command prompt */
static const char *ps2;

void ash_prompt_next(void)
{
    if ((ps2 = ash_var_get_value(ash_var_get(ASH_ENV_P2))))
        ash_print("%s", ps2);
}

const char *ash_env_get_pwd(void)
{
    return pwd ? pwd: DEFAULT_DIR;
}

size_t ash_env_get_pwd_max(void)
{
    return pwd_size ? pwd_size: DEFAULT_PATH_SIZE;
}

const char *ash_env_get_dir(void)
{
    return dir && pwd ? dir: DEFAULT_DIR;
}

const char *ash_env_get_home(void)
{
    return home ? home: DEFAULT_HOME;
}

const char *ash_env_get_uname(void)
{
    return uname ? uname: DEFAULT_UNAME;
}

const char *ash_env_get_host(void)
{
    return host ? host: DEFAULT_HOST;
}

const char *ash_env_get_path(void)
{
    return path ? path: "";
}

void ash_env_pwd(void)
{
    pwd = getcwd(pwd, pwd_size);
    ash_var_set_builtin(ASH_PWD, pwd);
    ash_env_dir();
}

void ash_env_dir(void)
{
    if(home && !strcmp(pwd, home))
        dir = ASH_HOME_DIR;
    else if (pwd){
        size_t len = strlen(pwd);
        while (len > 0){
            if (pwd[--len] == '/'){
                dir = &pwd[++len];
                return;
            }
        }
    } else
        dir = NULL;
}

static void ash_uname_host(void)
{
    uname = getpwuid(getuid())->pw_name;
    if (host)
        gethostname((char *)host, MAX_HOST_SIZE);
}

static void ash_home(void)
{
    home = getpwuid(getuid())->pw_dir;
}

void ash_env_init(void)
{
    ash_env_set_greeter();

    ash_env_prompt_default();

    if ((shell = getenv(ASH_ENV_SHELL)))
        ash_var_set(ASH_ENV_SHELL, shell, ASH_RODATA);

    pwd_size = pathconf(".", _PC_PATH_MAX);
    if ((pwd = malloc(sizeof (char) * pwd_size)) != NULL)
        ash_env_pwd();

    host = malloc(sizeof (char) * MAX_HOST_SIZE);
    ash_uname_host();
    path = getenv(ASH_ENV_PATH);
    home = getenv(ASH_ENV_HOME);
    if (!home)
        ash_home();

    ash_env_set_vars();
    ash_env_dir();
    ash_exec_set_path();
}

static const char *ash_env_set_greeter(void)
{
    /* the default ash greeter */
    static const char *ash_acorn =
    "        $$$      \n"
    "         $$      \n"
    "       $$$$$$    \n"
    "     $$$$$$$$$$  \n"
    "    $$$oooooo$$$ \n"
    "    $$oooooooo$$ \n"
    "     $oooooooo$  \n"
    "      oooooooo   \n"
    "        oooo     \n"
    "\n";

    ash_var_set(ASH_GREETER, ash_acorn, ASH_STATIC);
    return ash_acorn;
}
