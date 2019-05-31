//
// Created by igor on 17.02.19.
//

#ifndef TKOM2_LLVM_H
#define TKOM2_LLVM_H

#include "exception/handler.h"
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/TargetRegistry.h>
#include <parser/nodes/concrete.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>

#include <set>
#include <string>
#include <memory>

#include <parser/visitor.h>
#include <parser/ast.h>
#include <visitor/base.h>
#include <exception/handling_mixin.h>

#include <llvm/Pass.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>


namespace Visitor::LLVM {

    class Compiler: public Visitor::Base, public Exception::HandlingMixin {
    public:
        explicit Compiler(Parser::AST& ast, std::string ofname = "output.o");

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
        void visit(const Parser::Nodes::GlobVariableDecl &node) override;
        void visit(const Parser::Nodes::MemoryDecl& node) override;

        // Statement
        void visit(const Parser::Nodes::CodeBlock &node) override;
        void visit(const Parser::Nodes::VariableDecl &node) override;
        void visit(const Parser::Nodes::AtStmt& node) override;
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
        void visit(const Parser::Nodes::IndexExpr &node) override;
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
            llvm::Value* llvm_alloca;
        };

        struct GlobalVarWrapper {
            const Parser::Nodes::GlobVariableDecl* var;
            llvm::GlobalVariable* llvm_var;
        };

        struct FuncProtWrapper {
            const Parser::Nodes::FunctionProt* func;
            llvm::Function* llvm_func;
            
            bool is_method;    
        };

        struct StructWrapper {
            const Parser::Nodes::StructDecl* str;
            llvm::StructType* llvm_str;

            bool is_memory{false}; 

            std::map<std::string, FuncProtWrapper*> methods{};
            std::int32_t member_index(const std::string& name) const;
        };

        using func_map_t = std::map<std::string, FuncProtWrapper>;
        using var_map_t = std::map<std::string, VarWrapper>;
        using gvar_map_t = std::map<std::string, GlobalVarWrapper>;
        using str_map_t = std::map<std::string, StructWrapper>;
    private:
        Parser::AST& _ast;

        llvm::LLVMContext _context;
        std::unique_ptr<llvm::Module> _module = std::make_unique<llvm::Module>("Main", _context);
        llvm::IRBuilder<> _builder{_context};
        std::unique_ptr<llvm::legacy::FunctionPassManager> _func_pass_manager = std::make_unique<llvm::legacy::FunctionPassManager>(_module.get());

        llvm::TargetMachine* _target_machine;

        const std::string _this_identifier{"this"};
        const std::string _mem_init_meth_name{"init"};
        std::string _output_file_name;

        // what shall we do with pointers in llvm-ir
        enum class PtrAction {
            Store, Load, Address, None
        };

        struct CallContext {
            // set to true if compiling lhs of a callexpr
            bool is_call{false};
            FuncProtWrapper* func{nullptr};
            llvm::Value* this_instance{nullptr};
        };

        struct Context {
            var_map_t local_variables{};
            gvar_map_t global_variables{};
            func_map_t functions{};
            str_map_t structs{};

            // to pass values through calls
            llvm::Value* ret_value{nullptr};

            // start of the current loop block
            llvm::BasicBlock* loop{nullptr};
            // control block of the current loop
            llvm::BasicBlock* loop_contr{nullptr};

            // if compiling a struct points to it
            StructWrapper* curr_struct{nullptr};

            PtrAction ptr_action{PtrAction::Load};

            CallContext call_ctx{};

        };

        Context _ctx{};

        // helper 
        void init_func_pass_manager();
        void init_compile_target();
        void emit_obj_code();

        void add_memory_initializers();

        StructWrapper& declare_opaque(const Parser::Nodes::StructDecl &node);
        StructWrapper& declare_opaque(const Parser::Nodes::MemoryDecl &node);

        FuncProtWrapper* compile_method(
            const Parser::Nodes::StructDecl* str,
            const Parser::Nodes::FunctionDecl* meth);
        StructWrapper& declare_body(const Parser::Nodes::StructDecl& node);
        GlobalVarWrapper& add_memory_global_var(const Parser::Nodes::MemoryDecl& node);

        std::string meth_identifier(const std::string& m_name);
        std::string meth_identifier(const std::string& s_name, const std::string& m_name);

        std::string mem_glob_identifier(const std::string& m_name);

        std::pair<llvm::Value*, bool> access_struct_field(llvm::Value* str, const std::string& field_name); 
        std::pair<llvm::Value*, StructWrapper*> get_struct_value_with_info(llvm::Value* str);
        std::string get_struct_type_name(llvm::Value* str);

        llvm::Function* access_struct_method(llvm::Value* str, const std::string& meth_name);
        const Lexer::Token::Span* _curr_span;
        
        // find helpers
        std::optional<VarWrapper*> get_local_var(const std::string& name);
        std::optional<FuncProtWrapper*> get_function(const std::string& name);
        std::optional<llvm::Value*> get_struct_var(const std::string& name);
        std::optional<FuncProtWrapper*> get_struct_method(const std::string& name); 
        std::optional<GlobalVarWrapper*> get_global_var(const std::string& name);

        VarWrapper& create_local_var(llvm::Function &func, const Parser::Nodes::VariableDecl &node);
        VarWrapper& create_local_var(
            llvm::Function &func,
            const std::string& identifier,
            llvm::Type* type);
        llvm::AllocaInst* create_anon_var(
            llvm::Function &func,
            const std::string& identifier,
            llvm::Type* type);

        using call_info = std::tuple<llvm::Value*, llvm::Value*, FuncProtWrapper*>;    
        call_info compile_call_lhs(const Parser::Nodes::Expression &lhs);


        llvm::Type* strip_ptr_type(llvm::Type *v);
        llvm::Type* strip_ptr_type(llvm::Value* v);

        llvm::Value* perform_ptr_action(
            llvm::Value *ptr,
            llvm::Value *v = nullptr,
            const std::string &load_s = "");
    };
}

#endif //TKOM2_LLVM_H
