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

#ifndef ASH_SESSION_H
#define ASH_SESSION_H

#include "ash/ash.h"
#include "ash/type.h"

struct ash_session_meta {
    const char *id;
};

static inline void
ash_session_meta_init(struct ash_session_meta *meta,
                      const char *id)
{
    meta->id = id;
}

struct ash_session;
struct ash_session_profile;

extern void ash_session_init(struct ash_session *, struct ash_session_meta *);
extern void ash_session_start(struct ash_session *);
extern void ash_session_shutdown(struct ash_session *);
extern void ash_session_quick_shutdown(struct ash_session *);
extern void ash_session_abort(struct ash_session *);
extern bool ash_session_entry(struct ash_session *);
extern void ash_session_set_entry(struct ash_session *);
extern void ash_session_set_status(struct ash_session *, int);
extern struct ash_session *ash_session_default(void);
extern struct ash_session_profile *ash_session_profile(struct ash_session *);

static inline void
ash_session_shutdown_status(struct ash_session *session, int status)
{
    ash_session_set_status(session, status);
    ash_session_shutdown(session);
}

extern void
ash_session_profile_set_profile(struct ash_session_profile *, bool);

struct ash_profile {
    ash_flag login;
    ash_flag interact;
    ash_flag eval;
    ash_flag silent;
    ash_flag greet;
    ash_flag status;
    ash_flag profile;

    const char *entry;
};

#endif
