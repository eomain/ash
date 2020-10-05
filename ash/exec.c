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
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ash/ash.h"
#include "ash/command.h"
#include "ash/env.h"
#include "ash/exec.h"
#include "ash/io.h"
#include "ash/macro.h"

#ifdef ASH_PLATFORM_POSIX
    #include <unistd.h>
#endif

#define ASH_STDIN  0
#define ASH_STDOUT 1

#define ASH_EXIT_DEFAULT 0
#define ASH_EXIT_SUCCESS EXIT_SUCCESS
#define ASH_EXIT_FAILURE EXIT_FAILURE

#define ASH_DEFAULT_DIR_MAX 100

static const char *USAGE =
    "exec:\n"
    "    execute command\n"
    "usage:\n"
    "    exec [COMMAND [ARGS]...]\n";

const char *ash_exec_usage(void)
{
    return USAGE;
}

int ash_exec(int argc, const char * const *argv)
{
    if (argc == 1)
        return ASH_STATUS_OK;
    else if (argc > 1) {
        const char *prog = argv[1];

        if (argc == 2) {
            char * const args[] = { (char *const) prog, NULL };
            if (execvp(prog, args))
                return ASH_STATUS_ERR;
        } else {
            const char *args[argc];
            args[argc - 1] = NULL;

            for (int i = 0; i < argc - 1; ++i)
                args[i] = argv[i + 1];

            if (execvp(prog, (char *const *)args))
                return ASH_STATUS_ERR;
        }
    }

    return ASH_STATUS_OK;
}
