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

std::unique_ptr<Parser::Nodes::TopLevelDecl> Parser::Parser::parse_top_level_decl() {
    if(auto res = parse_var_decl(); res != nullptr) {
        return res;
    }
    if(auto res = parse_func_decl(); res != nullptr) {
        return res;
    }
    return {nullptr};
}

std::unique_ptr<Parser::Nodes::VariableDecl> Parser::Parser::parse_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return {nullptr};
    }
    auto identifier_res = parse_token(Lexer::Token::Id::Identifier);
    if(!identifier_res) {
        throw std::runtime_error("Expected variable name after let");
    }
    Lexer::Token var = identifier_res.value();

    auto type_res = parse_type();
    if(!identifier_res) {
        throw std::runtime_error("Expected type after variable name");
    }
    auto type_symbol = type_res.value();

    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        throw std::runtime_error("Missing semicolon");
    }

    return std::make_unique<Nodes::VariableDecl>(var.symbol, type_symbol);
}

std::unique_ptr<Parser::Nodes::FunctionDecl> Parser::Parser::parse_func_decl() {
    // todo identifier list
    // todo rewrite as combining functions
    auto type_res = parse_type();
    if(!type_res) {
        return {nullptr};
    }
    auto type_symbol = type_res.value();

    auto identifier_res = parse_token(Lexer::Token::Id::Identifier);
    if(!identifier_res) {
        throw std::runtime_error("Expected function identifier");
    }
    auto identifier = identifier_res.value().symbol;
    if(!parse_token(Lexer::Token::Id::LeftParenthesis)) {
        throw std::runtime_error("( expected");
    }
    // todo list but lets make it a functional parser of sorts
    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        throw std::runtime_error(") expected");
    }
    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        throw std::runtime_error("Missing semicolon");
    };

    return std::make_unique<Nodes::FunctionDecl>(
            identifier,
            type_symbol,
            std::vector<Nodes::VariableDecl>{});
}

std::unique_ptr<Parser::Nodes::End> Parser::Parser::parse_end_of_file() {
    auto tok = _lexer.curr_token();
    if(tok.id != Lexer::Token::Id::End) {
        return {nullptr};
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

std::optional<std::string> Parser::Parser::parse_type() {
    auto res = parse_token(Lexer::Token::Id::Identifier);
    return res ? std::optional{res.value().symbol} : std::nullopt;
}
