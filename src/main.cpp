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

void print_help() {
    std::string help_mess{R"(
Basic system language compiler.
Usage:
    input_file output_file_name
)"};
    std::cout << help_mess;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        print_help();
        return 1;
    }
    std::string file_name{argv[1]};
    Lexer::Source::File s(file_name);
    Parser::Parser p(s);
    Parser::AST ast;
    try {
        ast = p.parse();
        p.excp_handler().throw_if_able();
    } catch(Exception::Handler::Abort& e) {
        std::cerr << "Parsing error\n";
        std::cerr << e.what();
    }
    Visitor::LLVM::Compiler comp(ast, argv[2]);
    Visitor::Stringify v(ast);
    ast.accept(v);
    std::cerr << "Code parsed!\n\n""";
    std::cerr << v.repr();

    std::cerr << "\nCompiling\n";
    ast.accept(comp);
    std::cerr << "Compiled\n";

    return 0;
}
