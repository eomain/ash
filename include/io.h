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

#ifndef ASH_IO
#define ASH_IO

#define PROMPT '$'

/* ash error code used to set and print error message */
enum ash_errno {
    /* use error code of set errno */
    ASH_ERRNO = -1,

    /* error types */
    ARG_MSG_ERR = 0,
    TYPE_ERR,
    RODATA_ERR,
    PARSE_ERR,
    UREG_CMD_ERR,
    SIG_MSG_ERR,

    /* number of ash errors */
    ASH_ERR_NO
};

extern const char *ash_open(const char *);

extern char *ash_scan(void);
extern void  ash_print(const char *, ...);
extern void  ash_puts(const char *);
extern void  ash_putchar(char);
extern void  ash_print_msg(const char *);
extern void  ash_print_err(const char *);
extern void  ash_print_errno(const char *);
extern void  ash_print_err_builtin(const char *, const char *);
extern void  ash_io_init(void);
extern void  ash_set_errno(int);
extern const char *perr(int);

#endif
