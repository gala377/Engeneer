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
        void visit(const Parser::Nodes::Base &node) override = 0;
        void visit(const Parser::Nodes::BaseParent& node) override;
        void visit(const Parser::Nodes::Program& node) override;
        void visit(const Parser::Nodes::GlobVariableDecl& node) override;
        void visit(const Parser::Nodes::FunctionHeader& node) override;
        void visit(const Parser::Nodes::Statement &node) override;
        void visit(const Parser::Nodes::CodeBlock& node) override;
        void visit(const Parser::Nodes::VariableDecl &node) override;
        void visit(const Parser::Nodes::FunctionDecl& node) override;
        void visit(const Parser::Nodes::FunctionDef& node) override;

        //
        // Expressions
        //

        void visit(const Parser::Nodes::Expression &node) override;
        void visit(const Parser::Nodes::BinaryExpr &node) override;
        void visit(const Parser::Nodes::AssignmentExpr &node) override;
        void visit(const Parser::Nodes::AdditiveExpr &node) override;
        void visit(const Parser::Nodes::MultiplicativeExpr &node) override;
        void visit(const Parser::Nodes::UnaryExpr &node) override;
        void visit(const Parser::Nodes::PrimaryExpr &node) override;
        void visit(const Parser::Nodes::Constant &node) override;
        void visit(const Parser::Nodes::IntConstant &node) override;
        void visit(const Parser::Nodes::Identifier &node) override;
        void visit(const Parser::Nodes::ParenthesisExpr &node) override;
        void visit(const Parser::Nodes::StringConstant &node) override;
    };

}

#endif //TKOM2_BASE_H
