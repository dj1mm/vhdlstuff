
#include "character_stream.h"
#include <cassert>

bool vhdl::is_graphic_character(char c)
{
    return (c >= '\x20' && c <= '\x7E') || (c >= '\xA0' && c <= '\xFF');
}

bool vhdl::is_upper_case_letter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= '\xC0' && c <= '\xD6') ||
           (c >= '\xD8' && c <= '\xDE');
}

bool vhdl::is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

bool vhdl::is_special_character(char c)
{
    return false;
}

bool vhdl::is_space_character(char c)
{
    return false;
}

bool vhdl::is_lower_case_letter(char c)
{
    return false;
}

bool vhdl::is_other_special_character(char c)
{
    return false;
}

vhdl::character_stream::character_stream(const char* s, const char* e)
: start_(s), end_(e), current_(s)
{
    assert(end_ >= start_);
}

void vhdl::character_stream::set_position(ptrdiff_t offset)
{
    assert(start_ + offset <end_);
    current_ = start_ + offset;
}

ptrdiff_t vhdl::character_stream::get_position() const
{
    assert(current_ >= start_);
    return current_ - start_;
}

ptrdiff_t vhdl::character_stream::length() const
{
    assert(end_ >= start_);
    return end_ - start_;
}

char vhdl::character_stream::current_char() const
{
    return current_[0];
}

char vhdl::character_stream::next_char() const
{
    return  (current_ == end_) ? 0: current_[1];
}

char vhdl::character_stream::prev_char() const
{
    return  (current_ == start_) ? 0: *(current_-1);
}

char vhdl::character_stream::look_ahead(int offset) const
{
    return  (current_ + offset >= end_) ? 0: current_[offset];
}

void vhdl::character_stream::advance(int offset)
{
    current_ += offset;
}

bool vhdl::character_stream::next()
{
    if (current_ == end_)
    {
        return false;
    }
    current_ += 1;
    return true;
}

bool vhdl::character_stream::end_of_stream()
{
    return current_ >= end_;
}

void vhdl::character_stream::skip_to_eol()
{
    while (current_ != end_)
    {
        if (current_[0] == '\r' || current_[0] == '\v' || current_[0] == '\n' ||
            current_[0] == '\f')
            return;
        ++current_;
    }
}
