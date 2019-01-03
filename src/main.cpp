//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <lexer/source/string.h>
#include <lexer/source/file.h>
#include <visitor/log.h>
#include <parser/parser.h>

int main() {
    Lexer::Source::String s(R"(
let a int; let c int;
let d int;
)");
    // Lexer::Source::File s("test.txt");
    Visitor::Stringify v;
    Parser::Parser p(s);
    p.parse().accept(v);
    std::cout << v.repr();
    return 0;
}
