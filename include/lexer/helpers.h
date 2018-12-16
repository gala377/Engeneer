//
// Created by igor on 16.12.18.
//

#ifndef TKOM2_HELPERS_H
#define TKOM2_HELPERS_H

#include <set>
#include <iostream>

#include <lexer/token.h>

namespace Lexer {
    static const std::set<char> _OPERATORS = {
            '=',
            '(', ')',
            '{', '}',
            '[', ']',
            '+', '-',
            '*', '/',
            '%',
            '<', '>',
            ',',
            '!',
            ';',
    };

    bool is_new_line(const char ch);
    bool is_space(const char ch);
    bool is_blank(const char ch);
    bool is_part_of_identifier(const char ch);
    bool is_digit(const char ch);
    bool is_alpha(const char ch);
    bool is_beginning_of_the_identifier(const char ch);
    bool is_part_of_operator(const char ch);
    bool is_comment(const char ch);
    bool is_string(const char ch);
    bool is_eof(const char ch);

    std::ostream& operator<<(std::ostream& o, const Token::Id& id);
    std::string tok_to_string(const Token& tok);

}

#endif //TKOM2_HELPERS_H
