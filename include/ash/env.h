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

#ifndef ASH_ENV_H
#define ASH_ENV_H

#include <stddef.h>

#include "ash/ash.h"
#include "ash/type.h"
#include "ash/unit.h"

extern const struct ash_unit_module ash_module_env;

/* IF BUILDING FOR A POSIX-COMPLIANT PLATFORM */
#if defined (__unix__) || defined (HAVE_UNISTD_H)
    #define ASH_PLATFORM_POSIX
#endif


#ifdef ASH_PLATFORM_POSIX
    #include <sys/types.h>

    #define ASH_ENV_OS_FAMILY "unix"

    #define ASH_PATH_DELIM "/"

    typedef uid_t auid;
    typedef gid_t agid;
    typedef pid_t apid;
#endif

#define ASH_ENV_GREETER "ASH_GREETER"


extern auid   ash_env_uid(void);
extern apid   ash_env_pid(void);
extern bool   ash_env_root(void);

extern const char *ash_env_get_greeter(void);
extern void ash_env_prompt_default(void);

extern void ash_env_profile(void);
extern void ash_prompt(void);
extern void ash_prompt_next(void);
extern void ash_env_init(void);
extern size_t ash_env_get_pwd_max(void);
extern void ash_env_pwd(void);
extern void ash_env_dir(void);

extern const char *ash_env_get_pwd(void);
extern const char *ash_env_get_dir(void);
extern const char *ash_env_get_home(void);
extern const char *ash_env_get_uname(void);
extern const char *ash_env_get_host(void);
extern const char *ash_env_get_path(void);

#endif
