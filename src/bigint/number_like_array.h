
#ifndef BIGINT_NUMBER_LIKE_ARRAY_H
#define BIGINT_NUMBER_LIKE_ARRAY_H

namespace bigint
{

/* A number_like_array<blk_t> object holds a heap-allocated array of blk_t with
 * a length and a capacity and provides basic memory management features.
 * bigunsigned and bigunsigned_in_a_base both subclass it.
 *
 * number_like_array provides no information hiding.  Subclasses should use
 * nonpublic inheritance and manually expose members as desired using
 * declarations like this:
 *
 * public:
 *     number_like_array< the-type-argument >::get_length;
 */
template <class blk_t>
class number_like_array {
public:

    // Type for the index of a block in the array
    typedef unsigned int index_t;
    // The number of bits in a block, defined below.
    static const unsigned int N;

    // The current allocated capacity of this number_like_array (in blocks)
    index_t cap;
    // The actual length of the value stored in this number_like_array (in blocks)
    index_t len;
    // Heap-allocated array of the blocks (can be nullptr if len == 0)
    blk_t* blk;

    // Constructs a ``zero'' number_like_array with the given capacity.
    number_like_array(index_t c) : cap(c), len(0) {
        blk = (cap > 0) ? (new blk_t[cap]) : nullptr;
    }

    /* Constructs a zero number_like_array without allocating a backing array.
     * A subclass that doesn't know the needed capacity at initialization
     * time can use this constructor and then overwrite blk without first
     * deleting it. */
    number_like_array() : cap(0), len(0) {
        blk = nullptr;
    }

    // Destructor.  Note that `delete nullptr' is a no-op.
    ~number_like_array() {
        delete [] blk;
    }

    /* Ensures that the array has at least the requested capacity; may
     * destroy the contents. */
    void allocate(index_t c);

    /* Ensures that the array has at least the requested capacity; does not
     * destroy the contents. */
    void allocate_and_copy(index_t c);

    // Copy constructor
    number_like_array(const number_like_array<blk_t>& x);

    // Assignment operator
    void operator=(const number_like_array<blk_t>& x);

    // Constructor that copies from a given array of blocks
    number_like_array(const blk_t* b, index_t blen);

    // ACCESSORS
    index_t get_capacity()       const { return cap;      }
    index_t get_length()         const { return len;      }
    blk_t   get_block(index_t i) const { return blk[i];   }
    bool    is_empty()           const { return len == 0; }

    /* Equality comparison: checks if both objects have the same length and
     * equal (==) array elements to that length.  Subclasses may wish to
     * override. */
    bool operator==(const number_like_array<blk_t>& x) const;

    bool operator!=(const number_like_array<blk_t>& x) const {
        return !operator==(x);
    }
};

/* BEGIN TEMPLATE DEFINITIONS.  They are present here so that source files that
 * include this header file can generate the necessary real definitions. */

template <class blk_t>
const unsigned int number_like_array<blk_t>::N = 8 * sizeof(blk_t);

template <class blk_t>
void number_like_array<blk_t>::allocate(index_t c) {
    // If the requested capacity is more than the current capacity...
    if (c > cap) {
        // Delete the old number array
        delete [] blk;
        // Allocate the new array
        cap = c;
        blk = new blk_t[cap];
    }
}

template <class blk_t>
void number_like_array<blk_t>::allocate_and_copy(index_t c) {
    // If the requested capacity is more than the current capacity...
    if (c > cap) {
        blk_t* old_blk = blk;
        // Allocate the new number array
        cap = c;
        blk = new blk_t[cap];
        // Copy number blocks
        for (index_t i = 0; i < len; i++)
            blk[i] = old_blk[i];
        // Delete the old array
        delete [] old_blk;
    }
}

template <class blk_t>
number_like_array<blk_t>::number_like_array(const number_like_array<blk_t>& x)
        : len(x.len) {
    // Create array
    cap = len;
    blk = new blk_t[cap];
    // Copy blocks
    for (index_t i = 0; i < len; i++)
        blk[i] = x.blk[i];
}

template <class blk_t>
void number_like_array<blk_t>::operator=(const number_like_array<blk_t>& x) {
    /* Calls like a = a have no effect; catch them before the aliasing
     * causes a problem */
    if (this == &x)
        return;
    // Copy length
    len = x.len;
    // Expand array if necessary
    allocate(len);
    // Copy number blocks
    index_t i;
    for (i = 0; i < len; i++)
        blk[i] = x.blk[i];
}

template <class blk_t>
number_like_array<blk_t>::number_like_array(const blk_t* b, index_t blen)
        : cap(blen), len(blen) {
    // Create array
    blk = new blk_t[cap];
    // Copy blocks
    index_t i;
    for (i = 0; i < len; i++)
        blk[i] = b[i];
}

template <class blk_t>
bool number_like_array<blk_t>::operator==(const number_like_array<blk_t>& x) const {
    if (len != x.len)
        // Definitely unequal.
        return false;
    else {
        // Compare corresponding blocks one by one.
        for (index_t i = 0; i < len; i++)
            if (blk[i] != x.blk[i])
                return false;
        // No blocks differed, so the objects are equal.
        return true;
    }
}

}

#endif
