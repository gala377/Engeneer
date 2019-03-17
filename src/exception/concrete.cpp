//
// Created by rafalgal on 26.02.19.
//

#include <exception/concrete.h>
#include <boost/format.hpp>


Exception::UnexpectedToken::UnexpectedToken(Lexer::Token tok):
    BaseSyntax{tok, (boost::format("unexpected token: %1%(\"%2%\")") % Lexer::str(tok.id) % tok.symbol).str() }{
}

Exception::UnexpectedToken::UnexpectedToken(const Lexer::Token &tok, const std::string &details):
    BaseSyntax{tok, (boost::format("unexpected token: %1%(\"%2%\"). %3%")
                    % Lexer::str(tok.id)
                    % tok.symbol
                    % details).str()} {}


Exception::ExpectedToken::ExpectedToken(Lexer::Token expected, Lexer::Token actual):
    BaseSyntax{actual,
               (boost::format(R"(expected token: %1%("%2%") got: %3%("%4%"))")
               % Lexer::str(expected.id)
               % expected.symbol
               % Lexer::str(actual.id)
               % actual.symbol).str()} {}


Exception::ExpectedToken::ExpectedToken(const Lexer::Token &expected,
                                        const Lexer::Token &actual,
                                        const std::string &details):
        BaseSyntax{actual,
                   (boost::format(R"(expected token: %1%("%2%") got: %3%("%4%"). %5%)")
                    % Lexer::str(expected.id)
                    % expected.symbol
                    % Lexer::str(actual.id)
                    % actual.symbol
                    % details).str()} {}
