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

static char *ash_tidle;

const char *ash_ops_tidle(const char *s)
{
    assert(ash_tidle);
    if (s[0] != '~')
        return NULL;
    if (s[1] == '/')
        ++s;
    memset(ash_tidle, 0, ash_env_get_pwd_max());
    sprintf(ash_tidle, "%s%c%s", ash_env_get_home(), '/', ++s);
    return ash_tidle;
}

void ash_ops_init(void)
{
    size_t max = ash_env_get_pwd_max();
    ash_tidle = ash_alloc(max);
}
