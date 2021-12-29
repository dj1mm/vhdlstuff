
#include "position.h"

common::position::position(unsigned l, unsigned c) : line(l), column(c)
{
}

void common::position::initialize(unsigned l, unsigned c)
{
    line = l;
    column = c;
}

void common::position::lines(int count)
{
    if (count == 0)
        return;

    column = 1u;
    line = lhs_plus_rhs_or_one(line, count);
}

void common::position::columns(int count)
{
    column = lhs_plus_rhs_or_one(column, count);
}

unsigned common::position::lhs_plus_rhs_or_one(unsigned lhs, int rhs)
{
    return static_cast<unsigned>(std::max(1, static_cast<int>(lhs) + rhs));
}

common::position& common::position::operator+=(int rhs)
{
    columns(rhs);
    return *this;
}

common::position common::position::operator+(int rhs)
{
    common::position result(*this);
    result += rhs;
    return result;
}

common::position& common::position::operator-=(int rhs)
{
    return *this += -rhs;
}

common::position common::position::operator-(int rhs)
{
    return *this + -rhs;
}

bool common::position::operator==(const common::position& rhs) const
{
    if (line != rhs.line)
        return false;

    if (column != rhs.column)
        return false;

    return true;
}

bool common::position::operator!=(const common::position& rhs) const
{
    return !(*this == rhs);
}

bool common::position::operator<(const common::position& rhs) const
{
    if (line != rhs.line)
        return line < rhs.line;

    return column < rhs.column;
}

bool common::position::operator>(const common::position& rhs) const
{
    if (line != rhs.line)
        return line > rhs.line;

    return column > rhs.column;
}

bool common::position::operator<=(const common::position& rhs) const
{
    return !(*this > rhs);
}

bool common::position::operator>=(const common::position& rhs) const
{
    return !(*this < rhs);
}


