#!/usr/bin/env -S ash -e

# Copyright 2018 eomain
# this program is licensed under the 2-clause BSD license
# see COPYRIGHT for the full license info

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# ash (acorn shell) test script

# print ash title

def title(logo)
    clear;
    echo $logo;
    sleep 500000;
end

def run()
    title($ASH_0);
    title($ASH_1);
end

def main()
    tput init;
    tput civis;
    tput clear;

    while [ true ]
        run();
    end
end


let ASH_0 := "
     ░█████     ░█████   ░█     ░█
    ░█    ░█   ░█    ░█  ░█     ░█
    ░█    ░█   ░█        ░█     ░█
    ░███████   ░███████  ░████████
    ░█    ░█         ░█  ░█     ░█
    ░█    ░█   ░█    ░█  ░█     ░█
    ░█    ░█    ░████    ░█     ░█

                 $$$
                  $$
                $$$$$$
              $$$$$$$$$$
             $$$oooooo$$$
             $$oooooooo$$
              $oooooooo$
               oooooooo
                 oooo
";

let ASH_1 := "
     █░░░░░     █░░░░░   █░     █░
    █░    █░   █░    █░  █░     █░
    █░    █░   █░        █░     █░
    █░░░░░░░   █░░░░░░░  █░░░░░░░░
    █░    █░         █░  █░     █░
    █░    █░   █░    █░  █░     █░
    █░    █░    █░░░░    █░     █░

                 $$$
                  $$
                $$$$$$
              $$$$$$$$$$
             $$$oooooo$$$
             $$oooooooo$$
              $oooooooo$
               oooooooo
                 oooo
";
