//
// Created by igor on 17.02.19.
//

#ifndef TKOM2_LLVM_H
#define TKOM2_LLVM_H

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <parser/nodes/concrete.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <set>

#include <parser/visitor.h>
#include <parser/ast.h>
#include <string>
#include <visitor/base.h>

#include <llvm/Pass.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>


namespace Visitor::LLVM {

    class Compiler: public Visitor::Base {
    public:
        explicit Compiler(Parser::AST& ast);

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

        void visit(const Parser::Nodes::StructDecl &node) override;

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

        // Cast
        void visit(const Parser::Nodes::CastExpr &node) override;

        // Unary
        void visit(const Parser::Nodes::AddressAccessExpr &node) override;
        void visit(const Parser::Nodes::DereferenceExpr &node) override;

        // Postfix
        void visit(const Parser::Nodes::CallExpr &node) override;
        // todo Index zakłada że po leweje stronie zawsze ma wskaźnik
        // todo (bo zakłada allocainst), kiedy tak naprawdę
        // todo może mieć wartość (np zwróconą z funckji)
        void visit(const Parser::Nodes::IndexExpr &node) override;

        // todo ten sam problem instieje dla Access
        void visit(const Parser::Nodes::AccessExpr &node) override;

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

        struct StructWrapper {
            const Parser::Nodes::StructDecl* str;
            llvm::StructType* llvm_str;

            std::map<std::string, llvm::Function*> methods{};

            std::int32_t member_index(const std::string& name) const;
        };

        using func_map_t = std::map<std::string, FuncProtWrapper>;
        using var_map_t = std::map<std::string, VarWrapper>;
        using str_map_t = std::map<std::string, StructWrapper>;
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

        const std::string _this_identifier{"self"};

        // Context
        var_map_t _local_variables;
        func_map_t _functions;
        str_map_t _structs;

        llvm::BasicBlock* _curr_loop = nullptr;
        llvm::BasicBlock* _curr_loop_contr = nullptr;

        StructWrapper* _curr_struct = nullptr;

        // what shall we do with pointers in llvm-ir?
        enum class PtrAction {
            Store, Load, Address, None
        };
        PtrAction _ptr_action{PtrAction::Load};
        // to pass values through calls
        llvm::Value* _ret_value;

        // helper functions
        StructWrapper& declare_opaque(const Parser::Nodes::StructDecl &node);
        llvm::Function* compile_method(
            const Parser::Nodes::StructDecl* str,
            const Parser::Nodes::FunctionDecl* meth); 
        std::string meth_identifier(const std::string& m_name);
        std::string meth_identifier(const std::string& s_name, const std::string& m_name);

        llvm::Value* access_struct_field(llvm::Value* str, const std::string& field_name); 

        VarWrapper& create_local_var(llvm::Function &func, const Parser::Nodes::VariableDecl &node);
        VarWrapper& create_local_var(
            llvm::Function &func,
            const std::string& identifier,
            llvm::Type* type);
        llvm::AllocaInst* create_anon_var(
            llvm::Function &func,
            const std::string& identifier,
            llvm::Type* type);
            
        llvm::Value* cast(llvm::Value* from, llvm::Value* to);

        std::tuple<llvm::Value*, llvm::Value*> promote(llvm::Value* lhs, llvm::Value* rhs);
        llvm::Type* strip_ptr_type(llvm::Type *v);
        llvm::Type* strip_ptr_type(llvm::Value* v);

        llvm::Value* perform_ptr_action(
            llvm::Value *ptr,
            llvm::Value *v = nullptr,
            const std::string &load_s = "");
    };
}

#endif //TKOM2_LLVM_H
