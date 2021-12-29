
#ifndef BIGINT_BIGINTEGER_H
#define BIGINT_BIGINTEGER_H

#include "bigunsigned.h"

namespace bigint
{

/* A biginteger object represents a signed integer of size limited only by
 * available memory.  BigUnsigneds support most mathematical operators and can
 * be converted to and from most primitive integer types.
 *
 * A biginteger is just an aggregate of a bigunsigned and a sign.  (It is no
 * longer derived from bigunsigned because that led to harmful implicit
 * conversions.) */
class biginteger {

public:
    typedef bigunsigned::blk_t blk_t;
    typedef bigunsigned::index_t index_t;
    typedef bigunsigned::comparison_result comparison_result;
    static const comparison_result
        less    = bigunsigned::less   ,
        equal   = bigunsigned::equal  ,
        greater = bigunsigned::greater;
    // Enumeration for the sign of a biginteger.
    enum sign_t { negative = -1, zero = 0, positive = 1 };

protected:
    sign_t sign;
    bigunsigned mag;

public:
    // Constructs zero.
    biginteger() : sign(zero), mag() {}

    // Copy constructor
    biginteger(const biginteger& x) : sign(x.sign), mag(x.mag) {};

    // Assignment operator
    void operator=(const biginteger& x);

    // Constructor that copies from a given array of blocks with a sign.
    biginteger(const blk_t *b, index_t blen, sign_t s);

    // Nonnegative constructor that copies from a given array of blocks.
    biginteger(const blk_t* b, index_t blen) : mag(b, blen) {
        sign = mag.is_zero() ? zero : positive;
    }

    // Constructor from a bigunsigned and a sign
    biginteger(const bigunsigned& x, sign_t s);

    // Nonnegative constructor from a bigunsigned
    biginteger(const bigunsigned& x) : mag(x) {
        sign = mag.is_zero() ? zero : positive;
    }

    // Constructors from primitive integer types
    biginteger(unsigned long  x);
    biginteger(         long  x);
    biginteger(unsigned int   x);
    biginteger(         int   x);
    biginteger(unsigned short x);
    biginteger(         short x);

    /* Converters to primitive integer types
     * The implicit conversion operators caused trouble, so these are now
     * named. */
    unsigned long  to_unsigned_long () const;
    long           to_long          () const;
    unsigned int   to_unsigned_int  () const;
    int            to_int           () const;
    unsigned short to_unsigned_short() const;
    short          to_short         () const;
protected:
    // Helper
    template <class X> X convert_to_unsigned_primitive() const;
    template <class X, class UX> X convert_to_signed_primitive() const;
public:

    // ACCESSORS
    sign_t get_sign() const { return sign; }
    /* The client can't do any harm by holding a read-only reference to the
     * magnitude. */
    const bigunsigned& get_magnitude() const { return mag; }

    // Some accessors that go through to the magnitude
    index_t get_length() const { return mag.get_length(); }
    index_t get_capacity() const { return mag.get_capacity(); }
    blk_t get_block(index_t i) const { return mag.get_block(i); }
    bool is_zero() const { return sign == zero; } // A bit special

    // COMPARISONS

    // Compares this to x like Perl's <=>
    comparison_result compare_to(const biginteger& x) const;

    // Ordinary comparison operators
    bool operator==(const biginteger& x) const {
        return sign == x.sign && mag == x.mag;
    }
    bool operator!=(const biginteger& x) const { return !operator==(x); };
    bool operator< (const biginteger& x) const { return compare_to(x) == less   ; }
    bool operator<=(const biginteger& x) const { return compare_to(x) != greater; }
    bool operator>=(const biginteger& x) const { return compare_to(x) != less   ; }
    bool operator> (const biginteger& x) const { return compare_to(x) == greater; }

    // OPERATORS -- See the discussion in bigunsigned.h.
    void add     (const biginteger& a, const biginteger& b);
    void subtract(const biginteger& a, const biginteger& b);
    void multiply(const biginteger& a, const biginteger& b);
    /* See the comment on bigunsigned::divide_with_remainder.  Semantics
     * differ from those of primitive integers when negatives and/or zeros
     * are involved. */
    void divide_with_remainder(const biginteger& b, biginteger& q);
    void negate(const biginteger& a);
   
    /* Bitwise operators are not provided for BigIntegers.  Use
     * get_magnitude to get the magnitude and operate on that instead. */

    biginteger operator+(const biginteger& x) const;
    biginteger operator-(const biginteger& x) const;
    biginteger operator*(const biginteger& x) const;
    biginteger operator/(const biginteger& x) const;
    biginteger operator%(const biginteger& x) const;
    biginteger operator-() const;

    void operator+=(const biginteger& x);
    void operator-=(const biginteger& x);
    void operator*=(const biginteger& x);
    void operator/=(const biginteger& x);
    void operator%=(const biginteger& x);
    void flip_sign();

    // INCREMENT/DECREMENT OPERATORS
    void operator++(   );
    void operator++(int);
    void operator--(   );
    void operator--(int);
};

// NORMAL OPERATORS
/* These create an object to hold the result and invoke
 * the appropriate put-here operation on it, passing
 * this and x.  The new object is then returned. */
inline biginteger biginteger::operator+(const biginteger& x) const {
    biginteger ans;
    ans.add(*this, x);
    return ans;
}
inline biginteger biginteger::operator-(const biginteger& x) const {
    biginteger ans;
    ans.subtract(*this, x);
    return ans;
}
inline biginteger biginteger::operator*(const biginteger& x) const {
    biginteger ans;
    ans.multiply(*this, x);
    return ans;
}
inline biginteger biginteger::operator/(const biginteger& x) const {
    if (x.is_zero()) throw "biginteger::operator/: division by zero";
    biginteger q, r;
    r = *this;
    r.divide_with_remainder(x, q);
    return q;
}
inline biginteger biginteger::operator%(const biginteger& x) const {
    if (x.is_zero()) throw "biginteger::operator%: division by zero";
    biginteger q, r;
    r = *this;
    r.divide_with_remainder(x, q);
    return r;
}
inline biginteger biginteger::operator-() const {
    biginteger ans;
    ans.negate(*this);
    return ans;
}

/*
 * ASSIGNMENT OPERATORS
 *
 * Now the responsibility for making a temporary copy if necessary
 * belongs to the put-here operations.  See Assignment Operators in
 * bigunsigned.h.
 */
inline void biginteger::operator+=(const biginteger& x) {
    add(*this, x);
}
inline void biginteger::operator-=(const biginteger& x) {
    subtract(*this, x);
}
inline void biginteger::operator*=(const biginteger& x) {
    multiply(*this, x);
}
inline void biginteger::operator/=(const biginteger& x) {
    if (x.is_zero()) throw "biginteger::operator/=: division by zero";
    /* The following technique is slightly faster than copying *this first
     * when x is large. */
    biginteger q;
    divide_with_remainder(x, q);
    // *this contains the remainder, but we overwrite it with the quotient.
    *this = q;
}
inline void biginteger::operator%=(const biginteger& x) {
    if (x.is_zero()) throw "biginteger::operator%=: division by zero";
    biginteger q;
    // Mods *this by x.  Don't care about quotient left in q.
    divide_with_remainder(x, q);
}
// This one is trivial
inline void biginteger::flip_sign() {
    sign = sign_t(-sign);
}

}

#endif
