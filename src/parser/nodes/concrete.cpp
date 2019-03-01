//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/nodes/concrete.h>


// End
void Parser::Nodes::End::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Program
void Parser::Nodes::Program::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Top Level
void Parser::Nodes::TopLevelDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


// GlobVarDecl
Parser::Nodes::GlobVariableDecl::GlobVariableDecl(
        const std::string& symbol,
        const std::string& type_symbol):
        identifier(symbol), type_identifier(type_symbol) {}

void Parser::Nodes::GlobVariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Functions
void Parser::Nodes::FunctionDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::FunctionProt::FunctionProt(
        const std::string &identifier,
        const std::string &type_identifier,
        std::vector<std::unique_ptr<Parser::Nodes::VariableDecl>> &&arg_list) :
        identifier(identifier), type_identifier(type_identifier), arg_list(std::move(arg_list)) {}

void Parser::Nodes::FunctionProt::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::FunctionDef::FunctionDef(
        std::unique_ptr<Parser::Nodes::FunctionProt> &&decl,
        std::unique_ptr<Parser::Nodes::CodeBlock> &&body) :
        declaration(std::move(decl)), body(std::move(body)) {
    this->declaration->set_depth(_depth+1);
    this->body->set_depth(_depth+1);
}

void Parser::Nodes::FunctionDef::set_depth(std::uint32_t depth) {
    FunctionDecl::set_depth(depth);
    declaration->set_depth(_depth+1);
    body->set_depth(_depth+1);
}

void Parser::Nodes::FunctionDef::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


// Struct
Parser::Nodes::StructDecl::StructDecl(const std::string &identifier,
                                      std::vector<std::unique_ptr<Parser::Nodes::VariableDecl>>&& members,
                                      std::vector<std::unique_ptr<Parser::Nodes::FunctionDef>>&& methods,
                                      const std::optional<std::string>& wrapped_struct):
                                      identifier{identifier},
                                      members{std::move(members)},
                                      methods{std::move(methods)},
                                      wrapped_struct{wrapped_struct} {}

void Parser::Nodes::StructDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::StructDecl::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    for(auto& m: members) {
        m->set_depth(_depth+1);
    }
    for(auto& m: methods) {
        m->set_depth(_depth+1);
    }
}


// Statement
void Parser::Nodes::Statement::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::CodeBlock::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::VariableDecl::VariableDecl(
        const std::string& symbol,
        const std::string& type_symbol):
        identifier(symbol), type_identifier(type_symbol) {}

void Parser::Nodes::VariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Expressions
void Parser::Nodes::Expression::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Binary
Parser::Nodes::BinaryExpr::BinaryExpr(
        std::unique_ptr<Parser::Nodes::Expression> &&lhs,
        const Lexer::Token& op,
        std::unique_ptr<Parser::Nodes::Expression> &&rhs):
        lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {}

void Parser::Nodes::BinaryExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    lhs->set_depth(_depth+1);
    if(rhs) {
        rhs->set_depth(_depth+1);
    }
}

void Parser::Nodes::BinaryExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::AssignmentExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::AdditiveExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::MultiplicativeExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Unary
Parser::Nodes::UnaryExpr::UnaryExpr(
        const Lexer::Token& op,
        std::unique_ptr<Parser::Nodes::Expression> &&rhs):
        op(op), rhs(std::move(rhs)) {}

void Parser::Nodes::UnaryExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    rhs->set_depth(_depth + 1);
}

void Parser::Nodes::UnaryExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::NegativeExpr::NegativeExpr(std::unique_ptr<Parser::Nodes::Expression> &&rhs):
        UnaryExpr(Lexer::Token{Lexer::Token::Id::Minus, "-"}, std::move(rhs)){}

void Parser::Nodes::NegativeExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Postfix
Parser::Nodes::PostfixExpr::PostfixExpr(std::unique_ptr<Parser::Nodes::Expression> &&lhs):
    lhs(std::move(lhs)) {}

void Parser::Nodes::PostfixExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    lhs->set_depth(_depth+1);
}

void Parser::Nodes::PostfixExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::CallExpr::CallExpr(std::unique_ptr<Parser::Nodes::Expression> &&lhs,
                                  std::vector<std::unique_ptr<Parser::Nodes::Expression>> &&args):
                                  PostfixExpr(std::move(lhs)), args(std::move(args)){}


void Parser::Nodes::CallExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    for(auto& ch: args) {
        ch->set_depth(_depth+1);
    }
}

void Parser::Nodes::CallExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::IndexExpr::IndexExpr(std::unique_ptr<Parser::Nodes::Expression> &&lhs,
                                    std::unique_ptr<Parser::Nodes::Expression> &&index_expr):
                                   PostfixExpr(std::move(lhs)), index_expr(std::move(index_expr)) {}

void Parser::Nodes::IndexExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::IndexExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    lhs->set_depth(_depth+1);
    index_expr->set_depth(_depth+1);
}


Parser::Nodes::AccessExpr::AccessExpr(std::unique_ptr<Parser::Nodes::Expression> &&lhs,
                                      std::unique_ptr<Parser::Nodes::Expression> &&rhs):
                                      PostfixExpr(std::move(lhs)), rhs(std::move(rhs)) {}

void Parser::Nodes::AccessExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::AccessExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    lhs->set_depth(_depth+1);
    rhs->set_depth(_depth+1);
}




// Primary
void Parser::Nodes::PrimaryExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::Identifier::Identifier(const std::string& symbol) : symbol(symbol) {};

void Parser::Nodes::Identifier::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::ParenthesisExpr::ParenthesisExpr(std::unique_ptr<Parser::Nodes::Expression> &&expr):
        expr(std::move(expr)) {
    this->expr->set_depth(_depth + 1);
}

void Parser::Nodes::ParenthesisExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    expr->set_depth(_depth+1);
}

void Parser::Nodes::ParenthesisExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Consts
void Parser::Nodes::Constant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::IntConstant::IntConstant(int value): value(value) {}

void Parser::Nodes::IntConstant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::StringConstant::StringConstant(const std::string& value): value(value) {}

void Parser::Nodes::StringConstant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}