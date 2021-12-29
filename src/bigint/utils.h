#ifndef BIGINT_UTILS_H
#define BIGINT_UTILS_H

#include "biginteger.h"
#include <string>
#include <iostream>

namespace bigint
{

/* This file provides:
 * - Convenient std::string <-> bigunsigned/biginteger conversion routines
 * - std::ostream << operators for bigunsigned/biginteger */

// std::string conversion routines
std::string to_string(const bigunsigned& x, unsigned short base = 10);
std::string to_string(const biginteger& x, unsigned short base = 10);
bigunsigned to_unsigned(const std::string& s, unsigned short base = 10);
biginteger to_integer(const std::string& s, unsigned short base = 10);

// Creates a biginteger from data such as `char's; read below for details.
template <class T>
biginteger data_to_big_integer(const T* data, biginteger::index_t length, biginteger::sign_t sign);

// Outputs x to os, obeying the flags `dec', `hex', `bin', and `showbase'.
std::ostream& operator<<(std::ostream& os, const bigunsigned& x);

// Outputs x to os, obeying the flags `dec', `hex', `bin', and `showbase'.
// My somewhat arbitrary policy: a negative sign comes before a base indicator (like -0xFF).
std::ostream& operator<<(std::ostream& os, const biginteger& x);

// BEGIN TEMPLATE DEFINITIONS.

/*
 * Converts binary data to a biginteger.
 * Pass an array `data', its length, and the desired sign.
 *
 * Elements of `data' may be of any type `T' that has the following
 * two properties (this includes almost all integral types):
 *
 * (1) `sizeof(T)' correctly gives the amount of binary data in one
 * value of `T' and is a factor of `sizeof(blk_t)'.
 *
 * (2) When a value of `T' is casted to a `blk_t', the low bytes of
 * the result contain the desired binary data.
 */
template <class T>
biginteger data_to_big_integer(const T* data, biginteger::index_t length, biginteger::sign_t sign) {
    // really ceiling(numBytes / sizeof(biginteger::blk_t))
    unsigned int pieceSizeInBits = 8 * sizeof(T);
    unsigned int piecesPerBlock = sizeof(biginteger::blk_t) / sizeof(T);
    unsigned int numBlocks = (length + piecesPerBlock - 1) / piecesPerBlock;

    // Allocate our block array
    biginteger::blk_t *blocks = new biginteger::blk_t[numBlocks];

    biginteger::index_t blockNum, pieceNum, pieceNumHere;

    // Convert
    for (blockNum = 0, pieceNum = 0; blockNum < numBlocks; blockNum++) {
        biginteger::blk_t curBlock = 0;
        for (pieceNumHere = 0; pieceNumHere < piecesPerBlock && pieceNum < length;
            pieceNumHere++, pieceNum++)
            curBlock |= (biginteger::blk_t(data[pieceNum]) << (pieceSizeInBits * pieceNumHere));
        blocks[blockNum] = curBlock;
    }

    // Create the biginteger.
    biginteger x(blocks, numBlocks, sign);

    delete [] blocks;
    return x;
}

}

#endif
