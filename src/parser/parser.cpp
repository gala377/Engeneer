//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <lexer/token.h>

#include <parser/parser.h>
#include <exception/concrete.h>

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
    return std::move(ast);
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
            &Parser::parse_struct_decl);
}

std::unique_ptr<Parser::Nodes::GlobVariableDecl> Parser::Parser::parse_glob_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return nullptr;
    }
    auto identifier= parse_ident();
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            Lexer::Token{Lexer::Token::Id::Identifier, ""},
                _lexer.curr_token(),
                "Expected variable name after let");
    }
    auto type = parse_type();
    if(!type) {
        abort<Exception::ExpectedToken>(
            Lexer::Token{Lexer::Token::Id::Identifier, ""},
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
            Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
            _lexer.curr_token(),
            "Missing semicolon");
    }

    return std::make_unique<Nodes::GlobVariableDecl>(
        std::move(identifier),
        std::move(type),
        std::move(init_expr));
}

std::unique_ptr<Parser::Nodes::StructDecl> Parser::Parser::parse_struct_decl() {
    // todo we didnt want to have a struct keyword but its really hard to parse it
    // todo so I leave it for now and maybe delete it later? I dont know?
    if(!parse_token(Lexer::Token::Id::Struct)) {
        return nullptr;
    }

    auto identifier = parse_ident();
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            Lexer::Token{Lexer::Token::Id::Identifier, ""},
            _lexer.curr_token(),
            "Struct name expected");
    }

    auto wrapped_structs = parse_wraps_decl();
    auto [members, methods] = parse_struct_body();

    return std::make_unique<Nodes::StructDecl>(
        std::move(identifier),
        std::move(members),
        std::move(methods),
        std::move(wrapped_structs));
}

Parser::Parser::unique_vec<Parser::Nodes::Identifier> Parser::Parser::parse_wraps_decl() {
    unique_vec<Nodes::Identifier> wrapped_structs;
    if(!parse_token(Lexer::Token::Id::Wraps)) {
        return std::move(wrapped_structs);
    }
    auto ident = parse_ident();
    if(!ident) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "List of identifiers expected after wraps keyword");
    }
    wrapped_structs.emplace_back(std::move(ident));
    fold(
        make_tok_parser(Lexer::Token::Id::Comma),
        [this, &wrapped_structs](auto&& res) {
            auto id = parse_ident();
            if(!id) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Identifier expected");
            }
            wrapped_structs.emplace_back(std::move(id));
        });
    return std::move(wrapped_structs);
}

// todo refactor
Parser::Parser::struct_body_parse_res_t Parser::Parser::parse_struct_body() {
    unique_vec<Nodes::VariableDecl> members;
    unique_vec<Nodes::FunctionDecl> methods;
    if(!parse_token(Lexer::Token::Id::LeftBrace)) {
        error<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::LeftBrace, "{"},
                _lexer.curr_token(),
                "Struct body is expected to be a code block");
    }

    while(true) {
        auto first_id = parse_ident();
        if(!first_id) {
            auto func = parse_func_decl();
            if(!func) {
                break;
            }
            methods.emplace_back(std::move(func));
            continue;
        }
        auto second_id = parse_ident();
        if(!second_id) {
            auto type = parse_type();
            if(!type) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Couldn't parse member type. "
                    "Declarations in struct body need to be functions or variables");
            }
            if(!parse_token(Lexer::Token::Id::Semicolon)) {
                error<Exception::ExpectedToken>(
                    Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
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
                        Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
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
                Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
                _lexer.curr_token(),
                "Missing semicolon");
        }
        members.emplace_back(std::make_unique<Nodes::VariableDecl>(
            std::move(first_id),
            std::make_unique<Types::SimpleType>(std::move(second_id))));
    }

    if(!parse_token(Lexer::Token::Id::RightBrace)) {
        error<Exception::ExpectedToken>(
            Lexer::Token{Lexer::Token::Id::RightBrace, "}"},
            _lexer.curr_token(),
            "Struct body not closed");
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
        return std::move(header);
    }
    auto body = parse_code_block();
    if(!body) {
        error<Exception::ExpectedToken>(
            Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
            _lexer.curr_token(),
            "Missing semicolon after function prototype");
        return std::move(header);
    }
    return std::make_unique<Nodes::FunctionDef>(
        std::move(header), std::move(body));
}

