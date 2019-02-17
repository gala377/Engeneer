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
    std::unique_ptr<Nodes::Statement> res = parse_var_decl();
    if(res) {
    } else if(res = parse_expr(); res) {
    } else {
        return {nullptr};
    }
    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        throw std::runtime_error("Semicolon expected");
    }
    return res;
}

std::unique_ptr<Parser::Nodes::VariableDecl> Parser::Parser::parse_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return {nullptr};
    }
    auto identifier= parse_ident();
    if(!identifier) {
        throw std::runtime_error("Expected variable name after let");
    }
    auto type = parse_type();
    if(!type) {
        throw std::runtime_error("Expected type after variable name");
    }
    return std::make_unique<Nodes::VariableDecl>(identifier->symbol, type.value());
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


//
// Expressions
//

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_expr() {
    return parse_assig_expr();
}

std::unique_ptr<Parser::Nodes::AssignmentExpr> Parser::Parser::parse_assig_expr() {
    auto lhs = parse_add_expr();
    if(!lhs) {
        return {nullptr};
    }
    std::unique_ptr<Nodes::Expression> rhs = nullptr;
    Lexer::Token op = Lexer::Token{Lexer::Token::Id::None, ""};
    if(auto tok = parse_token(Lexer::Token::Id::Assignment); tok) {
        op = tok.value();
        rhs = parse_add_expr();
    }
    return std::make_unique<Nodes::AssignmentExpr>(
            std::move(lhs), op, std::move(rhs));
}

std::unique_ptr<Parser::Nodes::AdditiveExpr> Parser::Parser::parse_add_expr() {
    auto lhs = parse_mult_expr();
    if(!lhs) {
        return {nullptr};
    }
    std::unique_ptr<Nodes::Expression> rhs = nullptr;
    Lexer::Token op = Lexer::Token{Lexer::Token::Id::None, ""};
    if(auto tok = parse_token(Lexer::Token::Id::Plus); tok) {
        op = tok.value();
        rhs = parse_mult_expr();
    } else if (auto tok = parse_token(Lexer::Token::Id::Minus); tok) {
        op = tok.value();
        rhs = parse_mult_expr();
    }
    return std::make_unique<Nodes::AdditiveExpr>(
            std::move(lhs), op, std::move(rhs));
}

std::unique_ptr<Parser::Nodes::MultiplicativeExpr> Parser::Parser::parse_mult_expr() {
    auto lhs = parse_prim_expr();
    if(!lhs) {
        return {nullptr};
    }
    std::unique_ptr<Nodes::Expression> rhs = nullptr;
    Lexer::Token op = Lexer::Token{Lexer::Token::Id::None, ""};
    if(auto tok = parse_token(Lexer::Token::Id::Multiplication); tok) {
        op = tok.value();
        rhs = parse_prim_expr();
    } else if (auto tok = parse_token(Lexer::Token::Id::Division); tok) {
        op = tok.value();
        rhs = parse_prim_expr();
    }
    return std::make_unique<Nodes::MultiplicativeExpr>(
        std::move(lhs), op, std::move(rhs));
}

std::unique_ptr<Parser::Nodes::PrimaryExpr> Parser::Parser::parse_prim_expr() {
    return one_of<Nodes::PrimaryExpr>(
            &Parser::parse_ident,
            &Parser::parse_const,
            &Parser::parse_parenthesis);
}

std::unique_ptr<Parser::Nodes::Identifier> Parser::Parser::parse_ident() {
    auto res = parse_token(Lexer::Token::Id::Identifier);
    return res ? std::make_unique<Nodes::Identifier>(res->symbol) : nullptr;
}

std::unique_ptr<Parser::Nodes::Constant> Parser::Parser::parse_const() {
    return one_of<Nodes::Constant>(
            &Parser::parse_int,
            &Parser::parse_string);
}

std::unique_ptr<Parser::Nodes::IntConstant> Parser::Parser::parse_int() {
    auto res = parse_token(Lexer::Token::Id::Integer);
    return res ? std::make_unique<Nodes::IntConstant>(std::stoi(res->symbol)) : nullptr;
}

std::unique_ptr<Parser::Nodes::StringConstant> Parser::Parser::parse_string() {
    auto res = parse_token(Lexer::Token::Id::String);
    return res ? std::make_unique<Nodes::StringConstant>(res->symbol) : nullptr;
}

std::unique_ptr<Parser::Nodes::ParenthesisExpr> Parser::Parser::parse_parenthesis() {
    if(!parse_token(Lexer::Token::Id::LeftParenthesis)) {
        return {nullptr};
    }
    auto expr = parse_expr();
    if(!expr) {
        return {nullptr};
    }
    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        throw std::runtime_error("Right parenthesis ']' expected");
    }
    return std::make_unique<Nodes::ParenthesisExpr>(std::move(expr));
}
