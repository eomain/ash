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

#include "ash/ash.h"
#include "ash/command.h"
#include "ash/help.h"

static const char *USAGE =
    "help:\n"
    "    display built-in command usage\n"
    "usage:\n"
    "    help [COMMAND]\n";

const char *ash_help_usage(void)
{
    return USAGE;
}

int ash_help(int argc, const char * const *argv)
{
    if (argc == 1) {
        ash_print_help();
        return ASH_STATUS_OK;
    }

    enum ash_command_name command;
    command = ash_command_find(argv[1]);
    if (!ash_command_valid(command))
        return ASH_STATUS_ERR;

    ash_command_usage(command);

    return ASH_STATUS_OK;
}
