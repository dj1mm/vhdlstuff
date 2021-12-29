
#include "bigunsigned_in_a_base.h"

using namespace bigint;

bigunsigned_in_a_base::bigunsigned_in_a_base(const digit_t* d, index_t l, base_t base)
    : number_like_array<digit_t>(d, l), base(base) {
    // Check the base
    if (base < 2)
        throw "The base must be at least 2";

    // Validate the digits.
    for (index_t i = 0; i < l; i++)
        if (blk[i] >= base)
            throw "A digit is too large for the specified base";

    // Eliminate any leading zeros we may have been passed.
    zap_leading_zeros();
}

namespace {
    unsigned int bit_len(unsigned int x) {
        unsigned int len = 0;
        while (x > 0) {
            x >>= 1;
            len++;
        }
        return len;
    }
    unsigned int ceiling_div(unsigned int a, unsigned int b) {
        return (a + b - 1) / b;
    }
}

bigunsigned_in_a_base::bigunsigned_in_a_base(const bigunsigned& x, base_t base) {
    // Check the base
    if (base < 2)
        throw "The base must be at least 2";
    this->base = base;

    // Get an upper bound on how much space we need
    int maxBitLenOfX = x.get_length() * bigunsigned::N;
    int minBitsPerDigit = bit_len(base) - 1;
    int maxDigitLenOfX = ceiling_div(maxBitLenOfX, minBitsPerDigit);
    len = maxDigitLenOfX; // Another change to comply with `staying in bounds'.
    allocate(len); // Get the space

    bigunsigned x2(x), buBase(base);
    index_t digitNum = 0;

    while (!x2.is_zero()) {
        // Get last digit.  This is like `lastDigit = x2 % buBase, x2 /= buBase'.
        bigunsigned lastDigit(x2);
        lastDigit.divide_with_remainder(buBase, x2);
        // Save the digit.
        blk[digitNum] = lastDigit.to_unsigned_short();
        // Move on.  We can't run out of room: we figured it out above.
        digitNum++;
    }

    // Save the actual length.
    len = digitNum;
}

bigunsigned_in_a_base::operator bigunsigned() const {
    bigunsigned ans(0), buBase(base), temp;
    index_t digitNum = len;
    while (digitNum > 0) {
        digitNum--;
        temp.multiply(ans, buBase);
        ans.add(temp, bigunsigned(blk[digitNum]));
    }
    return ans;
}

bigunsigned_in_a_base::bigunsigned_in_a_base(const std::string& s, base_t base) {
    // Check the base.
    if (base > 36)
        throw "The default string conversion routines use the symbol set 0-9, A-Z and therefore support only up to base 36.  You tried a conversion with a base over 36; write your own string conversion routine.";
    // Save the base.
    // This pattern is seldom seen in C++, but the analogous ``this.'' is common in Java.
    this->base = base;

    // `s.length()' is a `size_t', while `len' is a `number_like_array::index_t',
    // also known as an `unsigned int'.  Some compilers warn without this cast.
    len = index_t(s.length());
    allocate(len);

    index_t digitNum, symbolNumInString;
    for (digitNum = 0; digitNum < len; digitNum++) {
        symbolNumInString = len - 1 - digitNum;
        char theSymbol = s[symbolNumInString];
        if (theSymbol >= '0' && theSymbol <= '9')
            blk[digitNum] = theSymbol - '0';
        else if (theSymbol >= 'A' && theSymbol <= 'Z')
            blk[digitNum] = theSymbol - 'A' + 10;
        else if (theSymbol >= 'a' && theSymbol <= 'z')
            blk[digitNum] = theSymbol - 'a' + 10;
        else
            throw "Bad symbol in input.  Only 0-9, A-Z, a-z are accepted.";

        if (blk[digitNum] >= base)
            throw "A digit is too large for the specified base";
    }
    zap_leading_zeros();
}

bigunsigned_in_a_base::operator std::string() const {
    if (base > 36)
        throw "The default string conversion routines use the symbol set 0-9, A-Z and therefore support only up to base 36.  You tried a conversion with a base over 36; write your own string conversion routine.";
    if (len == 0)
        return std::string("0");
    // Some compilers don't have push_back, so use a char * buffer instead.
    char* s = new char[len + 1];
    s[len] = '\0';
    index_t digitNum, symbolNumInString;
    for (symbolNumInString = 0; symbolNumInString < len; symbolNumInString++) {
        digitNum = len - 1 - symbolNumInString;
        digit_t theDigit = blk[digitNum];
        if (theDigit < 10)
            s[symbolNumInString] = char('0' + theDigit);
        else
            s[symbolNumInString] = char('A' + theDigit - 10);
    }
    std::string s2(s);
    delete [] s;
    return s2;
}
