//
// Created by igor on 30.12.18.
//

#include "exception/base.h"
#include "parser/nodes/concrete.h"
#include "parser/type.h"
#include <algorithm>
#include <iostream>

#include <lexer/token.h>

#include <memory>
#include <parser/parser.h>
#include <exception/concrete.h>
#include <vector>

// Class Interface

Parser::Parser::Parser(Lexer::Source::Base &s): HandlingMixin(), _lexer(s)  {}

Parser::Parser::Parser(
    Lexer::Source::Base &s,
    Exception::Handler& excp_handler):
    HandlingMixin(excp_handler), _lexer(s) {}

Parser::AST Parser::Parser::parse() {
    AST ast;
    fold(
        &Parser::parse_top_level_decl,
        [&ast](auto&& node) {
            ast.root().add_child(std::move(node));
        });
    auto curr_node = parse_end_of_file();
    if(!curr_node) {
        error<Exception::BaseSyntax>(
                _lexer.curr_token(),
                "End of source expected");
    }
    for(auto& s: _structs_decls) {
        unwind_wraps_decl(*s);
        s->set_depth(s->depth());
    }
    initialize_ast(ast);
    return ast;
}

void Parser::Parser::unwind_wraps_decl(Nodes::StructDecl& node) {
    for(auto& ident: node.wrapped_structs) {
        node.members.emplace_back(std::make_unique<Nodes::VariableDecl>(
            std::make_unique<Nodes::Identifier>(ident->symbol, ident->span()),
            std::make_unique<Types::SimpleType>(
                std::make_unique<Nodes::Identifier>(ident->symbol, ident->span()))));
        const auto& wrapping = std::find_if(
            _structs_decls.begin(),
            _structs_decls.end(),
            [&ident](Nodes::StructDecl* item) -> bool {
                return item->identifier->symbol == ident->symbol;
            });
        if(wrapping == _structs_decls.end()) {
            abort<Exception::BaseSyntax>(
                _lexer.curr_token(),
                std::string{"Undefined type in wraps decl "} + ident->symbol);
        }
        generate_wrapped_methods(node, **wrapping);
    }
}

void Parser::Parser::generate_wrapped_methods(Nodes::StructDecl& node, const Nodes::StructDecl& wrapping) {
    for(auto& meth: wrapping.methods) {
        auto it = std::find_if(
            node.methods.begin(),
            node.methods.end(), 
            [&meth](auto& item) -> bool {
                return meth->ident().symbol == item->ident().symbol;
            }
        );
        if(it != node.methods.end()) {
            continue;
        }
        auto func_prot = copy_func_prot(*meth);
        auto body = gen_delegates_body(wrapping, *func_prot); 
        node.methods.emplace_back(
             std::make_unique<Nodes::FunctionDef>(
                std::move(func_prot),
                std::move(body)));
    }   
}

std::unique_ptr<Parser::Nodes::FunctionProt> Parser::Parser::copy_func_prot(const Nodes::FunctionDecl& func) {
    const Nodes::FunctionProt* o_prot = nullptr;
    if(o_prot = dynamic_cast<const Nodes::FunctionProt*>(&func); !o_prot) {
        o_prot = dynamic_cast<const Nodes::FunctionDef*>(&func)->declaration.get();
    }
    auto ident = std::make_unique<Nodes::Identifier>(
        o_prot->identifier->symbol, o_prot->identifier->span());
    auto ret = o_prot->type->copy();
    std::vector<std::unique_ptr<Nodes::VariableDecl>> args;
    for(auto& arg: o_prot->arg_list) {
        args.emplace_back(
            std::make_unique<Nodes::VariableDecl>(
                std::make_unique<Nodes::Identifier>(arg->identifier->symbol, arg->identifier->span()),
                arg->type->copy()));
    }
    return std::make_unique<Nodes::FunctionProt>(
        std::move(ident),
        std::move(ret),
        std::move(args));
}

std::unique_ptr<Parser::Nodes::CodeBlock> Parser::Parser::gen_delegates_body(
        const Nodes::StructDecl& wrapping,
        const Nodes::FunctionProt& func) {
    auto code_block = std::make_unique<Nodes::CodeBlock>();
    std::vector<std::unique_ptr<Nodes::Expression>> args;
    for(auto& arg: func.arg_list) {
        args.emplace_back(
            std::make_unique<Nodes::Identifier>(arg->identifier->symbol, arg->identifier->span()));
    }

    auto ret = std::make_unique<Nodes::ReturnStmt>(
        std::make_unique<Nodes::CallExpr>(
            std::make_unique<Nodes::AccessExpr>(
                std::make_unique<Nodes::Identifier>(wrapping.identifier->symbol, wrapping.identifier->span()),
                std::make_unique<Nodes::Identifier>(func.identifier->symbol, wrapping.identifier->span())),
            std::move(args))
    );
    code_block->add_child(std::move(ret));
    return code_block;  
}   

