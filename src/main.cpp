//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <lexer/source/string.h>
#include <visitor/log.h>
#include <parser/parser.h>
#include <visitor/llvm.h>

#include <exception/base.h>
#include <exception/concrete.h>
#include <exception/handler.h>

int main() {
    Lexer::Source::String s(R"(
    int32 putchar(_ int32);

    void put(v int32) {
        putchar(65 + v);
    }

    int32 main() {
        let array &int32;
        let a byte = 0;
        let b float128 = a;
        let c int32 = b;
        put(c);
        return 0;
    }
)");
    Visitor::LLVM comp;

    Parser::Parser p(s);
    Parser::AST ast;
    try {
        ast = p.parse();
        p.excp_handler().throw_if_able();
    } catch(Exception::Handler::Abort& e) {
        std::cout << "Parsing error\n";
        std::cout << e.what();
    }
    Visitor::Stringify v(ast);
    ast.accept(v);
    std::cout << "Code parsed!\n\n""";
    std::cout << v.repr();

    std::cout << "\nCompiling\n";
    ast.accept(comp);
    std::cout << "Compiled\n";

    return 0;
}
