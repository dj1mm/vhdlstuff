
#include "biginteger.h"

namespace bigint
{

void biginteger::operator=(const biginteger& x) {
    // Calls like a = a have no effect
    if (this == &x)
        return;
    // Copy sign
    sign = x.sign;
    // Copy the rest
    mag = x.mag;
}

biginteger::biginteger(const blk_t* b, index_t blen, sign_t s) : mag(b, blen) {
    switch (s) {
    case zero:
        if (!mag.is_zero())
            throw "biginteger::biginteger(const blk_t *, index_t, sign_t): Cannot use a sign of zero with a nonzero magnitude";
        sign = zero;
        break;
    case positive:
    case negative:
        // If the magnitude is zero, force the sign to zero.
        sign = mag.is_zero() ? zero : s;
        break;
    default:
        /* g++ seems to be optimizing out this case on the assumption
         * that the sign is a valid member of the enumeration.  Oh well. */
        throw "biginteger::biginteger(const blk_t *, index_t, sign_t): Invalid sign";
    }
}

biginteger::biginteger(const bigunsigned& x, sign_t s) : mag(x) {
    switch (s) {
    case zero:
        if (!mag.is_zero())
            throw "biginteger::biginteger(const bigunsigned& , sign_t): Cannot use a sign of zero with a nonzero magnitude";
        sign = zero;
        break;
    case positive:
    case negative:
        // If the magnitude is zero, force the sign to zero.
        sign = mag.is_zero() ? zero : s;
        break;
    default:
        /* g++ seems to be optimizing out this case on the assumption
         * that the sign is a valid member of the enumeration.  Oh well. */
        throw "biginteger::biginteger(const bigunsigned& , sign_t): Invalid sign";
    }
}

/* CONSTRUCTION FROM PRIMITIVE INTEGERS
 * Same idea as in bigunsigned.cc, except that negative input results in a
 * negative biginteger instead of an exception. */

// Done longhand to let us use initialization.
biginteger::biginteger(unsigned long  x) : mag(x) { sign = mag.is_zero() ? zero : positive; }
biginteger::biginteger(unsigned int   x) : mag(x) { sign = mag.is_zero() ? zero : positive; }
biginteger::biginteger(unsigned short x) : mag(x) { sign = mag.is_zero() ? zero : positive; }

// For signed input, determine the desired magnitude and sign separately.

namespace {
    template <class X, class UX>
    biginteger::blk_t magOf(X x) {
        /* UX(...) cast needed to stop short(-2^15), which negates to
         * itself, from sign-extending in the conversion to blk_t. */
        return biginteger::blk_t(x < 0 ? UX(-x) : x);
    }
    template <class X>
    biginteger::sign_t signOf(X x) {
        return (x == 0) ? biginteger::zero
            : (x > 0) ? biginteger::positive
            : biginteger::negative;
    }
}

biginteger::biginteger(long  x) : sign(signOf(x)), mag(magOf<long , unsigned long >(x)) {}
biginteger::biginteger(int   x) : sign(signOf(x)), mag(magOf<int  , unsigned int  >(x)) {}
biginteger::biginteger(short x) : sign(signOf(x)), mag(magOf<short, unsigned short>(x)) {}

// CONVERSION TO PRIMITIVE INTEGERS

/* Reuse bigunsigned's conversion to an unsigned primitive integer.
 * The friend is a separate function rather than
 * biginteger::convert_to_unsigned_primitive to avoid requiring bigunsigned to
 * declare biginteger. */
template <class X>
inline X convert_bigunsigned_to_primitive_access(const bigunsigned& a) {
    return a.convert_to_primitive<X>();
}

template <class X>
X biginteger::convert_to_unsigned_primitive() const {
    if (sign == negative)
        throw "biginteger::to<Primitive>: "
            "Cannot convert a negative integer to an unsigned type";
    else
        return convert_bigunsigned_to_primitive_access<X>(mag);
}

/* Similar to bigunsigned::convert_to_primitive, but split into two cases for
 * nonnegative and negative numbers. */
template <class X, class UX>
X biginteger::convert_to_signed_primitive() const {
    if (sign == zero)
        return 0;
    else if (mag.get_length() == 1) {
        // The single block might fit in an X.  Try the conversion.
        blk_t b = mag.get_block(0);
        if (sign == positive) {
            X x = X(b);
            if (x >= 0 && blk_t(x) == b)
                return x;
        } else {
            X x = -X(b);
            /* UX(...) needed to avoid rejecting conversion of
             * -2^15 to a short. */
            if (x < 0 && blk_t(UX(-x)) == b)
                return x;
        }
        // Otherwise fall through.
    }
    throw "biginteger::to<Primitive>: "
        "Value is too big to fit in the requested type";
}

unsigned long  biginteger::to_unsigned_long () const { return convert_to_unsigned_primitive<unsigned long >       (); }
unsigned int   biginteger::to_unsigned_int  () const { return convert_to_unsigned_primitive<unsigned int  >       (); }
unsigned short biginteger::to_unsigned_short() const { return convert_to_unsigned_primitive<unsigned short>       (); }
long           biginteger::to_long         () const { return convert_to_signed_primitive  <long , unsigned long> (); }
int            biginteger::to_int          () const { return convert_to_signed_primitive  <int  , unsigned int>  (); }
short          biginteger::to_short        () const { return convert_to_signed_primitive  <short, unsigned short>(); }

// COMPARISON
biginteger::comparison_result biginteger::compare_to(const biginteger& x) const {
    // A greater sign implies a greater number
    if (sign < x.sign)
        return less;
    else if (sign > x.sign)
        return greater;
    else switch (sign) {
        // If the signs are the same...
    case zero:
        return equal; // Two zeros are equal
    case positive:
        // Compare the magnitudes
        return mag.compare_to(x.mag);
    case negative:
        // Compare the magnitudes, but return the opposite result
        return comparison_result(-mag.compare_to(x.mag));
    default:
        throw "biginteger internal error";
    }
}

/* COPY-LESS OPERATIONS
 * These do some messing around to determine the sign of the result,
 * then call one of bigunsigned's copy-less operations. */

// See remarks about aliased calls in bigunsigned.cc .
#define DTRT_ALIASED(cond, op) \
    if (cond) { \
        biginteger tmpThis; \
        tmpThis.op; \
        *this = tmpThis; \
        return; \
    }

void biginteger::add(const biginteger& a, const biginteger& b) {
    DTRT_ALIASED(this == &a || this == &b, add(a, b));
    // If one argument is zero, copy the other.
    if (a.sign == zero)
        operator=(b);
    else if (b.sign == zero)
        operator=(a);
    // If the arguments have the same sign, take the
    // common sign and add their magnitudes.
    else if (a.sign == b.sign) {
        sign = a.sign;
        mag.add(a.mag, b.mag);
    } else {
        // Otherwise, their magnitudes must be compared.
        switch (a.mag.compare_to(b.mag)) {
        case equal:
            // If their magnitudes are the same, copy zero.
            mag = 0;
            sign = zero;
            break;
            // Otherwise, take the sign of the greater, and subtract
            // the lesser magnitude from the greater magnitude.
        case greater:
            sign = a.sign;
            mag.subtract(a.mag, b.mag);
            break;
        case less:
            sign = b.sign;
            mag.subtract(b.mag, a.mag);
            break;
        }
    }
}

void biginteger::subtract(const biginteger& a, const biginteger& b) {
    // Notice that this routine is identical to biginteger::add,
    // if one replaces b.sign by its opposite.
    DTRT_ALIASED(this == &a || this == &b, subtract(a, b));
    // If a is zero, copy b and flip its sign.  If b is zero, copy a.
    if (a.sign == zero) {
        mag = b.mag;
        // Take the negative of _b_'s, sign, not ours.
        // Bug pointed out by Sam Larkin on 2005.03.30.
        sign = sign_t(-b.sign);
    } else if (b.sign == zero)
        operator=(a);
    // If their signs differ, take a.sign and add the magnitudes.
    else if (a.sign != b.sign) {
        sign = a.sign;
        mag.add(a.mag, b.mag);
    } else {
        // Otherwise, their magnitudes must be compared.
        switch (a.mag.compare_to(b.mag)) {
            // If their magnitudes are the same, copy zero.
        case equal:
            mag = 0;
            sign = zero;
            break;
            // If a's magnitude is greater, take a.sign and
            // subtract a from b.
        case greater:
            sign = a.sign;
            mag.subtract(a.mag, b.mag);
            break;
            // If b's magnitude is greater, take the opposite
            // of b.sign and subtract b from a.
        case less:
            sign = sign_t(-b.sign);
            mag.subtract(b.mag, a.mag);
            break;
        }
    }
}

void biginteger::multiply(const biginteger& a, const biginteger& b) {
    DTRT_ALIASED(this == &a || this == &b, multiply(a, b));
    // If one object is zero, copy zero and return.
    if (a.sign == zero || b.sign == zero) {
        sign = zero;
        mag = 0;
        return;
    }
    // If the signs of the arguments are the same, the result
    // is positive, otherwise it is negative.
    sign = (a.sign == b.sign) ? positive : negative;
    // Multiply the magnitudes.
    mag.multiply(a.mag, b.mag);
}

/*
 * DIVISION WITH REMAINDER
 * Please read the comments before the definition of
 * `bigunsigned::divide_with_remainder' in `bigunsigned.cc' for lots of
 * information you should know before reading this function.
 *
 * Following Knuth, I decree that x / y is to be
 * 0 if y==0 and floor(real-number x / y) if y!=0.
 * Then x % y shall be x - y*(integer x / y).
 *
 * Note that x = y * (x / y) + (x % y) always holds.
 * In addition, (x % y) is from 0 to y - 1 if y > 0,
 * and from -(|y| - 1) to 0 if y < 0.  (x % y) = x if y = 0.
 *
 * Examples: (q = a / b, r = a % b)
 *    a    b    q    r
 *    ===    ===    ===    ===
 *    4    3    1    1
 *    -4    3    -2    2
 *    4    -3    -2    -2
 *    -4    -3    1    -1
 */
void biginteger::divide_with_remainder(const biginteger& b, biginteger& q) {
    // Defend against aliased calls;
    // same idea as in bigunsigned::divide_with_remainder .
    if (this == &q)
        throw "biginteger::divide_with_remainder: Cannot write quotient and remainder into the same variable";
    if (this == &b || &q == &b) {
        biginteger tmpB(b);
        divide_with_remainder(tmpB, q);
        return;
    }

    // Division by zero gives quotient 0 and remainder *this
    if (b.sign == zero) {
        q.mag = 0;
        q.sign = zero;
        return;
    }
    // 0 / b gives quotient 0 and remainder 0
    if (sign == zero) {
        q.mag = 0;
        q.sign = zero;
        return;
    }

    // Here *this != 0, b != 0.

    // Do the operands have the same sign?
    if (sign == b.sign) {
        // Yes: easy case.  Quotient is zero or positive.
        q.sign = positive;
    } else {
        // No: harder case.  Quotient is negative.
        q.sign = negative;
        // Decrease the magnitude of the dividend by one.
        mag--;
        /*
         * We tinker with the dividend before and with the
         * quotient and remainder after so that the result
         * comes out right.  To see why it works, consider the following
         * list of examples, where A is the magnitude-decreased
         * a, Q and R are the results of bigunsigned division
         * with remainder on A and |b|, and q and r are the
         * final results we want:
         *
         *    a    A    b    Q    R    q    r
         *    -3    -2    3    0    2    -1    0
         *    -4    -3    3    1    0    -2    2
         *    -5    -4    3    1    1    -2    1
         *    -6    -5    3    1    2    -2    0
         *
         * It appears that we need a total of 3 corrections:
         * Decrease the magnitude of a to get A.  Increase the
         * magnitude of Q to get q (and make it negative).
         * Find r = (b - 1) - R and give it the desired sign.
         */
    }

    // Divide the magnitudes.
    mag.divide_with_remainder(b.mag, q.mag);

    if (sign != b.sign) {
        // More for the harder case (as described):
        // Increase the magnitude of the quotient by one.
        q.mag++;
        // Modify the remainder.
        mag.subtract(b.mag, mag);
        mag--;
    }

    // sign_t of the remainder is always the sign of the divisor b.
    sign = b.sign;

    // Set signs to zero as necessary.  (Thanks David Allen!)
    if (mag.is_zero())
        sign = zero;
    if (q.mag.is_zero())
        q.sign = zero;

    // WHEW!!!
}

// Negation
void biginteger::negate(const biginteger& a) {
    DTRT_ALIASED(this == &a, negate(a));
    // Copy a's magnitude
    mag = a.mag;
    // Copy the opposite of a.sign
    sign = sign_t(-a.sign);
}

// INCREMENT/DECREMENT OPERATORS

// Prefix increment
void biginteger::operator++() {
    if (sign == negative) {
        mag--;
        if (mag == 0)
            sign = zero;
    } else {
        mag++;
        sign = positive; // if not already
    }
}

// Postfix increment: same as prefix
void biginteger::operator++(int) {
    operator++();
}

// Prefix decrement
void biginteger::operator--() {
    if (sign == positive) {
        mag--;
        if (mag == 0)
            sign = zero;
    } else {
        mag++;
        sign = negative;
    }
}

// Postfix decrement: same as prefix
void biginteger::operator--(int) {
    operator--();
}

}

