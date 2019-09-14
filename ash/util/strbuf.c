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

#include "ash/mem.h"
#include "ash/ops.h"
#include "ash/util/strbuf.h"

void strbuf_init(struct strbuf *buf, size_t size)
{
    buf->buf = ash_zalloc( (sizeof *buf->buf)  * (size + 1));
    buf->index = 0;
    buf->size = (size + 1);
}

const char *strbuf_get(struct strbuf *buf)
{
    return buf->buf;
}

static inline size_t strbuf_length(struct strbuf *buf)
{
    return (buf->size - 1);
}

static void strbuf_resize(struct strbuf *buf, size_t n)
{
    if (buf->index < strbuf_length(buf)) {
        size_t nsize;
        nsize = (sizeof *buf->buf) * (buf->size + n);
        buf->buf = ash_realloc(buf->buf, nsize);
        buf->buf[buf->size] = '\0';
        buf->size = nsize;
    }
}

void strbuf_push_char(struct strbuf *buf, char c)
{
    strbuf_resize(buf, 1);

    buf->buf[buf->index] = c;
    buf->index++;
}

void strbuf_push_str(struct strbuf *buf, const char *s)
{
    size_t len;
    len = ash_strlen(s);
    strbuf_resize(buf, len);

    for (size_t i = 0; i < len; ++i)
        buf->buf[(buf->index + i)] = s[i];

    buf->index += len;
}
