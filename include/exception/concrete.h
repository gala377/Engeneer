//
// Created by rafalgal on 26.02.19.
//

#ifndef TKOM2_EXCEPTION_CONCRETE_H
#define TKOM2_EXCEPTION_CONCRETE_H

#include <exception/base.h>

namespace Exception {

    class UnexpectedToken: public BaseSyntax {
    public:
        explicit UnexpectedToken(Lexer::Token tok);
        UnexpectedToken(const Lexer::Token& tok, const std::string& details);
    };

    class ExpectedToken: public BaseSyntax {
    public:
        ExpectedToken(Lexer::Token expected, Lexer::Token actual);
        ExpectedToken(
                const Lexer::Token& expected,
                const Lexer::Token& actual,
                const std::string& details);

    };

}

#endif //TKOM2_CONCRETE_H
