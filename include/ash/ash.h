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

#ifndef ASH_ASH_H
#define ASH_ASH_H

/* program name */
#define PNAME "ash"

/* shell name */
#define ASH_NAME "acorn shell"

/* current shell version */
#define ASH_VERSION_MAJOR "0"
#define ASH_VERSION_MINOR "1"
#define ASH_VERSION_MICRO "8"

#define ASH_VERSION ASH_VERSION_MAJOR "." \
                    ASH_VERSION_MINOR "." \
                    ASH_VERSION_MICRO

#ifdef ASH_BUILD_STABLE
/* the type of the build, either stable or unstable */
    #define ASH_BUILD_TYPE "(stable)"
#else
    #define ASH_BUILD_TYPE "(unstable)"
#endif

/* the data-time of the build */
#define ASH_BUILD_DATETIME __DATE__ " " __TIME__

#define ASH_BUILD_INFO ASH_VERSION " " \
                       ASH_BUILD_TYPE " " \
                       ASH_BUILD_DATETIME

/* denotes an `ok` termination status */
#define ASH_STATUS_OK  0
/* denotes an `err` termination status */
#define ASH_STATUS_ERR 1

typedef enum {

    ASH_FLAG_SET   = 1,
    ASH_FLAG_RESET = 0

} ash_flag;

/* display the ash help prompt */
extern void ash_print_help(void);
/* logout of ash session */
extern void ash_logout(void);
/* abort shell session */
extern void ash_abort(const char *);

#endif
