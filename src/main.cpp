//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <lexer/source/string.h>
#include <visitor/log.h>
#include <parser/parser.h>
#include <visitor/llvm.h>

int main() {
    Lexer::Source::String s(R"(
int test(a int, b int);
)");
    Visitor::Stringify v;
    Visitor::LLVM comp;

    Parser::Parser p(s);
    auto ast = p.parse();
    ast.accept(v);
    std::cout << "Code parsed!\n\n""";
    std::cout << v.repr();

    std::cout << "\nCompiling\n";
    ast.accept(comp);
    std::cout << "Compiled\n";

    return 0;
}
