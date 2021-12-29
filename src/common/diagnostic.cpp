
#include "diagnostics.h"

common::diagnostic::diagnostic(std::string_view fmt, common::location loc)
: format(fmt), location(loc)
{

}


common::diagnostic& common::diagnostic::add_note(std::string_view fmt, common::location loc)
{
    notes.emplace_back(fmt, loc);
    return notes.back();
}

common::diagnostic& common::diagnostic::add_note(const common::diagnostic& d)
{
    notes.emplace_back(d);
    return notes.back();
}

common::diagnostic& common::diagnostic::operator<<(const std::string& arg)
{
    args.push_back(arg);
    return *this;
}
common::diagnostic& common::diagnostic::operator<<(std::string_view arg)
{
    args.push_back(std::string{arg});
    return *this;
}
common::diagnostic& common::diagnostic::operator<<(char* arg)
{
    args.push_back(std::string{arg});
    return *this;
}
common::diagnostic& common::diagnostic::operator<<(int arg)
{
    args.push_back(arg);
    return *this;
}

