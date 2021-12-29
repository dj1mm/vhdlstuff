
#include "lexer.h"
#include <cassert>

constexpr std::string_view Lsquare        = "'[' is not a valid vhdl character. Use '(' instead";
constexpr std::string_view Rsquare        = "']' is not a valid vhdl character. Use ')' instead";
constexpr std::string_view Not_eq         = "'!=' is not a valid vhdl operator. Use '\\=' instead";
constexpr std::string_view Eq_eq          = "'==' is not used to compare in vhdl. Use '=' instead";
constexpr std::string_view Hash_base      = "'#' is used to define a base literal and it must be preceded by a base";
constexpr std::string_view Lbracket       = "'{' is not a valid vhdl character. Use '(' instead";
constexpr std::string_view Rbracket       = "'}' is not a valid vhdl character. Use ')' instead";
constexpr std::string_view Xor_caret      = "'^' is not a valid vhdl operator. Use 'xor' instead";
constexpr std::string_view Not_tilda      = "'~' is not a valid vhdl operator. Use 'not' instead";
constexpr std::string_view Question_mark  = "'?' can only be used in strings or comments";
constexpr std::string_view Tool_directive = "Tool directives are ignored";
constexpr std::string_view Dollar_sign    = "'$' can only be used in strings or comments";
constexpr std::string_view At_sign        = "'@' can only be used in strings or comments";
constexpr std::string_view Vhdl87_extId   = "extended identifiers not allowed in vhdl87";
constexpr std::string_view Id_start_under = "Identifier cannot start with '_'";
constexpr std::string_view Unterm_str     = "Unterminated string.";
constexpr std::string_view Multiline_str  = "Multi line string not allowed.";
constexpr std::string_view Fmt_effect_str = "Format effector not allowed in a string.";
constexpr std::string_view Graphic_char   = "Expected a graphic character here";
constexpr std::string_view Graphic_str    = "Invalid character in string.";
constexpr std::string_view Unterm_xid     = "Unterminated extended identifier.";
constexpr std::string_view Multiline_xid  = "Multi line extended identifier not allowed.";
constexpr std::string_view Fmt_effect_xid = "Format effector not allowed in a extended identifier.";
constexpr std::string_view Graphic_xid    = "Invalid character in extended identifier.";
constexpr std::string_view Unterm_bstr    = "Unterminated bit string.";
constexpr std::string_view Multiline_bstr = "Multi line bit string not allowed.";
constexpr std::string_view Fmt_effect_bstr= "Format effector not allowed in a bit string.";
constexpr std::string_view Graphic_bstr   = "Invalid character in bit string.";

