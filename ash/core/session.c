/* Copyright 2019 eomain
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

#include <stdlib.h>

#include "ash/env.h"
#include "ash/bool.h"
#include "ash/int.h"
#include "ash/obj.h"
#include "ash/script.h"
#include "ash/session.h"
#include "ash/str.h"
#include "ash/type.h"
#include "ash/unit.h"
#include "ash/var.h"
#include "ash/type/map.h"

#define USER "USER"
#define ROOT_UID 0

#ifdef ASH_PLATFORM_POSIX
    #include <pwd.h>
    #include <unistd.h>
#endif

struct user {
    auid uid;
    auid euid;
    bool root;
    const char *name;
};

static void
user_init(struct user *user)
{
    struct passwd *pass;
    pass = getpwent();

    user->uid = getuid();
    user->euid = geteuid();
    user->root = (user->euid == ROOT_UID);
    user->name = pass->pw_name;

    endpwent();
}

static void
user_var(struct user *user)
{
    struct ash_obj *obj;
    obj = ash_map_new();

    ash_map_insert(obj, "uid",  ash_int_from((isize) user->uid));
    ash_map_insert(obj, "euid", ash_int_from((isize) user->euid));
    ash_map_insert(obj, "root", ash_bool_from(user->root));
    ash_map_insert(obj, "name", ash_str_from(user->name));

    ash_var_set(USER, obj);
}

static inline bool
ash_session_meta_login(struct ash_session_meta *meta)
{
    return (*meta->id == '-');
}

struct ash_session_profile {
    bool profile;
    bool logout;
    bool history;
};

static void
ash_session_profile_init(struct ash_session_profile *session)
{
    session->profile = true;
    session->logout = true;
    session->history = true;
}

void
ash_session_profile_set_profile(struct ash_session_profile *session, bool value)
{
    session->profile = value;
}

static void
ash_session_profile_start(struct ash_session_profile *session)
{
    if (session->profile)
        ash_profile_script();
}

static void
ash_session_profile_shutdown(struct ash_session_profile *session)
{
    if (session->logout)
        ash_logout_script();
}

struct ash_session {
    bool login;
    bool entry;
    bool script;
    int status;
    apid pid;
    struct user user;
    struct ash_session_profile profile;
};

struct ash_session session;

void ash_session_init(struct ash_session *session,
                      struct ash_session_meta *meta)
{
    session->login = ash_session_meta_login(meta);
    session->entry = false;
    session->status = 0;
    session->script = false;
    session->pid = getpid();
    user_init(&session->user);
    user_var(&session->user);
    ash_session_profile_init(&session->profile);
}

void ash_session_start(struct ash_session *session)
{
    if (session->login)
        ash_env_profile();

    ash_session_profile_start(&session->profile);
}

void ash_session_shutdown(struct ash_session *session)
{
    if (!session->script)
        ash_session_profile_shutdown(&session->profile);
    exit(session->status);
}

void ash_session_quick_shutdown(struct ash_session *session)
{
    exit(session->status);
}

bool ash_session_entry(struct ash_session *session)
{
    return session->entry;
}

void ash_session_set_entry(struct ash_session *session)
{
    session->entry = true;
}

void ash_session_set_script(struct ash_session *session)
{
    session->script = true;
}

void ash_session_abort(struct ash_session *session)
{
    abort();
}

struct ash_session *ash_session_default(void)
{
    return &session;
}

struct ash_session_profile *ash_session_profile(struct ash_session *session)
{
    return &session->profile;
}

void ash_session_set_status(struct ash_session *session, int status)
{
    session->status = status;
}
