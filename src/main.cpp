//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <lexer/source/string.h>
#include <visitor/log.h>
#include <parser/parser.h>
#include <visitor/llvm/compiler.h>

#include <exception/base.h>
#include <exception/concrete.h>
#include <exception/handler.h>

int main() {
    // todo wrapping a pointer
    // like so
    // struct A wraps &B 
    Lexer::Source::String s(R"(
    
    any malloc(_ i32);
    void free(_ any);    

    i32 putchar(_ i32);
    i32 put(v i32) {
        return putchar(v+65);
    }

    struct A {
        v i32;
        void put() {
            put(v);
        }
    }

    struct B wraps A {
        void inc() {
            A.v = A.v + 1;
        }
    }

    i32 main() {
        let b B;
        b.inc();
        b.put();
        return 0;
    }
)");
    Parser::Parser p(s);
    Parser::AST ast;
    try {
        ast = p.parse();
        p.excp_handler().throw_if_able();
    } catch(Exception::Handler::Abort& e) {
        std::cerr << "Parsing error\n";
        std::cerr << e.what();
    }
    Visitor::LLVM::Compiler comp(ast);
    Visitor::Stringify v(ast);
    ast.accept(v);
    std::cerr << "Code parsed!\n\n""";
    std::cerr << v.repr();

    std::cerr << "\nCompiling\n";
    ast.accept(comp);
    std::cerr << "Compiled\n";

    return 0;
}
