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
#include <string.h>

#include "builtin.h"
#include "env.h"
#include "io.h"
#include "sleep.h"

#ifdef ASH_UNIX
    #include <unistd.h>
#endif

const char *ash_sleep_usage(void)
{
    return "[sec] sleep for [sec] seconds";
}

int ash_sleep(int argc, const char * const *argv)
{
    int status;

    if(argc == 1)
        ash_print_err_builtin(argv[0], perr(ARG_MSG_ERR));
    else {
        status = 0;
        for (size_t i = 0; i < strlen(argv[1]); ++i){
            char c = argv[1][i];
            if (!(c <= '9' && c >= '0'))
                status = 1;
        }
        if (status == 0)
            sleep(atoi(argv[1]));
        else
            ash_print_err_builtin(argv[0], perr(TYPE_ERR));
    }
    return status;
}
