//
// Created by igor on 15.12.18.
//

#ifndef TKOM2_TOKEN_H
#define TKOM2_TOKEN_H

#include <string>

namespace Lexer {

    struct Token {
    public:
        enum class Id {
            // General ones
            Identifier=0,

            LeftParenthesis,
            RightParenthesis,

            LeftBrace,
            RightBrace,

            LeftSquareBracket,
            RightSquareBracket,


            // Keywords
            If=100,
            Else,
            For,
            Struct,
            Wraps,
            Let,
            Return,
            While,
            Throw,
            Break,
            Continue,
            Const,

            // Operators
            Assignment=200,

            // Logical
            Equality,
            Inequality,
            Negation,

            GreaterThan,
            LessThan,

            GreaterEq,
            LessEq,

            LeftShift,
            RightShift,
            LogicalOr,
            LogicalAnd,
            InclusiveOr,
            ExclusiveOr,
            And,


            // Arithmetic
            Plus,
            Minus,
            Multiplication,
            Division,
            Modulo,

            // Special
            Comma,
            Semicolon,
            Colon,
            Dot,

            DoubleMinus,
            DoublePlus,

            // ConstExpr
            Integer,
            Float,
            String,
            Char,

            // Other
            NewLine=300,
            Space,

            None,
            Comment,
            End,
        };

        Token(Id id, const std::string& symbol);

        struct Span {
            std::string source_name;
            std::uint32_t line;
            std::uint32_t beg_pos;
            std::uint32_t end_pos;
        };

        Id id;
        std::string symbol;
        Span span;
    };


    static Token none_tok = Token{Token::Id::None, ""};
    std::string str(Token::Id id);
}

#endif //TKOM2_TOKEN_H
