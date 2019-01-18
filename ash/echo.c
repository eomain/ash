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
    /* add newline */
    char n = 1;
    /* add whitespace */
    char s = 1;

    int i = 1;

    if(argc > 1){
        while (i < argc){
            const char *a = argv[i];
            if (a[0] == '-'){
                switch (a[1]){
                    case 'n':
                        n = 0;
                        ++i;
                        break;
                    case 's':
                        s = 0;
                        ++i;
                        break;
                }
            } else
                break;
        }

        const char *fmt = s == 0 ? "%s": "%s ";

        if (i < argc){
            for (; i < argc -1; ++i)
                ash_print(fmt, argv[i]);
            ash_print(argv[argc - 1]);
        }

    }

    if (n == 1)
        ash_putchar('\n');

    return 0;
}
