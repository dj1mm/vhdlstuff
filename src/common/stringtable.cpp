
#include "stringtable.h"

common::stringtable::stringtable()
{
    current = (page* ) malloc(PAGE_SIZE);
    current->previous = nullptr;
    current->buffer = (char* ) current + sizeof(page);

    end = (char* ) current + PAGE_SIZE;
}

common::stringtable::~stringtable()
{
    destroy();
}

void common::stringtable::destroy()
{
    while (current)
    {
        page* previous = current->previous;
        free(current);
        current = previous;
    }
}
common::stringtable::stringtable(stringtable&& other) noexcept
: current(other.current), end(other.end)
{
    other.current = nullptr;
}

common::stringtable& common::stringtable::operator=(stringtable&& other) noexcept
{
    if (this != &other)
    {
        this->destroy();
        current = other.current;
        end = other.end;

        other.current = nullptr;
    }
    return *this;
}

void common::stringtable::merge(stringtable&& other)
{
    page* new_page = other.current;
    if (!new_page)
        return;

    while (new_page->previous)
        new_page = new_page->previous;

    new_page->previous = current->previous;

    current->previous = other.current;
    other.current = nullptr;
}

std::string_view common::stringtable::get(const char* string, size_t size)
{
    std::hash<std::string_view> hasher;
    auto hash = hasher(std::string_view{string, size});

    auto it = table.find(hash);
    if (it != table.end())
        return it->second;

    char* allocated = allocate(size);
    std::memcpy(allocated, string, size);
    table[hash] = std::string_view(allocated, size);

    return table[hash];
}

std::string_view common::stringtable::get(const std::string& string)
{
    return get(string.c_str(), string.size());
}

char* common::stringtable::allocate(size_t size)
{
    char* allocated = current->buffer;
    char* next = allocated + size;

    if (next > end && size > (PAGE_SIZE << 1))
        return actually_allocate_before(size);
    else if (next > end)
        return actually_allocate_after(size);

    current->buffer = next;
    return allocated;
}

char* common::stringtable::actually_allocate_before(size_t size)
{
    auto pg = (page* ) malloc(size + sizeof(page));
    pg->previous = current->previous;
    pg->buffer = (char* ) pg + sizeof(page);
    current->previous = pg;
    return pg->buffer;
}

char* common::stringtable::actually_allocate_after(size_t size)
{
    auto pg = (page* ) malloc(PAGE_SIZE);
    pg->previous = current;
    pg->buffer = (char* ) pg + sizeof(page);
    current = pg;
    end = (char* ) current + PAGE_SIZE;
    char* allocated = current->buffer;
    char* next = allocated + size;
    current->buffer = next;
    return allocated;
}

