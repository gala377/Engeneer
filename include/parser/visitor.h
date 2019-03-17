//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_VISITATOR_H
#define TKOM2_VISITATOR_H

#include <parser/nodes/base.h>

namespace Parser {

    namespace Nodes {
        // Base
        class Base;
        class BaseParent;

        // End
        class End;

        // Program
        class Program;

        // Top Level
        class TopLevelDecl;
        class GlobVariableDecl;
        class StructDecl;

        // Function
        class FunctionDecl;
        class FunctionDef;
        class FunctionProt;

        // Statements
        class CodeBlock;
        class Statement;
        class VariableDecl;
        class BlockStmt;
        class IfStmt;
        class WhileStmt;
        class ReturnStmt;
        class BreakStmt;
        class ContinueStmt;

        // Expressions
        class Expression;

        // Binary
        class BinaryExpr;

        // Logical
        class LogicalOrExpr;
        class LogicalAndExpr;
        class InclusiveOrExpr;
        class ExclusiveOrExpr;
        class AndExpr;
        class EqualityExpr;
        class RelationalExpr;
        class ShiftExpr;

        // Arithmetic
        class AssignmentExpr;
        class AdditiveExpr;
        class MultiplicativeExpr;

        // Unary
        class UnaryExpr;
        class NegativeExpr;
        class NegationExpr;

        // Postfix
        class PostfixExpr;
        class IndexExpr;
        class CallExpr;
        class AccessExpr;

        // Primary
        class PrimaryExpr;
        class Identifier;
        class Constant;
        class IntConstant;
        class ParenthesisExpr;
        class StringConstant;
        class FloatConstant;

    }

    class Visitor {
    public:
        // Base
        virtual void visit(const Nodes::Base& node) = 0;
        virtual void visit(const Nodes::BaseParent& node) = 0;

        // End
        virtual void visit(const Nodes::End& node) = 0;

        //Program
        virtual void visit(const Nodes::Program& node) = 0;

        // Top level
        virtual void visit(const Nodes::TopLevelDecl& node) = 0;
        virtual void visit(const Nodes::GlobVariableDecl& node) = 0;
        virtual void visit(const Nodes::FunctionDecl& node) = 0;
        virtual void visit(const Nodes::FunctionProt& node) = 0;
        virtual void visit(const Nodes::FunctionDef& node) = 0;
        virtual void visit(const Nodes::StructDecl& node) = 0;

        // Statement
        virtual void visit(const Nodes::Statement& node) = 0;
        virtual void visit(const Nodes::CodeBlock& node) = 0;
        virtual void visit(const Nodes::VariableDecl& node) = 0;
        virtual void visit(const Nodes::BlockStmt& node) = 0;
        virtual void visit(const Nodes::IfStmt& node) = 0;
        virtual void visit(const Nodes::WhileStmt& node) = 0;
        virtual void visit(const Nodes::ReturnStmt& node) = 0;
        virtual void visit(const Nodes::BreakStmt& node) = 0;
        virtual void visit(const Nodes::ContinueStmt& node) = 0;

        // Expressions
        virtual void visit(const Nodes::Expression& node) = 0;

        // Binary

        // Logical
        virtual void visit(const Nodes::LogicalOrExpr& node) = 0;
        virtual void visit(const Nodes::LogicalAndExpr& node) = 0;
        virtual void visit(const Nodes::InclusiveOrExpr& node) = 0;
        virtual void visit(const Nodes::ExclusiveOrExpr& node) = 0;
        virtual void visit(const Nodes::AndExpr& node) = 0;
        virtual void visit(const Nodes::EqualityExpr& node) = 0;
        virtual void visit(const Nodes::RelationalExpr& node) = 0;
        virtual void visit(const Nodes::ShiftExpr& node) = 0;

        // Arithmetic
        virtual void visit(const Nodes::BinaryExpr& node) = 0;
        virtual void visit(const Nodes::AssignmentExpr& node) = 0;
        virtual void visit(const Nodes::AdditiveExpr& node) = 0;
        // todo modulo expr
        virtual void visit(const Nodes::MultiplicativeExpr& node) = 0;

        // Unary
        virtual void visit(const Nodes::UnaryExpr& node) = 0;
        virtual void visit(const Nodes::NegativeExpr& node) = 0;
        virtual void visit(const Nodes::NegationExpr& node) = 0;

        // Postfix
        virtual void visit(const Nodes::PostfixExpr& node) = 0;
        virtual void visit(const Nodes::CallExpr& node) = 0;
        virtual void visit(const Nodes::IndexExpr& node) = 0;
        virtual void visit(const Nodes::AccessExpr& node) = 0;

        // Primary
        virtual void visit(const Nodes::PrimaryExpr& node) = 0;
        virtual void visit(const Nodes::Identifier& node) = 0;
        virtual void visit(const Nodes::ParenthesisExpr& node) = 0;

        // Consts
        virtual void visit(const Nodes::Constant& node) = 0;
        virtual void visit(const Nodes::IntConstant& node) = 0;
        virtual void visit(const Nodes::StringConstant& node) = 0;
        virtual void visit(const Nodes::FloatConstant& node) = 0;
    };
}

#endif //TKOM2_VISITATOR_H