vhdl::lexer::lexer(const char* s, const char* e, common::stringtable* st,
                   std::vector<common::diagnostic>* ds, std::string fn,
                   vhdl::version v)
    : cs_(s, e), stringtable_(st), diagnostics_(ds), version_(v)
{
    filename_ = stringtable_->get(fn);
    yylloc_.initialize(filename_);

    current_one_ = 0;

    identifier_to_keyword_.insert({"abs"          , vhdl::token::kind_t::kw_abs});
    identifier_to_keyword_.insert({"access"       , vhdl::token::kind_t::kw_access});
    identifier_to_keyword_.insert({"after"        , vhdl::token::kind_t::kw_after});
    identifier_to_keyword_.insert({"alias"        , vhdl::token::kind_t::kw_alias});
    identifier_to_keyword_.insert({"all"          , vhdl::token::kind_t::kw_all});
    identifier_to_keyword_.insert({"and"          , vhdl::token::kind_t::kw_and});
    identifier_to_keyword_.insert({"architecture" , vhdl::token::kind_t::kw_architecture});
    identifier_to_keyword_.insert({"array"        , vhdl::token::kind_t::kw_array});
    identifier_to_keyword_.insert({"assert"       , vhdl::token::kind_t::kw_assert});
    identifier_to_keyword_.insert({"attribute"    , vhdl::token::kind_t::kw_attribute});

    identifier_to_keyword_.insert({"begin"        , vhdl::token::kind_t::kw_begin});
    identifier_to_keyword_.insert({"block"        , vhdl::token::kind_t::kw_block});
    identifier_to_keyword_.insert({"body"         , vhdl::token::kind_t::kw_body});
    identifier_to_keyword_.insert({"buffer"       , vhdl::token::kind_t::kw_buffer});
    identifier_to_keyword_.insert({"bus"          , vhdl::token::kind_t::kw_bus});

    identifier_to_keyword_.insert({"case"         , vhdl::token::kind_t::kw_case});
    identifier_to_keyword_.insert({"component"    , vhdl::token::kind_t::kw_component});
    identifier_to_keyword_.insert({"configuration", vhdl::token::kind_t::kw_configuration});
    identifier_to_keyword_.insert({"constant"     , vhdl::token::kind_t::kw_constant});
    identifier_to_keyword_.insert({"context"      , vhdl::token::kind_t::kw_context});

    identifier_to_keyword_.insert({"default"      , vhdl::token::kind_t::kw_default});
    identifier_to_keyword_.insert({"disconnect"   , vhdl::token::kind_t::kw_disconnect});
    identifier_to_keyword_.insert({"downto"       , vhdl::token::kind_t::kw_downto});

    identifier_to_keyword_.insert({"else"         , vhdl::token::kind_t::kw_else});
    identifier_to_keyword_.insert({"elsif"        , vhdl::token::kind_t::kw_elsif});
    identifier_to_keyword_.insert({"end"          , vhdl::token::kind_t::kw_end});
    identifier_to_keyword_.insert({"entity"       , vhdl::token::kind_t::kw_entity});
    identifier_to_keyword_.insert({"exit"         , vhdl::token::kind_t::kw_exit});

    identifier_to_keyword_.insert({"file"         , vhdl::token::kind_t::kw_file});
    identifier_to_keyword_.insert({"for"          , vhdl::token::kind_t::kw_for});
    identifier_to_keyword_.insert({"function"     , vhdl::token::kind_t::kw_function});

    identifier_to_keyword_.insert({"generate"     , vhdl::token::kind_t::kw_generate});
    identifier_to_keyword_.insert({"generic"      , vhdl::token::kind_t::kw_generic});
    identifier_to_keyword_.insert({"group"        , vhdl::token::kind_t::kw_group});
    identifier_to_keyword_.insert({"guarded"      , vhdl::token::kind_t::kw_guarded});

    identifier_to_keyword_.insert({"if"           , vhdl::token::kind_t::kw_if});
    identifier_to_keyword_.insert({"impure"       , vhdl::token::kind_t::kw_impure});
    identifier_to_keyword_.insert({"in"           , vhdl::token::kind_t::kw_in});
    identifier_to_keyword_.insert({"inertial"     , vhdl::token::kind_t::kw_inertial});
    identifier_to_keyword_.insert({"inout"        , vhdl::token::kind_t::kw_inout});
    identifier_to_keyword_.insert({"is"           , vhdl::token::kind_t::kw_is});

    identifier_to_keyword_.insert({"label"        , vhdl::token::kind_t::kw_label});
    identifier_to_keyword_.insert({"library"      , vhdl::token::kind_t::kw_library});
    identifier_to_keyword_.insert({"linkage"      , vhdl::token::kind_t::kw_linkage});
    identifier_to_keyword_.insert({"literal"      , vhdl::token::kind_t::kw_literal});
    identifier_to_keyword_.insert({"loop"         , vhdl::token::kind_t::kw_loop});

    identifier_to_keyword_.insert({"map"          , vhdl::token::kind_t::kw_map});
    identifier_to_keyword_.insert({"mod"          , vhdl::token::kind_t::kw_mod});

    identifier_to_keyword_.insert({"nand"         , vhdl::token::kind_t::kw_nand});
    identifier_to_keyword_.insert({"new"          , vhdl::token::kind_t::kw_new});
    identifier_to_keyword_.insert({"next"         , vhdl::token::kind_t::kw_next});
    identifier_to_keyword_.insert({"nor"          , vhdl::token::kind_t::kw_nor});
    identifier_to_keyword_.insert({"not"          , vhdl::token::kind_t::kw_not});
    identifier_to_keyword_.insert({"null"         , vhdl::token::kind_t::kw_null});

    identifier_to_keyword_.insert({"of"           , vhdl::token::kind_t::kw_of});
    identifier_to_keyword_.insert({"on"           , vhdl::token::kind_t::kw_on});
    identifier_to_keyword_.insert({"open"         , vhdl::token::kind_t::kw_open});
    identifier_to_keyword_.insert({"or"           , vhdl::token::kind_t::kw_or});
    identifier_to_keyword_.insert({"others"       , vhdl::token::kind_t::kw_others});
    identifier_to_keyword_.insert({"out"          , vhdl::token::kind_t::kw_out});

    identifier_to_keyword_.insert({"package"      , vhdl::token::kind_t::kw_package});
    identifier_to_keyword_.insert({"port"         , vhdl::token::kind_t::kw_port});
    identifier_to_keyword_.insert({"postponed"    , vhdl::token::kind_t::kw_postponed});
    identifier_to_keyword_.insert({"procedural"   , vhdl::token::kind_t::kw_procedural});
    identifier_to_keyword_.insert({"procedure"    , vhdl::token::kind_t::kw_procedure});
    identifier_to_keyword_.insert({"process"      , vhdl::token::kind_t::kw_process});
    identifier_to_keyword_.insert({"protected"    , vhdl::token::kind_t::kw_protected});
    identifier_to_keyword_.insert({"pure"         , vhdl::token::kind_t::kw_pure});

    identifier_to_keyword_.insert({"range"        , vhdl::token::kind_t::kw_range});
    identifier_to_keyword_.insert({"record"       , vhdl::token::kind_t::kw_record});
    identifier_to_keyword_.insert({"reference"    , vhdl::token::kind_t::kw_reference});
    identifier_to_keyword_.insert({"reject"       , vhdl::token::kind_t::kw_reject});
    identifier_to_keyword_.insert({"rem"          , vhdl::token::kind_t::kw_rem});
    identifier_to_keyword_.insert({"report"       , vhdl::token::kind_t::kw_report});
    identifier_to_keyword_.insert({"return"       , vhdl::token::kind_t::kw_return});
    identifier_to_keyword_.insert({"rol"          , vhdl::token::kind_t::kw_rol});
    identifier_to_keyword_.insert({"ror"          , vhdl::token::kind_t::kw_ror});

    identifier_to_keyword_.insert({"select"       , vhdl::token::kind_t::kw_select});
    identifier_to_keyword_.insert({"severity"     , vhdl::token::kind_t::kw_severity});
    identifier_to_keyword_.insert({"shared"       , vhdl::token::kind_t::kw_shared});
    identifier_to_keyword_.insert({"signal"       , vhdl::token::kind_t::kw_signal});
    identifier_to_keyword_.insert({"sla"          , vhdl::token::kind_t::kw_sla});
    identifier_to_keyword_.insert({"sll"          , vhdl::token::kind_t::kw_sll});
    identifier_to_keyword_.insert({"sra"          , vhdl::token::kind_t::kw_sra});
    identifier_to_keyword_.insert({"srl"          , vhdl::token::kind_t::kw_srl});
    identifier_to_keyword_.insert({"subtype"      , vhdl::token::kind_t::kw_subtype});

    identifier_to_keyword_.insert({"then"         , vhdl::token::kind_t::kw_then});
    identifier_to_keyword_.insert({"to"           , vhdl::token::kind_t::kw_to});
    identifier_to_keyword_.insert({"transport"    , vhdl::token::kind_t::kw_transport});
    identifier_to_keyword_.insert({"type"         , vhdl::token::kind_t::kw_type});

    identifier_to_keyword_.insert({"units"        , vhdl::token::kind_t::kw_units});
    identifier_to_keyword_.insert({"until"        , vhdl::token::kind_t::kw_until});
    identifier_to_keyword_.insert({"use"          , vhdl::token::kind_t::kw_use});

    identifier_to_keyword_.insert({"variable"     , vhdl::token::kind_t::kw_variable});

    identifier_to_keyword_.insert({"wait"         , vhdl::token::kind_t::kw_wait});
    identifier_to_keyword_.insert({"when"         , vhdl::token::kind_t::kw_when});
    identifier_to_keyword_.insert({"while"        , vhdl::token::kind_t::kw_while});
    identifier_to_keyword_.insert({"with"         , vhdl::token::kind_t::kw_with});

    identifier_to_keyword_.insert({"xnor"         , vhdl::token::kind_t::kw_xnor});
    identifier_to_keyword_.insert({"xor"          , vhdl::token::kind_t::kw_xor});
}