void Parser::Parser::initialize_ast(AST &ast) const {
    for(auto& p: _function_protos) {
        ast.note(p);
    }
    for(auto& f: _function_defs) {
        ast.note(f);
    }
    for(auto& g: _glob_var_decls) {
        ast.note(g);
    }
    for(auto& s: _structs_decls) {
        ast.note(s);
    }
    for(auto& m: _memory_decls) {
        ast.note(m);
    }
}

// Parsing

// End
std::unique_ptr<Parser::Nodes::End> Parser::Parser::parse_end_of_file() {
    auto tok = _lexer.curr_token();
    if(tok.id != Lexer::Token::Id::End) {
        return nullptr;
    }
    return std::make_unique<Nodes::End>();
}


// Top Level
std::unique_ptr<Parser::Nodes::TopLevelDecl> Parser::Parser::parse_top_level_decl() {
    return one_of<Nodes::TopLevelDecl>(
        &Parser::parse_glob_var_decl,
        &Parser::parse_func_decl,
        &Parser::parse_struct_decl,
        &Parser::parse_memory_decl);
}

std::unique_ptr<Parser::Nodes::GlobVariableDecl> Parser::Parser::parse_glob_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return nullptr;
    }
    auto identifier= parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Identifier, ""),
                _lexer.curr_token(),
                "Expected variable name after let");
    }
    auto type = parse_type();
    if(!type) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Identifier, ""),
                _lexer.curr_token(),
                "Expected type after variable name");
    }

    std::unique_ptr<Nodes::Expression> init_expr{nullptr};
    if(parse_token(Lexer::Token::Id::Assignment)) {
        init_expr = parse_expr();
        if(!init_expr) {
            abort<Exception::BaseSyntax>(
                _lexer.curr_token(),
                    "Expression expected after variable initialization");
        }
    }

    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Semicolon, ";"),
            _lexer.curr_token(),
            "Missing semicolon");
    }

    auto res = std::make_unique<Nodes::GlobVariableDecl>(
        std::move(identifier),
        std::move(type),
        std::move(init_expr));
    _glob_var_decls.push_back(res.get());
    return res;
}

std::unique_ptr<Parser::Nodes::MemoryDecl> Parser::Parser::parse_memory_decl() {
    if(!parse_token(Lexer::Token::Id::Memory)) {
        return nullptr;
    }

    auto identifier = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Identifier, ""),
            _lexer.curr_token(),
            "Memory name expected");
    }

    auto wrapped_structs = parse_wraps_decl();
    auto [members, methods] = parse_struct_body();

    auto res = std::make_unique<Nodes::MemoryDecl>(
        std::move(identifier),
        std::move(members),
        std::move(methods),
        std::move(wrapped_structs));
    _memory_decls.push_back(res.get());
    return res;
}

std::unique_ptr<Parser::Nodes::StructDecl> Parser::Parser::parse_struct_decl() {
    if(!parse_token(Lexer::Token::Id::Struct)) {
        return nullptr;
    }

    auto identifier = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Identifier, ""),
            _lexer.curr_token(),
            "Struct name expected");
    }

    auto wrapped_structs = parse_wraps_decl();
    auto [members, methods] = parse_struct_body();

    auto res = std::make_unique<Nodes::StructDecl>(
        std::move(identifier),
        std::move(members),
        std::move(methods),
        std::move(wrapped_structs));
    _structs_decls.push_back(res.get());
    return res;
}

Parser::Parser::unique_vec<Parser::Nodes::Identifier> Parser::Parser::parse_wraps_decl() {
    unique_vec<Nodes::Identifier> wrapped_structs;
    if(!parse_token(Lexer::Token::Id::Wraps)) {
        return wrapped_structs;
    }
    auto ident = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!ident) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "List of identifiers expected after wraps keyword");
    }
    wrapped_structs.emplace_back(std::move(ident));
    fold(
        make_tok_parser(Lexer::Token::Id::Comma),
        [this, &wrapped_structs](auto&& res) {
            auto id = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
            if(!id) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Identifier expected");
            }
            wrapped_structs.emplace_back(std::move(id));
        });
    return wrapped_structs;
}

