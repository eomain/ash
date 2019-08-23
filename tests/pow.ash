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

mod math

    def abs(n)
        return ? [ $n < 0 ] `$n * -1`: $n;
    end

    def pow(a, b)
        return match [ $b ]
            0 => 1,
            1 => $a,
            _ => `$a * pow($a, $b - 1)`
        end;
    end

    def sqr(n)
        return pow($n, 2);
    end

    def sqrt(n)
        if [ $n < 0 ]
            return;
        elif [ $n = 0 ]
            return 0;
        end

        let v := $n;
        while [ $v > 0 ]
            if [ sqr($v) = $n ]
                return $v;
            end
            v := `$v - 1`;
        end

        if [ $v != 0 ]
            return $v;
        end
    end

    def cube(n)
        return pow($n, 3);
    end

end

def main()
    echo "math::pow(2, 3):" math::pow(2, 3);
    echo "math::pow(3, 4):" math::pow(3, 4);
    echo "math::pow(2, 5):" math::pow(2, 5);
    echo "math::pow(4, 3):" math::pow(4, 3);
    echo "math::pow(6, 3):" math::pow(6, 3);
    echo "math::pow(8, 0):" math::pow(8, 0);
    echo;
    echo "math::sqr(2):"  math::sqr(2);
    echo "math::sqr(3):"  math::sqr(3);
    echo "math::sqr(5):"  math::sqr(5);
    echo "math::sqr(8):"  math::sqr(8);
    echo "math::sqr(13):" math::sqr(13);
    echo "math::sqr(math::abs(`-7`)):" math::sqr(math::abs(`-7`));
    echo;
    echo "math::cube(1):"     math::cube(1);
    echo "math::cube(`-7`):"  math::cube(`-7`);
    echo "math::cube(12):"    math::cube(12);
    echo "math::cube(`-25`):" math::cube(`-25`);
    echo "math::cube(15):"    math::cube(15);
    echo "math::cube(math::abs(`-9`)):" math::cube(math::abs(`-7`));
    echo;
    echo "math::sqrt(1):"   math::sqrt(1);
    echo "math::sqrt(16):"  math::sqrt(16);
    echo "math::sqrt(36):"  math::sqrt(36);
    echo "math::sqrt(49):"  math::sqrt(49);
    echo "math::sqrt(100):" math::sqrt(100);
    echo "math::sqrt(144):" math::sqrt(144);
    echo "math::sqrt(math::abs(`-225`)):" math::sqrt(math::abs(`-225`));
end
