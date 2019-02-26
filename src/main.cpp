//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <lexer/source/string.h>
#include <visitor/log.h>
#include <parser/parser.h>
#include <visitor/llvm.h>

#include <exception/base.h>

int main() {
//    Lexer::Source::String s(R"(
//int test(a int, b int) {
//    (a+b)*2;
//}
//)");
//    Visitor::Stringify v;
//    Visitor::LLVM comp;
//
//    Parser::Parser p(s);
//    auto ast = p.parse();
//    ast.accept(v);
//    std::cout << "Code parsed!\n\n""";
//    std::cout << v.repr();
//
//    std::cout << "\nCompiling\n";
//    ast.accept(comp);
//    std::cout << "Compiled\n";
    auto excp = Exception::BaseFormatted{"%1% %2%", "Hello", "World"};
    std::cout << excp.str();
    return 0;
}
