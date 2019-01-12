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
#include "io.h"
#include "ops.h"
#include "var.h"


#ifdef ASH_UNIX
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <pwd.h>
    #include <unistd.h>
#endif

#define ENV_HOME "HOME"
#define ENV_PATH "PATH"
#define ENV_SHELL "SHELL"
#define ENV_P1 "PS1"
#define ENV_P2 "PS2"
#define DEFAULT_UNAME "[?]"
#define DEFAULT_HOST "[unknown]"
#define DEFAULT_PATH_SIZE 225
#define MAX_HOST_SIZE 64

extern int gethostname(char *, size_t);

static char *pwd = NULL;
static size_t pwd_size = DEFAULT_PATH_SIZE;
static char *dir = NULL;
static const char *home = NULL;
static const char *uname = DEFAULT_UNAME;
static const char *host = DEFAULT_HOST;
static const char *path = NULL;
static const char *shell = NULL;
static size_t count = 0;

static const char *ps1;

void ash_prompt(void)
{
    ps1 = ash_var_get_value( ash_var_get(ENV_P1) );

    for (const char *s = ps1; *s != '\0'; s++){
        if (*s == '\\'){
            char c = *(s + 1);

            if (c == 'n')
                ash_putchar('\n');
            else if (c == '#')
                ash_print("%lu", count);
            else if (c == 'u')
                ash_print(uname);
            else if (c == 'h')
                ash_print(host);
            else if (c == 'w')
                ash_print(pwd);
            else if (c == 'W')
                ash_print(dir);
            else if (c ==  '$')
                ash_putchar(c);
            else {
                ash_putchar(*s);
                continue;
            }
            s++;

        } else
            ash_print("%c", *s);
    }
    ++count;
}

static const char *ps2;

void ash_prompt_next(void)
{
    if ((ps2 = ash_var_get_value( ash_var_get(ENV_P2) )))
        ash_print("%s", ps2);
}

const char *ash_env_get_pwd(void)
{
    return pwd;
}

size_t ash_env_get_pwd_max(void)
{
    return pwd_size;
}

const char *ash_env_get_dir(void)
{
    return dir;
}

const char *ash_env_get_home(void)
{
    return home;
}

const char *ash_env_get_uname(void)
{
    return uname;
}

const char *ash_env_get_host(void)
{
    return host;
}

const char *ash_env_get_path(void)
{
    return path;
}

void ash_env_pwd(void)
{
    pwd = getcwd(pwd, pwd_size);
    ash_var_set_builtin(ASH_PWD, pwd);
    ash_env_dir();
}

void ash_env_dir(void){
    if(home && !strcmp(pwd, home)){
        dir = "~";
    } else {
        if (pwd){
            size_t len = strlen(pwd);
            while (len > 0){
                if (pwd[--len] == '/'){
                    dir = &pwd[++len];
                    return;
                }
            }

        } else
            dir = "";
    }
}

static void ash_uname_host(void)
{
    uname = getpwuid(getuid())->pw_name;
    if (host)
        gethostname((char *)host, MAX_HOST_SIZE);
}

void ash_env_init(void)
{
    ash_vars_init();
    ash_ops_init();

    ash_var_set(ENV_P1, "\\u::\\h \\W|\\$ ", ASH_STATIC);
    ash_var_set(ENV_P2, "| ", ASH_STATIC);
    if ((shell = getenv(ENV_SHELL)))
        ash_var_set(ENV_SHELL, shell, ASH_RODATA);

    pwd_size = pathconf(".", _PC_PATH_MAX);
    if ((pwd = malloc(sizeof (char) * pwd_size)) != NULL)
        ash_env_pwd();

    host = malloc(sizeof (char) * MAX_HOST_SIZE);
    ash_uname_host();
    path = getenv(ENV_PATH);
    home = getenv(ENV_HOME);
    if (!home)
        home = getpwuid(getuid())->pw_dir;

    ash_var_set_builtin(ASH_HOST, host);
    ash_var_set_builtin(ASH_PATH, path);
    ash_var_set_builtin(ASH_PWD, pwd);
    ash_var_set_builtin(ASH_LOGNAME, uname);
    ash_var_set_builtin(ASH_HOME, home);
    ash_env_dir();
}
