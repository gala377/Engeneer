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
    Lexer::Source::String s(R"(
    
    any malloc(_ i32);
    void free(_ any);    

    i32 putchar(_ i32);
    i32 put(v i32) {
        return putchar(v+65);
    }

    i32 main() {
        let a &i32 = __memory__Array.dynamic_alloc(4) as &i32;
        val a = 1;
        put(val a);
        put(__memory__Array.index);
        return 0;
    }

    memory Heap {
        any dynamic_alloc(size i32) {
            return malloc(size);
        }

        void free(ptr any) {
            free(ptr);
        }
    }

    memory Array {
        mem [100]byte;
        index i32;

        void init() {
            index = 1;
        }

        any dynamic_alloc(size i32) {
            let m any = &mem[index] as any;
            index = index + size;
            return m;
        }
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
