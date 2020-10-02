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

#include <stdlib.h>

#include "ash/ash.h"
#include "ash/env.h"
#include "ash/ops.h"
#include "ash/sleep.h"

#ifdef ASH_PLATFORM_POSIX
    #include <unistd.h>
#endif

static const char *USAGE =
    "sleep:\n"
    "    put the shell to sleep for the specified time in microseconds\n"
    "usage:\n"
    "    sleep [SECS]\n";

const char *ash_sleep_usage(void)
{
    return USAGE;
}

int ash_sleep(int argc, const char * const *argv)
{
    if (argc == 1)
        return ASH_STATUS_OK;

    if (!ash_stoi_check(argv[1]))
        return ASH_STATUS_ERR;

    useconds_t msecs;
    msecs = atoi(argv[1]);
    usleep(msecs);

    return ASH_STATUS_OK;
}
