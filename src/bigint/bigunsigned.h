
#ifndef BIGINT_BIGUNSIGNED_H
#define BIGINT_BIGUNSIGNED_H

#include "number_like_array.h"

namespace bigint
{

/* A bigunsigned object represents a nonnegative integer of size limited only by
 * available memory.  BigUnsigneds support most mathematical operators and can
 * be converted to and from most primitive integer types.
 *
 * The number is stored as a number_like_array of unsigned longs as if it were
 * written in base 256^sizeof(unsigned long).  The least significant block is
 * first, and the length is such that the most significant block is nonzero. */
class bigunsigned : protected number_like_array<unsigned long> {

public:
    // Enumeration for the result of a comparison.
    enum comparison_result { less = -1, equal = 0, greater = 1 };

    // BigUnsigneds are built with a blk_t type of unsigned long.
    typedef unsigned long blk_t;

    typedef number_like_array<blk_t>::index_t index_t;
    using number_like_array<blk_t>::N;

protected:
    // Creates a bigunsigned with a capacity; for internal use.
    bigunsigned(int, index_t c) : number_like_array<blk_t>(0, c) {}

    // Decreases len to eliminate any leading zero blocks.
    void zap_leading_zeros() {
        while (len > 0 && blk[len - 1] == 0)
            len--;
    }

public:
    // Constructs zero.
    bigunsigned() : number_like_array<blk_t>() {}

    // Copy constructor
    bigunsigned(const bigunsigned& x) : number_like_array<blk_t>(x) {}

    // Assignment operator
    void operator=(const bigunsigned& x) {
        number_like_array<blk_t>::operator=(x);
    }

    // Constructor that copies from a given array of blocks.
    bigunsigned(const blk_t *b, index_t blen) : number_like_array<blk_t>(b, blen) {
        // Eliminate any leading zeros we may have been passed.
        zap_leading_zeros();
    }

    // Destructor.  number_like_array does the delete for us.
    ~bigunsigned() {}

    // Constructors from primitive integer types
    bigunsigned(unsigned long  x);
    bigunsigned(         long  x);
    bigunsigned(unsigned int   x);
    bigunsigned(         int   x);
    bigunsigned(unsigned short x);
    bigunsigned(         short x);
protected:
    // Helpers
    template <class X> void init_from_primitive       (X x);
    template <class X> void init_from_signed_primitive(X x);
public:

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
    // Helpers
    template <class X> X convert_to_signed_primitive() const;
    template <class X> X convert_to_primitive       () const;
public:

    // BIT/BLOCK ACCESSORS

    // Expose these from number_like_array directly.
    using number_like_array<blk_t>::get_capacity;
    using number_like_array<blk_t>::get_length;

    /* Returns the requested block, or 0 if it is beyond the length (as if
     * the number had 0s infinitely to the left). */
    blk_t get_block(index_t i) const { return i >= len ? 0 : blk[i]; }
    /* Sets the requested block.  The number grows or shrinks as necessary. */
    void set_block(index_t i, blk_t new_block);

    // The number is zero if and only if the canonical length is zero.
    bool is_zero() const { return number_like_array<blk_t>::is_empty(); }

    /* Returns the length of the number in bits, i.e., zero if the number
     * is zero and otherwise one more than the largest value of bi for
     * which get_bit(bi) returns true. */
    index_t bit_length() const;
    /* Get the state of bit bi, which has value 2^bi.  Bits beyond the
     * number's length are considered to be 0. */
    bool get_bit(index_t bi) const {
        return (get_block(bi / N) & (blk_t(1) << (bi % N))) != 0;
    }
    /* Sets the state of bit bi to newBit.  The number grows or shrinks as
     * necessary. */
    void set_bit(index_t bi, bool newBit);

    // COMPARISONS

    // Compares this to x like Perl's <=>
    comparison_result compare_to(const bigunsigned& x) const;

