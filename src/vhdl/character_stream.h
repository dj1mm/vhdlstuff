
#ifndef VHDL_CHARACTER_STREAM_H
#define VHDL_CHARACTER_STREAM_H

#include <cstddef>
#include <string>

namespace vhdl
{

// VHDL Source file should be encoded in ISO/IEC 8859-1
// https://en.wikipedia.org/wiki/ISO/IEC_8859-1

// basic_graphic_character ::=
//     upper_case_letter | digit | special_character | space_character
//
// graphic_character ::=
//     basic_graphic_character | lower_case_letter | other_special_character
//
bool is_graphic_character(char);

// a) Uppercase letters
// --------------------
// A B C D E F G H I J K L M N O P Q R S T U V W X Y Z À Á Â Ã Ä Å Æ Ç È É
// Ê Ë Ì Í Î Ï Ð Ñ Ò Ó Ô Õ Ö Ø Ù Ú Û Ü Ý Þ
bool is_upper_case_letter(char);

// b) Digits
// ---------
// 0 1 2 3 4 5 6 7 8 9
// TODO: write these functions
bool is_digit(char);

// c) Special characters
// ---------------------
// " # & ' () * + , - . / : ; < = > [ ] _ |
// TODO: write these functions
bool is_special_character(char);

// d) Space characters
// -------------------
// SPACE NBSP
// TODO: write these functions
bool is_space_character(char);

// e) Lowercase letters
// --------------------
// a b c d e f g h i j k l m n o p q r s t u v w x y z ß à á â ã ä å æ ç è
// é ê ë ì í î ï ð ñ ò ó ô õ ö ø ù ú û ü ý þ ÿ
// TODO: write these functions
bool is_lower_case_letter(char);

// f) Other special characters
// ---------------------------
// ! $ % @ ? \ ^ ` { } ~ ¡ ¢ £ ¤ ¥ ¦ § ¨ © ª « ¬ ® ¯ ° ± 2 3
// ´ (accute accent) µ ¶ • ¸ (cedille) 1 º » ¼ ½ ¾ ¿ × ÷ SHY (soft hyphen)
// TODO: write these functions
bool is_other_special_character(char);

class character_stream
{
    public:
    character_stream(const char*, const char*);
    ~character_stream() = default;

    void set_position(ptrdiff_t);
    ptrdiff_t get_position() const;
    ptrdiff_t length() const;

    char current_char() const;
    char next_char() const;
    char prev_char() const;
    char look_ahead(int) const;
    void advance(int);
    bool next();
    bool end_of_stream();

    void skip_to_eol();

    private:

    const char* current_;
    const char* start_;
    const char* end_;

};

}

#endif