std::unique_ptr<Parser::Nodes::FunctionProt> Parser::Parser::parse_func_header() {
    auto type = parse_type();
    if(!type) {
        return nullptr;
    }
    auto identifier = parse_ident();
    if(!identifier) {
        abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Identifier, ""},
                _lexer.curr_token(),
                "Function name expected");
    }
    auto arg_list = parse_func_arg_list();
    if(!arg_list) {
        abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::LeftParenthesis, "("},
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
    auto ident = parse_ident();
    while(ident) {
        auto type = parse_type();
        if(!type) {
            abort<Exception::ExpectedToken>(
                    Lexer::Token{Lexer::Token::Id::Identifier, ""},
                    _lexer.curr_token(),
                    "Expected type after variable name");
        }
        arg_list.emplace_back(
                std::make_unique<Nodes::VariableDecl>(std::move(ident), std::move(type)));
        if(parse_token(Lexer::Token::Id::Comma)) {
            ident = parse_ident();
            if(!ident) {
                abort<Exception::ExpectedToken>(
                        Lexer::Token{Lexer::Token::Id::Identifier, ""},
                        _lexer.curr_token());
            }
        }
    }
    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        error<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::RightParenthesis, ")"},
                _lexer.curr_token());
    }
    return std::optional(std::move(arg_list));
}


// Statement
std::unique_ptr<Parser::Nodes::Statement> Parser::Parser::parse_statement() {
    auto res = one_of<Nodes::Statement>(
        &Parser::parse_var_decl,
        &Parser::parse_expr,
        &Parser::parse_if_stmt,
        &Parser::parse_while_stmt,
        &Parser::parse_return_stmt,
        &Parser::parse_break_stmt,
        &Parser::parse_continue_stmt);
    if(!res) {
        return nullptr;
    }
    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        error<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
                _lexer.curr_token(),
                "Missing semicolon");
    }
    return res;
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
                Lexer::Token{Lexer::Token::Id::RightBrace, "}"},
                _lexer.curr_token(),
                "Code block left open");
    }
    return std::move(code_block);
}

std::unique_ptr<Parser::Nodes::VariableDecl> Parser::Parser::parse_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return nullptr;
    }
    auto identifier= parse_ident();
    if(!identifier) {
        abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Identifier, ""},
                _lexer.curr_token(),
                "Expected variable name after let");
    }
    auto type = parse_type();
    if(!type) {
        abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Identifier, ""},
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
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Expression expected after return statement");
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
// todo make assignment stack, for now it does not
std::unique_ptr<Parser::Nodes::AssignmentExpr> Parser::Parser::parse_assig_expr() {
    auto lhs = parse_logical_or_expr();
    if(!lhs) {
        return nullptr;
    }
    std::unique_ptr<Nodes::Expression> rhs = nullptr;
    Lexer::Token op = Lexer::Token{Lexer::Token::Id::None, ""};
    if(auto tok = parse_token(Lexer::Token::Id::Assignment); tok) {
        op = tok.value();
        rhs = parse_logical_or_expr();
    }
    return std::make_unique<Nodes::AssignmentExpr>(
            std::move(lhs), op, std::move(rhs));
}