    // Ordinary comparison operators
    bool operator==(const bigunsigned& x) const {
        return number_like_array<blk_t>::operator==(x);
    }
    bool operator!=(const bigunsigned& x) const {
        return number_like_array<blk_t>::operator!=(x);
    }
    bool operator< (const bigunsigned& x) const { return compare_to(x) == less   ; }
    bool operator<=(const bigunsigned& x) const { return compare_to(x) != greater; }
    bool operator>=(const bigunsigned& x) const { return compare_to(x) != less   ; }
    bool operator> (const bigunsigned& x) const { return compare_to(x) == greater; }

    /*
     * bigunsigned and biginteger both provide three kinds of operators.
     * Here ``big-integer'' refers to biginteger or bigunsigned.
     *
     * (1) Overloaded ``return-by-value'' operators:
     *     +, -, *, /, %, unary -, &, |, ^, <<, >>.
     * Big-integer code using these operators looks identical to code using
     * the primitive integer types.  These operators take one or two
     * big-integer inputs and return a big-integer result, which can then
     * be assigned to a biginteger variable or used in an expression.
     * Example:
     *     biginteger a(1), b = 1;
     *     biginteger c = a + b;
     *
     * (2) Overloaded assignment operators:
     *     +=, -=, *=, /=, %=, flip_sign, &=, |=, ^=, <<=, >>=, ++, --.
     * Again, these are used on big integers just like on ints.  They take
     * one writable big integer that both provides an operand and receives a
     * result.  Most also take a second read-only operand.
     * Example:
     *     biginteger a(1), b(1);
     *     a += b;
     *
     * (3) Copy-less operations: `add', `subtract', etc.
     * These named methods take operands as arguments and store the result
     * in the receiver (*this), avoiding unnecessary copies and allocations.
     * `divide_with_remainder' is special: it both takes the dividend from and
     * stores the remainder into the receiver, and it takes a separate
     * object in which to store the quotient.  NOTE: If you are wondering
     * why these don't return a value, you probably mean to use the
     * overloaded return-by-value operators instead.
     *
     * Examples:
     *     biginteger a(43), b(7), c, d;
     *
     *     c = a + b;   // Now c == 50.
     *     c.add(a, b); // Same effect but without the two copies.
     *
     *     c.divide_with_remainder(b, d);
     *     // 50 / 7; now d == 7 (quotient) and c == 1 (remainder).
     *
     *     // ``Aliased'' calls now do the right thing using a temporary
     *     // copy, but see note on `divide_with_remainder'.
     *     a.add(a, b);
     */

    // COPY-LESS OPERATIONS

    // These 8: Arguments are read-only operands, result is saved in *this.
    void add(const bigunsigned& a, const bigunsigned& b);
    void subtract(const bigunsigned& a, const bigunsigned& b);
    void multiply(const bigunsigned& a, const bigunsigned& b);
    void bit_and(const bigunsigned& a, const bigunsigned& b);
    void bit_or(const bigunsigned& a, const bigunsigned& b);
    void bit_xor(const bigunsigned& a, const bigunsigned& b);
    /* Negative shift amounts translate to opposite-direction shifts,
     * except for -2^(8*sizeof(int)-1) which is unimplemented. */
    void bit_shift_left(const bigunsigned& a, int b);
    void bit_shift_right(const bigunsigned& a, int b);

    /* `a.divide_with_remainder(b, q)' is like `q = a / b, a %= b'.
     * / and % use semantics similar to Knuth's, which differ from the
     * primitive integer semantics under division by zero.  See the
     * implementation in bigunsigned.cc for details.
     * `a.divide_with_remainder(b, a)' throws an exception: it doesn't make
     * sense to write quotient and remainder into the same variable. */
    void divide_with_remainder(const bigunsigned& b, bigunsigned& q);

    /* `divide' and `modulo' are no longer offered.  Use
     * `divide_with_remainder' instead. */

