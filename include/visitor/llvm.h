//
// Created by igor on 17.02.19.
//

#ifndef TKOM2_LLVM_H
#define TKOM2_LLVM_H

#include <parser/visitor.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

namespace Visitor {

    class LLVM: public Parser::Visitor {
    public:
        void visit(const Parser::Nodes::Base &node) override;

//        void visit(const Parser::Nodes::IntConstant &node) override;

        void visit(const Parser::Nodes::VariableDecl &node) override;
//        void visit(const Parser::Nodes::Identifier &node) override;

        void visit(const Parser::Nodes::AssignmentExpr &node) override;
//        void visit(const Parser::Nodes::AdditiveExpr &node) override;
//        void visit(const Parser::Nodes::MultiplicativeExpr &node) override;
//        void visit(const Parser::Nodes::UnaryExpr &node) override;
//        void visit(const Parser::Nodes::PrimaryExpr &node) override;
//        void visit(const Parser::Nodes::Constant &node) override;
//        void visit(const Parser::Nodes::ParenthesisExpr &node) override;
//        void visit(const Parser::Nodes::StringConstant &node) override;

    private:
        llvm::LLVMContext _context;
        llvm::IRBuilder<> _builder{_context};
        std::unique_ptr<llvm::Module> _module;
        std::map<std::string, llvm::Value*> _named_values;

    };
}

#endif //TKOM2_LLVM_H
