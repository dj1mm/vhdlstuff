
#include "fast_parser.h"

// ----------------------------------------------------------------------------
// parser methods
// ----------------------------------------------------------------------------

vhdl::fast_parser::fast_parser(common::stringtable* st, const char* s,
                               const char* e, std::string filename)
    : lexer_(s, e, st, nullptr, filename)
{
    scan();
    // first token is always invalid hence the skip
}

std::vector<std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
                       std::optional<std::string>, std::string, time_t>>
vhdl::fast_parser::parse()
{
    std::vector<
        std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
                   std::optional<std::string>, std::string, time_t>>
        result;

    while (current_token() != tk::eof)
    {
        std::optional<
            std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
                       std::optional<std::string>, std::string, time_t>>
            unit;

        switch (current_token())
        {
        case tk::kw_entity:
            unit = parse_entity();
            break;
        case tk::kw_architecture:
            unit = parse_architecture();
            break;
        case tk::kw_package:
            consume(tk::kw_package);
            if (current_token() == tk::kw_body)
                unit = parse_package_body();
            else
                unit = parse_package();
            break;
        case tk::kw_configuration:
            unit = parse_configuration();
            break;
        case tk::kw_library:
        case tk::kw_use:
            scan();
            [[fallthrough]];
        default:
            resync_to_next_unit();
        }

        if (unit)
        {
            result.push_back(unit.value());
        }
    }

    return result;
}

std::optional<
    std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
               std::optional<std::string>, std::string, time_t>>
vhdl::fast_parser::parse_entity()
{
    auto line = lexer_.get_current_line();
    auto column = lexer_.get_current_offset();
    auto filename = std::string{lexer_.get_file()};

    consume(tk::kw_entity);

    if (current_token() != tk::identifier)
        return std::nullopt;

    auto identifier = lexer_.get_identifier();
    consume(tk::identifier);

    return std::make_tuple(vhdl::library_unit_kind::entity, line, column,
                           identifier, std::nullopt, filename, 0);
}

std::optional<
    std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
               std::optional<std::string>, std::string, time_t>>
vhdl::fast_parser::parse_architecture()
{
    auto line = lexer_.get_current_line();
    auto column = lexer_.get_current_offset();
    auto filename = std::string{lexer_.get_file()};

    consume(tk::kw_architecture);

    if (current_token() != tk::identifier)
        return std::nullopt;

    auto identifier = lexer_.get_identifier();
    consume(tk::identifier);

    if (current_token() != tk::kw_of)
        return std::nullopt;

    consume(tk::kw_of);
    if (current_token() != tk::identifier)
        return std::nullopt;

    auto identifier2 = lexer_.get_identifier();
    consume(tk::identifier);

    return std::make_tuple(vhdl::library_unit_kind::architecture, line, column,
                           identifier, identifier2, filename, 0);
}

std::optional<
    std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
               std::optional<std::string>, std::string, time_t>>
vhdl::fast_parser::parse_package_body()
{
    auto line = lexer_.get_current_line();
    auto column = lexer_.get_current_offset();
    auto filename = std::string{lexer_.get_file()};

    consume(tk::kw_body);

    if (current_token() != tk::identifier)
        return std::nullopt;

    auto identifier = lexer_.get_identifier();
    consume(tk::identifier);

    return std::make_tuple(vhdl::library_unit_kind::package_body, line, column,
                           identifier, std::nullopt, filename, 0);
}

std::optional<
    std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
               std::optional<std::string>, std::string, time_t>>
vhdl::fast_parser::parse_package()
{
    auto line = lexer_.get_current_line();
    auto column = lexer_.get_current_offset();
    auto filename = std::string{lexer_.get_file()};

    if (current_token() != tk::identifier)
        return std::nullopt;

    auto identifier = lexer_.get_identifier();
    consume(tk::identifier);

    return std::make_tuple(vhdl::library_unit_kind::package, line, column,
                           identifier, std::nullopt, filename, 0);
}

std::optional<
    std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
               std::optional<std::string>, std::string, time_t>>
vhdl::fast_parser::parse_configuration()
{
    auto line = lexer_.get_current_line();
    auto column = lexer_.get_current_offset();
    auto filename = std::string{lexer_.get_file()};

    consume(tk::kw_configuration);

    if (current_token() != tk::identifier)
        return std::nullopt;

    auto identifier = lexer_.get_identifier();
    consume(tk::identifier);

    if (current_token() != tk::kw_of)
        return std::nullopt;

    consume(tk::kw_of);
    if (current_token() != tk::identifier)
        return std::nullopt;

    auto identifier2 = lexer_.get_identifier();
    consume(tk::identifier);

    return std::make_tuple(vhdl::library_unit_kind::configuration, line, column,
                           identifier, identifier2, filename, 0);
}

vhdl::token::kind_t vhdl::fast_parser::scan()
{
    return lexer_.scan().kind;
}

vhdl::token::kind_t vhdl::fast_parser::current_token()
{
    return lexer_.current_token().kind;
}

void vhdl::fast_parser::consume(token::kind_t token)
{
    lexer_.scan();
}

void vhdl::fast_parser::resync_to_next_unit()
{
    while (true)
    {
        // skip the current word
        lexer_.scan();

        switch (current_token()) {
        case tk::eof:
            return;
        case tk::kw_library:
        case tk::kw_use:
        case tk::kw_architecture:
        case tk::kw_entity:
        case tk::kw_package:
        case tk::kw_configuration:
            if (lexer_.previous_token() == tk::kw_end)
                break;
            return; // this is the possible start of a new unit
        case tk::semicolon:
        default:
            break;
        }
    }
}
