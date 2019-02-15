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
        if(!curr_node) {
            break;
        }
        ast.root().add_child(std::move(curr_node));
    }
    auto curr_node = parse_end_of_file();
    if(!curr_node) {
        throw std::runtime_error("End of program expected!");
    }
    return std::move(ast);
}

std::unique_ptr<Parser::Nodes::TopLevelDecl> Parser::Parser::parse_top_level_decl() {
    if(auto res = parse_glob_var_decl(); res != nullptr) {
        return res;
    }
    if(auto res = parse_func_decl(); res != nullptr) {
        return res;
    }
    return {nullptr};
}

std::unique_ptr<Parser::Nodes::GlobVariableDecl> Parser::Parser::parse_glob_var_decl() {
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

    return std::make_unique<Nodes::GlobVariableDecl>(var.symbol, type_symbol);
}

std::unique_ptr<Parser::Nodes::FunctionDecl> Parser::Parser::parse_func_decl() {
    auto header = parse_func_header();
    if(!header) {
        return {nullptr};
    }
    if(parse_token(Lexer::Token::Id::Semicolon)) {
        return std::move(header);
    }
    auto body = parse_code_block();
    if(!body) {
        return {nullptr};
    }
    return std::make_unique<Nodes::FunctionDef>(
        std::move(header), std::move(body));
}

std::unique_ptr<Parser::Nodes::FunctionHeader> Parser::Parser::parse_func_header() {
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

    auto arg_list = parse_func_arg_list();

    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        throw std::runtime_error(") expected");
    }

    return std::make_unique<Nodes::FunctionHeader>(
            identifier,
            type_symbol,
            std::move(arg_list));
}

Parser::Parser::arg_list_t Parser::Parser::parse_func_arg_list() {
    arg_list_t arg_list{};
    auto ident = parse_token(Lexer::Token::Id::Identifier);
    while(ident) {
        auto type = parse_type();
        if(!type) {
            throw std::runtime_error("Expected type identifier");
        }
        arg_list.emplace_back(
                std::make_unique<Nodes::GlobVariableDecl>(ident.value().symbol, type.value()));
        if(parse_token(Lexer::Token::Id::Comma)) {
            ident = parse_token(Lexer::Token::Id::Identifier);
            if(!ident) {
                throw std::runtime_error("Identifier expected");
            }
        } else {
            ident = std::nullopt;
        }
    }
    return arg_list;
}

std::unique_ptr<Parser::Nodes::CodeBlock> Parser::Parser::parse_code_block() {
    auto code_block = std::make_unique<Nodes::CodeBlock>();
    
    if(!parse_token(Lexer::Token::Id::LeftBrace)) {
        throw std::runtime_error("Left brace '{' expected");
    }

    for(auto curr_node = parse_statement(); curr_node; curr_node = parse_statement()) {
        code_block->add_child(std::move(curr_node));
    }

    if(!parse_token(Lexer::Token::Id::RightBrace)) {
        throw std::runtime_error("Right brace '}' expected");
    }
    return std::move(code_block);
}

std::unique_ptr<Parser::Nodes::Statement> Parser::Parser::parse_statement() {
    return parse_var_decl();
}

std::unique_ptr<Parser::Nodes::VariableDecl> Parser::Parser::parse_var_decl() {
    auto var_decl = parse_glob_var_decl();
    if(!var_decl) {
        return {nullptr};
    }
    return std::make_unique<Nodes::VariableDecl>(
        var_decl->identifier, var_decl->type_identifier);
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
