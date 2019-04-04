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

    struct Foo {
        bar f64;
        bar2 [3]i32;
    }

    i32 putchar(_ i32);
    i32 put(v i32) {
        return putchar(v+65);
    }

    i32 main() {
        let f Foo;
        f.bar = 1.5;
        let i i32 = 0;
        while i < 3 {
            f.bar2[i] = i;
            put(f.bar2[i]);
            #put(i);
            i = i+1;
        }
        return 0;
    }

)");
    // todo cast makes to so int32 is casted to int32*
    // todo I need to think about and implement some kind of
    // todo an implicit pointer dereferencing and shit
    // todo and control casting and promotion more

    Parser::Parser p(s);
    Parser::AST ast;
    try {
        ast = p.parse();
        p.excp_handler().throw_if_able();
    } catch(Exception::Handler::Abort& e) {
        std::cout << "Parsing error\n";
        std::cout << e.what();
    }
    Visitor::LLVM::Compiler comp(ast);
    Visitor::Stringify v(ast);
    ast.accept(v);
    std::cout << "Code parsed!\n\n""";
    std::cout << v.repr();

    std::cout << "\nCompiling\n";
    ast.accept(comp);
    std::cout << "Compiled\n";

    return 0;
}