Parser::Parser::struct_body_parse_res_t Parser::Parser::parse_struct_body() {
    unique_vec<Nodes::VariableDecl> members;
    unique_vec<Nodes::FunctionDecl> methods;
    if(!parse_token(Lexer::Token::Id::LeftBrace)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::LeftBrace, "{"),
            _lexer.curr_token(),
            "Struct or memory body is expected to be a code block");
    }

    while(true) {
        auto first_id = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
        if(!first_id) {
            auto func = parse_func_decl();
            if(!func) {
                break;
            }
            methods.emplace_back(std::move(func));
            continue;
        }
        auto second_id = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
        if(!second_id) {
            auto type = parse_type();
            if(!type) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Couldn't parse member type. "
                    "Declarations in struct or memory body need to be functions or variables");
            }
            if(!parse_token(Lexer::Token::Id::Semicolon)) {
                error<Exception::ExpectedToken>(
                    _lexer.make_token(Lexer::Token::Id::Semicolon, ";"),
                    _lexer.curr_token(),
                    "Missing semicolon after member declarations");
            }
            members.emplace_back(std::make_unique<Nodes::VariableDecl>(
                std::move(first_id),
                std::move(type)));
            continue;
        }
        if(auto args = parse_func_arg_list(); args) {
            // function decl
            auto header = std::make_unique<Nodes::FunctionProt>(
                std::move(second_id),
                std::make_unique<Types::SimpleType>(std::move(first_id)),
                std::move(args.value()));
            auto body = parse_code_block();
            if(!body) {
                if(!parse_token(Lexer::Token::Id::Semicolon)) {
                    error<Exception::ExpectedToken>(
                        _lexer.make_token(Lexer::Token::Id::Semicolon, ";"),
                        _lexer.curr_token(),
                        "Missing semicolon after method prototype");
                }
                methods.emplace_back(std::move(header));
                continue;
            }
            methods.emplace_back(
                std::make_unique<Nodes::FunctionDef>(
                    std::move(header),
                    std::move(body)));
            continue;

        }
        // variable decl
        if(!parse_token(Lexer::Token::Id::Semicolon)) {
            error<Exception::ExpectedToken>(
                _lexer.make_token(Lexer::Token::Id::Semicolon, ";"),
                _lexer.curr_token(),
                "Missing semicolon");
        }
        members.emplace_back(std::make_unique<Nodes::VariableDecl>(
            std::move(first_id),
            std::make_unique<Types::SimpleType>(std::move(second_id))));
    }

    if(!parse_token(Lexer::Token::Id::RightBrace)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::RightBrace, "}"),
            _lexer.curr_token(),
            "Struct or memory body not closed");
    }
    return std::make_tuple(std::move(members), std::move(methods));
}


// Function
std::unique_ptr<Parser::Nodes::FunctionDecl> Parser::Parser::parse_func_decl() {
    auto header = parse_func_header();
    if(!header) {
        return nullptr;
    }
    if(parse_token(Lexer::Token::Id::Semicolon)) {
        _function_protos.push_back(header.get());
        return std::move(header);
    }
    auto body = parse_code_block();
    if(!body) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Semicolon, ";"),
            _lexer.curr_token(),
            "Missing semicolon after function prototype");
        return std::move(header);
    }
    auto res = std::make_unique<Nodes::FunctionDef>(
        std::move(header), std::move(body));
    _function_defs.push_back(res.get());
    return std::move(res);
}

std::unique_ptr<Parser::Nodes::FunctionProt> Parser::Parser::parse_func_header() {
    auto type = parse_type();
    if(!type) {
        return nullptr;
    }
    auto identifier = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Identifier, ""),
            _lexer.curr_token(),
            "Function name expected");
    }
    auto arg_list = parse_func_arg_list();
    if(!arg_list) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::LeftParenthesis, "("),
            _lexer.curr_token());
    }
    return std::make_unique<Nodes::FunctionProt>(
        std::move(identifier),
        std::move(type),
        std::move(arg_list.value()));
}


// Function Helpers
std::optional<Parser::Parser::arg_list_t> Parser::Parser::parse_func_arg_list() {
    if(!parse_token(Lexer::Token::Id::LeftParenthesis)) {
        return std::nullopt;
    }
    arg_list_t arg_list{};
    auto ident = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    while(ident) {
        auto type = parse_type();
        if(!type) {
            abort<Exception::ExpectedToken>(
                _lexer.make_token(Lexer::Token::Id::Identifier, ""),
                _lexer.curr_token(),
                "Expected type after variable name");
        }
        arg_list.emplace_back(
                std::make_unique<Nodes::VariableDecl>(std::move(ident), std::move(type)));
        if(parse_token(Lexer::Token::Id::Comma)) {
            ident = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
            if(!ident) {
                abort<Exception::ExpectedToken>(
                    _lexer.make_token(Lexer::Token::Id::Identifier, ""),
                    _lexer.curr_token());
            }
        }
    }
    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::RightParenthesis, ")"),
            _lexer.curr_token());
    }
    return std::optional(std::move(arg_list));
}


// Statement
std::unique_ptr<Parser::Nodes::Statement> Parser::Parser::parse_statement() {
    return one_of<Nodes::Statement>(
        &Parser::parse_semicolon_terminated_stmt,
        &Parser::parse_custom_terminated_stmt);
}

