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

// Helpers

void Parser::Parser::add_member_or_method(
    std::pair<std::string, std::string> identifiers,
    Parser::Parser::unique_vec<Nodes::VariableDecl> &members,
    Parser::Parser::unique_vec<Nodes::FunctionDef> &methods) {

    auto& [first_id, second_id] = identifiers;
    auto arg_list = parse_func_arg_list();
    if(arg_list) {
        auto body = parse_code_block();
        methods.emplace_back(
            std::make_unique<Nodes::FunctionDef>(
                std::make_unique<Nodes::FunctionProt>(
                    second_id,
                    first_id,
                    std::move(arg_list.value())),
                std::move(body)));
    } else {
        if(!parse_token(Lexer::Token::Id::Semicolon)) {
            error<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
                _lexer.curr_token(),
                "Missing semicolon");
        }
        members.emplace_back(std::make_unique<Nodes::VariableDecl>(
            first_id, second_id));
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
            &Parser::parse_struct_decl);
}

std::unique_ptr<Parser::Nodes::GlobVariableDecl> Parser::Parser::parse_glob_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return nullptr;
    }
    auto identifier_res = parse_token(Lexer::Token::Id::Identifier);
    if(!identifier_res) {
        abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Identifier, ""},
                _lexer.curr_token(),
                "Expected variable name after let");
    }
    Lexer::Token var = identifier_res.value();

    auto type_res = parse_type();
    if(!identifier_res) {
        abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Identifier, ""},
                _lexer.curr_token(),
                "Expected type after variable name");
    }
    auto type_symbol = type_res.value();

    if(!parse_token(Lexer::Token::Id::Semicolon)) {
        error<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
                _lexer.curr_token(),
                "Missing semicolon");
    }

    return std::make_unique<Nodes::GlobVariableDecl>(var.symbol, type_symbol);
}

std::unique_ptr<Parser::Nodes::StructDecl> Parser::Parser::parse_struct_decl() {
    // todo we didnt want to have a struct keyword but its really hard to parse it
    // todo so I leave it for now and maybe delete it later? I dont know?
    if(!parse_token(Lexer::Token::Id::Struct)) {
        return nullptr;
    }

    auto identifier = parse_token(Lexer::Token::Id::Identifier);
    if(!identifier) {
        abort<Exception::ExpectedToken>(
            Lexer::Token{Lexer::Token::Id::Identifier, ""},
            _lexer.curr_token(),
            "Struct name expected");
    }

    std::optional<std::string> wrapped_id;
    if(parse_token(Lexer::Token::Id::Wraps)) {
        auto id = parse_token(Lexer::Token::Id::Identifier);
        if(!id) {
            abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Identifier, ""},
                _lexer.curr_token(),
                "Struct identifier expected after wraps keyword");
        }
        wrapped_id = id->symbol;
    }

    auto [members, methods] = parse_struct_body();

    return std::make_unique<Nodes::StructDecl>(
        identifier->symbol,
        std::move(members),
        std::move(methods),
        wrapped_id);
}

Parser::Parser::struct_body_parse_res_t Parser::Parser::parse_struct_body() {
    unique_vec<Nodes::VariableDecl> members;
    unique_vec<Nodes::FunctionDef> methods;
    if(!parse_token(Lexer::Token::Id::LeftBrace)) {
        error<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::LeftBrace, "{"},
                _lexer.curr_token(),
                "Struct body is expected to be a code block");
    }
    // todo type modifiers make it so much worst
    // todo like consts and pointers
    // todo because its no longer 2 identifiers
    // todo its type and identifier or identifier and type
    // todo but identifier alone, can also be a type
    // todo so we can have 4 situations
    // todo type identifier, identifier type, identifier identifier, type type
    // todo and right now we only have this one --------^
    fold(
        make_tok_parser(Lexer::Token::Id::Identifier),
        [this, &members, &methods](auto first_id) {
            auto second_id = parse_token(Lexer::Token::Id::Identifier);
            if(!second_id) {
                abort<Exception::ExpectedToken>(
                    Lexer::Token{Lexer::Token::Id::Identifier, ""},
                        _lexer.curr_token(),
                        "Declarations in struct body need to be functions or variables");
            }
            add_member_or_method(
                std::make_pair(first_id->symbol, second_id->symbol),
                members,
                methods);
        });
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
        return nullptr;
    }
    return std::make_unique<Nodes::FunctionDef>(
        std::move(header), std::move(body));
}

