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
        #put(array_example()[0]);
        #put(struct_example().i);
        #put_chars(&(new_B().array));
        let a A = struct_example();
        put(a.i);

        let b B = new_B();
        # i have no idea why this doesnt work? 
        let i i32 = 0;
        b.array[i] = 20;
        put(b.array[i]);
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

    struct B {
        array [3]i32;
    }

    B new_B() {
        let b B;
        let i i32 = 0;
        while i < 2 {
            b.array[i] = i;
            i = i +1;
        }
        b.array[2] = 999;
        put_chars(&b.array as &[]i32);
        return b;
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
