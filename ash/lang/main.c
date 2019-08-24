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

#include "ash/env.h"
#include "ash/io.h"
#include "ash/lang/ast.h"
#include "ash/lang/lang.h"
#include "ash/lang/lex.h"
#include "ash/lang/main.h"
#include "ash/lang/parser.h"
#include "ash/lang/runtime.h"

static int ash_main_scan(struct input *input, struct ash_tk_set *set)
{
    const char *content;
    content = input_text_content(input);

    if (lex_scan_input(set, content))
        return -1;
    return 0;
}

static int ash_main_parse(struct input *input, struct ast_prog *prog)
{
    struct ash_tk_set set;
    ash_tk_set_init(&set);
    if (ash_main_scan(input, &set))
        return -1;

    if (ash_tk_set_empty(&set))
        return -1;

    struct parser_meta meta;
    parser_meta_init(&meta, input, &set);
    return parser_ast_construct(prog, &meta);
}

static inline
void ash_main_prompt(struct input *input)
{
    const char *text;
    /*ash_prompt();*/

    if ((text = ash_scan()))
        input_text_init(input, text);
}

int ash_main_input(struct input *input)
{
    struct ast_prog prog;
    struct ash_runtime runtime;
    struct ash_runtime_env renv;
    struct ash_runtime_prog rprog;

    runtime_init(&runtime, RUNTIME_ID_DEFAULT);
    runtime_env_rt_init(&renv, &runtime, NULL, NULL);
    if (ash_main_parse(input, &prog))
        return -1;
    runtime_prog_init(&rprog, prog, renv);
    return runtime_exec(&rprog);
}

void ash_main(void)
{
    struct input input;
    struct ast_prog prog;
    struct ash_runtime runtime;
    struct ash_runtime_env renv;
    struct ash_runtime_prog rprog;

    runtime_init(&runtime, RUNTIME_ID_DEFAULT);
    runtime_env_rt_init(&renv, &runtime, NULL, NULL);

    for (;;) {
        ash_main_prompt(&input);
        if (ash_main_parse(&input, &prog))
            continue;

        runtime_prog_init(&rprog, prog, renv);
        runtime_exec(&rprog);
    }
}
