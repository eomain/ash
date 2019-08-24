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
#include <stdio.h>
#include <string.h>

#include "ash/ash.h"
#include "ash/bool.h"
#include "ash/func.h"
#include "ash/io.h"
#include "ash/macro.h"
#include "ash/mem.h"
#include "ash/obj.h"
#include "ash/ops.h"
#include "ash/script.h"
#include "ash/str.h"
#include "ash/type.h"
#include "ash/var.h"
#include "ash/lang/lang.h"
#include "ash/lang/main.h"

#define FILE_CHECK_SIZE 300

#define ASH_SCRIPT_MAIN "__MAIN__"
#define ASH_SCRIPT_FILE "__FILE__"

void ash_script_meta_get(struct script_meta *meta)
{
    meta->main = ash_var_obj(ash_var_get(ASH_SCRIPT_MAIN));
    meta->file = ash_var_obj(ash_var_get(ASH_SCRIPT_FILE));
    ash_obj_inc_rc(meta->main);
    ash_obj_inc_rc(meta->file);
}

void ash_script_meta_set(struct script_meta *meta)
{
    ash_var_set_override(ASH_SCRIPT_MAIN, meta->main);
    ash_var_set_override(ASH_SCRIPT_FILE, meta->file);
}

static void set_script_vars(struct script *script)
{
    struct ash_obj *main;
    struct ash_obj *file;

    main = ash_bool_from(script->main);
    file = ash_str_from(script->file.path);

    ash_var_set_override(ASH_SCRIPT_MAIN, main);
    ash_var_set_override(ASH_SCRIPT_FILE, file);
}

static void unset_script_vars(void)
{
    struct ash_var *av;

    const char *vars[] = {
        ASH_SCRIPT_MAIN,
        ASH_SCRIPT_FILE
    };

    for (size_t i = 0; i < array_length(vars); ++i) {
        if ((av = ash_var_get(vars[i])))
            ash_var_unset(av);
    }
}

struct script *
ash_script_open(const char *name, bool main)
{
    const char *content;
    if (!(content = ash_io_read(name)))
        return NULL;

    struct script *script;
    script = ash_alloc(sizeof *script);
    script->file.path = ash_strcpy(name);
    script->file.name = NULL;
    script->file.text = content;
    script->file.length = strlen(content);
    script->main = main;
    script->open = ASH_FLAG_RESET;
    script->exec = ASH_FLAG_RESET;
    script->ndeps = 0;
    script->deps = NULL;
    return script;
}

void ash_script_close(struct script *script)
{
    assert(script != NULL);

    if (script->file.path)
        ash_free((char *)script->file.path);
    if (script->file.text)
        ash_free((char *)script->file.text);
    ash_free(script);
}

int ash_script_exec(struct script *script)
{
    assert(script != NULL);

    struct input input;
    input_script_init(&input, script);

    script->exec = ASH_FLAG_SET;
    set_script_vars(script);
    ash_main_input(&input);
    unset_script_vars();
    return 0;
}

/*
  load and execute a given script
*/
int ash_script_load(const char *name, bool main)
{
    int status = -1;
    struct script *script;
    if ((script = ash_script_open(name, main))) {
        status = ash_script_exec(script);
        ash_script_close(script);
    }
    return status;
}

int ash_script_exec_entry(struct script *script, struct ash_obj *args)
{
    assert(script != NULL);

    struct input input;
    input_script_init(&input, script);

    script->exec = ASH_FLAG_SET;
    set_script_vars(script);
    ash_main_input(&input);

    struct ash_var *av;
    struct ash_obj *entry;
    struct ash_runtime_env env;

    runtime_env_init(&env, NULL, NULL);
    if ((av = ash_var_func_get("main"))) {
        if ((entry = ash_var_obj(av)))
            ash_func_exec(entry, &env, args);
    }

    unset_script_vars();
    return 0;
}

const char *ash_script_name(struct script *script)
{
    return script->file.path;
}

const char *ash_script_content(struct script *script)
{
    return script->file.text;
}

static int ash_script_tilde(const char *script)
{
    int status;
    const char *name;
    name = ash_ops_tilde(script);
    status = ash_script_load(name, false);
    ash_free((char *) name);
    return status;
}

/*
  load and execute the ash_profile script
  from the HOME directory if it exists
*/
int ash_profile_script(void)
{
    return ash_script_tilde(ASH_SCRIPT_PROFILE);
}

/*
  load and execute the ash_logout script
  from the HOME directory if it exists
*/
int ash_logout_script(void)
{
    return ash_script_tilde(ASH_SCRIPT_LOGOUT);
}
