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

    memory HeapChunk {

        base any;
        index i64;


        void init() {
            base = malloc(100);
            index = 0 as i64; 
        }

        any dynamic_alloc(size i64) {
            if index + size > 100 as i64 {
                return 0 as any;
            }
            index = index + size;
            let addr i64 = base as i64 + index; 
            return addr as any; 
        }
    }

    memory StaticHeap {
        base any;
        size i64;

        void init() {
            size = 100;
            base = malloc(size);
        }

        any static_alloc(address i64, size i64) {
            if address > this.size {
                return 0 as any;
            }
            return (base as i64 + address) as any;
        }
    }

    i32 main() {
        @HeapChunk let a i32 = 1;
        @HeapChunk let b i32 = 2;

        put(a);
        put(b);

        @StaticHeap(10) let d i32 = 1;
        @StaticHeap(20) let e i32 = 2;
        @StaticHeap(10) let f i32;

        put(d); put(e); put(f);
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