std::unique_ptr<Parser::Nodes::Statement> Parser::Parser::parse_semicolon_terminated_stmt() {
    auto res = one_of<Nodes::Statement>(
        &Parser::parse_var_decl,
        &Parser::parse_at_stmt,
        &Parser::parse_expr,
        &Parser::parse_break_stmt,
        &Parser::parse_continue_stmt);
    if(!res) {
        return nullptr;
    }
    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Semicolon, ";"),
            _lexer.curr_token(),
            "Missing semicolon");
    }
    return res;
}

std::unique_ptr<Parser::Nodes::Statement> Parser::Parser::parse_custom_terminated_stmt() {
    return one_of<Nodes::Statement>(
        &Parser::parse_if_stmt,
        &Parser::parse_while_stmt,
        &Parser::parse_return_stmt,
        &Parser::parse_block_stmt);
}

std::unique_ptr<Parser::Nodes::CodeBlock> Parser::Parser::parse_code_block() {
    if(!parse_token(Lexer::Token::Id::LeftBrace)) {
        return nullptr;
    }
    auto code_block = std::make_unique<Nodes::CodeBlock>();
    fold(
        &Parser::parse_statement,
        [&code_block](auto&& res) {
            code_block->add_child(std::move(res));
        });
    if(!parse_token(Lexer::Token::Id::RightBrace)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::RightBrace, "}"),
            _lexer.curr_token(),
            "Code block left open");
    }
    return code_block;
}

std::unique_ptr<Parser::Nodes::VariableDecl> Parser::Parser::parse_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return nullptr;
    }
    auto identifier= parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Identifier, ""),
            _lexer.curr_token(),
            "Expected variable name after let");
    }
    auto type = parse_type();
    if(!type) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Identifier, ""),
            _lexer.curr_token(),
            "Expected type after variable name");
    }
    std::unique_ptr<Nodes::Expression> init_expr{nullptr};
    if(parse_token(Lexer::Token::Id::Assignment)) {
        init_expr = parse_expr();
        if(!init_expr) {
            abort<Exception::BaseSyntax>(
                _lexer.curr_token(),
                "Expression expected after variable initialization");
        }
    }
    return std::make_unique<Nodes::VariableDecl>(
        std::move(identifier),
        std::move(type),
        std::move(init_expr));
}

std::unique_ptr<Parser::Nodes::AtStmt> Parser::Parser::parse_at_stmt() {
    if(!parse_token(Lexer::Token::Id::At)) {
        return nullptr;
    }
    auto identifier = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::At, "@"),
            _lexer.curr_token(),
            "Expected identifier after @ operator");
    }
    std::unique_ptr<Nodes::Expression> address;
    if(parse_token(Lexer::Token::Id::LeftParenthesis)) {
        address = parse_expr();
        if(!address) {
            abort<Exception::BaseSyntax>(
                _lexer.curr_token(),
                "Expression expected in static allocation");
        }
        if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
            error<Exception::ExpectedToken>(
                _lexer.make_token(Lexer::Token::Id::RightParenthesis, ")"),
                _lexer.curr_token());
        }
    }
    auto var_decl = parse_var_decl();
    if(!var_decl) {
        abort<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Let, "let"),
            _lexer.curr_token(),
            "Variable declaration expected after at statement");
    }
    return std::make_unique<Nodes::AtStmt>(
        std::move(identifier),
        std::move(var_decl),
        std::move(address));
}

std::unique_ptr<Parser::Nodes::BlockStmt> Parser::Parser::parse_block_stmt() {
    auto code_block = parse_code_block();
    if(!code_block) {
        return nullptr;
    }
    return std::make_unique<Nodes::BlockStmt>(std::move(code_block));
}

std::unique_ptr<Parser::Nodes::IfStmt> Parser::Parser::parse_if_stmt() {
    if(!parse_token(Lexer::Token::Id::If)) {
        return nullptr;
    }
    auto cond = parse_expr();
    if(!cond) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Expression expected after if keyword");
    }
    auto body = parse_code_block();
    if(!body) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "If statement body missing");
    }
    std::unique_ptr<Nodes::BlockStmt> else_clause;
    if(parse_token(Lexer::Token::Id::Else)) {
        else_clause = one_of<Nodes::BlockStmt>(
            &Parser::parse_block_stmt,
            &Parser::parse_if_stmt);
    }
    return std::make_unique<Nodes::IfStmt>(
        std::move(cond),
        std::move(body),
        std::move(else_clause));
}

std::unique_ptr<Parser::Nodes::WhileStmt> Parser::Parser::parse_while_stmt() {
    if(!parse_token(Lexer::Token::Id::While)) {
        return nullptr;
    }
    auto cond = parse_expr();
    if(!cond) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Expression expected after while keyword");
    }
    auto body = parse_code_block();
    if(!body) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "While statement body is missing");
    }
    return std::make_unique<Nodes::WhileStmt>(
        std::move(cond),
        std::move(body));
}

