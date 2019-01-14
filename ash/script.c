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

#include "ash.h"
#include "io.h"
#include "lang.h"
#include "mem.h"
#include "script.h"

#define ASH_LIB "libash"
#define ASH_INIT_SCRIPT 1

static const char *ash_init_scripts[ASH_INIT_SCRIPT] = {
    ASH_LIB
};

int ash_script_load(const char *script)
{
    const char *ash_script = ash_open(script);
    int status = ash_script ? 0: -1;
    if (status == 0)
        ash_lang_eval(ash_script);
    ash_free((void *)ash_script);
    return status;
}

int ash_script_init(void)
{
    for (int i = 0; i < ASH_INIT_SCRIPT; ++i)
        ash_script_load(ash_init_scripts[i]);
    return 0;
}
