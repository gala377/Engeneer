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

        void visit(const Parser::Nodes::WhileStmt &node) override;
        void visit(const Parser::Nodes::BlockStmt &node) override;
        void visit(const Parser::Nodes::IfStmt &node) override;

        void visit(const Parser::Nodes::BreakStmt &node) override;
        void visit(const Parser::Nodes::ContinueStmt &node) override;

        // Expression
        // Binary
        void visit(const Parser::Nodes::RelationalExpr &node) override;
        void visit(const Parser::Nodes::AssignmentExpr &node) override;
        void visit(const Parser::Nodes::AdditiveExpr &node) override;
        void visit(const Parser::Nodes::MultiplicativeExpr &node) override;

        // Unary

        // Postfix
        void visit(const Parser::Nodes::CallExpr &node) override;

        // Primary
        void visit(const Parser::Nodes::Identifier &node) override;
        void visit(const Parser::Nodes::ParenthesisExpr &node) override;

        // Consts
        void visit(const Parser::Nodes::IntConstant &node) override;
        void visit(const Parser::Nodes::StringConstant &node) override;
        void visit(const Parser::Nodes::FloatConstant &node) override;

        // Class Interface
    private:
        llvm::LLVMContext _context;
        llvm::IRBuilder<> _builder{_context};
        // todo more modules and linking against them
        std::unique_ptr<llvm::Module> _module = std::make_unique<llvm::Module>("TestJit", _context);
        // todo pass manager per module? Its marked as legacy?
        // todo its legacy and we cannot find the passes so
        // todo maybe it changed and the tutorial hasn't been updated?
        // todo remember about mem2reg for alloca optimalizations
        std::unique_ptr<llvm::legacy::FunctionPassManager> _func_pass_manager = std::make_unique<llvm::legacy::FunctionPassManager>(_module.get());

        struct VarWrapper {
            const Parser::Nodes::VariableDecl* var;
            llvm::AllocaInst* llvm_alloca;
        };
        std::map<std::string, VarWrapper> _local_variables;

        struct FuncProtWrapper {
            const Parser::Nodes::FunctionProt* func;
            llvm::Function* llvm_func;
        };
        std::map<std::string, FuncProtWrapper> _functions;

        // Returns
        llvm::Value* _ret_value;
        std::string _ret_symbol;

        // Context
        llvm::BasicBlock* _curr_loop = nullptr;
        llvm::BasicBlock* _curr_loop_contr = nullptr;
        bool _skip_load = false;

        // Type information
        struct IntTypeInfo {
            bool is_signed;
            uint32_t size;
        };


        const std::string void_id{"void"};
        const std::string bool_id{"bool"};
        const std::string byte_id{"byte"};
        const std::string i8_id{"i8"};
        const std::string i16_id{"i16"};
        const std::string i32_id{"i32"};
        const std::string i64_id{"i64"};
        const std::string u8_id{"u8"};
        const std::string u16_id{"u16"};
        const std::string u32_id{"u32"};
        const std::string u64_id{"u64"};
        const std::string f32_id{"f32"};
        const std::string f64_id{"f64"};
        const std::string f128_id{"f128"};

        // methods
        VarWrapper& create_local_var(llvm::Function &func, const Parser::Nodes::VariableDecl &node);

        llvm::Value* cast(llvm::Value* from, llvm::Value* to);
        std::tuple<llvm::Value*, llvm::Value*> promote(llvm::Value* lhs, llvm::Value* rhs);
        std::uint32_t float_size(llvm::Type* f);

        llvm::Type* to_llvm_type(const Parser::Types::BaseType& type);

        using type_handler = std::function<llvm::Type*(const Parser::Types::BaseType& type)>;
        std::map<std::string, type_handler> _type_handlers;

        llvm::Type* try_as_simple(const Parser::Types::BaseType& type);
        llvm::Type* try_as_array(const Parser::Types::BaseType& type);
        llvm::Type* try_as_complex(const Parser::Types::BaseType& type);

        void init_type_handlers();
    };
}

#endif //TKOM2_LLVM_H
