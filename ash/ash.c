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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ash.h"
#include "builtin.h"
#include "env.h"
#include "io.h"
#include "lang.h"
#include "script.h"
#include "var.h"

#define ASH_MAX_FAIL 3
#define ASH_ARG_LEN  5

/* set if login shell */
static int ash_login = 0;

/* start shell session after loading a script */
static int ash_interact = 0;

/* eval shell arguments as scipt */
static int ash_eval = 0;

/* begin shell without echo to standard out */
static int ash_silent = 0;

/* number of script arguments */
static char ash_nargs[ASH_ARG_LEN] = "0";

static void ash_print_acorn(void);

static void ash_set_args(int argc)
{
    sprintf(ash_nargs, "%d", argc);
    ash_var_set("@", ash_nargs, ASH_STATIC);
}

int ash_get_interactive(void)
{
    return ash_interact;
}

int ash_get_silent(void)
{
    return ash_silent;
}

/* ash status used as exit status */
static int e_stat = 0;

/* set ash exit status */
void ash_set_status(int status)
{
    e_stat = status;
}

/* ash main display prompt and read input */
static void ash_main(int argc, const char **argv)
{
    /* set up environment */
    /*ash_var_env_new(argc, argv);*/

    /* init io buffers and signal handlers */
    ash_io_init();

    char *buf;

    /* amount of failed attempts */
    int fcount = 0;

    /* read and evaluate input */
    for (;;){
        ash_prompt();
        if ((buf = ash_scan())){
            ash_lang_eval(buf);
        } else {
            ash_print_err("failed to scan input!");
            fcount++;
            if (fcount == ASH_MAX_FAIL)
                ash_abort(NULL);
        }
    }
}

/* print the current ash version */
static void ash_print_version(void)
{
    ash_print_msg(ASH_VERSION);
}

/* logout of ash session */
void ash_logout(void)
{
    exit(e_stat);
}

void ash_abort(const char *e_msg)
{
    if (!e_msg)
        e_msg = "cannot continue!";
    ash_set_status(EXIT_FAILURE);
    ash_print("(abort) [%d] %s \n", e_stat, e_msg);
    ash_logout();
}

/* parse all supplied command line arguments */
static int ash_option(int argc, const char **argv)
{
    for (size_t i = 0; i < argc; i++){
        const char *arg = argv[i];
        if (arg[0] == '-') {
            if (!arg[1]){
                ash_print_err("no option specified");
                return -1;
            }

            if (arg[1] == '-'){
                const char *s = &arg[2];
                if (!(*s))
                    ash_print_err("no option specified");

                if (!strcmp(s, "acorn"))
                    ash_print_acorn();
                if (!strcmp(s, "help"))
                    ash_print_help();
                else if (!strcmp(s, "version"))
                    ash_print_version();
                return -1;

            } else {
                size_t len = strlen(&arg[1]);

                if (len == 1){
                    char c = arg[1];

                    if (c == 'i')
                        ash_interact = 1;
                    else if (c == 'c')
                        ash_eval = 1;
                    else if (c == 's')
                        ash_silent = 1;
                    else if (c == 'p')
                        ash_print_help();
                    else if (c == 'v')
                        ash_print_version();
                }
            }

        } else {
            ash_set_args((argc - i - 1));
            ash_script_load(argv[i]);
            return ash_interact? 0: -1;
        }
    }
    return 0;
}

int ash_get_login(void)
{
    return ash_login;
}

/* display the ash acorn prompt */
static void ash_print_acorn(void)
{
    ash_print("ash: acorn shell %s\n", ASH_VERSION);
    ash_print("usage: type commands e.g. help\n\n");
    ash_print("        $$$      \n");
    ash_print("         $$      \n");
    ash_print("       $$$$$$    \n");
    ash_print("     $$$$$$$$$$  \n");
    ash_print("    $$$oooooo$$$ \n");
    ash_print("    $$oooooooo$$ \n");
    ash_print("     $oooooooo$  \n");
    ash_print("      oooooooo   \n");
    ash_print("        oooo     \n");
    ash_print("\n");
}

/* display the ash help prompt */
void ash_print_help(void)
{
    ash_print_acorn();
    /*ash_print_builtin();*/
}

static inline void ash_assert_login(const char *shell)
{
    if (*shell == '-')
        ash_login = 1;
}

int main(int argc, const char *argv[])
{
    /* check if login shell */
    ash_assert_login(argv[0]);
    /* init the env variables */
    ash_env_init();
    /* init the shell interpreter */
    ash_lang_init();

    /* set default variables */
    ash_var_set("0", argv[0], ASH_STATIC);
    ash_var_set("#", "0", ASH_STATIC);
    ash_var_set("ASH", "acorn shell", ASH_STATIC);
    ash_var_set("ASH_VERSION", ASH_VERSION, ASH_STATIC);
    ash_var_set("ASH_MAJOR", ASH_VERSION_MAJOR, ASH_STATIC);
    ash_var_set("ASH_MINOR", ASH_VERSION_MINOR, ASH_STATIC);
    ash_var_set("ASH_MICRO", ASH_VERSION_MICRO, ASH_STATIC);

    int status = 0;
    if ((status = ash_option(--argc, ++argv)) == 0)
        ash_main(argc, argv);
    return status;
}
