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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ash/ash.h"
#include "ash/cd.h"
#include "ash/env.h"
#include "ash/io.h"

#ifdef ASH_PLATFORM_POSIX
    #include <unistd.h>
#endif

static const char *USAGE =
    "cd:\n"
    "    change directory\n"
    "usage:\n"
    "    cd [DIRECTORY]\n";

const char *ash_cd_usage(void)
{
    return USAGE;
}

static int ash_chdir(const char *pname, const char *dir)
{
    int status;
    if ((status = chdir(dir)))
        ash_print("%s: error: '%s': %s.\n", pname, dir, strerror(errno));
    else
        ash_env_pwd();

    return status;
}

int ash_cd(int argc, const char * const *argv)
{
    int status;
    const char *pname;
    pname = argv[0];

    if (argc == 1)
        status = ash_chdir(pname, ash_env_get_home());
    else if (argc > 1)
        status = ash_chdir(pname, argv[1]);

    status = (status) ? ASH_STATUS_ERR: ASH_STATUS_OK;

    return status;
}
