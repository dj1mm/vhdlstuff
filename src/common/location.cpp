
#include "location.h"

common::location::location(const common::position& b, const common::position& e)
    : begin(b), end(e)
{
}

common::location::location(const common::position& p)
    : begin(p), end(p)
{
}

common::location::location(std::string_view f, unsigned l, unsigned c)
    : filename(f), begin(l, c), end(l, c)
{
}

common::location::location(std::string_view f, unsigned l, unsigned c, unsigned l2, unsigned c2)
    : filename(f), begin(l, c), end(l2, c2)
{
}

void common::location::initialize(std::string_view f, unsigned l, unsigned c)
{
    filename = f;
    begin.initialize(l, c);
    end.initialize(l, c);
}

void common::location::initialize(std::string_view f, unsigned l, unsigned c, unsigned l2, unsigned c2)
{
    filename = f;
    begin.initialize(l, c);
    end.initialize(l2, c2);
}

void common::location::step()
{
    begin = end;
}

void common::location::columns(int count)
{
    end += count;
}

void common::location::lines(int count)
{
    end.lines(count);
}

bool common::location::contains(const position& candidate) const
{
    return begin <= candidate && end >= candidate;
}

common::location& common::location::operator+=(const common::location& rhs)
{
    end = rhs.end;
    return *this;
}

common::location& common::location::operator+=(int rhs)
{
    columns(rhs);
    return *this;
}

common::location common::location::operator+(const common::location& rhs)
{
    common::location result(*this);
    result += rhs;
    return result;
}

common::location common::location::operator+(int rhs)
{
    common::location result(*this);
    result += rhs;
    return result;
}

common::location& common::location::operator-=(int rhs)
{
    return *this += -rhs;
}

common::location common::location::operator-(int rhs)
{
    return *this += -rhs;
}

bool common::location::operator==(const common::location& rhs) const
{
    if (filename != rhs.filename)
        return false;
        
    if (begin != rhs.begin)
        return false;
        
    if (end != rhs.end)
        return false;
    
    return true;
}

bool common::location::operator!=(const common::location& rhs) const
{
    return !(*this == rhs);
}

bool common::location::operator==(const common::position& candidate) const
{
    return contains(candidate);
}

bool common::location::operator<(const common::position& candidate) const
{
    return end < candidate;
}

bool common::location::operator<=(const common::position& candidate) const
{
    return begin < candidate;
}

bool common::location::operator>(const common::position& candidate) const
{
    return begin > candidate;
}

bool common::location::operator>=(const common::position& candidate) const
{
    return end > candidate;
}

