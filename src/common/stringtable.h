
#ifndef COMMON_STRINGTABLE_H
#define COMMON_STRINGTABLE_H

#include <unordered_map>
#include <string>
#include <cstring>

namespace common
{

class stringtable
{
    public:
    stringtable();
    ~stringtable();
    void destroy();

    stringtable(stringtable&& other) noexcept;
    stringtable& operator=(stringtable&& other) noexcept;

    stringtable(const stringtable&) = delete;
    stringtable& operator=(const stringtable&) = delete;

    void merge(stringtable&& other);

    std::string_view get(const char* string, size_t size);
    std::string_view get(const std::string& string);

    private:

    static constexpr int PAGE_SIZE = 4096;
    std::unordered_map<std::size_t, std::string_view> table;

    struct page
    {
        page* previous;
        char* buffer;
    };
    page* current;
    char* end;

    char* allocate(size_t size);
    char* actually_allocate_before(size_t size);
    char* actually_allocate_after(size_t size);

};

}

#endif
