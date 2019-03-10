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

    int32 print_hello() {
        putchar(72); putchar(101); putchar(108); putchar(108); putchar(111);
        #   H           E               L            L             O
        return 0;
    }

    int32 print_space() {
        putchar(32);
        return 0;
    }

    int32 print_world() {
        putchar(87); putchar(111); putchar(114); putchar(108); putchar(100);
        #   W           O               R            L              D
        return 0;
    }

    int32 cond_hello(a int32) {
        if a > 5 {
            print_hello();
        };
        print_world();
        return 0;
    }

    int32 if_else_hello(a int32) {
        if a > 5 {
            print_hello();
        } else {
            print_world();
        };
        return 0;
    }

    #int32 if_else_if_else_hello(a int32) {
    #    if(a > 5) {
    #        if(a > 10) {
    #            print_hello();
    #        } else if a > 15 {
    #            print_world();
    #        };
    #        print_hello();
    #    };
    #    return 0;
    #}

    int32 main() {
        if_else_hello(2);
        #if_else_if_else_hello(2);
        #if_else_if_else_hello(6);
        #if_else_if_else_hello(11);
        #if_else_if_else_hello(16);
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
//
    std::cout << "\nCompiling\n";
    ast.accept(comp);
    std::cout << "Compiled\n";

    return 0;
}
