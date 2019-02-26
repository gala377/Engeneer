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
int test(a int, b int) {
    (a+b)*2;
}
)");
//    Visitor::Stringify v;
//    Visitor::LLVM comp;
//
//    Parser::Parser p(s);
//    auto ast = p.parse();
//    ast.accept(v);
//    std::cout << "Code parsed!\n\n""";
//    std::cout << v.repr();
//
//    std::cout << "\nCompiling\n";
//    ast.accept(comp);
//    std::cout << "Compiled\n";

    Lexer::Lexer l{s};
    Lexer::Token tok{Lexer::Token::Id::End, ""};


    Exception::Handler h;

    tok = l.curr_token();
    h.error(std::make_unique<Exception::ExpectedToken>(tok, l.next_token()));
    tok = l.curr_token();
    h.error(std::make_unique<Exception::ExpectedToken>(tok, l.next_token()));
    tok = l.curr_token();
    h.error(std::make_unique<Exception::ExpectedToken>(tok, l.next_token()));
    tok = l.curr_token();
    h.error(std::make_unique<Exception::ExpectedToken>(tok, l.next_token()));
    tok = l.curr_token();
    h.error(std::make_unique<Exception::ExpectedToken>(tok, l.next_token()));

    try {
        h.abort(std::make_unique<Exception::UnexpectedToken>(l.next_token()));
    } catch(Exception::Handler::Abort& e) {
        std::cout << "Aborted/n";
        std::cout << e.what();
    }
    return 0;
}
