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

#ifndef ASH_CORE_EXEC_H
#define ASH_CORE_EXEC_H

#include "ash/ash.h"
#include "ash/obj.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/var.h"
#include "ash/util/vec.h"

#define ASH_SYMBOL_EXIT "__STATUS__"
#define ASH_SYMBOL_RESULT "__RESULT__"

extern const struct ash_unit_module ash_module_exec;

/* the type of redirection within a command */
enum ash_exec_redirect {
    /* no redirection takes place */
    ASH_DIRECT,
    /* `>` redirect to file */
    ASH_REDIRECTION,
    /* `<` redirect from file */
    ASH_INDIRECTION,
    /* `|>` redirect between commands */
    ASH_PIPE
};

struct ash_exec {
    int argc;
    char *const *argv;
    enum ash_exec_redirect redirect;
};

struct ash_exec_command {
    struct ash_vec *argv;
    enum ash_exec_redirect redirect;
};

extern int ash_exec_pipeline(int, struct ash_exec **);

struct ash_runtime_env;

extern int ash_exec_command(struct vec *, struct ash_runtime_env *);
extern int ash_exec_set_path(void);

#endif
