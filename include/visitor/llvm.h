//
// Created by igor on 17.02.19.
//

#ifndef TKOM2_LLVM_H
#define TKOM2_LLVM_H

#include <set>

#include <parser/visitor.h>

#include <llvm/Pass.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>

#include <visitor/base.h>

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

        void visit(const Parser::Nodes::BlockStmt &node) override;

        void visit(const Parser::Nodes::IfStmt &node) override;

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
        void visit(const Parser::Nodes::CallExpr &node) override;

        // Primary
        void visit(const Parser::Nodes::Identifier &node) override;
        void visit(const Parser::Nodes::ParenthesisExpr &node) override;

        // Consts
        void visit(const Parser::Nodes::IntConstant &node) override;


        // Class Interface
    private:
        llvm::LLVMContext _context;
        llvm::IRBuilder<> _builder{_context};
        // todo more modules and linking against them
        std::unique_ptr<llvm::Module> _module = std::make_unique<llvm::Module>("TestJit", _context);
        // todo pass manager per module? Its marked as legacy?
        // todo its legacy and we cannot find the passes so
        // todo maybe it changed and the tutorial hasn't been updated?
        std::unique_ptr<llvm::legacy::FunctionPassManager> _func_pass_manager = std::make_unique<llvm::legacy::FunctionPassManager>(_module.get());

        std::map<std::string, llvm::AllocaInst*> _named_values;

        llvm::Value* _ret_value;
        llvm::Function* _ret_func;
        std::string _ret_symbol;

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

        bool _skip_load = false;
    };
}

#endif //TKOM2_LLVM_H
