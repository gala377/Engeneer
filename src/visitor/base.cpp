//
// Created by igor on 30.12.18.
//

#include <visitor/base.h>

void Visitor::Base::visit(const Parser::Nodes::Program &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::GlobVariableDecl &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::BaseParent &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::FunctionHeader &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::Statement &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::CodeBlock &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::VariableDecl &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::FunctionDef& node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::FunctionDecl& node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::Identifier &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::Expression &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::BinaryExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::AssignmentExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::AdditiveExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::MultiplicativeExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::UnaryExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::PrimaryExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::Constant &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::IntConstant &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::ParenthesisExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::StringConstant &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}
