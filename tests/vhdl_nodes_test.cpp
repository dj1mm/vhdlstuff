
#include <catch2/catch.hpp>
#include <iostream>
#include <sstream>

#include "vhdl_syntax.h"
#include "vhdl_syntax_debug.h"

TEST_CASE("vhdl nodes 1", "[vhdl_nodes][one]")
{
    auto file = new vhdl::syntax::design_file;

    file->units.push_back(new vhdl::syntax::design_unit);
    REQUIRE(file->units.size() == 1);

    std::stringstream ss;
    vhdl::syntax::vhdl_syntax_debug d(ss);
    file->traverse(d);

    delete file;

    // CHECK_THAT(ss.str(), Catch::Contains("<design_file>"));
    // CHECK_THAT(ss.str(), Catch::Contains("<entity_declaration>"));
}
