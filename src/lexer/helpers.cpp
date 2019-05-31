//
// Created by igor on 16.12.18.
//

#include <iostream>
#include <string>

#include <lexer/helpers.h>
#include <lexer/token.h>


namespace Lexer {

    std::ostream& operator<<(std::ostream& o, const Token::Id& id) {
        switch (id) {
            case Token::Id::Identifier:
                o << "Identifier";
                break;
            case Token::Id::End:
                o << "End";
                break;
            case Token::Id::Space:
                o << "Space";
                break;
            case Token::Id::NewLine:
                o << "NewLine";
                break;
            default:
                o << "Token not handled";
        }
        return o;
    }

    std::string tok_to_string(const Token& tok) {
        switch (tok.id) {
            case Token::Id::Identifier:
                return "Identifier";
            case Token::Id::End:
                return "End";
            default:
                return "Token not handled";
        }
    }

    bool operator==(const Token& t1, const Token& t2) {
        return t1.id == t2.id && t1.symbol == t2.symbol;
    }

    bool is_new_line(const char ch)  {
        return ch == '\n';
    }

    bool is_space(const char ch)  {
        return isspace(ch);
    }

    bool is_blank(const char ch)  {
        return isblank(ch) | is_new_line(ch);
    }

    bool is_part_of_identifier(const char ch)  {
        return is_beginning_of_the_identifier(ch) || is_digit(ch);
    }

    bool is_digit(const char ch)  {
        return isdigit(ch);
    }

    bool is_alpha(const char ch)  {
        return isalpha(ch);
    }

    bool is_beginning_of_the_identifier(const char ch)  {
        return is_alpha(ch) || ch == '_';
    }

    bool is_part_of_operator(const char ch) {
        return (bool)_OPERATORS.count(ch);
    }

    bool is_comment(const char ch) {
        return ch == '#';
    }

    bool is_string(const char ch) {
        return ch == '"';
    }

    bool is_eof(const char ch) {
        return ch == '\0';
    }
}