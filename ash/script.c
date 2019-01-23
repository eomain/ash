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
#include <string.h>

#include "ash.h"
#include "io.h"
#include "lang.h"
#include "mem.h"
#include "ops.h"
#include "script.h"

#define CHSIZE 300

#define ASH_LIB "libash"
#define ASH_INIT_SCRIPT 1

/* path to ash_profile */
static const char *profile = NULL;
/* path to ash_logout */
static const char *logout = NULL;
static const char *history = NULL;


int ash_profile(void)
{
    if (!profile)
        profile = ash_ops_tilde(ASH_PROFILE);
    return ash_script_load(profile, ASH_IGNORE);
}

int ash_logout_script(void)
{
    if (!logout)
        logout = ash_ops_tilde(ASH_LOGOUT);
    return ash_script_load(logout, ASH_IGNORE);
}

static const char *ash_init_scripts[ASH_INIT_SCRIPT] = {
    ASH_LIB
};

int ash_script_load(const char *script, int err)
{
    const char *ash_script = ash_open(script, ASH_IO_READ, err);
    int status = ash_script ? 0: -1;
    if (status == 0){
        size_t len = strlen(ash_script);
        size_t ch = len -1 < CHSIZE? len -1: CHSIZE;
        if (memchr(ash_script, '\0', ch)){
            if (err = ASH_ALERT)
                ash_print(PNAME ": %s: not a text file, cannot scan!\n", script);
            return -1;
        }
        ash_lang_eval(ash_script);
        ash_free((void *)ash_script);
    }
    return status;
}
