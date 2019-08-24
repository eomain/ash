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

#include <signal.h>

#include "ash/func.h"
#include "ash/int.h"
#include "ash/macro.h"
#include "ash/obj.h"
#include "ash/signal.h"
#include "ash/type.h"
#include "ash/tuple.h"
#include "ash/unit.h"

static const char *signals[] = {
    [ ASH_SIGABRT ] = "(abort)",
    [ ASH_SIGFPE  ] = "(floating point error)",
    [ ASH_SIGILL  ] = "(illegal operation)",
    [ ASH_SIGINT  ] = "(interrupt)",
    [ ASH_SIGSEGV ] = "(segfault)",
    [ ASH_SIGTERM ] = "(terminated)"
};

static inline const char *ash_signal_msg(enum ash_signal_type type)
{
    return signals[type];
}

static inline enum ash_signal_type ash_signal_map(int signal)
{
    switch (signal) {
        case SIGABRT:   return ASH_SIGABRT;
        case SIGFPE:    return ASH_SIGFPE;
        case SIGILL:    return ASH_SIGILL;
        case SIGINT:    return ASH_SIGINT;
        case SIGSEGV:   return ASH_SIGSEGV;
        case SIGTERM:
        default:
                        return ASH_SIGTERM;
    }
}

struct ash_signal_handle {
    struct ash_runtime_env env;
    struct ash_obj *callback;
};

static struct ash_signal_handle handle = {
    .callback = NULL
};

static inline struct ash_obj *
ash_signal_get_callback(struct ash_signal_handle *handle)
{
    return handle->callback;
}

static inline struct ash_runtime_env *
ash_signal_get_env(struct ash_signal_handle *handle)
{
    return &handle->env;
}

static inline bool
ash_signal_has_callback(struct ash_signal_handle *handle)
{
    return (handle->callback) ? true: false;
}

static void ash_signal_handle(enum ash_signal_type type)
{
    isize value;
    struct ash_obj *func;
    struct ash_obj *argv;
    struct ash_obj *signal;
    struct ash_runtime_env *renv;

    value = (isize) type;
    signal = ash_int_from(value);

    struct ash_obj *args[] = {
        signal
    };

    argv = ash_tuple_from(1, args);
    func = ash_signal_get_callback(&handle);
    renv = ash_signal_get_env(&handle);
    ash_func_exec(func, renv, argv);
}

const char *ash_signal_get(int signal)
{
    enum ash_signal_type type;
    type = ash_signal_map(signal);
    return ash_signal_msg(type);
}

static void ash_signal_handle_assert(int signal)
{
    if (!ash_signal_has_callback(&handle))
        return;

    enum ash_signal_type type;
    type = ash_signal_map(signal);
    ash_signal_handle(type);
}

static void init(void)
{
    struct sigaction act;
    act.sa_handler = ash_signal_handle_assert;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    const int const SIGNALS[] = {
        SIGINT,
        SIGQUIT,
        SIGTSTP
    };

    for (size_t i = 0; i < array_length(SIGNALS); ++i)
        sigaction(SIGNALS[i], &act, NULL);
}

const struct ash_unit_module ash_module_signal = {
    .init = init,
    .destroy = NULL
};