vhdl::token vhdl::lexer::scan()
{
    peek();

    if (has_checkpoint())
        checkpoints_.top().push_back(tokens_[current_one_]);

    current_one_ = (current_one_ + 1) % MAX_LOOKBACK;

    tokens_[current_one_] = lookahead_tokens_.front();
    lookahead_tokens_.pop_front();

    return tokens_[current_one_];
}

vhdl::token vhdl::lexer::peek(unsigned nth)
{
    if (nth == 0)
        return tokens_[current_one_];

    while (lookahead_tokens_.size() < nth)
    {
        auto item = _lex();
        lookahead_tokens_.push_back(item);
    }

    return lookahead_tokens_[nth - 1];
}

bool vhdl::lexer::look_for(const look_params& param)
{
    vhdl::token::kind_t kind = vhdl::token::kind_t::invalid;

    for (unsigned int n = 0, nest = 0;;)
    {
        kind = peek(n++).kind;

        if (kind == vhdl::token::kind_t::eof || kind == param.abort)
            return false;
        if (kind == param.nest_in)
            nest++;
        if (nest == param.depth)
        {
            for (int i = 0; i < sizeof(param.look) / sizeof(param.look[0]); i++)
            {
                if (kind == param.look[i])
                    return true;
            }
            for (int i = 0; i < sizeof(param.stop) / sizeof(param.stop[0]); i++)
            {
                if (kind == param.stop[i])
                    return false;
            }
        }
        if (kind == param.nest_out && nest > 0)
            nest--;
    }

    return false;
}

