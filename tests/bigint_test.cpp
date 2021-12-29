
#include <catch2/catch.hpp>
#include "bigint/library.h"

using namespace bigint;

TEST_CASE("sample")
{
    biginteger a;
    int b = 535;

    a = b;

    b = a.to_int();

    biginteger c(a);

    biginteger d(-314159265);

    std::string s("3141592653589793238462643383279");
    biginteger f = to_integer(s);

    REQUIRE(to_string(f) == "3141592653589793238462643383279");

    biginteger g(314159), h(265);

    REQUIRE(to_string(g+h) == "314424");
    REQUIRE(to_string(g-h) == "313894");
    REQUIRE(to_string(g*h) == "83252135");
    REQUIRE(to_string(g/h) == "1185");
    REQUIRE(to_string(g%h) == "134");

    bigunsigned i(0xFF0000FF), j(0x0000FFFF);

    REQUIRE(to_string(i & j, 16) == "FF");
    REQUIRE(to_string(i | j, 16) == "FF00FFFF");
    REQUIRE(to_string(i ^ j, 16) == "FF00FF00");
    REQUIRE(to_string(j << 21, 16) == "1FFFE00000");
    REQUIRE(to_string(j >> 10, 16) == "3F");

    REQUIRE(gcd(bigunsigned(60), 72) == 12);
    REQUIRE(modinv(bigunsigned(7), 11) == 8);
    REQUIRE(modexp(bigunsigned(314), 159, 2653) == 1931);
}

