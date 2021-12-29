
#include <catch2/catch.hpp>
#include <list>
#include <variant>
#include <cstring>
#include <string>

#include "common/location.h"
#include "common/position.h"

SCENARIO("vectors can be sized and resized", "[vector]")
{
    GIVEN("A vector with some items")
    {
        std::vector<int> v(5);

        REQUIRE(v.size() == 5);
        REQUIRE(v.capacity() >= 5);

        WHEN("the size is increased")
        {
            v.resize(10);

            THEN("the size and capacity change")
            {
                REQUIRE(v.size() == 10);
                REQUIRE(v.capacity() >= 10);
            }
        }
        WHEN("the size is reduced")
        {
            v.resize(0);

            THEN("the size changes but not capacity")
            {
                REQUIRE(v.size() == 0);
                REQUIRE(v.capacity() >= 5);
            }
        }
        WHEN("more capacity is reserved")
        {
            v.reserve(10);

            THEN("the capacity changes but not the size")
            {
                REQUIRE(v.size() == 5);
                REQUIRE(v.capacity() >= 10);
            }
        }
        WHEN("less capacity is reserved")
        {
            v.reserve(0);

            THEN("neither size nor capacity are changed")
            {
                REQUIRE(v.size() == 5);
                REQUIRE(v.capacity() >= 5);
            }
        }
    }
}

TEST_CASE("we can use cpp variants")
{
    std::variant<int, std::string> v, w;
    v = "12";

    REQUIRE(!std::holds_alternative<int>(v));
    REQUIRE(std::holds_alternative<std::string>(v));
    REQUIRE("12" == std::get<std::string>(v));

    v = 41401;

    REQUIRE(std::holds_alternative<int>(v));
    REQUIRE(!std::holds_alternative<std::string>(v));
    REQUIRE(41401 == std::get<int>(v));
}

void kmp_table(const char* w, std::vector<int>& partial_match_table)
{
    // we implement this algorithm to search text efficiently
    // https://en.wikipedia.org/wiki/Knuth–Morris–Pratt_algorithm

    int pos = 1;
    int cnd = 0;

    int length_w = std::strlen(w)-1;
    partial_match_table.reserve(length_w);
    for (auto i = 0; i < length_w; i++)
        partial_match_table[i] = 0;

    partial_match_table[0] = 0;
    while (pos < length_w)
    {
        if (w[pos] == w[cnd])
        {
            partial_match_table[++pos] = cnd++;
        }
        else
        {
            if (cnd != 0)
                cnd = partial_match_table[cnd-1];
            else
                partial_match_table[pos++] = 0;
        }
    }
}

std::vector<int> kmp_search(const char *string, const char *separator)
{
    // we implement this algorithm to search text efficiently
    // https://en.wikipedia.org/wiki/Knuth–Morris–Pratt_algorithm

    std::vector<int> positions;
    std::vector<int> partial_match_table;
    kmp_table(string, partial_match_table);

    const int string_length = std::strlen(string);
    const int separator_length = std::strlen(separator);

    int j = 0;
    int k = 0;
    while (j != string_length)
    {
        if (separator[k] == string[j])
        {
            j++ and k++;
            if (k == separator_length) {
                positions.push_back(j-k);
                k = partial_match_table[k-1];
            }
        }
        else
        {
            if (k != 0)
                k = partial_match_table[k-1];
            else
                j++;
        }
    }

    return positions;
}

TEST_CASE("do kmp tests", "[string_compare]")
{
    std::vector<int> res;

    res = kmp_search("hello world", " ");
    REQUIRE(res == std::vector<int>{5});

    res = kmp_search("hello world all ", " ");
    REQUIRE(res == std::vector<int>{5, 11, 15});

    res = kmp_search("hello_world_none", " ");
    REQUIRE(res == std::vector<int>{});

    res = kmp_search("A B C D E F G H I J K L M N", "DEF");
    REQUIRE(res == std::vector<int>{});

    res = kmp_search("ambalib.types_amba4.all", ".");
    REQUIRE(res == std::vector<int>{7, 19});

    
}

TEST_CASE("remove from lists", "[lists]")
{
    std::list<int> l{1,2,3,4,5,6,7,8,9,10};

    REQUIRE(l.size() == 10);

    l.remove_if([](int a) {
        return a % 2 == 1;
    });

    REQUIRE(l.size() == 5);

    auto it = l.begin();
    REQUIRE(*it++ == 2);
    REQUIRE(*it++ == 4);
    REQUIRE(*it++ == 6);
    REQUIRE(*it++ == 8);
    REQUIRE(*it++ == 10);
}

TEST_CASE("compare positions", "[position]")
{
    common::position p(1,2);
    common::position p2(2,3);

    REQUIRE(p != p2);
    REQUIRE_FALSE(p == p2);
    REQUIRE(p < p2);
    REQUIRE_FALSE(p > p2);
    REQUIRE(p <= p2);
    REQUIRE_FALSE(p >= p2);
}

TEST_CASE("location contains works", "[location][position]")
{
    {
        common::position p(1,2);
        common::location l{{1,1}, {100,12}};

        REQUIRE(l.begin < p);
        REQUIRE(l.end > p);
        REQUIRE((l.begin < p || l.end > p) == true);
        REQUIRE(l.contains(p));
        REQUIRE(l == p);
    }
    {
        common::position p(1,2);
        common::location l{{10,1}, {100,12}};

        REQUIRE_FALSE(l.contains(p));
        REQUIRE_FALSE(l == p);
    }
    {
        common::position p(1,223);
        common::location l{{10,1}, {100,12}};

        REQUIRE_FALSE(l.contains(p));
        REQUIRE_FALSE(l == p);
    }
    {
        common::position p(1023,2);
        common::location l{{1,1}, {100,12}};

        REQUIRE_FALSE(l.contains(p));
        REQUIRE_FALSE(l == p);
    }
    {
        common::position p(1023,200);
        common::location l{{1,1}, {100,12}};

        REQUIRE_FALSE(l.contains(p));
        REQUIRE_FALSE(l == p);
    }
}