void vhdl::lexer::add_checkpoint()
{
    std::vector<vhdl::token> tokens;
    tokens.push_back(tokens_[(current_one_ + 1) % MAX_LOOKBACK]);
    tokens.push_back(tokens_[current_one_]);
    checkpoints_.push(tokens);
}

bool vhdl::lexer::has_checkpoint()
{
    return !checkpoints_.empty();
}

void vhdl::lexer::backtrack()
{
    if (!has_checkpoint())
        return;

    std::vector<vhdl::token> tokens = checkpoints_.top();
    checkpoints_.pop();

    if (tokens.size() <= 2)
        return;

    lookahead_tokens_.push_front(tokens_[current_one_]);

    for (auto i = tokens.size()-1; i > 2; i--)
    {
        lookahead_tokens_.push_front(tokens[i]);
    }

    tokens_[current_one_] = tokens[1];
    tokens_[(current_one_ + 1) % MAX_LOOKBACK] = tokens[0];
}

void vhdl::lexer::drop_checkpoint()
{
    if (!has_checkpoint())
        return;

    std::vector<vhdl::token> tokens = checkpoints_.top();
    checkpoints_.pop();

    if (checkpoints_.size() == 0)
        return;

    auto last_checkpoint = checkpoints_.top();
    last_checkpoint.insert(last_checkpoint.end(), tokens.begin() + 2,
                           tokens.end());
}

vhdl::token vhdl::lexer::current_token() const
{
    return tokens_[current_one_];
}

vhdl::token vhdl::lexer::previous_token() const
{
    return tokens_[(current_one_ + 1) % MAX_LOOKBACK];
}

std::string vhdl::lexer::get_identifier() const
{
    auto tok = tokens_[(current_one_) % MAX_LOOKBACK];
    switch (tok.kind) {
    case vhdl::token::kind_t::identifier:
        return std::string{tok.value};
    case vhdl::token::kind_t::bitstring:
    case vhdl::token::kind_t::stringliteral:
    case vhdl::token::kind_t::integer:
    case vhdl::token::kind_t::real:
    case vhdl::token::kind_t::character:
        return std::string{tok.value};
    default:
        return std::string{tok.value};
    }
}

std::string vhdl::lexer::get_string() const
{
    auto tok = tokens_[(current_one_) % MAX_LOOKBACK];
    switch (tok.kind) {
    case vhdl::token::kind_t::bitstring:
        return std::string{tok.value};
    case vhdl::token::kind_t::stringliteral:
    case vhdl::token::kind_t::character: {
        std::string str = std::string(tok.value);
        if (str.size() < 2)
            throw "String expected to be 2 characters or more";
        if (str[0] != str[str.size() - 1])
            throw "String is surrounded by inconsistend characters";
        if (str[0] != '\"' && str[0] != '\'')
            throw "String is surrounded by invalid characters";
        return str.substr(1, str.size() - 2);
    }
    case vhdl::token::kind_t::integer:
    case vhdl::token::kind_t::real:
        return std::string{tok.value};
    default:
        return "";
    }
}

int vhdl::lexer::get_integer() const
{
    auto tok = tokens_[(current_one_) % MAX_LOOKBACK];
    switch (tok.kind) {
    case vhdl::token::kind_t::integer:
        // TODO : check if integer has non numeric characers first as vhdl
        // allows integers to be defined as <base>#<num>#, eg 16#CAFEF00D#
        return std::stoi(std::string{tok.value});
    default:
        return 0;
    }
}

double vhdl::lexer::get_real() const
{
    auto tok = tokens_[(current_one_) % MAX_LOOKBACK];
    switch (tok.kind) {
    case vhdl::token::kind_t::real:
        return std::stod(std::string{tok.value}, nullptr);
    default:
        return 0;
    }
}

std::string_view vhdl::lexer::get_file()
{
    return filename_;
}

unsigned vhdl::lexer::get_current_line()
{
    return tokens_[current_one_].location.begin.line;
}

unsigned vhdl::lexer::get_current_offset()
{
    return tokens_[current_one_].location.begin.column;
}

common::location vhdl::lexer::get_current_location()
{
    return tokens_[current_one_].location;
}

common::position vhdl::lexer::get_current_position()
{
    return tokens_[current_one_].location.begin;
}

