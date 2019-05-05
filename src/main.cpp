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

    void iprint(n i64) { 
        if n > 9 as i64 { 
            let a i64 = n / 10 as i64;
            n = n - 10 as i64 * a;
            iprint(a);
        }
        putchar(48 + n as i32);
    }

    struct A {
        v i32;
    }

    i32 main() {
        let a A;
        a.v = 1;
        put(a.v);
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
