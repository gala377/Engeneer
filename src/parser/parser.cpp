//
// Created by igor on 30.12.18.
//

#include <lexer/token.h>

#include <parser/parser.h>

Parser::Parser::Parser(Lexer::Source::Base &s): _lexer(s) {}

Parser::AST Parser::Parser::parse() {
    AST ast;
    while(true) {
        auto curr_node = parse_top_level_decl(); 
        if(curr_node.get() == nullptr) {
            break;
        }
        ast.root().add_child(std::move(curr_node));
    }
    auto curr_node = parse_end_of_file();
    if(curr_node.get() == nullptr) {
        throw std::runtime_error("End of program expected!");
    }
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