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

#ifndef ASH_BUILTIN
#define ASH_BUILTIN

/* list of all ash builtin commands */
enum ash_builtin {
    ASH_BUILTIN_ASSERT = 0,
    ASH_BUILTIN_BUILTIN,
    ASH_BUILTIN_CD,
    ASH_BUILTIN_ECHO,
    ASH_BUILTIN_EXIT,
    ASH_BUILTIN_EXPORT,
    ASH_BUILTIN_HELP,
    ASH_BUILTIN_HISTORY,
    ASH_BUILTIN_READ,
    ASH_BUILTIN_SLEEP,
    ASH_BUILTIN_SOURCE,
    ASH_BUILTIN_UNSET,

    /* number of ash builtin commands */
    ASH_BUILTIN_NO
};

extern int ash_builtin_exec(int, int, const char * const *);
extern int ash_builtin_find(const char *);
extern void  ash_print_builtin(void);
extern void  ash_print_err_builtin(const char *, const char *);

#endif
