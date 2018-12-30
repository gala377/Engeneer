//
// Created by igor on 30.12.18.
//

#include <parser/parser.h>

Parser::Parser::Parser(Lexer::Source::Base &s): _lexer(s) {}

Parser::AST Parser::Parser::parse() {
    return AST();
}
