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
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ash.h"
#include "io.h"
#include "mem.h"

#define MIN_BUFFER_SIZE 2096
#define MAX_BUFFER_SIZE 16384

static int ash_errno = ASH_ERRNO;

void ash_set_errno(int err)
{
    ash_errno = err;
}

static inline size_t fsize(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    rewind(fp);
    return len;
}

const char *ash_open(const char *name)
{
    FILE *fp;
    fp = fopen(name, "r");
    if (fp != NULL){
        size_t len = fsize(fp);
        char *buf = ash_alloc(len + 1 * sizeof (char));
        int n = (fread(buf, sizeof (char), len, fp) == len) ? 1: 0;
        fclose(fp);
        if (n == 1)
            return buf;
    }

    return NULL;
}

static char buf[MIN_BUFFER_SIZE];

static void ash_io_handle(int sig)
{
    /* todo */
    if (sig == SIGQUIT || sig == SIGTSTP || sig == SIGINT)
        return;
}

static void ash_io_signal(void)
{
    struct sigaction act;
    act.sa_handler = ash_io_handle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);
}

char *ash_scan(void)
{
    memset(buf, 0, MIN_BUFFER_SIZE);
    if (fgets(buf, MIN_BUFFER_SIZE, stdin))
        return buf;
    return NULL;
}

void ash_print(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (!ash_get_silent())
        vfprintf(stdout, fmt, ap);
    va_end(ap);
}

void ash_puts(const char *s)
{
    ash_print("%s\n", s);
}

void ash_putchar(char c)
{
    ash_print("%c", c);
}

void ash_print_msg(const char *msg)
{
    ash_print(PNAME " %s\n", msg);
}

void ash_print_err(const char *msg)
{
    ash_print(PNAME ": error: %s\n", msg);
}

void ash_print_errno(const char *msg)
{
    ash_print(PNAME ": error: %s: %s\n", msg, strerror(errno));
}

void ash_io_init(void)
{
    ash_io_signal();
}

const char *ash_merr[ ASH_ERR_NO ] = {
    [ ARG_MSG_ERR ]     = "expected argument",
    [ TYPE_ERR ]        = "incorrect value type",
    [ RODATA_ERR ]      = "read-only data",
    [ PARSE_ERR ]       = "parsed with errors",
    [ UREG_CMD_ERR ]    = "unrecognized command",
    [ SIG_MSG_ERR ]     = "abnormal termination"
};

const char *perr(int msg)
{
    if (msg == ASH_ERRNO)
        msg = ash_errno;
    if (msg < ASH_ERR_NO)
        return ash_merr[msg];
    return "internal error";
}
