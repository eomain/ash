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

def main()
    let UPPER_BOUND := 100;

    def rand()
        rand;
        return `$__EXIT__ % $UPPER_BOUND`;
    end

    def input(prompt)
        read -p $prompt input;
        return $input;
    end

    while [ true ]
        let num := rand();
        let guess := input("Guess a number: ");

        if [ $guess = "" ]
            next;
        end

        echo -s "Your guess is " match [ $guess ]
            "{ $num }" => "correct",
            _ => "incorrect"
        end ", the answer is " $num ".";

        if [ input("Try again? (y or n): ") != "y" ]
            break;
        end
    end

end
