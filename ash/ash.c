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
#include "exec.h"
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

/* begin shell session with greeter */
static int ash_greet = 0;

/* number of script arguments */
static char ash_nargs[ASH_ARG_LEN] = "0";

static void ash_print_greeter(void);

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
static int ash_main(int argc, const char **argv)
{
    /* set up environment */
    /*if (argc > 0)
        ash_var_env_new(argc, argv);*/

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
    return fcount ? 1: 0;
}

/* print the current ash version */
static void ash_print_version(void)
{
    ash_print_msg(ASH_VERSION);
}

/* print the current ash build */
static void ash_print_build(void)
{
    ash_print(PNAME " " ASH_VERSION " "
    ASH_BUILD_TYPE " " ASH_BUILD_DATETIME "\n");
}

/* logout of ash session */
void ash_logout(void)
{
    ash_logout_script();
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

static void ash_option_none(void)
{
    ash_print_err("no option specified");
}

static void ash_option_invalid(const char *s)
{
    ash_print(PNAME ": no such option `%s`\n", s);
}

static void ash_option_long(const char *s)
{
    if (!(*s))
        ash_option_none();

    if (!strcmp(s, "build"))
        ash_print_build();
    else if (!strcmp(s, "help"))
        ash_print_help();
    else if (!strcmp(s, "version"))
        ash_print_version();
    else
        ash_option_invalid(s);
}

static void ash_option_short(char c)
{
    if (c == 'b')
        ash_print_build();
    else if (c == 'c')
        ash_eval = 1;
    else if (c == 'g')
        ash_print_greeter();
    else if (c == 'i')
        ash_interact = 1;
    else if (c == 'n')
        ash_greet = 1;
    else if (c == 's')
        ash_silent = 1;
    else if (c == 'h')
        ash_print_help();
    else if (c == 'v')
        ash_print_version();
    else
        ash_print(PNAME": no such option `%c`\n", c);
}

struct {
    int pos;
    int len;
    const char **ops;
}
static ash_ops = {
    .pos = 0,
    .len = 0,
    .ops = NULL
};

static void setops(int len, const char **ops)
{
    ash_ops.len = len;
    ash_ops.ops = ops;
}

static inline int getnext(void)
{
    return ash_ops.len - ash_ops.pos - 1;
}

static const char *getopt(void)
{
    int pos = ash_ops.pos;
    if (ash_ops.ops && pos < ash_ops.len){
        ash_ops.pos++;
        return ash_ops.ops[pos];
    }
    return NULL;
}

/* parse arguments */
static int ash_option(void)
{
    const char *option;
    while ((option = getopt())){

        if (!option[0])
            ash_option_none();

        if (option[0] == '-') {
            if (option[1] == '-')
                ash_option_long(&option[2]);

            else if (strlen(&option[1]) == 1)
                ash_option_short(option[1]);

            else
                ash_option_invalid(&option[1]);
        }
        else {
            ash_set_args(getnext());
            ash_script_load(option, ASH_ALERT);
            return ash_get_interactive()? 0: -1;
        }
    }
    return 0;
}

int ash_get_login(void)
{
    return ash_login;
}

/* display the ash acorn prompt - default greeter */
static void ash_print_greeter(void)
{
    ash_print("ash: acorn shell %s\n", ASH_VERSION);
    /*ash_print("version: %s %s\n", ASH_VERSION, ASH_BUILD_TYPE);*/
    ash_print("usage: type commands e.g. help\n\n");
    ash_print(ash_env_get_greeter());
}

/* display the ash help prompt */
void ash_print_help(void)
{
    ash_print_greeter();
    /* todo: print commands */
}

static inline void ash_assert_login(const char *ash)
{
    if (*ash == '-')
        ash_login = 1;
}

static const char **getopts(void)
{
    int pos = ash_ops.pos;
    return &ash_ops.ops[pos];
}

int main(int argc, const char *argv[])
{
    /* check if login shell */
    ash_assert_login(argv[0]);

    /* check user */
    ash_check_root();

    /* init the env variables */
    ash_vars_init();
    ash_env_init();

    /* init the shell interpreter */
    ash_lang_init();

    /* read ash profile */
    ash_profile();

    ash_exec_set_exit(0);

    /* set default variables */
    ash_var_set("0", argv[0], ASH_STATIC);
    ash_var_set("#", "0", ASH_STATIC);
    ash_var_set("ASH", ASH_NAME, ASH_STATIC);
    ash_var_set("ASH_VERSION", ASH_VERSION, ASH_STATIC);
    ash_var_set("ASH_MAJOR", ASH_VERSION_MAJOR, ASH_STATIC);
    ash_var_set("ASH_MINOR", ASH_VERSION_MINOR, ASH_STATIC);
    ash_var_set("ASH_MICRO", ASH_VERSION_MICRO, ASH_STATIC);

    setops(--argc, ++argv);

    if ((ash_option()) == 0){
        if (ash_greet)
            ash_print_greeter();

        argc = getnext();
        argv = getopts();
        ash_main(argc, argv);
    }

    return 0;
}
