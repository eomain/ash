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

#include <stddef.h>

#include "ash/alias.h"
#include "ash/env.h"
#include "ash/io.h"
#include "ash/module.h"
#include "ash/signal.h"
#include "ash/unit.h"
#include "ash/core/exec.h"
#include "ash/ffi/ffi.h"

static const struct ash_unit_module * const unit[] = {
    &ash_module_module,
    &ash_module_env,
    &ash_module_io,
    &ash_module_signal,
    &ash_module_alias,
    &ash_module_exec,
    &ash_module_ffi,
};

void ash_unit_init(void)
{
    for (size_t i = 0; i < array_length(unit); i++)
        ash_unit_module_init(unit[i]);
}

void ash_unit_destroy(void)
{
    for (size_t i = 0; i < array_length(unit); i++)
        ash_unit_module_destory(unit[i]);
}
