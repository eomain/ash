#!/usr/bin/env -S ../build/ash/ash -e

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

def range(n)
    return match [ $n ]
         0 to  9 => "0 <= { $n } <= 9",
        10 to 19 => "10 <= { $n } <= 19",
        20 to 29 => "20 <= { $n } <= 29",
        _ => "{ $n } >= 30"
    end;
end

def main()

    for n in 1 to 20
        echo "{ $n }:" match [ $n % 2 ]
            0 => "even",
            _ => "odd"
        end;
    end

    def each(range)
        for n in $range
            echo "range({ $n }):" range($n);
        end
    end

    each(0 to 50);

end
