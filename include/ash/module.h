/* Copyright 2019 eomain
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

#ifndef ASH_MODULE_H
#define ASH_MODULE_H

#include "ash/obj.h"
#include "ash/unit.h"

extern const struct ash_unit_module ash_module_module;

struct ash_module;

struct ash_module *
ash_module_new(const char *);

extern struct ash_module *
ash_module_from(const char *, struct ash_module *);

extern struct ash_var *
ash_module_var_get(struct ash_module *, const char *);
extern struct ash_var *
ash_module_var_set(struct ash_module *,
                   const char *, struct ash_obj *);
extern struct ash_var *
ash_module_var_set_override(struct ash_module *,
                            const char *, struct ash_obj *);
extern void ash_module_var_unset(struct ash_module *,
                                 struct ash_var *);

extern struct ash_var *
ash_module_func_get(struct ash_module *, const char *);
extern struct ash_var *
ash_module_func_set(struct ash_module *, const char *, struct ash_obj *);
extern struct ash_var *
ash_module_func_unset(struct ash_module *, const char *);

extern struct ash_module *
ash_module_sub_set(struct ash_module *, const char *);

enum ash_module_path_type {
    ASH_PATH_ABS,
    ASH_PATH_CUR,
    ASH_PATH_REL,
};

struct ash_module_path {
    enum ash_module_path_type type;
    size_t length;
    const char **path;
};

extern struct ash_module *
ash_module_path(struct ash_module *,
                struct ash_module_path *);

extern struct ash_module *
ash_module_root(void);

struct map;

extern struct ash_var *
var_get(struct map *, const char *);

extern struct ash_var *
var_set(struct map *, const char *, struct ash_obj *);

extern struct ash_var *
var_set_override(struct map *, const char *,
                 struct ash_obj *);

extern void
var_unset(struct map *, const char *);

#endif
