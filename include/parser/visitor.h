//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_VISITATOR_H
#define TKOM2_VISITATOR_H

#include <parser/nodes/base.h>

namespace Parser {

    namespace Nodes {
        class Base;
        class BaseParent;
        class Program;
        class GlobVariableDecl;
        class Statement;
        class CodeBlock;
        class VariableDecl;
        class FunctionDecl;
        class FunctionDef;
        class FunctionHeader;

        // Expressions

        class Expression;
        class BinaryExpr;
        class AssignmentExpr;
        class AdditiveExpr;
        class MultiplicativeExpr;
        class UnaryExpr;
        class PrimaryExpr;
        class Identifier;
        class Constant;
        class IntConstant;
        class ParenthesisExpr;
        class StringConstant;
    }

    class Visitor {
    public:
        virtual void visit(const Nodes::Base& node) = 0;
        virtual void visit(const Nodes::BaseParent& node) = 0;
        virtual void visit(const Nodes::Program& node) = 0;
        virtual void visit(const Nodes::GlobVariableDecl& node) = 0;
        virtual void visit(const Nodes::FunctionHeader& node) = 0;
        virtual void visit(const Nodes::Statement& node) = 0;
        virtual void visit(const Nodes::CodeBlock& node) = 0;
        virtual void visit(const Nodes::VariableDecl& node) = 0;
        virtual void visit(const Nodes::FunctionDecl& node) = 0;
        virtual void visit(const Nodes::FunctionDef& node) = 0;

        // Expressions

        virtual void visit(const Nodes::Expression& node) = 0;
        virtual void visit(const Nodes::BinaryExpr& node) = 0;
        virtual void visit(const Nodes::AssignmentExpr& node) = 0;
        virtual void visit(const Nodes::AdditiveExpr& node) = 0;
        virtual void visit(const Nodes::MultiplicativeExpr& node) = 0;
        virtual void visit(const Nodes::UnaryExpr& node) = 0;
        virtual void visit(const Nodes::PrimaryExpr& node) = 0;
        virtual void visit(const Nodes::Identifier& node) = 0;
        virtual void visit(const Nodes::Constant& node) = 0;
        virtual void visit(const Nodes::IntConstant& node) = 0;
        virtual void visit(const Nodes::ParenthesisExpr& node) = 0;
        virtual void visit(const Nodes::StringConstant& node) = 0;
    };
}

#endif //TKOM2_VISITATOR_H
