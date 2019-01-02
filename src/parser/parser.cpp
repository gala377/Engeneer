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
        if(curr_node == nullptr) {
            break;
        }
        ast.root().add_child(std::move(curr_node));
    }
    auto curr_node = parse_end_of_file();
    if(curr_node == nullptr) {

        throw std::runtime_error("End of program expected!");
    }
    return std::move(ast);
}

std::unique_ptr<Parser::Nodes::Base> Parser::Parser::parse_top_level_decl() {
    return parser_var_decl();
}

std::unique_ptr<Parser::Nodes::VariableDecl> Parser::Parser::parser_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return {nullptr};
    }
    auto parse_res = parse_token(Lexer::Token::Id::Identifier);
    if(!parse_res) {
        throw std::runtime_error("Expected variable name after let");
    }
    Lexer::Token var = parse_res.value();

    parse_res = parse_token(Lexer::Token::Id::Identifier);
    if(!parse_res) {
        throw std::runtime_error("Expected type after variable name");
    }
    Lexer::Token type = parse_res.value();
    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        throw std::runtime_error("Missing semicolon");
    }
    return std::make_unique<Nodes::VariableDecl>(
        var.symbol,
        type.symbol);
}

std::unique_ptr<Parser::Nodes::Base> Parser::Parser::parse_end_of_file() {
    auto tok = _lexer.curr_token();
    if(tok.id != Lexer::Token::Id::End) {
        return std::unique_ptr<Nodes::Base>(nullptr);
    }
    return std::make_unique<Nodes::End>();
}

std::optional<Lexer::Token> Parser::Parser::parse_token(Lexer::Token::Id id) {
    auto tok = _lexer.curr_token();
    if(tok.id != id) {
        return {};
    }
    _lexer.next_token();
    return {tok};
}
