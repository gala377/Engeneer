//
// Created by igor on 15.12.18.
//

#include <lexer/token.h>

Lexer::Token::Token(Id id, const std::string& symbol):
        id{id},
        symbol{symbol},
        span{{"Undefined source"}, 0, 0, 0} {}

std::string Lexer::str(Lexer::Token::Id id) {
    switch(id) {
        case Token::Id::Identifier:
            return "Identifier";
        case Token::Id::LeftParenthesis:
            return "LeftParenthesis";
        case Token::Id::RightParenthesis:
            return "RightParenthesis";
        case Token::Id::LeftBrace:
            return "LeftBrace";
        case Token::Id::RightBrace:
            return "RightBrace";
        case Token::Id::LeftSquareBracket:
            return "LeftSquareBracket";
        case Token::Id::RightSquareBracket:
            return "RightSquareBracket";
        case Token::Id::If:
            return "If";
        case Token::Id::Else:
            return "Else";
        case Token::Id::For:
            return "For";
        case Token::Id::Struct:
            return "Struct";
        case Token::Id::Let:
            return "Let";
        case Token::Id::Return:
            return "Return";
        case Token::Id::While:
            return "While";
        case Token::Id::Throw:
            return "Throw";
        case Token::Id::Break:
            return "Break";
        case Token::Id::Continue:
            return "Continue";
        case Token::Id::Assignment:
            return "Assignment";
        case Token::Id::Equality:
            return "Equality";
        case Token::Id::Inequality:
            return "Inequality";
        case Token::Id::Negation:
            return "Negation";
        case Token::Id::GreaterThan:
            return "GreaterThan";
        case Token::Id::LessThan:
            return "LessThan";
        case Token::Id::GreaterEq:
            return "GreaterEq";
        case Token::Id::LessEq:
            return "LessEq";
        case Token::Id::Plus:
            return "Plus";
        case Token::Id::Minus:
            return "Minus";
        case Token::Id::Multiplication:
            return "Multiplication";
        case Token::Id::Division:
            return "Division";
        case Token::Id::Modulo:
            return "Modulo";
        case Token::Id::Comma:
            return "Comma";
        case Token::Id::Semicolon:
            return "Semicolon";
        case Token::Id::Colon:
            return "Colon";
        case Token::Id::Dot:
            return "Dot";
        case Token::Id::DoubleMinus:
            return "DoubleMinus";
        case Token::Id::DoublePlus:
            return "DoublePlus";
        case Token::Id::Integer:
            return "Integer";
        case Token::Id::Float:
            return "Float";
        case Token::Id::String:
            return "String";
        case Token::Id::Char:
            return "Char";
        case Token::Id::NewLine:
            return "NewLine";
        case Token::Id::Space:
            return "Space";
        case Token::Id::None:
            return "None";
        case Token::Id::End:
            return "End";
        case Token::Id::Wraps:
            return "Wraps";
        default:
            return "Undefined";
    }
}
