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

#include "ash/ash.h"
#include "ash/env.h"
#include "ash/int.h"
#include "ash/io.h"
#include "ash/macro.h"
#include "ash/ops.h"
#include "ash/script.h"
#include "ash/session.h"
#include "ash/str.h"
#include "ash/tuple.h"
#include "ash/type.h"
#include "ash/var.h"
#include "ash/lang/main.h"
#include "ash/type/array.h"
#include "ash/util/queue.h"
#include "ash/util/vec.h"

/* print the current ash version */
static void ash_print_version(void)
{
    ash_print_msg(ASH_VERSION);
}

/* print the current ash build */
static void ash_print_build(void)
{
    ash_print_msg(ASH_BUILD_INFO);
}

/* logout of ash session */
void ash_logout(void)
{
    struct ash_session *session;
    session = ash_session_default();
    ash_session_quick_shutdown(session);
}

static void ash_exit_fail(void)
{
    exit(EXIT_FAILURE);
}

void ash_abort(const char *e_msg)
{
    struct ash_session *session;
    session = ash_session_default();

    e_msg = (!e_msg) ? "(abort)!": e_msg;
    ash_print(PNAME ": %s\n", e_msg);
    ash_session_abort(session);
}

static void ash_option_none(void)
{
    ash_print_err("no option specified.");
    ash_exit_fail();
}

static void ash_option_invalid(const char *s)
{
    ash_print(PNAME ": found unrecognized option `--%s`.\n", s);
    ash_exit_fail();
}

static void ash_option_long(const char *s)
{
    if (!(*s))
        ash_option_none();
    else if (!strcmp(s, "build"))
        ash_print_build();
    else if (!strcmp(s, "help"))
        ash_print_help();
    else if (!strcmp(s, "version"))
        ash_print_version();
    else
        ash_option_invalid(s);

    ash_logout();
}

static void option_command(const char *command)
{
    struct input input;
    input_text_init(&input, command);
    ash_main_input(&input);
}

static void ash_option_short(struct queue *opt, char c)
{
    struct ash_session *session;
    struct ash_session_profile *profile;

    session = ash_session_default();
    profile = ash_session_profile(session);

    switch (c) {
        case 'b':
            ash_print_build();
            ash_logout();
            break;

        case 'c':
            option_command(queue_dequeue(opt));
            ash_logout();
            break;

        case 'e':
            ash_session_set_entry(session);
            break;

        case 'i':
            break;

        case 'p':
            ash_session_profile_set_profile(profile, false);
            break;

        case 's':
            ash_io_silent(true);
            break;

        case 'h':
            ash_print_help();
            ash_logout();
            break;

        case 'v':
            ash_print_version();
            ash_logout();
            break;

        default:
            ash_print(PNAME ": found unrecognized option `-%c`.\n", c);
            ash_exit_fail();
    }
}

static struct ash_var *
option_args(struct queue *opt, int argc)
{
    const char *args;
    struct ash_obj *argv = NULL;
    struct vec *vec;

    vec = vec_from(argc);

    for (size_t i = 0; i < argc; ++i) {
        args = ash_strcpy(queue_dequeue(opt));
        vec_push(vec, ash_str_from(args));
    }

    argv = ash_array_from(vec);

    return ash_var_set("@", argv);
}

static void
script(struct queue *opt, const char *name)
{
    size_t argc;
    struct ash_obj *args = NULL;

    if ((argc = queue_len(opt)) > 0) {
        struct ash_var *av;
        av = option_args(opt, argc);
        args = ash_var_obj(av);
    }

    struct script *script;
    if (!(script = ash_script_open(name, true))) {
        ash_print(PNAME ": error: unable to find file `%s`.\n", name);
        ash_exit_fail();
    }

    struct ash_session *session;
    session = ash_session_default();
    ash_session_set_script(session);

    if (ash_session_entry(session))
        ash_script_exec_entry(script, ash_tuple_from(1, &args));
    else
        ash_script_exec(script);

    ash_script_close(script);
}

static void
option(struct queue *opt)
{
    const char *o;
    while ((o = queue_dequeue(opt))) {

        if (!o[0])
            ash_option_none();

        if (o[0] == '-') {
            if (o[1] == '-')
                ash_option_long(&o[2]);
            else if (strlen(&o[1]) == 1)
                ash_option_short(opt, o[1]);
            else
                ash_option_none();

        } else {
            script(opt, o);
            ash_logout();
        }
    }
}

static void ash_print_greeter(void)
{
    ash_print("ash: %s %s\n", ASH_NAME, ASH_VERSION);
    ash_print("usage: help <command>\n");
}

void ash_print_help(void)
{
    ash_print_greeter();
}

static void ash_set_static_var(const char *id, const char *str)
{
    struct ash_obj *obj;
    obj = ash_str_from(str);
    ash_var_set(ash_strcpy(id), obj);
}

int main(int argc, const char *argv[])
{
    /* initialize the shell */
    ash_unit_init();

    struct ash_session *session;
    struct ash_session_meta meta;

    session = ash_session_default();
    ash_session_meta_init(&meta, argv[0]);
    ash_session_init(session, &meta);

    /* set default variables */
    ash_set_static_var("0", argv[0]);
    ash_set_static_var("ASH", ASH_NAME);
    ash_set_static_var("ASH_VERSION", ASH_VERSION);
    ash_set_static_var("ASH_MAJOR", ASH_VERSION_MAJOR);
    ash_set_static_var("ASH_MINOR", ASH_VERSION_MINOR);
    ash_set_static_var("ASH_MICRO", ASH_VERSION_MICRO);

    if (argc > 1) {
        struct queue *opt;
        opt = queue_from((argc - 1));

        for (size_t i = 1; i < argc; ++i)
            queue_enqueue(opt, (char *)argv[i]);

        option(opt);
        queue_destroy(opt);
    }

    ash_session_start(session);
    ash_main();

    return EXIT_SUCCESS;
}
