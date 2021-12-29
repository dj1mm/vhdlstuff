
#ifndef VHDL_TOKEN_H
#define VHDL_TOKEN_H

#include <string>
#include <string_view>
#include <sstream>
#include "common/location.h"

namespace vhdl
{

enum version { vhdl87, vhdl93, vhdl02, vhdl08 };

enum base { b, o, x, d };
enum sign { s, u, n };

class token
{
    public:

    enum kind_t
    {
        invalid,
        implicit,
        eof,

        // LRM87 13.1 Lexical elements, separators, and delimiters
        // LRM93 13.2 Lexical elements, separators, and delimiters
        // LRM02 13.2 Lexical elements, separators, and delimiters
        // LRM08
        // A delimiter is either: one of the following special characters or
        // one of the following compound delimiters
        concat,         // &
        tick,           // '
        leftpar,        // (
        rightpar,       // )
        times,          // *
        plus,           // +
        comma,          // ,
        minus,          // -
        dot,            // .
        div,            // /
        colon,          // :
        semicolon,      // ;
        lt,             // <
        eq,             // =
        gt,             // >
        bar,            // |
        leftsquare,     // [
        rightsquare,    // ]
        rightarrow,     // =>
        pow,            // **
        coloneq,        // :=
        ne,             // /=
        gte,            // >=
        lte,            // <=
        box,            // <>

        // LRM87 13.3 Identifiers (only basic identifiers supported)
        // LRM93 13.3 Identifiers (basic identifiers and extended identifiers)
        // LRM02 13.3 Identifiers (basic identifiers and extended identifiers)
        // LRM08
        identifier,
        extended_identifier,

        // LRM87 13.4 Abstract literals
        // LRM93 13.4 Abstract literals
        // LRM02 13.4 Abstract literals
        // LRM08
        integer,
        real,

        // LRM87 13.5 Character literals
        // LRM93 13.5 Character literals
        // LRM02 13.5 Character literals
        // LRM08
        character,

        // LRM87 13.6 String literals
        // LRM93 13.6 String literals
        // LRM02 13.6 String literals
        // LRM08
        stringliteral,

        // LRM87 13.7 Bit string literals
        // LRM93 13.7 Bit string literals
        // LRM02 13.7 Bit string literals
        // LRM08
        bitstring,

        // LRM87 13.9 Reserved words
        // LRM93 13.9 Reserved words
        // LRM02 13.9 Reserved words
        // LRM08
        kw_abs,
        kw_access,
        kw_after,
        kw_alias,
        kw_all,
        kw_and,
        kw_architecture,
        kw_array,
        kw_assert,
        kw_attribute,

        kw_begin,
        kw_block,
        kw_body,
        kw_buffer,
        kw_bus,

        kw_case,
        kw_component,
        kw_configuration,
        kw_constant,
        kw_context,

        kw_default,
        kw_disconnect,
        kw_downto,

        kw_else,
        kw_elsif,
        kw_end,
        kw_entity,
        kw_exit,

        kw_file,
        kw_for,
        kw_function,

        kw_generate,
        kw_generic,
        kw_group,
        kw_guarded,

        kw_if,
        kw_impure,
        kw_in,
        kw_inertial,
        kw_inout,
        kw_is,

        kw_label,
        kw_library,
        kw_linkage,
        kw_literal,
        kw_loop,

        kw_map,
        kw_mod,

        kw_nand,
        kw_new,
        kw_next,
        kw_nor,
        kw_not,
        kw_null,

        kw_of,
        kw_on,
        kw_open,
        kw_or,
        kw_others,
        kw_out,

        kw_package,
        kw_port,
        kw_postponed,
        kw_procedural,
        kw_procedure,
        kw_process,
        kw_protected,
        kw_pure,

        kw_range,
        kw_record,
        kw_reference,
        kw_register,
        kw_reject,
        kw_rem,
        kw_report,
        kw_return,
        kw_rol,
        kw_ror,

