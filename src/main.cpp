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

    memory Heap {

        any dynamic_alloc(size i64) {
            put(size);
            return malloc(size);
        }

        void free(ptr any) {
            free(ptr);
        }
    }

    i32 main() {
        @Heap let a i32 = 1;

        

        let b i32;
        let c &i8 = &b as &i32;
        
        a = a + 1;
        put(a);
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
