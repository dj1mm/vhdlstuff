
#include "token.h"

std::string vhdl::token::to_string() const
{
    return std::string{value};
}

std::string vhdl::token::debug() const
{
    std::stringstream ss;

    if (is_delimiter || is_identifier || is_literal)
        ss << get_token_string_name(kind) << " '" << value << "'";
    else
        ss << get_token_string_name(kind);
    ss << " Loc=<" << location << ">";
    return ss.str();
}

bool vhdl::token::operator==(vhdl::token rhs) const
{
    return kind == rhs.kind && value == rhs.value && location == rhs.location;
}

bool vhdl::token::operator==(vhdl::token::kind_t rhs) const
{
    return kind == rhs;
}

bool vhdl::token::operator!=(vhdl::token rhs) const
{
    return kind != rhs.kind || value != rhs.value || location != rhs.location;
}

bool vhdl::token::operator!=(vhdl::token::kind_t rhs) const
{
    return kind != rhs;
}
