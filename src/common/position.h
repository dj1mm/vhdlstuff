
#ifndef COMMON_POSITION_H
#define COMMON_POSITION_H

#include <ostream>

namespace common
{

class position
{
    public:

    // default constructors
    position(const position& position) = default;
    position(position&& position) = default;

    // create position object given line and column number
    //
    // Usage: position pos(123, 31);
    position(unsigned l = 1, unsigned c = 1);

    position& operator=(const position& other) = default;
    position& operator=(position&& other) = default;

    // initialise this position to a given line and column number
    void initialize(unsigned l = 1u, unsigned c = 1u);

    // increase the current line number by count, and reset column
    //
    // If the resulting line is invalid (ie < 1), set the line number to 1
    void lines(int count = 1);

    // increase the current column number by count
    //
    // If the resulting column is invalid (ie < 1), set the column number to 1
    void columns(int count = 1);

    unsigned line;
    unsigned column;

    position& operator+=(int rhs);
    position operator+(int rhs);

    position& operator-=(int rhs);
    position operator-(int rhs);

    // compare two position objects.
    bool operator==(const position& rhs) const;
    bool operator!=(const position& rhs) const;
    bool operator<(const position& rhs) const;
    bool operator>(const position& rhs) const;
    bool operator<=(const position& rhs) const;
    bool operator>=(const position& rhs) const;

    private:

    static unsigned lhs_plus_rhs_or_one(unsigned lhs, int rhs);
};

}

template <typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& ostr, const common::position& pos)
{
    return ostr << pos.line << '.' << pos.column;
}

#endif
