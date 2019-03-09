//
// Created by igor on 17.02.19.
//

#ifndef TKOM2_LLVM_H
#define TKOM2_LLVM_H

#include <set>

#include <parser/visitor.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include "base.h"

namespace Visitor {

    class LLVM: public Visitor::Base {
    public:
        // Visitor
        // Base
        void visit(const Parser::Nodes::Base &node) override;

        // End

        // Program
        void visit(const Parser::Nodes::Program &node) override;

        // Top Level
        // Function
        void visit(const Parser::Nodes::FunctionProt &node) override;
        void visit(const Parser::Nodes::FunctionDef &node) override;

        // Statement
        void visit(const Parser::Nodes::CodeBlock &node) override;
        void visit(const Parser::Nodes::VariableDecl &node) override;
        void visit(const Parser::Nodes::ReturnStmt &node) override;

        // Expression
        // Binary
        void visit(const Parser::Nodes::LogicalOrExpr &node) override;
        void visit(const Parser::Nodes::LogicalAndExpr &node) override;
        void visit(const Parser::Nodes::InclusiveOrExpr &node) override;
        void visit(const Parser::Nodes::ExclusiveOrExpr &node) override;
        void visit(const Parser::Nodes::AndExpr &node) override;
        void visit(const Parser::Nodes::EqualityExpr &node) override;
        void visit(const Parser::Nodes::RelationalExpr &node) override;
        void visit(const Parser::Nodes::ShiftExpr &node) override;

        void visit(const Parser::Nodes::AssignmentExpr &node) override;
        void visit(const Parser::Nodes::AdditiveExpr &node) override;
        void visit(const Parser::Nodes::MultiplicativeExpr &node) override;

        // Unary
        void visit(const Parser::Nodes::UnaryExpr &node) override;

        // Postfix
        void visit(const Parser::Nodes::PostfixExpr &node) override;

        // Primary
        void visit(const Parser::Nodes::Identifier &node) override;
        void visit(const Parser::Nodes::ParenthesisExpr &node) override;

        // Consts
        void visit(const Parser::Nodes::IntConstant &node) override;


        // Class Interface
    private:
        llvm::LLVMContext _context;
        llvm::IRBuilder<> _builder{_context};
        std::unique_ptr<llvm::Module> _module = std::make_unique<llvm::Module>("TestJit", _context);

        std::map<std::string, llvm::Value*> _named_values;

        llvm::Value* _ret_value;
        llvm::Function* _ret_func;

        struct IntTypeInfo {
            bool is_signed;
            uint32_t size;
        };

        std::map<std::string, IntTypeInfo> _int_types{
                {"int8", {true, 8}},
                {"int16", {true, 16}},
                {"int32", {true, 32}},
                {"int64", {true, 64}},

                {"uint8", {false, 8}},
                {"uint16", {false, 16}},
                {"uint32", {false, 32}},
                {"uint64", {false, 64}},

                {"byte", {false, 8}},
        };

        std::set<std::string> _basic_types{
           "byte",
           "int8", "int16", "int32", "int64",
           "uint8", "uint16", "uint32", "uint64",
        };
    };
}

#endif //TKOM2_LLVM_H