        kw_select,
        kw_severity,
        kw_shared,
        kw_signal,
        kw_sla,
        kw_sll,
        kw_sra,
        kw_srl,
        kw_subtype,

        kw_then,
        kw_to,
        kw_transport,
        kw_type,

        kw_unaffected,
        kw_units,
        kw_until,
        kw_use,

        kw_variable,

        kw_wait,
        kw_when,
        kw_while,
        kw_with,

        kw_xnor,
        kw_xor,
    };

    kind_t kind;
    std::string_view value;

    // returns true if the token kind is a delimiter as defined by the vhdl lrm
    //
    // LRM87 13.1 Lexical elements, separators, and delimiters
    // LRM93 13.2 Lexical elements, separators, and delimiters
    // LRM02 13.2 Lexical elements, separators, and delimiters
    bool is_delimiter;

    // returns true if the token kind is an identifier
    //
    // LRM87 13.3 Identifiers (only basic identifiers supported)
    // LRM93 13.3 Identifiers (basic identifiers and extended identifiers)
    // LRM02 13.3 Identifiers (basic identifiers and extended identifiers)
    bool is_identifier;

    // returns true if the token kind is a string, character or bit str literal
    //
    // LRM87 13.4 Abstract literals
    // LRM93 13.4 Abstract literals
    // LRM02 13.4 Abstract literals
    bool is_literal;

    // returns true if the token kind is a keyword as defined by the vhdl lrm.
    // Note the lrm calls this a reserved word
    //
    // LRM87 13.9 Reserved words
    // LRM93 13.9 Reserved words
    // LRM02 13.9 Reserved words
    bool is_keyword;

    common::location location;

    token()
    {
        kind = kind_t::invalid;
        value = "";
        location.initialize("", 0, 0);
    }

    token(kind_t k, std::string_view v, bool d, bool i, bool lit, bool kw, common::location l)
    {
        kind = k;
        value = v;
        is_delimiter = d;
        is_identifier = i;
        is_literal = lit;
        is_keyword = kw;
        location = l;
    }

    token(std::string_view v)
    {
        kind = implicit;
        value = v;
        is_delimiter = false;
        is_identifier = true;
        is_literal = false;
        is_keyword = false;
        location = {};
    }

    token(token const&) = default;
    token(token&&) = default;
    token& operator=(token const&) = default;
    token& operator=(token&&) = default;

    // short way of doign vhdl::get_token_string_view(this_token.kind)
    std::string to_string() const;

    // return debug string of this token
    std::string debug() const;

    bool operator==(const token) const;
    bool operator==(const kind_t) const;

