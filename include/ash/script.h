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

#ifndef ASH_SCRIPT_H
#define ASH_SCRIPT_H

#include "ash/ash.h"
#include "ash/bool.h"
#include "ash/obj.h"

#define ASH_SCRIPT_PROFILE "~.ash_profile"
#define ASH_SCRIPT_LOGOUT  "~.ash_logout"
#define ASH_SCRIPT_HISTORY "~.ash_history"

#define DEFAULT_HISTORY_SIZE 500

struct script_meta {
    struct ash_obj *main;
    struct ash_obj *file;
};

extern void ash_script_meta_get(struct script_meta *);
extern void ash_script_meta_set(struct script_meta *);

struct file {
    const char *path;
    const char *name;
    const char *text;
    size_t length;
};

struct script {
    struct file file;
    bool        main;
    ash_flag    open;
    ash_flag    exec;

    int ndeps;
    struct script **deps;
};

extern struct script *ash_script_open(const char *, bool);
extern int ash_script_load(const char *, bool);
extern void ash_script_close(struct script *);
extern int ash_script_exec(struct script *);
extern int ash_script_exec_entry(struct script *, struct ash_obj *);
extern const char *ash_script_name(struct script *);
extern const char *ash_script_content(struct script *);

/*
  load and execute the ash_profile script
  from the HOME directory if it exists
*/
extern int ash_profile_script(void);

/*
  load and execute the ash_logout script
  from the HOME directory if it exists
*/
extern int ash_logout_script(void);

#endif