std::unique_ptr<Parser::Nodes::FunctionProt> Parser::Parser::parse_func_header() {
    auto type_res = parse_token(Lexer::Token::Id::Identifier);
    if(!type_res) {
        return nullptr;
    }
    auto type_identifier = type_res.value().symbol;
    auto identifier_res = parse_token(Lexer::Token::Id::Identifier);
    if(!identifier_res) {
        abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::Identifier, ""},
                _lexer.curr_token(),
                "Function name expected");
    }
    auto identifier = identifier_res.value().symbol;
    auto arg_list = parse_func_arg_list();
    if(!arg_list) {
        abort<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::LeftParenthesis, "("},
                _lexer.curr_token());
    }
    return std::make_unique<Nodes::FunctionProt>(
            identifier,
            type_identifier,
            std::move(arg_list.value()));
}


// Function Helpers
std::optional<Parser::Parser::arg_list_t> Parser::Parser::parse_func_arg_list() {
    if(!parse_token(Lexer::Token::Id::LeftParenthesis)) {
        return std::nullopt;
    }
    arg_list_t arg_list{};
    auto ident = parse_token(Lexer::Token::Id::Identifier);
    while(ident) {
        auto type = parse_type();
        if(!type) {
            abort<Exception::ExpectedToken>(
                    Lexer::Token{Lexer::Token::Id::Identifier, ""},
                    _lexer.curr_token(),
                    "Expected type after variable name");
        }
        arg_list.emplace_back(
                std::make_unique<Nodes::VariableDecl>(ident.value().symbol, type.value()));
        if(parse_token(Lexer::Token::Id::Comma)) {
            ident = parse_token(Lexer::Token::Id::Identifier);
            if(!ident) {
                abort<Exception::ExpectedToken>(
                        Lexer::Token{Lexer::Token::Id::Identifier, ""},
                        _lexer.curr_token());
            }
        } else {
            ident = std::nullopt;
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
        &Parser::parse_expr);
    if(!res) {
        return res;
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
    auto code_block = std::make_unique<Nodes::CodeBlock>();

    if(!parse_token(Lexer::Token::Id::LeftBrace)) {
        error<Exception::ExpectedToken>(
                Lexer::Token{Lexer::Token::Id::LeftBrace, "{"},
                _lexer.curr_token());
    }
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
    return std::make_unique<Nodes::VariableDecl>(identifier->symbol, type.value());
}


// Expression
std::unique_ptr<Parser::Nodes::Expression> Parser::Parser::parse_expr() {
    return parse_assig_expr();
}


// Binary
std::unique_ptr<Parser::Nodes::AssignmentExpr> Parser::Parser::parse_assig_expr() {
    std::cerr << "Parse assig\n";
    auto lhs = parse_relational_expr();
    if(!lhs) {
        return nullptr;
    }
    std::unique_ptr<Nodes::Expression> rhs = nullptr;
    Lexer::Token op = Lexer::Token{Lexer::Token::Id::None, ""};
    if(auto tok = parse_token(Lexer::Token::Id::Assignment); tok) {
        op = tok.value();
        rhs = parse_relational_expr();
    }
    return std::make_unique<Nodes::AssignmentExpr>(
            std::move(lhs), op, std::move(rhs));
}


// Logical
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
    std::cerr << "Parse add\n";
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
    std::cerr << "Parse mult\n";
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
    std::cerr << "Parse unary\n";
    return one_of<Nodes::UnaryExpr>(
        &Parser::parse_negative_expr,
        &Parser::parse_postfix_to_unary_expr);
}

std::unique_ptr<Parser::Nodes::UnaryExpr> Parser::Parser::parse_postfix_to_unary_expr() {
    std::cerr << "Parse postfix to unary\n";
    auto res = parse_postfix_expr();
    if(!res) {
        std::cerr << "postfix to unary: LHS is null\n";
        return nullptr;
    }
    return std::make_unique<Nodes::UnaryExpr>(
        Lexer::none_tok,
        std::move(res));
}

std::unique_ptr<Parser::Nodes::NegativeExpr> Parser::Parser::parse_negative_expr() {
    std::cerr << "Parse negative?\n";
    if(!parse_token(Lexer::Token::Id::Minus)) {
        std::cerr << "Negaitive: No minus sign\n";
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


// Postfix
std::unique_ptr<Parser::Nodes::PostfixExpr> Parser::Parser::parse_postfix_expr() {
    std::cerr << "Parse postfix\n";
    auto lhs = parse_single_postfix_expr();
    if(!lhs) {
        std::cerr << "postfix expr: LHS is null\n";
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

std::optional<std::string> Parser::Parser::parse_type() {
    auto res = parse_token(Lexer::Token::Id::Identifier);
    return res ? std::optional{res.value().symbol} : std::nullopt;
}

std::function<std::optional<Lexer::Token>(Parser::Parser*)> Parser::Parser::make_tok_parser(Lexer::Token::Id id) {
    return [id](Parser* p) {
        return p->parse_token(id);
    };
}