std::unique_ptr<Parser::Nodes::ReturnStmt> Parser::Parser::parse_return_stmt() {
    if(!parse_token(Lexer::Token::Id::Return)) {
        return nullptr;
    }
    auto expr = parse_expr();
    if(!expr) {
        if(parse_token(Lexer::Token::Id::Semicolon)) {
            return std::make_unique<Nodes::ReturnStmt>(nullptr);
        }
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Expression expected after return statement");
    }
    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Semicolon, ";"),
            _lexer.curr_token(),
            "Missing semicolon after return statement");
    }
    return std::make_unique<Nodes::ReturnStmt>(
        std::move(expr));
}

std::unique_ptr<Parser::Nodes::BreakStmt> Parser::Parser::parse_break_stmt() {
    if(!parse_token(Lexer::Token::Id::Break)) {
        return nullptr;
    }
    return std::make_unique<Nodes::BreakStmt>();
}

std::unique_ptr<Parser::Nodes::ContinueStmt> Parser::Parser::parse_continue_stmt() {
    if(!parse_token(Lexer::Token::Id::Continue)) {
        return nullptr;
    }
    return std::make_unique<Nodes::ContinueStmt>();
}

// Expression
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_expr() {
    return parse_assig_expr();
}


// Binary
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_assig_expr() {
    auto lhs = parse_logical_or_expr();
    if(!lhs) {
        return nullptr;
    }
    auto tok = parse_token(Lexer::Token::Id::Assignment);
    if(!tok) {
        return lhs;
    }
    auto op = tok.value();
    auto rhs = parse_logical_or_expr();
    return std::make_unique<Nodes::AssignmentExpr>(
            std::move(lhs), op, std::move(rhs));
}


