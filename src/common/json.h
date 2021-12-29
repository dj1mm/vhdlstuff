
#ifndef COMMON_JSON_H
#define COMMON_JSON_H

#include <iostream>
#include <variant>
#include <string>

namespace json
{

// json::null type defines a json null value
struct null
{
    null() = default;

    friend bool operator==(null const& lhs, null const& rhs)
    {
        return true;
    }

    friend bool operator!=(null const& lhs, null const& rhs)
    {
        return false;
    }
};

// json::string type defines a json string, instead of an std::string which is
// used throughout lsp/protocol.h to distinguish between the two
inline constexpr null null_value;

// json::nullable type represents: <ANYTHING> | null
template<typename T>
class nullable
{
    public:

    nullable() = default;
    nullable(nullable const&) = default;
    nullable(nullable&&) = default;
    nullable(T const& x) : storage_(x) {}
    nullable(T&& x) : storage_(std::forward<T>(x)) {}
    nullable(json::null const& x) : storage_(x) {}
    nullable(json::null&& x) : storage_(std::forward<json::null>(x)) {}

    bool is_null() const
    {
        return std::holds_alternative<json::null>(storage_);
    }

    T& value()
    {
        return std::get<T>(storage_);
    }

    std::variant<T, json::null>* operator->()
    {
        return &storage_;
    }

    T& operator*()
    {
        return std::get<T>(storage_);
    }

    nullable& operator=(nullable const&) = default;
    nullable& operator=(nullable&&) = default;
    nullable& operator=(T const& x)
    {
        storage_ = std::forward<T>(x);
        return *this;
    }

    nullable& operator=(json::null const& x)
    {
        storage_ = x;
        return *this;
    }

    nullable& operator=(T&& x)
    {
        storage_ = std::forward<T>(x);
        return *this;
    }

    nullable& operator=(json::null&& x)
    {
        storage_ = std::move(x);
        return *this;
    }

    friend bool operator==(nullable const& lhs, nullable const& rhs)
    {
        return lhs.storage_ == rhs.storage_;
    }

    friend bool operator==(nullable const& lhs, null const& rhs)
    {
        return lhs.is_null();
    }

    friend bool operator!=(nullable const& lhs, nullable const& rhs)
    {
        return lhs.storage_ != rhs.storage_;
    }

    friend bool operator!=(nullable const& lhs, null const& rhs)
    {
        return !lhs.is_null();
    }

    private:
    std::variant<T, json::null> storage_ = null_value;
};

struct string
{
    string() = default;
    string(string const&) = default;
    string(string&&) = default;
    string(std::string s)
    {
        str = s;
    }

    string(const char* s)
    {
        str = std::string(s);
    }

    string& operator=(string const&) = default;
    string& operator=(string&&) = default;

    std::string* operator->()
    {
        return &str;
    }

    std::string str;

    friend std::ostream& operator<<(std::ostream& os, const string& json)
    {
        os << json.str;
        return os;
    }

    friend bool operator==(string const& lhs, string const& rhs)
    {
        return lhs.str == rhs.str;
    }
};

}

#endif
