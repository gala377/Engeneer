//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <lexer/token.h>

#include <parser/parser.h>

Parser::Parser::Parser(Lexer::Source::Base &s): _lexer(s) {}

Parser::AST Parser::Parser::parse() {
    AST ast;
    while(true) {
        auto curr_node = parse_top_level_decl(); 
        std::cout << "Parsing top level decl\n";
        if(curr_node.get() == nullptr) {
            std::cout << "It's a nullptr\n";
            break;
        }
        std::cout << "Adding decl to an ast child\n";
        ast.root().add_child(std::move(curr_node));
    }
    std::cout << "Parsing end of file\n";
    auto curr_node = parse_end_of_file();
    if(curr_node.get() == nullptr) {
        std::cout << "It's a nullptr\n";
        throw std::runtime_error("End of program expected!");
    }
    std::cout << "It is not\n";
}

std::unique_ptr<Parser::Nodes::Base> Parser::Parser::parse_top_level_decl() {
    return std::unique_ptr<Nodes::Base>(nullptr);
}

std::unique_ptr<Parser::Nodes::Base> Parser::Parser::parse_end_of_file() {
    auto tok = _lexer.curr_token();
    if(tok.id != Lexer::Token::Id::End) {
        return std::unique_ptr<Nodes::Base>(nullptr);
    }
    return std::make_unique<Nodes::End>();
}