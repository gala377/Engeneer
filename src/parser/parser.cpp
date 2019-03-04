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
        return {nullptr};
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
        return {nullptr};
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
        wrapped_id = id.value().symbol;
    }

    auto [members, methods] = parse_struct_body();

    return std::make_unique<Nodes::StructDecl>(
        identifier.value().symbol,
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
    // todo actually its really complicated if we want to be able to
    // todo have syntax for pointers

    // todo <- parse 2 identifiers
    // then parse semicolon if so then add a member
    // if no parse function arg list if no then error missing semicolon
    // if ok make function proto and parse body if no then missing function body
    // if ok make a method
    while(true) {
        if(parse_token(Lexer::Token::Id::RightBrace)) {
            break;
        }
        if(parse_token(Lexer::Token::Id::End)) {
            abort<Exception::UnexpectedToken>(
                Lexer::Token{Lexer::Token::Id::End, ""},
                    "Unexpected end during struct parsing");
        }
        auto first_id = parse_token(Lexer::Token::Id::Identifier);
        if(!first_id) {
            abort<Exception::ExpectedToken>(
                    Lexer::Token{Lexer::Token::Id::Identifier, ""},
                    _lexer.curr_token(),
                    "Declarations in struct body need to be functions or variables");
        }
        auto second_id = parse_token(Lexer::Token::Id::Identifier);
        if(!second_id) {
            abort<Exception::ExpectedToken>(
                    Lexer::Token{Lexer::Token::Id::Identifier, ""},
                    _lexer.curr_token(),
                    "Declarations in struct body need to be functions or variables");
        }
        auto arg_list = parse_func_arg_list();
        if(arg_list) {
            auto body = parse_code_block();
            if(!body) {
                abort<Exception::BaseSyntax>(
                    _lexer.curr_token(),
                    "Missing function body");
            }
            // make a method
            methods.emplace_back(
                std::make_unique<Nodes::FunctionDef>(
                        std::make_unique<Nodes::FunctionProt>(
                            second_id.value().symbol,
                            first_id.value().symbol,
                            std::move(arg_list.value())),
                        std::move(body)));
        } else {
            if(!parse_token(Lexer::Token::Id::Semicolon)) {
                error<Exception::ExpectedToken>(
                    Lexer::Token{Lexer::Token::Id::Semicolon, ";"},
                    _lexer.curr_token(),
                    "Missing semicolon");
            }
            // make a variable
            members.emplace_back(std::make_unique<Nodes::VariableDecl>(
                first_id.value().symbol, second_id.value().symbol));

        }
    }
    return std::make_tuple(std::move(members), std::move(methods));
}

// Function
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

std::unique_ptr<Parser::Nodes::FunctionProt> Parser::Parser::parse_func_header() {
    auto type_res = parse_token(Lexer::Token::Id::Identifier);
    if(!type_res) {
        return {nullptr};
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
                _lexer.curr_token());
    }
    return std::move(code_block);
}

std::unique_ptr<Parser::Nodes::VariableDecl> Parser::Parser::parse_var_decl() {
    if(!parse_token(Lexer::Token::Id::Let)) {
        return {nullptr};
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
    auto add_expr = parse_single_add_expr();
    fold(
        [](Parser* p) {
            return p->one_of_op<Lexer::Token>(
                p->make_tok_parser(Lexer::Token::Id::Plus),
                p->make_tok_parser(Lexer::Token::Id::Minus));
            },
        [this, &add_expr] (auto&& op) {
            auto res = parse_mult_expr();
            std::make_unique<Nodes::AdditiveExpr>(
                std::move(add_expr), op.value(), std::move(res));
        });
    return add_expr;
}

std::unique_ptr<Parser::Nodes::AdditiveExpr> Parser::Parser::parse_single_add_expr() {
    std::unique_ptr<Nodes::MultiplicativeExpr> lhs = parse_mult_expr();
    if(!lhs) {
        return {nullptr};
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
            auto res = parse_prim_expr();
            std::make_unique<Nodes::MultiplicativeExpr>(
                std::move(mult_expr), op.value(), std::move(res));
        });
    return mult_expr;
}

std::unique_ptr<Parser::Nodes::MultiplicativeExpr> Parser::Parser::parse_single_mult_expr() {
    std::unique_ptr<Nodes::Expression> lhs = parse_prim_expr();
    if(!lhs) {
        return {nullptr};
    }
    auto op = one_of_op<Lexer::Token>(
        make_tok_parser(Lexer::Token::Id::Multiplication),
        make_tok_parser(Lexer::Token::Id::Division));
    std::unique_ptr<Nodes::Expression> rhs{nullptr};
    if(op) {
        rhs = parse_prim_expr();
    } else {
        op = std::optional{Lexer::none_tok};
    }
    return std::make_unique<Nodes::MultiplicativeExpr>(
        std::move(lhs), op.value(), std::move(rhs));
}

// Unary
std::unique_ptr<Parser::Nodes::UnaryExpr> Parser::Parser::parse_unary_expr() {
    return one_of<Nodes::UnaryExpr>(
//        &Parser::parse_prim_expr, todo Primary Expr is not an unary expr
        &Parser::parse_negative_expr
//            &Parser::parse_postfix_expr,
    );
}

std::unique_ptr<Parser::Nodes::NegativeExpr> Parser::Parser::parse_negative_expr() {
    if(!parse_token(Lexer::Token::Id::Minus)) {
        return {nullptr};
    }
    // todo how should I do this?
    // should it be just parse_prim?
    // well no, what if we have many unary op?
    auto rhs = parse_unary_expr();
    if(!rhs) {
        abort<Exception::BaseSyntax>(
                _lexer.curr_token(),
                "Expression expected after unary - operator");
    }
    return std::make_unique<Nodes::NegativeExpr>(std::move(rhs));
}


// Postfix
std::unique_ptr<Parser::Nodes::PostfixExpr> Parser::Parser::parse_postfix_expr() {
    auto lhs = parse_prim_expr();
    switch (_lexer.curr_token().id) {
        case Lexer::Token::Id::LeftSquareBracket:
            break;
        case Lexer::Token::Id::LeftParenthesis:
            break;
        case Lexer::Token::Id::Dot:
            break;
        default:
            // todo
            //return std::make_unique<Nodes::AccessExpr>();
            throw std::runtime_error("Unimplemented!");
    }
    return nullptr;
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
        return {nullptr};
    }
    auto expr = parse_expr();
    if(!expr) {
        return {nullptr};
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

std::optional<std::string> Parser::Parser::parse_type() {
    auto res = parse_token(Lexer::Token::Id::Identifier);
    return res ? std::optional{res.value().symbol} : std::nullopt;
}

std::function<std::optional<Lexer::Token>(Parser::Parser*)> Parser::Parser::make_tok_parser(Lexer::Token::Id id) {
    return [id](Parser* p) {
        return p->parse_token(id);
    };
}




