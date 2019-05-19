//
// Created by igor on 30.09.18.
//

#include <iostream>

#include <visitor/log.h>
#include <parser/parser.h>
#include <visitor/llvm/compiler.h>
#include <lexer/source/file.h>
#include <exception/base.h>
#include <exception/concrete.h>
#include <exception/handler.h>

int main() {
    // todo wrapping a pointer
    // like so
    // struct A wraps &B 
    Lexer::Source::File s("input.esl");
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
