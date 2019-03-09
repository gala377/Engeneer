//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <lexer/source/string.h>
#include <visitor/log.h>
#include <parser/parser.h>
#include <visitor/llvm.h>

#include <exception/base.h>
#include <exception/concrete.h>
#include <exception/handler.h>

int main() {
    Lexer::Source::String s(R"(

    struct Test {

        foo &int32;
        bar [10]const int64;

        [10][10]uint64 tabliczka_mnozenia() {
            let tabliczka [10][10]int;
            let i int32 = 0;
            while (i < 10) {
                let j int32 = 0;
                while j < 10 {
                    tabliczka[i][j] = i * j;
                    j = j + 1;
                };
                i = i + 1;
            };
            return tabliczka;
        }
    }
)");
    Visitor::LLVM comp;

    Parser::Parser p(s);
    Parser::AST ast;
    try {
        ast = p.parse();
        p.excp_handler().throw_if_able();
    } catch(Exception::Handler::Abort& e) {
        std::cout << "Parsing error\n";
        std::cout << e.what();
    }
    Visitor::Stringify v(ast);
    ast.accept(v);
    std::cout << "Code parsed!\n\n""";
    std::cout << v.repr();
//
//    std::cout << "\nCompiling\n";
//    ast.accept(comp);
//    std::cout << "Compiled\n";

    return 0;
}
