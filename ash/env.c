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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ash/ash.h"
#include "ash/env.h"
#include "ash/exec.h"
#include "ash/io.h"
#include "ash/int.h"
#include "ash/macro.h"
#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/script.h"
#include "ash/str.h"
#include "ash/type.h"
#include "ash/var.h"


#ifdef ASH_PLATFORM_POSIX
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <pwd.h>
    #include <unistd.h>

    #define DEFAULT_PATH_SIZE 225
    #define MAX_HOST_SIZE 64
#endif

#define ASH_HOME_DIR "~"

#define ASH_ENV_HOME  "HOME"
#define ASH_ENV_LANG  "LANG"
#define ASH_ENV_MAIL  "MAIL"
#define ASH_ENV_PATH  "PATH"
#define ASH_ENV_SHELL "SHELL"

#define ASH_ENV_HOST  "HOST"
#define ASH_ENV_PWD   "PWD"
#define ASH_ENV_LOGNAME  "LOGNAME"

#define ASH_ENV_P1 "_ps1_"
#define ASH_ENV_P2 "_ps2_"

#define ASH_ENV_ROOT "__ROOT__"

#define ASH_ENV_PROFILE "/ash/profile"

/* default value for prompt 1 */
#define DEFAULT_P1 "\\$ "
/* default value for prompt 2 */
#define DEFAULT_P2 "| "

#define DEFAULT_USER '$'
#define DEFAULT_ROOT '#'

#define DEFAULT_UNAME "[?]"
#define DEFAULT_HOME  ""
#define DEFAULT_HOST  "[localhost]"
#define DEFAULT_DIR   ""

#define ROOT_UID 0

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
/* the terminal language */
static const char *lang = NULL;
/* the user default mail */
static const char *mail = NULL;

/* number of commands entered at the prompt */
static size_t count = 0;

/* user id */
static auid uid;
/* if user is 'root' */
static bool root = false;

void ash_env_profile(void)
{
    ash_script_load("/etc" ASH_ENV_PROFILE, false);
}

auid ash_env_uid(void)
{
    return uid;
}

apid ash_env_pid(void)
{
    return getpid();
}

bool ash_env_root(void)
{
    return root;
}

static void
ash_getuid(void)
{
    uid = getuid();
    root = (uid == ROOT_UID);
    ash_var_set(ASH_ENV_ROOT, ash_bool_from(root));
}

static void ash_default_greeter(void);

/* ash greeter */
static struct ash_obj *greeter;

const char *ash_env_get_greeter(void)
{
    return ash_str_get(greeter);
}

/* primary command prompt */
static struct ash_obj *ps1 = NULL;

/* secondary command prompt */
static struct ash_obj *ps2 = NULL;

void ash_env_prompt_default(void)
{
    ps1 = ash_str_from(DEFAULT_P1);
    ps2 = ash_str_from(DEFAULT_P2);
    ash_var_set(ASH_ENV_P1, ps1);
    ash_var_set(ASH_ENV_P2, ps2);
}

static inline void
ash_env_set_var(const char *id, const char *value)
{
    if (id && value)
        ash_var_set(id, ash_str_from(value));
}

struct ash_env_set {
    const char *id;
    const char *value;
};

static void ash_env_set_vars(void)
{
    struct ash_env_set vars[] = {
        { ASH_ENV_HOST,    host  },
        { ASH_ENV_PATH,    path  },
        { ASH_ENV_PWD,     pwd   },
        { ASH_ENV_LOGNAME, uname },
        { ASH_ENV_HOME,    home  },
        { ASH_ENV_LANG,    lang  },
        { ASH_ENV_MAIL,    mail  },
        { ASH_ENV_SHELL,   shell },

        { "_OS_FAMILY_", ASH_ENV_OS_FAMILY }
    };

    for (size_t i = 0; i < array_length(vars); ++i)
        ash_env_set_var(vars[i].id, vars[i].value);
}

struct ash_prompt_fmt {
    const char *p1;
    const char *fmt;
};

void ash_prompt(void)
{
    char c;
    const char *fmt;
    ps1 = ash_var_obj(ash_var_get(ASH_ENV_P1));
    if (!ps1 || !(fmt = ash_str_get(ps1)))
        return;

    while ((c = *(fmt++))) {
        if (c == '\\') {
            c = *fmt;
            switch (c) {
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
                    ash_putchar(root ? DEFAULT_ROOT: DEFAULT_USER);
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

void ash_prompt_next(void)
{
    const char *fmt;
    ps2 = ash_var_obj(ash_var_get(ASH_ENV_P2));

    if (!ps2 || (fmt = ash_str_get(ps2)))
        ash_print("%s", fmt);
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
    return dir ? dir: DEFAULT_DIR;
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
    ash_env_set_var(ASH_ENV_PWD, pwd);
    ash_env_dir();
    setenv(ASH_ENV_PWD, pwd, 1);
}

void ash_env_dir(void)
{
    if (home && pwd) {
        if (!strcmp(home, pwd)) {
            dir = ASH_HOME_DIR;
            return;
        }

        size_t len, pos;
        pos = len = strlen(pwd);

        while (len > 0) {
            if (pwd[--pos] == '/') {
                size_t index = (pos + 1 < len) ? pos + 1: pos;
                dir = &pwd[index];
                return;
            }
        }
    }

    dir = NULL;
}

static void ash_uname_host(void)
{
    uname = getpwuid(getuid())->pw_name;
    if (host)
        gethostname((char *)host, MAX_HOST_SIZE);
}

static void ash_env_set_var_home(void)
{
    home = getpwuid(getuid())->pw_dir;
}

static void ash_env_var_alloc(void)
{
    pwd_size = pathconf(".", _PC_PATH_MAX);
    pwd = ash_alloc((sizeof *pwd) * pwd_size);
    host = ash_alloc((sizeof *host) * MAX_HOST_SIZE);
}

static void ash_env_set_util(void)
{
    ash_var_set("_RAND_MAX_", ash_int_from(RAND_MAX));
}

static void ash_env_vars(void)
{
    greeter = ash_str_new();
    ash_default_greeter();

    ash_env_var_alloc();
    ash_env_pwd();
    ash_uname_host();

    shell = getenv(ASH_ENV_SHELL);
    path  = getenv(ASH_ENV_PATH);
    lang  = getenv(ASH_ENV_LANG);
    mail  = getenv(ASH_ENV_MAIL);
    home  = getenv(ASH_ENV_HOME);
    if (!home)
        ash_env_set_var_home();

    ash_env_set_vars();
    ash_env_set_util();
    ash_env_dir();
}

static void init(void)
{
    ash_getuid();
    ash_env_prompt_default();
    ash_env_vars();
}

const struct ash_unit_module ash_module_env = {
    .init = init,
    .destroy = NULL
};

/* the default ash greeter */
static const char acorn[] =
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

static void ash_default_greeter(void)
{
    ash_str_set(greeter, acorn);
    ash_var_set(ASH_ENV_GREETER, greeter);
}
