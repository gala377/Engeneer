//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_LOG_H
#define TKOM2_LOG_H

#include <sstream>

#include <visitor/base.h>
#include <parser/nodes/concrete.h>

namespace Visitor {

    class Stringify: public Base {
    public:
        // Visitor Interface

        // Base
        void visit(const Parser::Nodes::Base &node) override;
        void visit(const Parser::Nodes::BaseParent& node) override;

        // End

        //Program
        void visit(const Parser::Nodes::Program& node) override;

        // Top Level
        void visit(const Parser::Nodes::GlobVariableDecl& node) override;

        // Function
        void visit(const Parser::Nodes::FunctionProt &node) override;

        // Statement
        void visit(const Parser::Nodes::Statement &node) override;
        void visit(const Parser::Nodes::CodeBlock &node) override;
        void visit(const Parser::Nodes::VariableDecl &node) override;
        void visit(const Parser::Nodes::FunctionDef &node) override;
        void visit(const Parser::Nodes::FunctionDecl &node) override;

        // Expression
        void visit(const Parser::Nodes::Expression &node) override;

        // Binary
        void visit(const Parser::Nodes::BinaryExpr &node) override;
        void visit(const Parser::Nodes::AssignmentExpr &node) override;
        void visit(const Parser::Nodes::AdditiveExpr &node) override;
        void visit(const Parser::Nodes::MultiplicativeExpr &node) override;

        // Unary
        void visit(const Parser::Nodes::UnaryExpr &node) override;

        // Postfix

        // Primary
        void visit(const Parser::Nodes::PrimaryExpr &node) override;
        void visit(const Parser::Nodes::Identifier &node) override;
        void visit(const Parser::Nodes::ParenthesisExpr &node) override;

        // Consts
        void visit(const Parser::Nodes::Constant &node) override;
        void visit(const Parser::Nodes::IntConstant &node) override;
        void visit(const Parser::Nodes::StringConstant &node) override;

        // Class Interface
        std::string repr();

    protected:
        std::stringstream _stream;

        void add_margin(std::uint32_t depth);
        void stringify(const Parser::Nodes::Base& node, std::string&& as);
    };

}

#endif //TKOM2_LOG_H