common::location vhdl::lexer::get_previous_location()
{
    return tokens_[(current_one_ + 1) % MAX_LOOKBACK].location;
}

common::position vhdl::lexer::get_previous_position()
{
    return tokens_[(current_one_ + 1) % MAX_LOOKBACK].location.begin;
}

void vhdl::lexer::_diagnose(const std::string_view msg)
{
    if (diagnostics_ == nullptr)
        return;

    diagnostics_->emplace_back(msg, yylloc_);
}

vhdl::token vhdl::lexer::_lex()
{
    using tk = vhdl::token::kind_t;

    // is this the end of the text?
    while (!cs_.end_of_stream())
    {
        yylloc_.step();

        // handle character
        switch (cs_.current_char()) {

        case '_':
            _diagnose(Id_start_under);
            [[fallthrough]];

        // uppercase letters
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':

        // lowercase letters
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
            return _lex_identifier_or_keyword_or_bitstring();

        // digits
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return _lex_number();

        // special characters
        case '"':
            return _lex_string();

        case '#':
            cs_.next();
            yylloc_.columns();
            _diagnose(Hash_base);
            break;

        case '&':
            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::concat);

        case '\'':
            if (cs_.look_ahead(2) == '\'') {
                switch (yyltok_) {
                case vhdl::token::kind_t::rightsquare:
                case vhdl::token::kind_t::rightpar:
                case vhdl::token::kind_t::kw_all:
                case vhdl::token::kind_t::identifier:
                    break;

                default:
                    return _lex_character();
                }
            }

            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::tick);

        case '(':
            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::leftpar);

        case ')':
            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::rightpar);

        case '*':
            cs_.next();
            yylloc_.columns();
            if (cs_.current_char() != '*')
                return _current_token_is_delimiter(tk::times);

            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::pow);

        case '+':
            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::plus);

        case ',':
            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::comma);

        case '-':
            cs_.next();
            yylloc_.columns();
            if (cs_.current_char() != '-') {
                return _current_token_is_delimiter(tk::minus);
            }

            // this is a comment
            cs_.skip_to_eol();

            break;

        case '.':
            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::dot);

        case '/':
            cs_.next();
            yylloc_.columns();
            if (cs_.current_char() != '=')
                return _current_token_is_delimiter(tk::div);

            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::ne);

        case ':':
            cs_.next();
            yylloc_.columns();
            if (cs_.current_char() != '=')
                return _current_token_is_delimiter(tk::colon);

            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::coloneq);

        case ';':
            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::semicolon);

        case '<':
            switch (cs_.look_ahead(1)) {
            case '=':
                cs_.advance(2);
                yylloc_.columns(2);
                return _current_token_is_delimiter(tk::lte);
            case '>':
                cs_.advance(2);
                yylloc_.columns(2);
                return _current_token_is_delimiter(tk::box);
            default:
                cs_.next();
                yylloc_.columns();
                return _current_token_is_delimiter(tk::lt);
            }

        case '=':
            switch (cs_.look_ahead(1)) {
            case '>':
                cs_.advance(2);
                yylloc_.columns(2);
                return _current_token_is_delimiter(tk::rightarrow);
            case '=':
                cs_.advance(2);
                yylloc_.columns(2);
                _diagnose(Eq_eq);
                return _current_token_is_delimiter(tk::eq);
            default:
                cs_.next();
                yylloc_.columns();
                return _current_token_is_delimiter(tk::eq);
            }

        case '>':
            switch (cs_.look_ahead(1)) {
            case '=':
                cs_.advance(2);
                yylloc_.columns(2);
                return _current_token_is_delimiter(tk::gte);
            default:
                cs_.next();
                yylloc_.columns();
                return _current_token_is_delimiter(tk::gt);
            }

        case '[':
            cs_.next();
            yylloc_.columns();
            if (version_ == vhdl87) {
                _diagnose(Lsquare);
                return _current_token_is_delimiter(tk::leftpar);
            }

            return _current_token_is_delimiter(tk::leftsquare);

        case ']':
            cs_.next();
            yylloc_.columns();
            if (version_ == vhdl87) {
                _diagnose(Lsquare);
                return _current_token_is_delimiter(tk::rightpar);
            }

            return _current_token_is_delimiter(tk::rightsquare);

        case '|':
            cs_.next();
            yylloc_.columns();
            return _current_token_is_delimiter(tk::bar);

        // --------------------------------------------------------------------
        // format effectors and space characters
        // --------------------------------------------------------------------
        case ' ':
        case '\t': {
            auto l = 1;
            auto c = cs_.next_char();
            while (c == ' ' || c == -96) {
                ++l;
                c = cs_.look_ahead(l);
            }
            yylloc_.columns(l);
            cs_.advance(l);
            break;
        }

        case -96: // NBSP 0xA0
            cs_.next();
            yylloc_.columns();
            break;

        case '\r':
            if (cs_.look_ahead(1) == '\n')
                cs_.next();
            [[fallthrough]];

        case '\v':
        case '\n':
        case '\f':
            // a sequence of one or more format effectors must cause at least
            // one end of line
            yylloc_.lines();
            cs_.next();
            break;

        case '!':
            cs_.next();
            yylloc_.columns();
            if (cs_.current_char() != '=')
                // According to LRM93 13.10: A vectical line can be replaced by
                // an exclamation mark where used as a delimiter
                return _current_token_is_delimiter(tk::bar);

            // if we are here, we saw this: '!='. This is not vanilla vhdl and
            // so we complain about it
            cs_.next();
            yylloc_.columns();
            _diagnose(Not_eq);
            return _current_token_is_delimiter(tk::ne);

        case '%':
            // LRM93 13.10: the " used as string brackets at both ends of a
            // string literal can be replaced by %.
            return _lex_string();

        case '{':
            cs_.next();
            yylloc_.columns();
            _diagnose(Lbracket);
            return _current_token_is_delimiter(tk::leftpar);

        case '}':
            cs_.next();
            yylloc_.columns();
            _diagnose(Rbracket);
            return _current_token_is_delimiter(tk::rightpar);

        case '\\':
            if (version_ == vhdl87) {
                _diagnose(Vhdl87_extId);
            }
            return _lex_extended_identifier();

        case '^':
            cs_.next();
            yylloc_.columns();
            _diagnose(Xor_caret);
            return _current_token_is_delimiter(tk::kw_xor);

        case '~':
            cs_.next();
            yylloc_.columns();
            _diagnose(Not_tilda);
            return _current_token_is_delimiter(tk::kw_not);

        case '?':
            cs_.next();
            yylloc_.columns();
            _diagnose(Question_mark);
            break;

        case '`':
            cs_.next();
            yylloc_.columns();
            _diagnose(Tool_directive);

            // skip to next line because we dont care about tool directives
            cs_.skip_to_eol();

            break;

        case '$':
            cs_.next();
            yylloc_.columns();
            _diagnose(Dollar_sign);
            break;

        case '@':
            cs_.next();
            yylloc_.columns();
            _diagnose(At_sign);
            break;

        default:
            cs_.next();
            yylloc_.columns();
            break;
        }
    }

    yylloc_.step();
    return vhdl::token(tk::eof, "", false, false, false, false, yylloc_);
}