// Logical
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_logical_or_expr() {
    auto logical_or_expr = parse_logical_and_expr();
    fold(
        make_tok_parser(Lexer::Token::Id::LogicalOr),
        [this, &logical_or_expr](auto &&op) {
            auto res = parse_logical_and_expr();
            if (!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after logical or operator");
            }
            logical_or_expr = std::make_unique<Nodes::LogicalOrExpr>(
                std::move(logical_or_expr), op.value(), std::move(res));
        });
    return logical_or_expr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_logical_and_expr() {
    auto logical_and_expr = parse_inclusive_or_expr();
    fold(
        make_tok_parser(Lexer::Token::Id::LogicalAnd),
        [this, &logical_and_expr](auto &&op) {
            auto res = parse_inclusive_or_expr();
            if (!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after logical and operator");
            }
            logical_and_expr = std::make_unique<Nodes::LogicalAndExpr>(
                std::move(logical_and_expr), op.value(), std::move(res));
        });
    return logical_and_expr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_inclusive_or_expr() {
    auto inclusive_or_expr = parse_exclusive_or_expr();
    fold(
        make_tok_parser(Lexer::Token::Id::InclusiveOr),
        [this, &inclusive_or_expr](auto &&op) {
            auto res = parse_exclusive_or_expr();
            if (!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after inclusive or operator");
            }
            inclusive_or_expr = std::make_unique<Nodes::InclusiveOrExpr>(
                std::move(inclusive_or_expr), op.value(), std::move(res));
        });
    return inclusive_or_expr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_exclusive_or_expr() {
    auto exclusive_or_expr = parse_and_expr();
    fold(
        make_tok_parser(Lexer::Token::Id::ExclusiveOr),
        [this, &exclusive_or_expr](auto &&op) {
            auto res = parse_and_expr();
            if (!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after exclusive or operator");
            }
            exclusive_or_expr = std::make_unique<Nodes::ExclusiveOrExpr>(
                std::move(exclusive_or_expr), op.value(), std::move(res));
        });
    return exclusive_or_expr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_and_expr() {
    auto and_expr = parse_equality_expr();
    fold(
        make_tok_parser(Lexer::Token::Id::And),
        [this, &and_expr](auto &&op) {
            auto res = parse_equality_expr();
            if (!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after and operator");
            }
            and_expr = std::make_unique<Nodes::AndExpr>(
                std::move(and_expr), op.value(), std::move(res));
        });
    return and_expr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_equality_expr() {
    auto equality_expr = parse_relational_expr();
    fold(
        [](Parser *p) {
            return p->one_of_op<Lexer::Token>(
                p->make_tok_parser(Lexer::Token::Id::Equality),
                p->make_tok_parser(Lexer::Token::Id::Inequality));
        },
        [this, &equality_expr](auto &&op) {
            auto res = parse_relational_expr();
            if (!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after equality operator");
            }
            equality_expr = std::make_unique<Nodes::EqualityExpr>(
                std::move(equality_expr), op.value(), std::move(res));
        });
    return equality_expr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_relational_expr() {
    auto rel_expr = parse_shift_expr();
    fold(
        &Parser::parse_relational_op,
        [this, &rel_expr] (auto&& op) {
            auto res = parse_shift_expr();
            if(!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after relation operator");
            }
            rel_expr = std::make_unique<Nodes::RelationalExpr>(
                    std::move(rel_expr), op.value(), std::move(res));
        });
    return rel_expr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_shift_expr() {
    auto shift_expr = parse_add_expr();
    fold(
        [](Parser *p) {
            return p->one_of_op<Lexer::Token>(
                p->make_tok_parser(Lexer::Token::Id::LeftShift),
                p->make_tok_parser(Lexer::Token::Id::RightShift));
        },
        [this, &shift_expr](auto &&op) {
            auto res = parse_add_expr();
            if (!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after shift operator");
            }
            shift_expr = std::make_unique<Nodes::ShiftExpr>(
                    std::move(shift_expr), op.value(), std::move(res));
        });
    return shift_expr;
}


// Arithmetic
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_add_expr() {
    auto add_expr = parse_mult_expr();
    fold(
        [](Parser* p) {
            return p->one_of_op<Lexer::Token>(
                p->make_tok_parser(Lexer::Token::Id::Plus),
                p->make_tok_parser(Lexer::Token::Id::Minus));
            },
        [this, &add_expr] (auto&& op) {
            auto res = parse_mult_expr();
            if(!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after addition/subtraction operator");
            }
            add_expr = std::make_unique<Nodes::AdditiveExpr>(
                std::move(add_expr), op.value(), std::move(res));
        });
    return add_expr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_mult_expr() {
    auto mult_expr = parse_cast_expr();
    fold(
        [](Parser* p) {
            return p->one_of_op<Lexer::Token>(
                p->make_tok_parser(Lexer::Token::Id::Multiplication),
                p->make_tok_parser(Lexer::Token::Id::Division));
        },
        [this, &mult_expr] (auto&& op) {
            auto res = parse_cast_expr();
            if(!res) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Expression expected after multiplication/division operator");
            }
            mult_expr = std::make_unique<Nodes::MultiplicativeExpr>(
                std::move(mult_expr), op.value(), std::move(res));
        });
    return mult_expr;
}

// Cast
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_cast_expr() {
    auto lhs = parse_unary_expr();
    if(!parse_token(Lexer::Token::Id::As)) {
        return lhs;
    }
    auto type = parse_type();
    if(!type) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Type expected after 'as' keyword");
    }
    return std::make_unique<Nodes::CastExpr>(std::move(lhs), std::move(type));
}

// Unary
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_unary_expr() {
    return one_of<Nodes::Expression>(
        &Parser::parse_negative_expr,
        &Parser::parse_negation_expr,
        &Parser::parse_postfix_expr,
        &Parser::parse_address_access_expr,
        &Parser::parse_dereference_expr);
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_negative_expr() {
    auto tok = parse_token(Lexer::Token::Id::Minus);
    if(!tok) {
        return nullptr;
    }
    // note: double unary only after parenthesis
    auto rhs = parse_postfix_expr();
    if(!rhs) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Primary expression expected after unary - operator "
            "(use parenthesis for multiple unary operators)");
    }
    return std::make_unique<Nodes::NegativeExpr>(
            tok.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_negation_expr() {
    auto tok = parse_token(Lexer::Token::Id::Negation);
    if(!tok) {
        return nullptr;
    }
    auto rhs = parse_postfix_expr();
    if(!rhs) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Primary expression expected after unary ! operator "
            "(use parenthesis for multiple unary operators)");
    }
    return std::make_unique<Nodes::NegationExpr>(tok.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_address_access_expr() {
    auto tok = parse_token(Lexer::Token::Id::And);
    if(!tok) {
        return nullptr;
    }
    auto rhs = parse_postfix_expr();
    if(!rhs) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Primary expression expected after unary & operator "
            "(use parenthesis for multiple unary operators)");
    }
    return std::make_unique<Nodes::AddressAccessExpr>(tok.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_dereference_expr() {
    auto tok = parse_token(Lexer::Token::Id::Val);
    if(!tok) {
        return nullptr;
    }
    auto rhs = parse_postfix_expr();
    if(!rhs) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Primary expression expected after unary value operator "
            "(use parenthesis for multiple unary operators)");
    }
    return std::make_unique<Nodes::DereferenceExpr>(tok.value(), std::move(rhs));
}

// Postfix
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_postfix_expr() {
    auto lhs = parse_prim_expr();
    if(!lhs) {
        return nullptr;
    }
    fold(
        [&lhs] (Parser* p) -> std::unique_ptr<Nodes::Expression> {
            return p->one_of<Nodes::Expression>(
                [&lhs](Parser *p) {
                    return p->parse_call_expr(lhs);
                },
                [&lhs](Parser *p) {
                    return p->parse_index_expr(lhs);
                },
                [&lhs](Parser *p) {
                    return p->parse_access_expr(lhs);
                });
        },
        [&lhs](auto&& res) {
            lhs = std::move(res);
        });
    return lhs;
}


std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_call_expr(
    std::unique_ptr<Nodes::Expression> &lhs) {

    auto res = parse_call_parameters();
    if(!res) {
        return nullptr;
    }
    return std::make_unique<Nodes::CallExpr>(
        std::move(lhs),
        std::move(res.value()));
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_index_expr(
    std::unique_ptr<Nodes::Expression> &lhs) {

    auto idx = parse_index_parameters();
    if(!idx) {
        return nullptr;
    }
    return std::make_unique<Nodes::IndexExpr>(
        std::move(lhs),
        std::move(idx));
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_access_expr(
    std::unique_ptr<Nodes::Expression> &lhs) {

    auto rhs = parse_access_parameters();
    if(!rhs) {
        return nullptr;
    }
    return std::make_unique<Nodes::AccessExpr>(
        std::move(lhs),
        std::move(rhs));
}


std::optional<Parser::Parser::call_args_t> Parser::Parser::parse_call_parameters() {
    if(!parse_token(Lexer::Token::Id::LeftParenthesis)) {
        return std::nullopt;
    }
    call_args_t args;
    auto expr = parse_expr();
    if(expr) {
        args.emplace_back(std::move(expr));
        fold(
            make_tok_parser(Lexer::Token::Id::Comma),
            [this, &args](auto&& _) {
                auto expr = parse_expr();
                if(!expr) {
                    abort<Exception::BaseSyntax>(
                        _lexer.curr_token(),
                        "Call parameter is expected to be an expression");
                }
                args.emplace_back(std::move(expr));
            });
    }
    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::RightParenthesis, ")"),
            _lexer.curr_token(),
            "Call argument list unclosed");
    }
    return std::optional{std::move(args)};
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_index_parameters() {
    if(!parse_token(Lexer::Token::Id::LeftSquareBracket)) {
        return nullptr;
    }
    auto lhs = parse_expr();
    if(!lhs) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Indexing can only be done with expressions");
    }
    if(!parse_token(Lexer::Token::Id::RightSquareBracket)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::RightSquareBracket, "]"),
            _lexer.curr_token(),
            "Indexing not closed");
    }
    return lhs;
}

std::unique_ptr<Parser::Nodes::Identifier> Parser::Parser::parse_access_parameters() {
    if(!parse_token(Lexer::Token::Id::Dot)) {
        return nullptr;
    }
    auto ident = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!ident) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Access can only be done with identifiers");
    }
    auto casted = std::unique_ptr<Nodes::Identifier>(
        dynamic_cast<Nodes::Identifier*>(ident.release()));
    return casted;
}

