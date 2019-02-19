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
            Let,
            Return,
            While,
            Throw,
            Brake,
            Continue,


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
            End,
        };

        Id id;
        std::string symbol;
    };

}

#endif //TKOM2_TOKEN_H
