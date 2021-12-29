
#include "utils.h"
#include "bigunsigned_in_a_base.h"

namespace bigint
{

std::string to_string(const bigunsigned &x, unsigned short base)
{
    return std::string(bigunsigned_in_a_base(x, base));
}

std::string to_string(const biginteger& x, unsigned short base)
{
    return (x.get_sign() == biginteger::negative)
        ? (std::string("-") + to_string(x.get_magnitude(), base))
        : (to_string(x.get_magnitude(), base));
}

bigunsigned to_unsigned(const std::string& s, unsigned short base)
{
    return bigunsigned(bigunsigned_in_a_base(s, base));
}

biginteger to_integer(const std::string& s, unsigned short base)
{
    // Recognize a sign followed by a bigunsigned.
    return (s[0] == '-') ? biginteger(to_unsigned(s.substr(1, s.length() - 1), base), biginteger::negative)
        : (s[0] == '+') ? biginteger(to_unsigned(s.substr(1, s.length() - 1), base))
        : biginteger(to_unsigned(s, base));
}

std::ostream& operator<<(std::ostream& os, const bigunsigned& x) {
    bigunsigned_in_a_base::base_t base;
    long os_flags = os.flags();
    if (os_flags&  os.dec)
        base = 10;
    else if (os_flags&  os.hex) {
        base = 16;
        if (os_flags&  os.showbase)
            os << "0x";
    } else if (os_flags&  os.oct) {
        base = 8;
        if (os_flags&  os.showbase)
            os << '0';
    } else
        throw "Could not determine the desired base from output-stream flags";
    std::string s = std::string(bigunsigned_in_a_base(x, base));
    os << s;
    return os;
}

std::ostream& operator<<(std::ostream& os, const biginteger& x) {
    if (x.get_sign() == biginteger::negative)
        os << '-';
    os << x.get_magnitude();
    return os;
}

}