// Primary
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_prim_expr() {
    return one_of<Nodes::Expression>(
        &Parser::parse_ident,
        &Parser::parse_const,
        &Parser::parse_parenthesis);
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_ident() {
    auto res = parse_token(Lexer::Token::Id::Identifier);
    return res ? std::make_unique<Nodes::Identifier>(*res) : nullptr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_parenthesis() {
    if(!parse_token(Lexer::Token::Id::LeftParenthesis)) {
        return nullptr;
    }
    auto expr = parse_expr();
    if(!expr) {
        return nullptr;
    }
    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::RightParenthesis, ")"),
            _lexer.curr_token());
    }
    return std::make_unique<Nodes::ParenthesisExpr>(std::move(expr));
}


// Consts
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_const() {
    return one_of<Nodes::Expression>(
        &Parser::parse_int,
        &Parser::parse_string,
        &Parser::parse_float);
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_int() {
    auto res = parse_token(Lexer::Token::Id::Integer);
    return res ? std::make_unique<Nodes::IntConstant>(std::stoi(res->symbol), res->span) : nullptr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_string() {

    auto res = parse_token(Lexer::Token::Id::String);
    return res ? std::make_unique<Nodes::StringConstant>(res->symbol, res->span) : nullptr;
}

std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_float() {
    auto res = parse_token(Lexer::Token::Id::Float);
    return res ? std::make_unique<Nodes::FloatConstant>(std::stod(res->symbol), res->span) : nullptr;
}

// Token Parsers
std::optional<Lexer::Token> Parser::Parser::parse_token(Lexer::Token::Id id) {
    auto tok = _lexer.curr_token();
    if(tok.id != id) {
        return {};
    }
    _lexer.next_token();
    return {tok};
}

std::optional<Lexer::Token> Parser::Parser::parse_relational_op() {
    return one_of_op<Lexer::Token>(
        make_tok_parser(Lexer::Token::Id::GreaterThan),
        make_tok_parser(Lexer::Token::Id::GreaterEq),
        make_tok_parser(Lexer::Token::Id::LessThan),
        make_tok_parser(Lexer::Token::Id::LessEq));
}

std::function<std::optional<Lexer::Token>(Parser::Parser*)> Parser::Parser::make_tok_parser(Lexer::Token::Id id) {
    return [id](Parser* p) {
        return p->parse_token(id);
    };
}



    // Type parsers
std::unique_ptr<Parser::Types::BaseType> Parser::Parser::parse_type() {
    return one_of<Types::BaseType>(
        &Parser::parse_func_type,
        &Parser::parse_complex_type,
        &Parser::parse_array_type,
        &Parser::parse_simple_type);
}

std::unique_ptr<Parser::Types::ComplexType> Parser::Parser::parse_complex_type() {
    auto complex = std::make_unique<Types::ComplexType>();
    if(parse_token(Lexer::Token::Id::Const)) {
        complex->is_const = true;
    }
    if(parse_token(Lexer::Token::Id::And)) {
        complex->is_ptr = true;
    }
    if(complex->is_ptr || complex->is_const) {
        auto underlying_type = parse_type();
        if(!underlying_type) {
            abort<Exception::BaseSyntax>(
                _lexer.curr_token(),
                "No type to apply identifiers to");
        }
        complex->underlying_type = std::move(underlying_type);
        return complex;
    }
    return nullptr;
}

std::unique_ptr<Parser::Types::ArrayType> Parser::Parser::parse_array_type() {
    std::optional<Lexer::Token> tok;
    if(tok = parse_token(Lexer::Token::Id::LeftSquareBracket); !tok) {
        return nullptr;
    }
    auto size = std::make_unique<Nodes::IntConstant>(0, tok->span);
    if(!parse_token(Lexer::Token::Id::RightSquareBracket)) {
        size = parse_cast<Nodes::IntConstant>(&Parser::parse_int);
        if(!size) {
            abort<Exception::BaseSyntax>(
                _lexer.curr_token(),
                "Array size needs to be a positive integer");
        }
        if(!parse_token(Lexer::Token::Id::RightSquareBracket)) {
            error<Exception::ExpectedToken>(
                _lexer.make_token(Lexer::Token::Id::RightSquareBracket, "]"),
                _lexer.curr_token());
        }
    }
    auto underlying_type = parse_type();
    if(!underlying_type) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "No type after array declaration");
    }
    return std::make_unique<Types::ArrayType>(
        (unsigned int)size->value,
        std::move(underlying_type));
}

