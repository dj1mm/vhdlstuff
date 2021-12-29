
#include "algorithms.h"

namespace bigint
{

bigunsigned gcd(bigunsigned a, bigunsigned b) {
    bigunsigned trash;
    // Neat in-place alternating technique.
    for (;;) {
        if (b.is_zero())
            return a;
        a.divide_with_remainder(b, trash);
        if (a.is_zero())
            return b;
        b.divide_with_remainder(a, trash);
    }
}

void extended_euclidean(biginteger m, biginteger n,
        biginteger &g, biginteger &r, biginteger &s) {
    if (&g == &r || &g == &s || &r == &s)
        throw "biginteger extended_euclidean: Outputs are aliased";
    biginteger r1(1), s1(0), r2(0), s2(1), q;
    /* Invariants:
     * r1*m(orig) + s1*n(orig) == m(current)
     * r2*m(orig) + s2*n(orig) == n(current) */
    for (;;) {
        if (n.is_zero()) {
            r = r1; s = s1; g = m;
            return;
        }
        // Subtract q times the second invariant from the first invariant.
        m.divide_with_remainder(n, q);
        r1 -= q*r2; s1 -= q*s2;

        if (m.is_zero()) {
            r = r2; s = s2; g = n;
            return;
        }
        // Subtract q times the first invariant from the second invariant.
        n.divide_with_remainder(m, q);
        r2 -= q*r1; s2 -= q*s1;
    }
}

bigunsigned modinv(const biginteger &x, const bigunsigned &n) {
    biginteger g, r, s;
    extended_euclidean(x, n, g, r, s);
    if (g == 1)
        // r*x + s*n == 1, so r*x === 1 (mod n), so r is the answer.
        return (r % n).get_magnitude(); // (r % n) will be nonnegative
    else
        throw "biginteger modinv: x and n have a common factor";
}

bigunsigned modexp(const biginteger &base, const bigunsigned &exponent,
        const bigunsigned &modulus) {
    bigunsigned ans = 1, base2 = (base % modulus).get_magnitude();
    bigunsigned::index_t i = exponent.bit_length();
    // For each bit of the exponent, most to least significant...
    while (i > 0) {
        i--;
        // Square.
        ans *= ans;
        ans %= modulus;
        // And multiply if the bit is a 1.
        if (exponent.get_bit(i)) {
            ans *= base2;
            ans %= modulus;
        }
    }
    return ans;
}

}

