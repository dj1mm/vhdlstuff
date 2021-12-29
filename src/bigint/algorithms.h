
#ifndef BIGINT_ALGORITHMS_H
#define BIGINT_ALGORITHMS_H

#include "biginteger.h"

namespace bigint
{

/* Some mathematical algorithms for big integers.
 * This code is new and, as such, experimental. */

// Returns the greatest common divisor of a and b.
bigunsigned gcd(bigunsigned a, bigunsigned b);

/* Extended Euclidean algorithm.
 * Given m and n, finds gcd g and numbers r, s such that r*m + s*n == g. */
void extended_euclidean(biginteger m, biginteger n,
        biginteger& g, biginteger& r, biginteger& s);

/* Returns the multiplicative inverse of x modulo n, or throws an exception if
 * they have a common factor. */
bigunsigned modinv(const biginteger& x, const bigunsigned& n);

// Returns (base ^ exponent) % modulus.
bigunsigned modexp(const biginteger& base, const bigunsigned& exponent,
        const bigunsigned& modulus);

}

#endif
