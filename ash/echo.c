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

#include <stddef.h>

#include "echo.h"
#include "io.h"

const char *ash_echo_usage(void)
{
    return "print to stdout";
}

int ash_echo(int argc, const char * const *argv)
{
    int n = 1;
    size_t i = 1;
    /* todo: add flag for space */

    if(argc > 1){
        for (size_t o = 1; o < argc -1; ++o){
            if (argv[1][0] == '-'){
                if (argv[1][1] == 'n' && !argv[1][2]){
                    n = 0;
                    ++i;
                }
            }
        }

        for (; i < argc; ++i)
            ash_print("%s ", argv[i]);

        if (n == 1)
            ash_print("%c", '\n');
    }
    return 0;
}
