
#ifndef COMMON_LOCATION_H
#define COMMON_LOCATION_H

#include <string>
#include "position.h"

namespace common
{

class location
{
    public:

    // default constructors
    location(const location& location) = default;
    location(location&& location) = default;

    // create location from a pair of positions
    //
    // Usage: location loc(p1, p2);
    location(const position& b, const position& e);

    // create location from a position
    //
    // Usage: location loc(p1);
    explicit location(const position& p);

    // create location from a file, and line and column number
    //
    // Usage: location loc("myfile.test.example", 123, 31);
    location(std::string_view = "", unsigned = 1, unsigned = 1);
    location(std::string_view, unsigned, unsigned, unsigned, unsigned = 0);

    location& operator=(const location& other) = default;
    location& operator=(location&& other) = default;

    // initialise this location to a given line and column number
    void initialize(std::string_view = "", unsigned = 1, unsigned = 1);
    void initialize(std::string_view, unsigned, unsigned, unsigned, unsigned = 0);

    /// Reset initial position to final position.
    void step();

    /// Extend the current number to count next columns.
    void columns(int count = 1);

    /// Extend the current number to count next lines.
    void lines(int count = 1);

    // check whether a position is contained within this location
    bool contains(const position& candidate) const;

    std::string_view filename;
    position begin;
    position end;
    
    location& operator+=(const location& rhs);
    location& operator+=(int rhs);
    
    location operator+(const location& rhs);
    location operator+(int rhs);

    location& operator-=(int rhs);
    location operator-(int rhs);

    bool operator==(const location& rhs) const;

    bool operator!=(const location& rhs) const;

    // the range of location contains position
    bool operator==(const position& candidate) const;

    // the range of location is strictly before position
    bool operator<(const position& candidate) const;

    // the range of location is before or contains position
    bool operator<=(const position& candidate) const;

    // the range of location is strictly after position
    bool operator>(const position& candidate) const;

    // the range of location is after or contains position
    bool operator>=(const position& candidate) const;
};

}

template <typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& ostr, const common::location& loc)
{
    unsigned end_col = 0 < loc.end.column ? loc.end.column - 1 : 0;

    ostr << loc.filename << ':' << loc.begin;
    if(loc.begin.line < loc.end.line)
        ostr << '-' << loc.end.line << '.' << end_col;
    else if(loc.begin.column < end_col)
        ostr << '-' << end_col;
    return ostr;
}

#endif
