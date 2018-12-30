//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <lexer/source/string.h>
#include <visitor/log.h>
#include <parser/parser.h>

int main() {
    Lexer::Source::String s("");
    Visitor::Log v;

    Parser::Parser p(s);
    p.parse().visit(v);
    std::cout << v.repr();
    return 0;
}
