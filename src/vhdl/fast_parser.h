
#ifndef VHDL_FAST_PARSER_H
#define VHDL_FAST_PARSER_H

#include <memory>
#include <optional>
#include <vector>

#include "common/stringtable.h"

#include "lexer.h"
#include "library_manager.h"
#include "token.h"

namespace vhdl
{

class fast_parser
{

    using tk = vhdl::token::kind_t;

    public:
    //
    // Create a parser with content to parse and optional file name
    //
    fast_parser(common::stringtable*, const char*, const char*,
                std::string = "");

    // ------------------------------------------------------------------------
    // Fast Parser methods
    // ------------------------------------------------------------------------

    //
    // Fast parse the initialized file and look for design units
    //
    std::vector<std::tuple<library_unit_kind, unsigned, unsigned, std::string,
                           std::optional<std::string>, std::string, time_t>>
    parse();

    private:
    std::optional<std::tuple<library_unit_kind, unsigned, unsigned, std::string,
                             std::optional<std::string>, std::string, time_t>>
    parse_entity();

    std::optional<std::tuple<library_unit_kind, unsigned, unsigned, std::string,
                             std::optional<std::string>, std::string, time_t>>
    parse_architecture();

    std::optional<std::tuple<library_unit_kind, unsigned, unsigned, std::string,
                             std::optional<std::string>, std::string, time_t>>
    parse_package();

    std::optional<std::tuple<library_unit_kind, unsigned, unsigned, std::string,
                             std::optional<std::string>, std::string, time_t>>
    parse_package_body();

    std::optional<std::tuple<library_unit_kind, unsigned, unsigned, std::string,
                             std::optional<std::string>, std::string, time_t>>
    parse_configuration();

    // ------------------------------------------------------------------------
    // Miscellaneous functions used to assist parsing
    // ------------------------------------------------------------------------

    //
    // scan the next token and discard the current token
    //
    tk scan();

    //
    // get the current token
    //
    tk current_token();

    //
    // consume current token
    //
    // Always return true
    //
    void consume(token::kind_t token);

    void resync_to_next_unit();

    vhdl::lexer lexer_;
};

}

#endif
