//
// Created by rafalgal on 26.02.19.
//

#include <exception/concrete.h>
#include <boost/format.hpp>


Exception::UnexpectedToken::UnexpectedToken(Lexer::Token tok):
    BaseSyntax{tok, (boost::format("Unexpected token: %1%(\"%2%\")") % Lexer::str(tok.id) % tok.symbol).str() }{
}

Exception::ExpectedToken::ExpectedToken(Lexer::Token expected, Lexer::Token actual):
    BaseSyntax{actual,
               (boost::format(R"(Expected token: %1%("%2%") got: %3%("%4%"))")
               % Lexer::str(expected.id)
               % expected.symbol
               % Lexer::str(actual.id)
               % actual.symbol).str()} {}