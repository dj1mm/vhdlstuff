
#ifndef VHDL_LEXER_H
#define VHDL_LEXER_H

#include <algorithm>
#include <deque>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "character_stream.h"

#include "common/diagnostics.h"
#include "common/location.h"
#include "common/position.h"
#include "common/stringtable.h"

#include "token.h"

namespace vhdl
{

struct look_params
{
    token::kind_t look[4];
    token::kind_t stop[4];
    token::kind_t abort;
    token::kind_t nest_in;
    token::kind_t nest_out;
    unsigned int depth;
};

class lexer
{
    public:
    lexer(const char*, const char*, common::stringtable*,
          std::vector<common::diagnostic>*, std::string = "", version = vhdl93);

    ~lexer() = default;

    //
    // Instructs the lexer to scan the next vhdl token. Takes the first token
    // on the lookahead queue, passes it to the callee, and discards it.
    // The lexer has a lookback history of 1. Therefore, the last discarded
    // token can still be accessed.
    //
    vhdl::token scan();

    //
    // Peeks the nth vhdl token. Note that nth >= 0.
    //
    // Doing peek(0) is equivalent to doing get_current_token()
    // If nth > 1, we will return the next token in the sequence we are lexing.
    // These peeked/looked ahead tokens are still made available to subsequent
    // current_token(). This is done by adding the tokens that we peeked to a
    // queue of tokens.
    //
    // The lexer has a lookahead of (infinity).
    //
    // Consider the sequence of token:
    // <identifier> <colon> <null> <semicolon> <process> <this> <that>
    //
    // Also consider that we just did a lexer.scan() and lexer.current_token()
    // returns:
    //     <identifier>
    //
    // Then lexer.peek(nth) will return values as follows:
    //  nth |  returned value       |  notable note
    //  0   |   <identifier>        | this is the same as current_token()
    //  1   |   <colon>             | same as doing peek() with no args
    //  2   |   <null>              |
    //  3   |   <semicolon>         |
    //  4   |   <process>           |
    //  5   |   <this>              |
    //  6   |   <that>              |
    // >= 7 |   <EOF>               |
    //
    vhdl::token peek(unsigned = 1);

    //
    // Look for a vhdl token::kind_t ahead in the token::kind_t stream.
    //
    bool look_for(const look_params&);

    //
    // Add a checkpoint to the current token so that lexer stores following
    // tokens for replay if needed. Multiple checkpoints can be added
    //
    void add_checkpoint();

    //
    // Return true if there is an active checkpoint that the lexer is keeping
    // track of
    //
    bool has_checkpoint();

    //
    // Stop and replay any tokens that were lexed since the last checkpoint.
    // Does nothing if there was no checkpoint.
    // If there are multiple checkpoints, lexer replays to the last checkpoint
    //
    void backtrack();

    //
    // Drop the last checkpoint.
    // If there is no other checkpoint, discard all the tokens that were lexed
    // Do nothing if there was no checkpoint
    // If there are multiple checkpoints, lexer should not drop tokens, we may
    // still want to replay those!
    //
    void drop_checkpoint();

    //
    // Get the current token that has been scanned
    //
    vhdl::token current_token() const;

    //
    // Get the previous token that has been discarded
    //
    vhdl::token previous_token() const;

    //
    // when current_token() is an identifier, a char or a string, we can get
    // the identifier string by doing get_identifier().
    //
    std::string get_identifier() const;

    //
    // when current_token() returns string, we can get the string by calling
    // get_string()
    //
    std::string get_string() const;

    //
    // when current_token() returns integer, we can get the integer by calling
    // get_integer()
    //
    int get_integer() const;

    //
    // when current_token() returns real, we can get the real number by calling
    // get_real(). The real number is represented as a cpp double
    //
    double get_real() const;

    //
    // Get filename
    //
    std::string_view get_file();

    //
    // Get the start line of the current token
    //
    unsigned get_current_line();

    //
    // Get the start line offset of the current token
    //
    unsigned get_current_offset();

    //
    // Get the location of the current token. Refer to vhdl/location.h for more
    // information. Note location contains a begin and an end position - should
    // really have called it range!!
    //
    common::location get_current_location();

    //
    // Get the begin position of the current token. Refer to vhdl/position.h
    // for more information
    //
    common::position get_current_position();

    //
    // Get the location of the previous token
    //
    common::location get_previous_location();

    //
    // Get the begin position of the previous token
    //
    common::position get_previous_position();

    private:

    void _diagnose(const std::string_view);

    vhdl::token _lex();

    // LRM93 13.6
    //
    // string_literal ::= " { graphic_character } "
    vhdl::token _lex_string();

    // LRM93 13.5
    //
    // character_literal ::= ' graphic_character '
    vhdl::token _lex_character();

    // LRM93 13.3.2
    //
    // extended_identifier ::= \ graphic_character { graphic_character }
    vhdl::token _lex_extended_identifier();

    // LRM93 13.3
    //
    // basic_identifier ::= letter { [ underline ] letter_or_digit }
    // letter_or_digit ::= letter | digit
    // letter ::= upper_case_letter | lower_case_letter
    vhdl::token _lex_identifier_or_keyword_or_bitstring();

    // LRM93 13.4
    //
    // decimal_literal ::= integer [ . integer ] [ exponent ]
    // exponent ::= e [ + ] integer | e - integer
    //
    // based_literal ::= base # based_integer [ . based_integer ] # exponent
    // base ::= integer
    vhdl::token _lex_number();

    // LRM93 13.7
    //
    // bit_string_literal ::= base_specifier " [ bit_value ] "
    // bit_value ::= extended_digit { [ underline ] extended_digit }
    vhdl::token _lex_bitstring();

    // returns True:  all good
    //         False: eof reached
    bool _lex_integer(std::vector<char>&);
    bool _lex_based_integer(std::vector<char>&);

    vhdl::token _current_token_is_delimiter(vhdl::token::kind_t);
    vhdl::token _current_token_is_identifier(vhdl::token::kind_t, std::string_view);
    vhdl::token _current_token_is_literal(vhdl::token::kind_t, std::string_view, ptrdiff_t);
    vhdl::token _current_token_is_keyword(vhdl::token::kind_t);

    version version_ = vhdl93;

    common::stringtable* stringtable_;
    std::vector<common::diagnostic>* diagnostics_;
    vhdl::character_stream cs_;

    token::kind_t yyltok_;    // last token that was lexed
    common::location yylloc_; // location of last token that was lexed

    std::deque<token> lookahead_tokens_;

    constexpr static int MAX_LOOKBACK = 2;
    token tokens_[MAX_LOOKBACK];
    int current_one_;

    std::string_view filename_;

    std::stack<std::vector<token>> checkpoints_;

    // map of identifier to reserved keywords
    std::unordered_map<std::string_view, vhdl::token::kind_t>
        identifier_to_keyword_;



};

}

#endif
