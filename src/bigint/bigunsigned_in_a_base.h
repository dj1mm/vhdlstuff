
#ifndef BIGINT_BIGUNSIGNED_IN_A_BASE_H
#define BIGINT_BIGUNSIGNED_IN_A_BASE_H

#include "number_like_array.h"
#include "bigunsigned.h"
#include <string>

namespace bigint
{

/*
 * A bigunsigned_in_a_base object represents a nonnegative integer of size limited
 * only by available memory, represented in a user-specified base that can fit
 * in an `unsigned short' (most can, and this saves memory).
 *
 * bigunsigned_in_a_base is intended as an intermediary class with little
 * functionality of its own.  bigunsigned_in_a_base objects can be constructed
 * from, and converted to, BigUnsigneds (requiring multiplication, mods, etc.)
 * and `std::string's (by switching digit values for appropriate characters).
 *
 * bigunsigned_in_a_base is similar to bigunsigned.  Note the following:
 *
 * (1) They represent the number in exactly the same way, except that
 * bigunsigned_in_a_base uses ``digits'' (or digit_t) where bigunsigned uses
 * ``blocks'' (or blk_t).
 *
 * (2) Both use the management features of number_like_array.  (In fact, my desire
 * to add a bigunsigned_in_a_base class without duplicating a lot of code led me to
 * introduce number_like_array.)
 *
 * (3) The only arithmetic operation supported by bigunsigned_in_a_base is an
 * equality test.  Use bigunsigned for arithmetic.
 */

class bigunsigned_in_a_base : protected number_like_array<unsigned short> {

public:
    // The digits of a bigunsigned_in_a_base are unsigned shorts.
    typedef unsigned short digit_t;
    // That's also the type of a base.
    typedef digit_t base_t;

protected:
    // The base in which this bigunsigned_in_a_base is expressed
    base_t base;

    // Creates a bigunsigned_in_a_base with a capacity; for internal use.
    bigunsigned_in_a_base(int, index_t c) : number_like_array<digit_t>(0, c) {}

    // Decreases len to eliminate any leading zero digits.
    void zap_leading_zeros() {
        while (len > 0 && blk[len - 1] == 0)
            len--;
    }

public:
    // Constructs zero in base 2.
    bigunsigned_in_a_base() : number_like_array<digit_t>(), base(2) {}

    // Copy constructor
    bigunsigned_in_a_base(const bigunsigned_in_a_base& x) : number_like_array<digit_t>(x), base(x.base) {}

    // Assignment operator
    void operator=(const bigunsigned_in_a_base& x) {
        number_like_array<digit_t>::operator=(x);
        base = x.base;
    }

    // Constructor that copies from a given array of digits.
    bigunsigned_in_a_base(const digit_t *d, index_t l, base_t base);

    // Destructor.  number_like_array does the delete for us.
    ~bigunsigned_in_a_base() {}

    // LINKS TO BIGUNSIGNED
    bigunsigned_in_a_base(const bigunsigned& x, base_t base);
    operator bigunsigned() const;

    /* LINKS TO STRINGS
     *
     * These use the symbols ``0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'' to
     * represent digits of 0 through 35.  When parsing strings, lowercase is
     * also accepted.
     *
     * All string representations are big-endian (big-place-value digits
     * first).  (Computer scientists have adopted zero-based counting; why
     * can't they tolerate little-endian numbers?)
     *
     * No string representation has a ``base indicator'' like ``0x''.
     *
     * An exception is made for zero: it is converted to ``0'' and not the
     * empty string.
     *
     * If you want different conventions, write your own routines to go
     * between bigunsigned_in_a_base and strings.  It's not hard.
     */
    operator std::string() const;
    bigunsigned_in_a_base(const std::string& s, base_t base);

public:

    // ACCESSORS
    base_t get_base() const { return base; }

    // Expose these from number_like_array directly.
    using number_like_array<digit_t>::get_capacity;
    using number_like_array<digit_t>::get_length;

    /* Returns the requested digit, or 0 if it is beyond the length (as if
     * the number had 0s infinitely to the left). */
    digit_t get_digit(index_t i) const { return i >= len ? 0 : blk[i]; }

    // The number is zero if and only if the canonical length is zero.
    bool is_zero() const { return number_like_array<digit_t>::is_empty(); }

    /* Equality test.  For the purposes of this test, two bigunsigned_in_a_base
     * values must have the same base to be equal. */
    bool operator==(const bigunsigned_in_a_base& x) const
    {
        return base == x.base && number_like_array<digit_t>::operator==(x);
    }
    bool operator!=(const bigunsigned_in_a_base& x) const
    {
        return !operator==(x);
    }

};

}

#endif