    // OVERLOADED RETURN-BY-VALUE OPERATORS
    bigunsigned operator+(const bigunsigned& x) const;
    bigunsigned operator-(const bigunsigned& x) const;
    bigunsigned operator*(const bigunsigned& x) const;
    bigunsigned operator/(const bigunsigned& x) const;
    bigunsigned operator%(const bigunsigned& x) const;
    /* OK, maybe unary minus could succeed in one case, but it really
     * shouldn't be used, so it isn't provided. */
    bigunsigned operator&(const bigunsigned& x) const;
    bigunsigned operator|(const bigunsigned& x) const;
    bigunsigned operator^(const bigunsigned& x) const;
    bigunsigned operator<<(int b) const;
    bigunsigned operator>>(int b) const;

    // OVERLOADED ASSIGNMENT OPERATORS
    void operator+=(const bigunsigned& x);
    void operator-=(const bigunsigned& x);
    void operator*=(const bigunsigned& x);
    void operator/=(const bigunsigned& x);
    void operator%=(const bigunsigned& x);
    void operator&=(const bigunsigned& x);
    void operator|=(const bigunsigned& x);
    void operator^=(const bigunsigned& x);
    void operator<<=(int b);
    void operator>>=(int b);

    /* INCREMENT/DECREMENT OPERATORS
     * To discourage messy coding, these do not return *this, so prefix
     * and postfix behave the same. */
    void operator++(   );
    void operator++(int);
    void operator--(   );
    void operator--(int);

    // Helper function that needs access to bigunsigned internals
    friend blk_t get_shifted_block(const bigunsigned& num, index_t x,
            unsigned int y);