vhdl::token vhdl::lexer::_lex_string()
{
    auto start = cs_.get_position();
    char quote = cs_.current_char();
    assert(quote == '"' || quote == '%');

    std::vector<char> str;
    str.push_back(quote);

again:
    cs_.next();
    yylloc_.columns();

    if (cs_.end_of_stream())
    {
        _diagnose(Unterm_str);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);
    }

    switch (cs_.current_char()) {
    case '%':
    case '"':
        if (cs_.current_char() != quote)
            break;

        if (cs_.next_char() != quote)
        {
            str.push_back(cs_.current_char());
            cs_.next();
            yylloc_.columns();
            auto sv = stringtable_->get(str.data(), str.size());
            return _current_token_is_literal(vhdl::token::kind_t::stringliteral,
                                             sv, cs_.get_position() - start);
        }

        assert(cs_.current_char() == quote);
        assert(cs_.next_char() == quote);

        // If a quotation-mark value is to be represented in the sequence
        // of characters, then a pair of adjacent quotation marks must be
        // written.
        //
        // In english, %% or "" in a string counts as one % or one "
        cs_.next();
        break;

    case '\r':
    case '\n':
        _diagnose(Multiline_str);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);

    case '\v':
    case '\f':
    case '\t':
        _diagnose(Fmt_effect_str);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);

    default:
        if (!is_graphic_character(cs_.current_char()))
        {
            _diagnose(Graphic_str);
            goto again;
        }
        break;
    }

    str.push_back(cs_.current_char());
    goto again;
}

vhdl::token vhdl::lexer::_lex_character()
{
    assert(cs_.current_char() == '\'');
    assert(cs_.look_ahead(2) == '\'');

    if (auto c = cs_.look_ahead(1); is_graphic_character(c))
    {
        char buffer[3] = {'\'', c, '\''};
        cs_.advance(3);
        yylloc_.columns(3);

        auto sv = stringtable_->get(buffer, 3);
        return _current_token_is_literal(vhdl::token::kind_t::character, sv, 3);
    }

    _diagnose(Graphic_char);
    return vhdl::token(vhdl::token::kind_t::invalid, "", false, false, false,
                       false, yylloc_);
}