    bool operator!=(const token) const;
    bool operator!=(const kind_t) const;
};

// get a string view representation of the token kind
//
// for example, consider the token tick '. this function returns "'"
constexpr std::string_view get_token_string_view(token::kind_t kind)
{
    switch (kind)
    {
        case token::eof:              return "";

        // LRM87 13.1 Lexical elements, separators, and delimiters
        // LRM93 13.2 Lexical elements, separators, and delimiters
        // LRM02 13.2 Lexical elements, separators, and delimiters
        case token::concat:           return "&";
        case token::tick:             return "'";
        case token::leftpar:          return "(";
        case token::rightpar:         return ")";
        case token::times:            return "*";
        case token::plus:             return "+";
        case token::comma:            return ",";
        case token::minus:            return "-";
        case token::dot:              return ".";
        case token::div:              return "/";
        case token::colon:            return ":";
        case token::semicolon:        return ";";
        case token::lt:               return "<";
        case token::eq:               return "=";
        case token::gt:               return ">";
        case token::bar:              return "|";
        case token::leftsquare:       return "[";
        case token::rightsquare:      return "]";
        case token::rightarrow:       return "=>";
        case token::pow:              return "**";
        case token::coloneq:          return ":=";
        case token::ne:               return "/=";
        case token::gte:              return ">=";
        case token::lte:              return "<=";
        case token::box:              return "<>";

        // LRM87 13.3 Identifiers (only basic identifiers supported)
        // LRM93 13.3 Identifiers (basic identifiers and extended identifiers)
        // LRM02 13.3 Identifiers (basic identifiers and extended identifiers)
        case token::identifier:       return "identifier";
        case token::extended_identifier:return "extended_identifier";
        
        // LRM87 13.4 Abstract literals
        // LRM93 13.4 Abstract literals
        // LRM02 13.4 Abstract literals
        case token::integer:          return "integer";
        case token::real:             return "real";

        // LRM87 13.5 Character literals
        // LRM93 13.5 Character literals
        // LRM02 13.5 Character literals
        case token::character:        return "character";
        
        // LRM87 13.6 String literals
        // LRM93 13.6 String literals
        // LRM02 13.6 String literals
        case token::stringliteral:    return "string";

        // LRM87 13.7 Bit string literals
        // LRM93 13.7 Bit string literals
        // LRM02 13.7 Bit string literals
        case token::bitstring:        return "bitstring";

        // LRM87 13.9 Reserved words
        // LRM93 13.9 Reserved words
        // LRM02 13.9 Reserved words
        case token::kw_abs:           return "abs";
        case token::kw_access:        return "access";
        case token::kw_after:         return "after";
        case token::kw_alias:         return "alias";
        case token::kw_all:           return "all";
        case token::kw_and:           return "and";
        case token::kw_architecture:  return "architecture";
        case token::kw_array:         return "array";
        case token::kw_assert:        return "assert";
        case token::kw_attribute:     return "attribute";

        case token::kw_begin:         return "begin";
        case token::kw_block:         return "block";
        case token::kw_body:          return "body";
        case token::kw_buffer:        return "buffer";
        case token::kw_bus:           return "bus";

        case token::kw_case:          return "case";
        case token::kw_component:     return "component";
        case token::kw_configuration: return "configuration";
        case token::kw_constant:      return "constant";
        case token::kw_context:       return "context";

        case token::kw_default:       return "default";
        case token::kw_disconnect:    return "disconnect";
        case token::kw_downto:        return "downto";

        case token::kw_else:          return "else";
        case token::kw_elsif:         return "elsif";
        case token::kw_end:           return "end";
        case token::kw_entity:        return "entity";
        case token::kw_exit:          return "exit";

        case token::kw_file:          return "file";
        case token::kw_for:           return "for";
        case token::kw_function:      return "function";

        case token::kw_generate:      return "generate";
        case token::kw_generic:       return "generic";
        case token::kw_group:         return "group";
        case token::kw_guarded:       return "guarded";

        case token::kw_if:            return "if";
        case token::kw_impure:        return "impure";
        case token::kw_in:            return "in";
        case token::kw_inertial:      return "inertial";
        case token::kw_inout:         return "inout";
        case token::kw_is:            return "is";

        case token::kw_label:         return "label";
        case token::kw_library:       return "library";
        case token::kw_linkage:       return "linkage";
        case token::kw_literal:       return "literal";
        case token::kw_loop:          return "loop";

        case token::kw_map:           return "map";
        case token::kw_mod:           return "mod";

        case token::kw_nand:          return "nand";
        case token::kw_new:           return "new";
        case token::kw_next:          return "next";
        case token::kw_nor:           return "nor";
        case token::kw_not:           return "not";
        case token::kw_null:          return "null";

        case token::kw_of:            return "of";
        case token::kw_on:            return "on";
        case token::kw_open:          return "open";
        case token::kw_or:            return "or";
        case token::kw_others:        return "others";
        case token::kw_out:           return "out";

        case token::kw_package:       return "package";
        case token::kw_port:          return "port";
        case token::kw_postponed:     return "postponed";
        case token::kw_procedural:    return "procedural";
        case token::kw_procedure:     return "procedure";
        case token::kw_process:       return "process";
        case token::kw_protected:     return "protected";
        case token::kw_pure:          return "pure";

        case token::kw_range:         return "range";
        case token::kw_record:        return "record";
        case token::kw_reference:     return "reference";
        case token::kw_reject:        return "reject";
        case token::kw_rem:           return "rem";
        case token::kw_report:        return "report";
        case token::kw_return:        return "return";
        case token::kw_rol:           return "rol";
        case token::kw_ror:           return "ror";

        case token::kw_select:        return "select";
        case token::kw_severity:      return "severity";
        case token::kw_shared:        return "shared";
        case token::kw_signal:        return "signal";
        case token::kw_sla:           return "sla";
        case token::kw_sll:           return "sll";
        case token::kw_sra:           return "sra";
        case token::kw_srl:           return "srl";
        case token::kw_subtype:       return "subtype";

        case token::kw_then:          return "then";
        case token::kw_to:            return "to";
        case token::kw_transport:     return "transport";
        case token::kw_type:          return "type";

        case token::kw_units:         return "units";
        case token::kw_until:         return "until";
        case token::kw_use:           return "use";

        case token::kw_variable:      return "variable";

        case token::kw_wait:          return "wait";
        case token::kw_when:          return "when";
        case token::kw_while:         return "while";
        case token::kw_with:          return "with";

        case token::kw_xnor:          return "xnor";
        case token::kw_xor:           return "xor";
        default:            return "n/a";
    }
}

// get a string_view of the name of the token kind
//
// for example, consider the token tick '. this function returns "tick"
constexpr std::string_view get_token_string_name(token::kind_t kind)
{
    switch (kind)
    {
        case token::eof:              return "eof";

        // LRM87 13.1 Lexical elements, separators, and delimiters
        // LRM93 13.2 Lexical elements, separators, and delimiters
        // LRM02 13.2 Lexical elements, separators, and delimiters
        case token::concat:           return "concat";
        case token::tick:             return "tick";
        case token::leftpar:          return "leftpar";
        case token::rightpar:         return "rightpar";
        case token::times:            return "times";
        case token::plus:             return "plus";
        case token::comma:            return "comma";
        case token::minus:            return "minus";
        case token::dot:              return "dot";
        case token::div:              return "div";
        case token::colon:            return "colon";
        case token::semicolon:        return "semicolon";
        case token::lt:               return "lt";
        case token::eq:               return "eq";
        case token::gt:               return "gt";
        case token::bar:              return "bar";
        case token::leftsquare:       return "leftsquare";
        case token::rightsquare:      return "rightsquare";
        case token::rightarrow:       return "rightarrow";
        case token::pow:              return "pow";
        case token::coloneq:          return "coloneq";
        case token::ne:               return "ne";
        case token::gte:              return "gte";
        case token::lte:              return "lte";
        case token::box:              return "box";

        // LRM87 13.3 Identifiers (only basic identifiers supported)
        // LRM93 13.3 Identifiers (basic identifiers and extended identifiers)
        // LRM02 13.3 Identifiers (basic identifiers and extended identifiers)
        case token::identifier:       return "identifier";
        case token::extended_identifier:return "extended_identifier";

        // LRM87 13.4 Abstract literals
        // LRM93 13.4 Abstract literals
        // LRM02 13.4 Abstract literals
        case token::integer:          return "integer";
        case token::real:             return "real";

        // LRM87 13.5 Character literals
        // LRM93 13.5 Character literals
        // LRM02 13.5 Character literals
        case token::character:        return "character";
        
        // LRM87 13.6 String literals
        // LRM93 13.6 String literals
        // LRM02 13.6 String literals
        case token::stringliteral:    return "string";

        // LRM87 13.7 Bit string literals
        // LRM93 13.7 Bit string literals
        // LRM02 13.7 Bit string literals
        case token::bitstring:        return "bitstring";

        // LRM87 13.9 Reserved words
        // LRM93 13.9 Reserved words
        // LRM02 13.9 Reserved words
        case token::kw_abs:           return "abs";
        case token::kw_access:        return "access";
        case token::kw_after:         return "after";
        case token::kw_alias:         return "alias";
        case token::kw_all:           return "all";
        case token::kw_and:           return "and";
        case token::kw_architecture:  return "architecture";
        case token::kw_array:         return "array";
        case token::kw_assert:        return "assert";
        case token::kw_attribute:     return "attribute";

        case token::kw_begin:         return "begin";
        case token::kw_block:         return "block";
        case token::kw_body:          return "body";
        case token::kw_buffer:        return "buffer";
        case token::kw_bus:           return "bus";

        case token::kw_case:          return "case";
        case token::kw_component:     return "component";
        case token::kw_configuration: return "configuration";
        case token::kw_constant:      return "constant";
        case token::kw_context:       return "context";

        case token::kw_default:       return "default";
        case token::kw_disconnect:    return "disconnect";
        case token::kw_downto:        return "downto";

        case token::kw_else:          return "else";
        case token::kw_elsif:         return "elsif";
        case token::kw_end:           return "end";
        case token::kw_entity:        return "entity";
        case token::kw_exit:          return "exit";

        case token::kw_file:          return "file";
        case token::kw_for:           return "for";
        case token::kw_function:      return "function";

        case token::kw_generate:      return "generate";
        case token::kw_generic:       return "generic";
        case token::kw_group:         return "group";
        case token::kw_guarded:       return "guarded";

        case token::kw_if:            return "if";
        case token::kw_impure:        return "impure";
        case token::kw_in:            return "in";
        case token::kw_inertial:      return "inertial";
        case token::kw_inout:         return "inout";
        case token::kw_is:            return "is";

        case token::kw_label:         return "label";
        case token::kw_library:       return "library";
        case token::kw_linkage:       return "linkage";
        case token::kw_literal:       return "literal";
        case token::kw_loop:          return "loop";

        case token::kw_map:           return "map";
        case token::kw_mod:           return "mod";

        case token::kw_nand:          return "nand";
        case token::kw_new:           return "new";
        case token::kw_next:          return "next";
        case token::kw_nor:           return "nor";
        case token::kw_not:           return "not";
        case token::kw_null:          return "null";

        case token::kw_of:            return "of";
        case token::kw_on:            return "on";
        case token::kw_open:          return "open";
        case token::kw_or:            return "or";
        case token::kw_others:        return "others";
        case token::kw_out:           return "out";

        case token::kw_package:       return "package";
        case token::kw_port:          return "port";
        case token::kw_postponed:     return "postponed";
        case token::kw_procedural:    return "procedural";
        case token::kw_procedure:     return "procedure";
        case token::kw_process:       return "process";
        case token::kw_protected:     return "protected";
        case token::kw_pure:          return "pure";

        case token::kw_range:         return "range";
        case token::kw_record:        return "record";
        case token::kw_reference:     return "reference";
        case token::kw_reject:        return "reject";
        case token::kw_rem:           return "rem";
        case token::kw_report:        return "report";
        case token::kw_return:        return "return";
        case token::kw_rol:           return "rol";
        case token::kw_ror:           return "ror";

        case token::kw_select:        return "select";
        case token::kw_severity:      return "severity";
        case token::kw_shared:        return "shared";
        case token::kw_signal:        return "signal";
        case token::kw_sla:           return "sla";
        case token::kw_sll:           return "sll";
        case token::kw_sra:           return "sra";
        case token::kw_srl:           return "srl";
        case token::kw_subtype:       return "subtype";

        case token::kw_then:          return "then";
        case token::kw_to:            return "to";
        case token::kw_transport:     return "transport";
        case token::kw_type:          return "type";

        case token::kw_units:         return "units";
        case token::kw_until:         return "until";
        case token::kw_use:           return "use";

        case token::kw_variable:      return "variable";

        case token::kw_wait:          return "wait";
        case token::kw_when:          return "when";
        case token::kw_while:         return "while";
        case token::kw_with:          return "with";

        case token::kw_xnor:          return "xnor";
        case token::kw_xor:           return "xor";
        default:                      return "n/a";
    }
}

}

template <typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, const vhdl::token& t)
{
    os << t.to_string();
    return os;
}


#endif
