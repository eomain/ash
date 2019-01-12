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

#ifndef ASH_VAR
#define ASH_VAR

#include <stddef.h>

struct ash_var;

enum  {
    ASH_VERSION_ = 0x00,
    ASH_HOST     = 0x01,
    ASH_PATH     = 0x02,
    ASH_HOME     = 0x03,
    ASH_PWD      = 0x04,
    ASH_LOGNAME  = 0x05
};

#define ASH_DATA   0
#define ASH_RODATA 1
#define ASH_STATIC 2

#define ASH_NIL '\0'

extern int ash_var_check_composite(struct ash_var *);
extern int ash_var_check_nil(struct ash_var *);

extern void ash_vars_init(void);
extern struct ash_var *ash_var_find_builtin(int);
extern struct ash_var *ash_var_find(const char *);
extern void ash_var_set_builtin(int, const char *);
extern const char *ash_var_get_value(struct ash_var *);
extern struct ash_var *ash_var_set(const unsigned char *, const char *, int);
extern struct ash_var *ash_var_set_array(const unsigned char *, int, const char **, int);
extern int ash_var_unset(const unsigned char *);
extern struct ash_var *ash_var_get(const char *);
extern const char *ash_var_array_value(struct ash_var *, size_t);
extern int ash_var_insert_array(struct ash_var *, int, const char *);

extern void ash_var_env_new(int, const char **);
extern struct ash_var *ash_var_env_get(const unsigned char *);

extern void *ash_func_set(const unsigned char *, void *);
extern void *ash_func_get(const char *);
extern int ash_func_unset(const unsigned char *);

#endif
