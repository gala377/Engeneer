//
// Created by igor on 30.12.18.
//

#include <parser/nodes/concrete.h>
#include <iostream>

/*
 *  Program
 */

void Parser::Nodes::Program::accept(Parser::Visitor &v) const {
    v.visit(*this);
//    accept_children(v);
}

/*
*   Identifier
*/


Parser::Nodes::Identifier::Identifier(const std::string& symbol) : symbol(symbol) {};

void Parser::Nodes::Identifier::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

/*
 *  CodeBlock
 */

void Parser::Nodes::CodeBlock::accept(Parser::Visitor &v) const {
    v.visit(*this);
//    accept_children(v);
}


/*
 * GlobVariableDecl
 */

void Parser::Nodes::GlobVariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::GlobVariableDecl::GlobVariableDecl(
        const std::string& symbol,
        const std::string& type_symbol):
        identifier(symbol), type_identifier(type_symbol) {}

/*
 * VariableDecl
 */

Parser::Nodes::VariableDecl::VariableDecl(
        const std::string& symbol,
        const std::string& type_symbol):
        identifier(symbol), type_identifier(type_symbol) {}

void Parser::Nodes::VariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

/*
 *  FunctionDecl
 */

void Parser::Nodes::FunctionHeader::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::FunctionHeader::FunctionHeader(
        const std::string &identifier,
        const std::string &type_identifier,
        std::vector<std::unique_ptr<Parser::Nodes::GlobVariableDecl>> &&arg_list) :
        identifier(identifier), type_identifier(type_identifier), arg_list(std::move(arg_list)) {}


/*
 * FunctionDef
 */

Parser::Nodes::FunctionDef::FunctionDef(
        std::unique_ptr<Parser::Nodes::FunctionHeader> &&decl,
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
//    declaration->accept(v);
//    body->accept(v);
}


/*
 *
 * Expressions in general
 *
 */

void Parser::Nodes::Expression::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::BinaryExpr::BinaryExpr(
        std::unique_ptr<Parser::Nodes::Expression> &&lhs,
        const Lexer::Token& op,
        std::unique_ptr<Parser::Nodes::Expression> &&rhs):
        lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}

void Parser::Nodes::BinaryExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    lhs->set_depth(_depth+1);
    if(rhs) {
        rhs->set_depth(_depth+1);
    }
}

void Parser::Nodes::BinaryExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
//    lhs->accept(v);
//    if (rhs) {
//        rhs->accept(v);
//    }
}

void Parser::Nodes::AssignmentExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
//    lhs->accept(v);
//    if (rhs) {
//        rhs->accept(v);
//    }
}

void Parser::Nodes::AdditiveExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
//    lhs->accept(v);
//    if (rhs) {
//        rhs->accept(v);
//    }
}


void Parser::Nodes::MultiplicativeExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
//    lhs->accept(v);
//    if (rhs) {
//        rhs->accept(v);
//    }
}

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
//    rhs->accept(v);
}

void Parser::Nodes::PrimaryExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::Constant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::IntConstant::IntConstant(int value): value(value) {}

void Parser::Nodes::IntConstant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::ParenthesisExpr::ParenthesisExpr(std::unique_ptr<Parser::Nodes::Expression> &&expr):
    expr(std::move(expr)) {
    this->expr->set_depth(_depth + 1);
}

void Parser::Nodes::ParenthesisExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
//    expr->accept(v);
}

void Parser::Nodes::ParenthesisExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    expr->set_depth(_depth+1);
}

Parser::Nodes::StringConstant::StringConstant(const std::string& value): value(value) {}

void Parser::Nodes::StringConstant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}
