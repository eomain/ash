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
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ash/ash.h"
#include "ash/env.h"
#include "ash/io.h"
#include "ash/mem.h"
#include "ash/session.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/term/term.h"

#ifdef ASH_PLATFORM_POSIX
    #include <sys/stat.h>
#endif

static inline long fsize(FILE *fp)
{
    if (fseek(fp, 0, SEEK_END) == -1)
        return 0;

    long len = ftell(fp);
    if (len == -1)
        return 0;

    rewind(fp);
    return len;
}

static int fcheck(const char *name)
{
    struct stat f_stat;
    stat(name, &f_stat);
    return S_ISREG(f_stat.st_mode) == 0 ? -1: 0;
}

const char *ash_io_read(const char *name)
{
    if (fcheck(name))
        return NULL;

    FILE *fp;
    char *content = NULL;

    if (!(fp = fopen(name, "r")))
        return NULL;

    long len = fsize(fp);
    if (!(len == 0 || ferror(fp))) {
        char *buf = ash_zalloc(len + 1 * sizeof (*buf));

        if ((fread(buf, sizeof (char), len, fp) == len))
            content = buf;
        else
            ash_free(buf);
    }

    fclose(fp);

    return content;
}

const char *ash_scan(const char *prompt)
{
    return ash_term_get(prompt);
}

const char *ash_scan_prompt(const char *prompt)
{
    return ash_term_get_raw(prompt);
}

void ash_print(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    ash_vprint(fmt, ap);
    va_end(ap);
}

struct ash_io_setting {
    bool silent;
};

static struct ash_io_setting setting = {
    .silent = false
};

void ash_io_silent(bool value)
{
    setting.silent = value;
}

void ash_vprint(const char *fmt, va_list ap)
{
    if (!setting.silent)
        vfprintf(stdout, fmt, ap);
    ash_flush();
}

void ash_puts(const char *s)
{
    ash_print("%s\n", s);
}

void ash_putchar(char c)
{
    ash_print("%c", c);
}

void ash_flush(void)
{
    fflush(stdout);
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

static void init(void)
{
    ash_unit_module_init(&ash_module_term);
}

static void destroy(void)
{
    ash_unit_module_destory(&ash_module_term);
}

const struct ash_unit_module ash_module_io = {
    .init = init,
    .destroy = destroy
};
