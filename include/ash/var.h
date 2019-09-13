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

#ifndef ASH_VAR_H
#define ASH_VAR_H

#include <stddef.h>

#include "ash/ash.h"
#include "ash/module.h"
#include "ash/obj.h"
#include "ash/type.h"

struct ash_var;

extern struct ash_var *ash_var_new(const char *);
extern struct ash_obj *ash_var_obj(struct ash_var *);
extern void ash_var_destroy(struct ash_var *);
extern bool ash_var_mutable(struct ash_var *);
extern void ash_var_bind(struct ash_var *, struct ash_obj *);
extern void ash_var_bind_override(struct ash_var *, struct ash_obj *);
extern void ash_var_unbind(struct ash_var *);
extern const char *ash_var_id(struct ash_var *);

/* ASH GLOBAL VARIABLE FUNCTIONS */
extern struct ash_var *ash_var_set(const char *, struct ash_obj *);
extern struct ash_var *ash_var_get(const char *);
extern void ash_var_unset(struct ash_var *);

extern struct ash_var *ash_var_set_override(const char *, struct ash_obj *);

extern struct ash_var *ash_var_func_set(const char *, struct ash_obj *);
extern struct ash_var *ash_var_func_get(const char *);

struct ash_env;

extern struct ash_env *ash_env_new(struct ash_module *);
extern void ash_env_set(struct ash_env *);
extern struct ash_env *ash_env_parent(struct ash_env *);
extern struct ash_module *ash_env_module(struct ash_env *);
extern struct ash_env *ash_env_get(void);

/* ASH LOCAL VARIABLE FUNCTIONS */
extern struct ash_env *ash_env_new_from(struct ash_module *, struct ash_env *);
extern void ash_env_destroy(struct ash_env *);
extern struct ash_var *ash_var_env_set(struct ash_env *, const char *, struct ash_obj *);
extern struct ash_var *ash_var_env_get(struct ash_env *, const char *);
extern void ash_var_env_unset(struct ash_env *, struct ash_var *);

extern struct ash_var *ash_var_env_func_set(struct ash_env *, const char *, struct ash_obj *);
extern struct ash_var *ash_var_env_func_get(struct ash_env *, const char *);
extern void ash_var_env_func_unset(struct ash_env *, struct ash_var *);

#endif
