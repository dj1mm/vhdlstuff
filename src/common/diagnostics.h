
#ifndef COMMON_DIAGNOSTICS_H
#define COMMON_DIAGNOSTICS_H

#include <variant>
#include <vector>
#include <string_view>

#include "location.h"

namespace common
{

class diagnostic
{
    using arg_t = std::variant<std::string, int>;

    public:

    std::string_view format;
    std::vector<arg_t> args;
    common::location location;

    std::vector<diagnostic> notes;

    diagnostic(std::string_view, common::location);
    diagnostic(const diagnostic&) = default;
    diagnostic(diagnostic&&) = default;
    diagnostic& operator=(const diagnostic&) = default;
    diagnostic& operator=(diagnostic&&) = default;
    ~diagnostic() = default;

    diagnostic& add_note(std::string_view, common::location);
    diagnostic& add_note(const diagnostic&);

    diagnostic& operator<<(const std::string& arg);
    diagnostic& operator<<(std::string_view arg);
    diagnostic& operator<<(char* arg);
    diagnostic& operator<<(int arg);
};

}

#endif
