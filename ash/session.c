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
#include "ash/script.h"
#include "ash/session.h"
#include "ash/type.h"
#include "ash/unit.h"

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
    bool root;
    bool entry;
    bool script;
    int status;
    auid uid;
    apid pid;
    struct ash_session_profile profile;
};

struct ash_session session;

void ash_session_init(struct ash_session *session,
                      struct ash_session_meta *meta)
{
    session->login = ash_session_meta_login(meta);
    session->root = ash_env_root();
    session->entry = false;
    session->status = 0;
    session->script = false;
    session->uid = ash_env_uid();
    session->pid = ash_env_pid();
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
