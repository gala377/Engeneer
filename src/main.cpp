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

    i32 putchar(_ i32);
    i32 put(v i32) {
        return putchar(v+65);
    }

    struct A {
        i i32;

        void add() {
            i = i+1;
        }
    }

    void put_chars(a &[]i32) {
        let i i32 = 0;
        while a[i] < 999 {
            put(a[i]);
            i = i + 1; 
        }
    }

    i32 main() {
        #struct_example();
        # todo this one doesnt work actually
        #put(array_example()[0]);
        put(struct_example().i);
        put(struct_example_2().i);
        put(struct_example().i);
        put(struct_example().i);
        put(struct_example_2().i);
        #let i i32 = 0;
        #put(val i);
        return 0;
    }

    [22]i32 array_example() {
        let i i32 = 0;
        let a [22]i32;
        while i < 21 { 
            a[i] = i;
            i = i+1;
        }
        a[21] = 999;
        put_chars(&a as &[]i32);
        return a;
    }

    A struct_example() {
        let a A;
        a.i = 1;
        __A__meth__add(&a);
        __A__meth__add(&a);
        put(a.i);
        return a;        
    }

    A struct_example_2() {
        let a A;
        a.i = 7;
        return a;
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
