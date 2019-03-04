//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <lexer/source/string.h>
#include <visitor/log.h>
#include <parser/parser.h>
//#include <visitor/llvm.h>

#include <exception/base.h>
#include <exception/concrete.h>
#include <exception/handler.h>

int main() {
    Lexer::Source::String s(R"(
    struct Test wraps Foo {
        a int;
        b double;

        void foo() {
            a + b;
        }

        c Test

        void bar(a int, b int, c int) {}
    }

    let test_var int;

    Test test_func(a int, b int) {
        a+b;
    }
)");
    Visitor::Stringify v;
//    Visitor::LLVM comp;

    Parser::Parser p(s);
    Parser::AST ast;
    try {
        ast = p.parse();
        p.excp_handler().throw_if_able();
    } catch(Exception::Handler::Abort& e) {
        std::cout << "Parsing error\n";
        std::cout << e.what();
    }
    ast.accept(v);
    std::cout << "Code parsed!\n\n""";
    std::cout << v.repr();

//    std::cout << "\nCompiling\n";
//    ast.accept(comp);
//    std::cout << "Compiled\n";

    return 0;
}
