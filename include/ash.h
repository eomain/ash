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

#ifndef ASH_ASH
#define ASH_ASH

#define PNAME "ash"

#define ASH_VERSION_MAJOR "0"
#define ASH_VERSION_MINOR "1"
#define ASH_VERSION_MICRO "5"

#define ASH_VERSION ASH_VERSION_MAJOR "." \
                    ASH_VERSION_MINOR "." \
                    ASH_VERSION_MICRO

/* set ash exit status */
extern void ash_set_status(int);
/* get interactive mode status */
extern int  ash_get_interactive(void);
/* get silent mode status */
extern int ash_get_silent(void);
/* display the ash help prompt */
extern void ash_print_help(void);
/* logout of ash session */
extern void ash_logout(void);
/* get login shell status */
extern int  ash_get_login(void);
/* abort shell session */
extern void ash_abort(const char *);

#endif