std::unique_ptr<Parser::Types::SimpleType> Parser::Parser::parse_simple_type() {
    auto res = parse_cast<Nodes::Identifier>(&Parser::parse_ident);
    if(!res) {
        return nullptr;
    }
    return std::make_unique<Types::SimpleType>(std::move(res));
}

std::unique_ptr<Parser::Types::FunctionType> Parser::Parser::parse_func_type() {
    if(!parse_token(Lexer::Token::Id::Fn)) {
        return nullptr;
    }
    auto args = parse_func_type_args();
    if(!parse_token(Lexer::Token::Id::Arrow)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::Arrow, "->"),
            _lexer.curr_token(),
            "Expected arrow token after function ptr argument list");
    }
    auto ret_type = parse_type();
    if(!ret_type) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Expected return type for function ptr decl");
    }
    return std::make_unique<Types::FunctionType>(std::move(ret_type), std::move(args));
}

Parser::Parser::unique_vec<Parser::Types::BaseType> Parser::Parser::parse_func_type_args() {
    if(!parse_token(Lexer::Token::Id::LeftParenthesis)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::LeftParenthesis, "("),
            _lexer.curr_token(),
            "Expected the function ptr arguments list to be inside parenthesis");
    }
    unique_vec<Types::BaseType> args;
    auto arg = parse_type();
    if(arg) {
        args.emplace_back(std::move(arg));
        fold(
            make_tok_parser(Lexer::Token::Id::Comma),
            [&arg, &args, this] (auto&& node) {
                arg = parse_type();
                if(!arg) {
                    abort<Exception::BaseSyntax>(
                        _lexer.curr_token(),
                        "Expected type after comma in function ptr declaration");
                }
            args.emplace_back(std::move(arg));
        });
    }
    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        error<Exception::ExpectedToken>(
            _lexer.make_token(Lexer::Token::Id::RightParenthesis, ")"),
            _lexer.curr_token(),
            "Expected the function ptr arguments list to be inside parenthesis");
    }
    return args;
}
