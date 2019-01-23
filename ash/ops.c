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
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "env.h"
#include "ops.h"
#include "mem.h"

const char *ash_ops_tilde(const char *s)
{
    if (s[0] != '~')
        return NULL;
    if (s[1] == '/')
        ++s;

    size_t max = ash_env_get_pwd_max();
    char *fmt = ash_alloc(max + 1);
    memset(fmt, 0, max + 1);
    sprintf(fmt, "%s%c%s", ash_env_get_home(), '/', ++s);
    return fmt;
}

/* todo: check hex */

int ash_stoi_ck(const char *s)
{
    char c;
    int ck = 1;
    while ((c = *(s++))){
        if (!(c >= '0' && c <= '9'))
            ck = 0;
    }
    return ck;
}
