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

#ifndef ASH_IO_H
#define ASH_IO_H

#include <stdarg.h>
#include <stddef.h>

#include "ash/ash.h"
#include "ash/type.h"
#include "ash/unit.h"

extern const struct ash_unit_module ash_module_io;

extern const char *ash_io_read(const char *);
extern void ash_io_silent(bool);

extern const char *ash_scan(void);
extern int   ash_scan_buffer(char *, size_t);
extern void  ash_print(const char *, ...);
extern void  ash_vprint(const char *, va_list);
extern void  ash_puts(const char *);
extern void  ash_putchar(char);
extern void  ash_flush(void);
extern void  ash_print_msg(const char *);
extern void  ash_print_err(const char *);
extern void  ash_print_errno(const char *);
extern void  ash_io_init(void);

#endif
