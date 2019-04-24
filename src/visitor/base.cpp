//
// Created by igor on 30.12.18.
//

#include "parser/nodes/concrete.h"
#include <visitor/base.h>


// Base
void Visitor::Base::visit(const Parser::Nodes::BaseParent &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}


// End
void Visitor::Base::visit(const Parser::Nodes::End &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}


// Program
void Visitor::Base::visit(const Parser::Nodes::Program &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}


// Top Level
void Visitor::Base::visit(const Parser::Nodes::TopLevelDecl &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::GlobVariableDecl &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::StructDecl &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::MemoryDecl &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

// Function
void Visitor::Base::visit(const Parser::Nodes::FunctionProt &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::FunctionDecl& node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::FunctionDef& node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}


// Statement
void Visitor::Base::visit(const Parser::Nodes::Statement &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::CodeBlock &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::VariableDecl &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::AtStmt &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::BlockStmt &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::IfStmt &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::WhileStmt &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::ReturnStmt &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::BreakStmt &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::ContinueStmt &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

// Expressions
void Visitor::Base::visit(const Parser::Nodes::Expression &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}


// Binary

// Logical
void Visitor::Base::visit(const Parser::Nodes::LogicalOrExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::LogicalAndExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::InclusiveOrExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::ExclusiveOrExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::AndExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::EqualityExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::RelationalExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::ShiftExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}


// Arithmetic
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


// Unary
void Visitor::Base::visit(const Parser::Nodes::UnaryExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::NegativeExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::NegationExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::AddressAccessExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::DereferenceExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

// Postfix
void Visitor::Base::visit(const Parser::Nodes::PostfixExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::CallExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));

}

void Visitor::Base::visit(const Parser::Nodes::IndexExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::AccessExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::CastExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

// Primary
void Visitor::Base::visit(const Parser::Nodes::PrimaryExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::Identifier &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::ParenthesisExpr &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}


// Const
void Visitor::Base::visit(const Parser::Nodes::Constant &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::IntConstant &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::StringConstant &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::FloatConstant &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}




