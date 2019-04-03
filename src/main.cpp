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
    i32 main() {
        # get some array
        let a [128]i32 = get_array();

        # have a ptr to some of its content
        let d &i32 = &a[1];

        test(d as &[]i32);
        return 0;
    }

    void test(ptr &[]i32) {
        # value of v is a[3];
        let v i32 = (val ptr)[1];
        put(v);
    }

    [128]i32 get_array() {
        let a [128]i32;
        let i i32 = 0;
        while i < 128 {
            a[i] = i;
            i = i + 1;
        }
        return a;
    }

    void put(v i32) {
        putchar(65 + v);
    }


    i32 putchar(_ i32);
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