// Logical
std::unique_ptr<Parser::Nodes::LogicalOrExpr> Parser::Parser::parse_logical_or_expr() {
    auto logical_or_expr = parse_single_logical_or_expr();
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

std::unique_ptr<Parser::Nodes::LogicalOrExpr> Parser::Parser::parse_single_logical_or_expr() {
    std::unique_ptr<Nodes::LogicalAndExpr> lhs = parse_logical_and_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = parse_token(Lexer::Token::Id::LogicalOr);
    std::unique_ptr<Nodes::LogicalAndExpr> rhs{nullptr};
    if(op) {
        rhs = parse_logical_and_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::LogicalOrExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::LogicalAndExpr> Parser::Parser::parse_logical_and_expr() {
    auto logical_and_expr = parse_single_logical_and_expr();
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

std::unique_ptr<Parser::Nodes::LogicalAndExpr> Parser::Parser::parse_single_logical_and_expr() {
    std::unique_ptr<Nodes::InclusiveOrExpr> lhs = parse_inclusive_or_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = parse_token(Lexer::Token::Id::LogicalAnd);
    std::unique_ptr<Nodes::InclusiveOrExpr> rhs{nullptr};
    if(op) {
        rhs = parse_inclusive_or_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::LogicalAndExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::InclusiveOrExpr> Parser::Parser::parse_inclusive_or_expr() {
    auto inclusive_or_expr = parse_single_inclusive_or_expr();
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

std::unique_ptr<Parser::Nodes::InclusiveOrExpr> Parser::Parser::parse_single_inclusive_or_expr() {
    std::unique_ptr<Nodes::ExclusiveOrExpr> lhs = parse_exclusive_or_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = parse_token(Lexer::Token::Id::InclusiveOr);
    std::unique_ptr<Nodes::ExclusiveOrExpr> rhs{nullptr};
    if(op) {
        rhs = parse_exclusive_or_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::InclusiveOrExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::ExclusiveOrExpr> Parser::Parser::parse_exclusive_or_expr() {
    auto exclusive_or_expr = parse_single_exclusive_or_expr();
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

std::unique_ptr<Parser::Nodes::ExclusiveOrExpr> Parser::Parser::parse_single_exclusive_or_expr() {
    std::unique_ptr<Nodes::AndExpr> lhs = parse_and_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = parse_token(Lexer::Token::Id::ExclusiveOr);
    std::unique_ptr<Nodes::AndExpr> rhs{nullptr};
    if(op) {
        rhs = parse_and_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::ExclusiveOrExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::AndExpr> Parser::Parser::parse_and_expr() {
    auto and_expr = parse_single_and_expr();
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

std::unique_ptr<Parser::Nodes::AndExpr> Parser::Parser::parse_single_and_expr() {
    std::unique_ptr<Nodes::EqualityExpr> lhs = parse_equality_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = parse_token(Lexer::Token::Id::And);
    std::unique_ptr<Nodes::EqualityExpr> rhs{nullptr};
    if(op) {
        rhs = parse_equality_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::AndExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::EqualityExpr> Parser::Parser::parse_equality_expr() {
    auto equality_expr = parse_single_equality_expr();
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

std::unique_ptr<Parser::Nodes::EqualityExpr> Parser::Parser::parse_single_equality_expr() {
    std::unique_ptr<Nodes::RelationalExpr> lhs = parse_relational_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = one_of_op<Lexer::Token>(
        make_tok_parser(Lexer::Token::Id::Equality),
        make_tok_parser(Lexer::Token::Id::Inequality));
    std::unique_ptr<Nodes::RelationalExpr> rhs{nullptr};
    if(op) {
        rhs = parse_relational_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::EqualityExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::RelationalExpr> Parser::Parser::parse_relational_expr() {
    auto rel_expr = parse_single_relational_expr();
    fold(&Parser::parse_relational_op,
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

std::unique_ptr<Parser::Nodes::RelationalExpr> Parser::Parser::parse_single_relational_expr() {
    std::unique_ptr<Nodes::ShiftExpr> lhs = parse_shift_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = parse_relational_op();
    std::unique_ptr<Nodes::ShiftExpr> rhs{nullptr};
    if(op) {
        rhs = parse_shift_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::RelationalExpr>(
            std::move(lhs), op.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::ShiftExpr> Parser::Parser::parse_shift_expr() {
    auto shift_expr = parse_single_shift_expr();
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

std::unique_ptr<Parser::Nodes::ShiftExpr> Parser::Parser::parse_single_shift_expr() {
    std::unique_ptr<Nodes::AdditiveExpr> lhs = parse_add_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = one_of_op<Lexer::Token>(
            make_tok_parser(Lexer::Token::Id::LeftShift),
            make_tok_parser(Lexer::Token::Id::RightShift));
    std::unique_ptr<Nodes::AdditiveExpr> rhs{nullptr};
    if(op) {
        rhs = parse_add_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::ShiftExpr>(
            std::move(lhs), op.value(), std::move(rhs));
}

// Arithmetic
std::unique_ptr<Parser::Nodes::AdditiveExpr> Parser::Parser::parse_add_expr() {
    auto add_expr = parse_single_add_expr();
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

std::unique_ptr<Parser::Nodes::AdditiveExpr> Parser::Parser::parse_single_add_expr() {
    std::unique_ptr<Nodes::MultiplicativeExpr> lhs = parse_mult_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = one_of_op<Lexer::Token>(
        make_tok_parser(Lexer::Token::Id::Plus),
        make_tok_parser(Lexer::Token::Id::Minus));
    std::unique_ptr<Nodes::MultiplicativeExpr> rhs{nullptr};
    if(op) {
        rhs = parse_mult_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::AdditiveExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

std::unique_ptr<Parser::Nodes::MultiplicativeExpr> Parser::Parser::parse_mult_expr() {
    auto mult_expr = parse_single_mult_expr();
    fold(
        [](Parser* p) {
            return p->one_of_op<Lexer::Token>(
                p->make_tok_parser(Lexer::Token::Id::Multiplication),
                p->make_tok_parser(Lexer::Token::Id::Division));
        },
        [this, &mult_expr] (auto&& op) {
            auto res = parse_unary_expr();
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

std::unique_ptr<Parser::Nodes::MultiplicativeExpr> Parser::Parser::parse_single_mult_expr() {
    std::unique_ptr<Nodes::Expression> lhs = parse_unary_expr();
    if(!lhs) {
        return nullptr;
    }
    auto op = one_of_op<Lexer::Token>(
        make_tok_parser(Lexer::Token::Id::Multiplication),
        make_tok_parser(Lexer::Token::Id::Division));
    std::unique_ptr<Nodes::Expression> rhs{nullptr};
    if(op) {
        rhs = parse_unary_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::MultiplicativeExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

// Unary
std::unique_ptr<Parser::Nodes::UnaryExpr> Parser::Parser::parse_unary_expr() {
    return one_of<Nodes::UnaryExpr>(
        &Parser::parse_negative_expr,
        &Parser::parse_negation_expr,
        &Parser::parse_postfix_to_unary_expr);
}

std::unique_ptr<Parser::Nodes::UnaryExpr> Parser::Parser::parse_postfix_to_unary_expr() {

    auto res = parse_postfix_expr();
    if(!res) {

        return nullptr;
    }
    return std::make_unique<Nodes::UnaryExpr>(
        Lexer::none_tok,
        std::move(res));
}

std::unique_ptr<Parser::Nodes::NegativeExpr> Parser::Parser::parse_negative_expr() {
    if(!parse_token(Lexer::Token::Id::Minus)) {
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
    return std::make_unique<Nodes::NegativeExpr>(std::move(rhs));
}

std::unique_ptr<Parser::Nodes::NegationExpr> Parser::Parser::parse_negation_expr() {
    if(!parse_token(Lexer::Token::Id::Negation)) {
        return nullptr;
    }
    auto rhs = parse_postfix_expr();
    if(!rhs) {
        abort<Exception::BaseSyntax>(
                _lexer.curr_token(),
                "Primary expression expected after unary ! operator "
                "(use parenthesis for multiple unary operators)");
    }
    return std::make_unique<Nodes::NegationExpr>(std::move(rhs));
}

// Postfix
std::unique_ptr<Parser::Nodes::PostfixExpr> Parser::Parser::parse_postfix_expr() {
    auto lhs = parse_single_postfix_expr();
    if(!lhs) {
        return nullptr;
    }
    fold(
        [&lhs] (Parser* p) -> std::unique_ptr<Nodes::PostfixExpr> {
            return p->one_of<Nodes::PostfixExpr>(
                    [&lhs] (Parser* p) {
                        return p->parse_call_expr(lhs);
                    },
                    [&lhs] (Parser* p) {
                        return p->parse_index_expr(lhs);
                    },
                    [&lhs] (Parser* p) {
                        return p->parse_access_expr(lhs);
                    });
        },
        [&lhs](auto&& res) {
            lhs = std::move(res);
        });
    return std::move(lhs);
}


std::unique_ptr<Parser::Nodes::CallExpr> Parser::Parser::parse_call_expr(
    std::unique_ptr<Nodes::PostfixExpr> &lhs) {
    auto res = parse_call_parameters();
    if(!res) {
        return nullptr;
    }
    return std::make_unique<Nodes::CallExpr>(
        std::move(lhs),
        std::move(res.value()));
}

std::unique_ptr<Parser::Nodes::IndexExpr> Parser::Parser::parse_index_expr(
    std::unique_ptr<Nodes::PostfixExpr> &lhs) {

    auto idx = parse_index_parameters();
    if(!idx) {
        return nullptr;
    }
    return std::make_unique<Nodes::IndexExpr>(
        std::move(lhs),
        std::move(idx));
}

std::unique_ptr<Parser::Nodes::AccessExpr> Parser::Parser::parse_access_expr(
    std::unique_ptr<Nodes::PostfixExpr> &lhs) {

    auto rhs = parse_access_parameters();
    if(!rhs) {
        return nullptr;
    }
    return std::make_unique<Nodes::AccessExpr>(
        std::move(lhs),
        std::move(rhs));
}

std::unique_ptr<Parser::Nodes::PostfixExpr> Parser::Parser::parse_single_postfix_expr() {
    auto lhs = parse_prim_expr();
    if(!lhs) {

        return nullptr;
    }
    if(auto res = parse_call_parameters(); res) {
        return std::make_unique<Nodes::CallExpr>(
            std::move(lhs),
            std::move(res.value()));
    }
    if(auto idx = parse_index_parameters(); idx) {
        return std::make_unique<Nodes::IndexExpr>(
            std::move(lhs),
            std::move(idx));
    }
    if(auto rhs = parse_access_parameters(); rhs) {
        return std::make_unique<Nodes::AccessExpr>(
            std::move(lhs),
            std::move(rhs));
    }
    return std::make_unique<Nodes::PostfixExpr>(std::move(lhs));
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
            Lexer::Token{Lexer::Token::Id::RightParenthesis, ")"},
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
            Lexer::Token{Lexer::Token::Id::RightSquareBracket, "]"},
            _lexer.curr_token(),
            "Indexing not closed");
    }
    return std::move(lhs);
}

std::unique_ptr<Parser::Nodes::Identifier> Parser::Parser::parse_access_parameters() {
    if(!parse_token(Lexer::Token::Id::Dot)) {
        return nullptr;
    }
    auto ident = parse_ident();
    if(!ident) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Access can only be done with identifiers");
    }
    return std::move(ident);
}

// Primary
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

std::unique_ptr<Parser::Nodes::ParenthesisExpr> Parser::Parser::parse_parenthesis() {
    if(!parse_token(Lexer::Token::Id::LeftParenthesis)) {
        return nullptr;
    }
    auto expr = parse_expr();
    if(!expr) {
        return nullptr;
    }
    if(!parse_token(Lexer::Token::Id::RightParenthesis)) {
        error<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::RightParenthesis, ")"},
                _lexer.curr_token());
    }
    return std::make_unique<Nodes::ParenthesisExpr>(std::move(expr));
}


// Consts
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
    if(!parse_token(Lexer::Token::Id::LeftSquareBracket)) {
        return nullptr;
    }
    // todo for now just integers, later maybe expressions which are consts
    // todo that would be nice
    auto size = parse_int();
    if(!size) {
        abort<Exception::BaseSyntax>(
            _lexer.curr_token(),
            "Array size needs to be a positive integer");
    }
    if(!parse_token(Lexer::Token::Id::RightSquareBracket)) {
        return nullptr;
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
    auto res = parse_ident();
    if(!res) {
        return nullptr;
    }
    return std::make_unique<Types::SimpleType>(std::move(res));
}
