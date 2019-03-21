//
// Created by igor on 17.02.19.
//

#ifndef TKOM2_LLVM_H
#define TKOM2_LLVM_H

#include <set>

#include <parser/visitor.h>
#include <parser/ast.h>
#include <visitor/base.h>

#include <llvm/Pass.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>


namespace Visitor::LLVM {

    class Compiler: public Visitor::Base {
    public:
        Compiler(Parser::AST& ast);

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
        void visit(const Parser::Nodes::AddressAccessExpr &node) override;

        // Postfix
        void visit(const Parser::Nodes::CallExpr &node) override;
        void visit(const Parser::Nodes::IndexExpr &node) override;

        // Primary
        void visit(const Parser::Nodes::Identifier &node) override;
        void visit(const Parser::Nodes::ParenthesisExpr &node) override;

        // Consts
        void visit(const Parser::Nodes::IntConstant &node) override;
        void visit(const Parser::Nodes::StringConstant &node) override;
        void visit(const Parser::Nodes::FloatConstant &node) override;

        // Class Interface
        struct VarWrapper {
            const Parser::Nodes::VariableDecl* var;
            llvm::AllocaInst* llvm_alloca;
        };

        struct FuncProtWrapper {
            const Parser::Nodes::FunctionProt* func;
            llvm::Function* llvm_func;
        };

        using func_map_t = std::map<std::string, FuncProtWrapper>;
        using var_map_t = std::map<std::string, VarWrapper>;
    private:
        Parser::AST& _ast;

        llvm::LLVMContext _context;
        llvm::IRBuilder<> _builder{_context};
        // todo more modules and linking against them
        std::unique_ptr<llvm::Module> _module = std::make_unique<llvm::Module>("TestJit", _context);
        // todo pass manager per module? Its marked as legacy?
        // todo its legacy and we cannot find the passes so
        // todo maybe it changed and the tutorial hasn't been updated?
        // todo remember about mem2reg for alloca optimalizations
        std::unique_ptr<llvm::legacy::FunctionPassManager> _func_pass_manager = std::make_unique<llvm::legacy::FunctionPassManager>(_module.get());

        // Context
        var_map_t _local_variables;
        func_map_t _functions;

        // Returns
        llvm::Value* _ret_value;
        std::string _ret_symbol;

        llvm::BasicBlock* _curr_loop = nullptr;
        llvm::BasicBlock* _curr_loop_contr = nullptr;

        enum class PtrAction {
            Store, Load, Address, None
        };
        PtrAction _ptr_action{PtrAction::Load};

        // methods
        VarWrapper& create_local_var(llvm::Function &func, const Parser::Nodes::VariableDecl &node);

        llvm::Value* cast(llvm::Value* from, llvm::Value* to);
        std::tuple<llvm::Value*, llvm::Value*> promote(llvm::Value* lhs, llvm::Value* rhs);

        llvm::Type* strip_ptr_type(llvm::Value *v);
        llvm::Value* perform_ptr_action(llvm::Value *ptr, llvm::Value *v = nullptr, const std::string &load_s = "");

    };
}

#endif //TKOM2_LLVM_H
