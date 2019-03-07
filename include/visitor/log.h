//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_LOG_H
#define TKOM2_LOG_H

#include <sstream>

#include <visitor/base.h>
#include <parser/nodes/concrete.h>
#include <parser/type.h>

namespace Visitor {

    class Stringify: public Base {
    public:
        // Visitor Interface

        // Base
        void visit(const Parser::Nodes::Base &node) override;

        // End

        //Program
        void visit(const Parser::Nodes::Program& node) override;

        // Top Level
        void visit(const Parser::Nodes::GlobVariableDecl& node) override;
        void visit(const Parser::Nodes::StructDecl &node) override;

        // Function
        void visit(const Parser::Nodes::FunctionProt &node) override;

        // Statement
        void visit(const Parser::Nodes::CodeBlock &node) override;
        void visit(const Parser::Nodes::VariableDecl &node) override;
        void visit(const Parser::Nodes::FunctionDef &node) override;
        void visit(const Parser::Nodes::FunctionDecl &node) override;
        void visit(const Parser::Nodes::BlockStmt &node) override;
        void visit(const Parser::Nodes::IfStmt &node) override;
        void visit(const Parser::Nodes::WhileStmt &node) override;
        void visit(const Parser::Nodes::ReturnStmt &node) override;
        void visit(const Parser::Nodes::BreakStmt &node) override;
        void visit(const Parser::Nodes::ContinueStmt &node) override;

        // Expression

        // Binary
        void visit(const Parser::Nodes::BinaryExpr &node) override;

        // Logical
        void visit(const Parser::Nodes::LogicalOrExpr &node) override;
        void visit(const Parser::Nodes::LogicalAndExpr &node) override;
        void visit(const Parser::Nodes::InclusiveOrExpr &node) override;
        void visit(const Parser::Nodes::ExclusiveOrExpr &node) override;
        void visit(const Parser::Nodes::AndExpr &node) override;
        void visit(const Parser::Nodes::EqualityExpr &node) override;
        void visit(const Parser::Nodes::RelationalExpr &node) override;
        void visit(const Parser::Nodes::ShiftExpr &node) override;

        // Arithmetic
        void visit(const Parser::Nodes::AssignmentExpr &node) override;
        void visit(const Parser::Nodes::AdditiveExpr &node) override;
        void visit(const Parser::Nodes::MultiplicativeExpr &node) override;

        // Unary
        void visit(const Parser::Nodes::UnaryExpr &node) override;
        void visit(const Parser::Nodes::NegativeExpr &node) override;
        void visit(const Parser::Nodes::NegationExpr &node) override;

        // Postfix
        void visit(const Parser::Nodes::PostfixExpr &node) override;
        void visit(const Parser::Nodes::CallExpr &node) override;
        void visit(const Parser::Nodes::IndexExpr &node) override;
        void visit(const Parser::Nodes::AccessExpr &node) override;

        // Primary
        void visit(const Parser::Nodes::Identifier &node) override;
        void visit(const Parser::Nodes::ParenthesisExpr &node) override;

        // Consts
        void visit(const Parser::Nodes::IntConstant &node) override;
        void visit(const Parser::Nodes::StringConstant &node) override;

        // Class Interface
        std::string repr();

    protected:
        std::stringstream _stream;

        void add_margin(std::uint32_t depth);
        void stringify(const Parser::Nodes::Base& node, std::string&& as);
        std::string strf_type(const std::unique_ptr<Parser::Types::BasicType>& type);
    };

}

#endif //TKOM2_LOG_H
