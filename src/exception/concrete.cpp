//
// Created by rafalgal on 26.02.19.
//

#include <exception/concrete.h>

Exception::UnexpectedToken::UnexpectedToken(Lexer::Token tok):
    BaseSyntax{tok, "Unexpected token: " + Lexer::str(tok.id)}{
}

Exception::ExpectedToken::ExpectedToken(Lexer::Token expected, Lexer::Token actual):
    BaseSyntax{actual, "Expected token: " + Lexer::str(expected.id) + " got: " + Lexer::str(actual.id)} {}