    // See biginteger.cc.
    template <class X>
    friend X convert_bigunsigned_to_primitive_access(const bigunsigned& a);
};

/* Implementing the return-by-value and assignment operators in terms of the
 * copy-less operations.  The copy-less operations are responsible for making
 * any necessary temporary copies to work around aliasing. */

inline bigunsigned bigunsigned::operator+(const bigunsigned& x) const {
    bigunsigned ans;
    ans.add(*this, x);
    return ans;
}
inline bigunsigned bigunsigned::operator-(const bigunsigned& x) const {
    bigunsigned ans;
    ans.subtract(*this, x);
    return ans;
}
inline bigunsigned bigunsigned::operator*(const bigunsigned& x) const {
    bigunsigned ans;
    ans.multiply(*this, x);
    return ans;
}
inline bigunsigned bigunsigned::operator/(const bigunsigned& x) const {
    if (x.is_zero()) throw "bigunsigned::operator/: division by zero";
    bigunsigned q, r;
    r = *this;
    r.divide_with_remainder(x, q);
    return q;
}
inline bigunsigned bigunsigned::operator%(const bigunsigned& x) const {
    if (x.is_zero()) throw "bigunsigned::operator%: division by zero";
    bigunsigned q, r;
    r = *this;
    r.divide_with_remainder(x, q);
    return r;
}
inline bigunsigned bigunsigned::operator&(const bigunsigned& x) const {
    bigunsigned ans;
    ans.bit_and(*this, x);
    return ans;
}
inline bigunsigned bigunsigned::operator|(const bigunsigned& x) const {
    bigunsigned ans;
    ans.bit_or(*this, x);
    return ans;
}
inline bigunsigned bigunsigned::operator^(const bigunsigned& x) const {
    bigunsigned ans;
    ans.bit_xor(*this, x);
    return ans;
}
inline bigunsigned bigunsigned::operator<<(int b) const {
    bigunsigned ans;
    ans.bit_shift_left(*this, b);
    return ans;
}
inline bigunsigned bigunsigned::operator>>(int b) const {
    bigunsigned ans;
    ans.bit_shift_right(*this, b);
    return ans;
}

inline void bigunsigned::operator+=(const bigunsigned& x) {
    add(*this, x);
}
inline void bigunsigned::operator-=(const bigunsigned& x) {
    subtract(*this, x);
}
inline void bigunsigned::operator*=(const bigunsigned& x) {
    multiply(*this, x);
}
inline void bigunsigned::operator/=(const bigunsigned& x) {
    if (x.is_zero()) throw "bigunsigned::operator/=: division by zero";
    /* The following technique is slightly faster than copying *this first
     * when x is large. */
    bigunsigned q;
    divide_with_remainder(x, q);
    // *this contains the remainder, but we overwrite it with the quotient.
    *this = q;
}
inline void bigunsigned::operator%=(const bigunsigned& x) {
    if (x.is_zero()) throw "bigunsigned::operator%=: division by zero";
    bigunsigned q;
    // Mods *this by x.  Don't care about quotient left in q.
    divide_with_remainder(x, q);
}
inline void bigunsigned::operator&=(const bigunsigned& x) {
    bit_and(*this, x);
}
inline void bigunsigned::operator|=(const bigunsigned& x) {
    bit_or(*this, x);
}
inline void bigunsigned::operator^=(const bigunsigned& x) {
    bit_xor(*this, x);
}
inline void bigunsigned::operator<<=(int b) {
    bit_shift_left(*this, b);
}
inline void bigunsigned::operator>>=(int b) {
    bit_shift_right(*this, b);
}

/* Templates for conversions of bigunsigned to and from primitive integers.
 * biginteger.cc needs to instantiate convert_to_primitive, and the uses in
 * bigunsigned.cc didn't do the trick; I think g++ inlined convert_to_primitive
 * instead of generating linkable instantiations.  So for consistency, I put
 * all the templates here. */

// CONSTRUCTION FROM PRIMITIVE INTEGERS

/* Initialize this bigunsigned from the given primitive integer.  The same
 * pattern works for all primitive integer types, so I put it into a template to
 * reduce code duplication.  (Don't worry: this is protected and we instantiate
 * it only with primitive integer types.)  Type X could be signed, but x is
 * known to be nonnegative. */
template <class X>
void bigunsigned::init_from_primitive(X x) {
    if (x == 0)
        ; // number_like_array already initialized us to zero.
    else {
        // Create a single block.  blk is nullptr; no need to delete it.
        cap = 1;
        blk = new blk_t[1];
        len = 1;
        blk[0] = blk_t(x);
    }
}

/* Ditto, but first check that x is nonnegative.  I could have put the check in
 * init_from_primitive and let the compiler optimize it out for unsigned-type
 * instantiations, but I wanted to avoid the warning stupidly issued by g++ for
 * a condition that is constant in *any* instantiation, even if not in all. */
template <class X>
void bigunsigned::init_from_signed_primitive(X x) {
    if (x < 0)
        throw "bigunsigned constructor: "
            "Cannot construct a bigunsigned from a negative number";
    else
        init_from_primitive(x);
}

// CONVERSION TO PRIMITIVE INTEGERS

/* Template with the same idea as init_from_primitive.  This might be slightly
 * slower than the previous version with the masks, but it's much shorter and
 * clearer, which is the library's stated goal. */
template <class X>
X bigunsigned::convert_to_primitive() const {
    if (len == 0)
        // The number is zero; return zero.
        return 0;
    else if (len == 1) {
        // The single block might fit in an X.  Try the conversion.
        X x = X(blk[0]);
        // Make sure the result accurately represents the block.
        if (blk_t(x) == blk[0])
            // Successful conversion.
            return x;
        // Otherwise fall through.
    }
    throw "Value is too big to fit in the requested type";
}

/* Wrap the above in an x >= 0 test to make sure we got a nonnegative result,
 * not a negative one that happened to convert back into the correct nonnegative
 * one.  (E.g., catch incorrect conversion of 2^31 to the long -2^31.)  Again,
 * separated to avoid a g++ warning. */
template <class X>
X bigunsigned::convert_to_signed_primitive() const {
    X x = convert_to_primitive<X>();
    if (x >= 0)
        return x;
    else
        throw "Value is too big to fit in the requested type";
}

}

#endif