vhdl::token vhdl::lexer::_lex_extended_identifier()
{
    auto start = cs_.get_position();
    assert(cs_.current_char() == '\\');

    std::vector<char> str;
    str.push_back('\\');

again:
    cs_.next();
    yylloc_.columns();

    if (cs_.end_of_stream())
    {
        _diagnose(Unterm_xid);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);
    }

    switch (cs_.current_char()) {
    case '\\':
        if (cs_.next_char() != '\\')
        {
            cs_.next();
            auto sv = stringtable_->get(str.data(), str.size());
            return _current_token_is_literal(
                vhdl::token::kind_t::extended_identifier, sv,
                cs_.get_position() - start);
        }

        // LRM 13.3.2
        // If a backslash is to be used as one of the graphic characters of an
        // extended literal, it must be doubled.

        cs_.next();
        break;

    case '\r':
    case '\n':
        _diagnose(Multiline_xid);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);

    case '\v':
    case '\f':
    case '\t':
        _diagnose(Fmt_effect_xid);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);

    default:
        if (!is_graphic_character(cs_.current_char()))
        {
            _diagnose(Graphic_xid);
            goto again;
        }
        break;
    }

    str.push_back(cs_.current_char());
    goto again;
}

vhdl::token vhdl::lexer::_lex_identifier_or_keyword_or_bitstring()
{
    std::vector<char> identifier;
    auto start = cs_.get_position();

again:

    auto c = cs_.current_char();
    switch (c) {

    // uppercase letters
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
        c = c + 32;
        break;

    // lowercase letters
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':

    // digits
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':

    case '_':
        break;

    case '"':
    case '%':
        if (cs_.get_position() - start == 1)
        {
            cs_.set_position(start);
            return _lex_bitstring();
        }

        [[fallthrough]]; // intentional fallthrough
    default:
        auto sv = stringtable_->get(identifier.data(), identifier.size());

        auto it = identifier_to_keyword_.find(sv);
        if (it == identifier_to_keyword_.end())
        {
            return _current_token_is_identifier(vhdl::token::kind_t::identifier,
                                                sv);
        }
        return _current_token_is_keyword(it->second);
    }

    identifier.push_back(c);
    cs_.next();
    yylloc_.columns();
    goto again;
}

vhdl::token vhdl::lexer::_lex_number()
{
    auto start = cs_.get_position();
    assert(is_digit(cs_.current_char()));

    std::vector<char> str;
    _lex_integer(str);

    if (cs_.current_char() == '#')
    {
        str.push_back(cs_.current_char());
        cs_.next();
        yylloc_.columns();
        _lex_based_integer(str);

        bool this_is_an_integer_literal = true;
        if (cs_.current_char() == '.')
        {
            this_is_an_integer_literal = false;
            str.push_back(cs_.current_char());
            cs_.next();
            yylloc_.columns();

            _lex_based_integer(str);
        }

        // fix me
        assert(cs_.current_char() == '#');
        str.push_back(cs_.current_char());
        cs_.next();
        yylloc_.columns();
        _lex_based_integer(str);

        if ((cs_.current_char() == 'e' || cs_.current_char() == 'E'))
        {
            str.push_back(cs_.current_char());
            cs_.next();
            yylloc_.columns();

            if (cs_.current_char() == '+')
            {
                str.push_back(cs_.current_char());
                cs_.next();
                yylloc_.columns();
            }
            else if (cs_.current_char() == '-')
            {
                str.push_back(cs_.current_char());
                cs_.next();
                yylloc_.columns();
            }

            _lex_based_integer(str);
        }

        auto kind = this_is_an_integer_literal ? vhdl::token::kind_t::integer
                                               : vhdl::token::kind_t::real;

        auto sv = stringtable_->get(str.data(), str.size());
        return _current_token_is_literal(kind, sv, cs_.get_position() - start);
    }

    bool this_is_an_integer_literal = true;
    if (cs_.current_char() == '.')
    {
        this_is_an_integer_literal = false;
        str.push_back(cs_.current_char());
        cs_.next();
        yylloc_.columns();

        _lex_integer(str);
    }

    if ((cs_.current_char() == 'e' || cs_.current_char() == 'E'))
    {
        str.push_back(cs_.current_char());
        cs_.next();
        yylloc_.columns();

        if (cs_.current_char() == '+')
        {
            str.push_back(cs_.current_char());
            cs_.next();
            yylloc_.columns();
        }
        else if (cs_.current_char() == '-')
        {
            str.push_back(cs_.current_char());
            cs_.next();
            yylloc_.columns();
        }

        _lex_integer(str);
    }

    auto kind = this_is_an_integer_literal ? vhdl::token::kind_t::integer
                                           : vhdl::token::kind_t::real;

    auto sv = stringtable_->get(str.data(), str.size());
    return _current_token_is_literal(kind, sv, cs_.get_position() - start);
}

