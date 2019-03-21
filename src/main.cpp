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

    void put(v i32) {
        putchar(65 + v);
    }

    #f32 itof(i i32) {
    #    let f f32 = i;
    #    return f;
    #}

    #i32 ftoi(f f32) {
    #    let i i32 = f;
    #    return i;
    #}

    #i32 factorial(n i32) {
    #    if n <= 1 {
    #        return 1;
    #    }
    #    return n * factorial(n-1);
    #}

    i32 main() {
        let a [4]i32;
        let i i32 = 0;
        while i < 4 {
            a[i] = 3 - i;
            i = i + 1;
        }

        i = 0;
        while i < 4 {
            put(a[i]);
            i = i + 1;
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
