//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_BASE_H
#define TKOM2_BASE_H

#include <parser/visitor.h>
#include <parser/nodes/concrete.h>

namespace Visitor {

    class Base: public Parser::Visitor {
    public:
        // Base
        void visit(const Parser::Nodes::Base &node) override = 0;
        void visit(const Parser::Nodes::BaseParent& node) override;

        // End
        void visit(const Parser::Nodes::End &node) override;

        // Program
        void visit(const Parser::Nodes::Program& node) override;

        // Top Level
        void visit(const Parser::Nodes::TopLevelDecl &node) override;
        void visit(const Parser::Nodes::GlobVariableDecl& node) override;
        void visit(const Parser::Nodes::StructDecl& node) override;

        // Function
        void visit(const Parser::Nodes::FunctionProt& node) override;
        void visit(const Parser::Nodes::FunctionDecl& node) override;
        void visit(const Parser::Nodes::FunctionDef& node) override;

        // Statement
        void visit(const Parser::Nodes::Statement &node) override;
        void visit(const Parser::Nodes::CodeBlock& node) override;
        void visit(const Parser::Nodes::VariableDecl &node) override;

        // Expressions
        void visit(const Parser::Nodes::Expression &node) override;

        // Binary
        void visit(const Parser::Nodes::BinaryExpr &node) override;
        void visit(const Parser::Nodes::AssignmentExpr &node) override;
        void visit(const Parser::Nodes::AdditiveExpr &node) override;
        void visit(const Parser::Nodes::MultiplicativeExpr &node) override;

        // Unary
        void visit(const Parser::Nodes::UnaryExpr &node) override;
        void visit(const Parser::Nodes::NegativeExpr &node) override;

        // Postfix
        void visit(const Parser::Nodes::PostfixExpr &node) override;
        void visit(const Parser::Nodes::CallExpr &node) override;
        void visit(const Parser::Nodes::IndexExpr &node) override;
        void visit(const Parser::Nodes::AccessExpr &node) override;

        // Primary
        void visit(const Parser::Nodes::PrimaryExpr &node) override;
        void visit(const Parser::Nodes::Identifier &node) override;
        void visit(const Parser::Nodes::ParenthesisExpr &node) override;

        // Consts
        void visit(const Parser::Nodes::Constant &node) override;
        void visit(const Parser::Nodes::IntConstant &node) override;
        void visit(const Parser::Nodes::StringConstant &node) override;
    };

}

#endif //TKOM2_BASE_H