vhdl::token vhdl::lexer::_lex_bitstring()
{
    auto start = cs_.get_position();
    auto base = cs_.current_char();
    cs_.next();
    yylloc_.columns();

    char quote = cs_.current_char();
    assert(quote == '"' || quote == '%');

    std::vector<char> str;
    str.push_back(base);
    str.push_back(quote);

again:
    cs_.next();
    yylloc_.columns();

    if (cs_.end_of_stream())
    {
        _diagnose(Unterm_bstr);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);
    }

    switch (cs_.current_char())
    {
    case '%':
    case '"':
        if (cs_.current_char() != quote)
            break;

        if (cs_.next_char() != quote)
        {
            str.push_back(cs_.current_char());
            cs_.next();
            auto sv = stringtable_->get(str.data(), str.size());
            return _current_token_is_literal(vhdl::token::kind_t::bitstring, sv,
                                             cs_.get_position() - start);
        }

        assert(cs_.current_char() == quote);
        assert(cs_.next_char() == quote);

        // If a quotation-mark value is to be represented in the sequence
        // of characters, then a pair of adjacent quotation marks must be
        // written.
        //
        // In english, %% or "" in a string counts as one % or one "
        cs_.next();
        break;

    case '_':
        cs_.next();
        yylloc_.columns();
        break;

    case '\r':
    case '\n':
        _diagnose(Multiline_bstr);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);

    case '\v':
    case '\f':
    case '\t':
        _diagnose(Fmt_effect_bstr);
        return vhdl::token(vhdl::token::kind_t::invalid, "", false, false,
                           false, false, yylloc_);

    default:
        if (!is_graphic_character(cs_.current_char()))
        {
            _diagnose(Graphic_bstr);
            goto again;
        }
        break;
    }

    str.push_back(cs_.current_char());
    goto again;
}

bool vhdl::lexer::_lex_integer(std::vector<char>& buffer)
{
    while (!cs_.end_of_stream())
    {
        if (is_digit(cs_.current_char()))
        {
            buffer.push_back(cs_.current_char());
            cs_.next();
            yylloc_.columns();
        }
        else if (cs_.current_char() == '_')
        {
            cs_.next();
            yylloc_.columns();
        }
        else
        {
            return true;
        }
    }
    return false;
}

bool vhdl::lexer::_lex_based_integer(std::vector<char>& buffer)
{
    while (!cs_.end_of_stream())
    {
        if (is_digit(cs_.current_char()))
        {
            buffer.push_back(cs_.current_char());
            cs_.next();
            yylloc_.columns();
        }
        else if (cs_.current_char() >= 'A' && cs_.current_char() <= 'F')
        {
            // apply lowercase
            buffer.push_back(cs_.current_char()); // TODO: fixme + 32);
            cs_.next();
            yylloc_.columns();
        }
        else if (cs_.current_char() >= 'a' && cs_.current_char() <= 'f')
        {
            buffer.push_back(cs_.current_char());
            cs_.next();
            yylloc_.columns();
        }
        else if (cs_.current_char() == '_')
        {
            cs_.next();
            yylloc_.columns();
        }
        else
        {
            return true;
        }
    }
    return false;
}

vhdl::token vhdl::lexer::_current_token_is_delimiter(vhdl::token::kind_t tk)
{
    auto sv = vhdl::get_token_string_view(tk);
    yyltok_ = tk;
    return vhdl::token(tk, sv, true, false, false, false, yylloc_);
}

vhdl::token vhdl::lexer::_current_token_is_identifier(vhdl::token::kind_t tk,
                                                      std::string_view sv)
{
    yyltok_ = tk;
    return vhdl::token(tk, sv, false, true, false, false, yylloc_);
}

vhdl::token vhdl::lexer::_current_token_is_literal(vhdl::token::kind_t tk,
                                                   std::string_view sv,
                                                   ptrdiff_t ln)
{
    yyltok_ = tk;
    return vhdl::token(tk, sv, false, false, true, false, yylloc_);
}

vhdl::token vhdl::lexer::_current_token_is_keyword(vhdl::token::kind_t tk)
{
    auto sv = vhdl::get_token_string_view(tk);
    yyltok_ = tk;
    return vhdl::token(tk, sv, false, false, false, true, yylloc_);
}
