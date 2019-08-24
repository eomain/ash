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

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ash/env.h"
#include "ash/ops.h"
#include "ash/mem.h"
#include "ash/str.h"
#include "ash/type.h"
#include "ash/util.h"
#include "ash/var.h"
#include "ash/lang/runtime.h"

static const char *format_check(const char *fmt)
{
    char c;
    bool ws = true;
    bool end = false;
    const char *str = NULL;

    fmt++;

    while ((c = *fmt)) {

        if (c == '}')
            return str;

        if (c != ' ') {

            if (end) {
                return NULL;
            } else if (c == '$') {
                if (ws) {
                    ws = false;
                    str = fmt;
                } else {
                    return NULL;
                }
            } else if (!str) {
                return NULL;
            }

        } else if (!ws) {
            end = true;
        }

        fmt++;
    }

    return NULL;
}

static bool format_occur(const char *s, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '{') {
            if (format_check(&s[i]))
                return true;
        }
    }

    return false;
}

static size_t format_length(const char *s)
{
    char c;
    size_t i = 0;

    while ((c = *(s++))) {
        if (c == '}' || c == ' ')
            break;
        i++;
    }

    return i;
}

static const char *
format_value(const char *s, struct ash_runtime_env *renv)
{
    const char *value = NULL;
    struct ash_obj *obj;
    struct ash_var *av;

    size_t size;
    size = format_length(s);
    char var[size];
    memset(var, 0, sizeof var);
    strncpy(var, ++s, (size - 1));

    if (renv)
        av = runtime_get_var(renv, var);
    else
        av = ash_var_get(var);

    if (av && (obj = ash_var_obj(av))) {
        value = ash_str_get(ash_obj_str(obj));
        ash_obj_dec_rc(obj);
    }

    return value;
}

const char *
format(const char *fmt, struct ash_runtime_env *renv)
{
    size_t len;
    bool occur;
    len = strlen(fmt);

    if ((occur = format_occur(fmt, len))) {
        char c = '\0';
        const char *s, *var;
        struct strbuf buf;
        strbuf_init(&buf, len);

        for (size_t i = 0; i < len; ++i) {
            if (fmt[i] == '{' && c != '\\') {
                if ((s = format_check(&fmt[i]))) {
                    if ((var = format_value(s, renv)))
                        strbuf_push_str(&buf, var);
                    while ((fmt[i]) != '}' && i != len)
                        i++;
                    c = '\0';
                    continue;
                }
            }

            if (fmt[i] == '\\') {
                if (c != '\\') {
                    c = fmt[i];
                    continue;
                }
            }

            c = fmt[i];
            strbuf_push_char(&buf, c);
        }

        return strbuf_get(&buf);
    }

    return NULL;
}

const char *
ash_ops_format(const char *fmt, struct ash_runtime_env *renv)
{
    const char *string;
    if ((string = format(fmt, renv)))
        fmt = string;

    if ((string = ash_ops_tilde(fmt)))
        fmt = string;
    return fmt;
}

const char *ash_ops_tilde(const char *s)
{
    if (s[0] != '~')
        return NULL;
    if (s[1] == '/')
        ++s;

    size_t max;
    char *fmt;
    const char *home;

    max = ash_env_get_pwd_max();
    home = ash_env_get_home();
    fmt = ash_zalloc(max + 1);

    if ((*(++s)))
        sprintf(fmt, "%s%c%s", home, '/', s);
    else
        strcpy(fmt, home);
    return fmt;
}

size_t ash_strlen(const char *s)
{
    return strlen(s);
}

const char *ash_strcpy(const char *s)
{
    if (!s)
        return NULL;
    return strcpy(ash_alloc(ash_strlen(s) + 1), s);
}

const char *ash_strcat(const char *s1, const char *s2)
{
    if (!s1 && !s2)
        return NULL;
    else if (!s1)
        return ash_strcpy(s2);
    else if (!s2)
        return ash_strcpy(s1);

    size_t len;
    char *string;

    len = strlen(s1) + strlen(s2);
    string = ash_alloc(len + 1);
    sprintf(string, "%s%s", s1, s2);

    return string;
}

void ash_ops_fmt_num(char *fmt, long num, size_t size)
{
    snprintf(fmt, size, "%ld", num);
}

/* TODO: check hex */

bool ash_stoi_check(const char *s)
{
    char c;
    bool ck = true;

    while ((c = *(s++))) {
        if (!(c >= '0' && c <= '9'))
            ck = false;
    }

    return ck;
}
