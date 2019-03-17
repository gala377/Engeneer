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
    int32 putchar(_ int32);

    int32 put(v int32) {
        return putchar(65 + v);
    }

    int32 add(a int32, b int32) {
        return a + b;
    }

    int32 main() {
        let a int32 = 0;
        let b int32 = a;

        put(a);
        put(b);

        if a > 5 {
            a = 1;
        } else if a >= 5 {
            a = 2;
        } else if a < 5 {
            a = 3;
        } else {
            a = 4;
        }

        put(a);

        b = add(11, b);
        if b > 10 {
            a = 5;
        }

        {
            put(a);
            return a;
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
    std::cout << "\nCompiling\n";
    ast.accept(comp);
    std::cout << "Compiled\n";

    return 0;
}
