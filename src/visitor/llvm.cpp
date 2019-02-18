//
// Created by igor on 17.02.19.
//

#include <visitor/llvm.h>
#include <parser/nodes/concrete.h>

void Visitor::LLVM::visit(const Parser::Nodes::Base &node) {
    throw std::runtime_error("Cannot compile base node!");
}

void Visitor::LLVM::visit(const Parser::Nodes::VariableDecl &node) {
    llvm::Value *v = _named_values[node.identifier];
    if(v) {
        throw std::runtime_error("Redeclaration of a variable! " + node.identifier);
    }
    if(node.type_identifier != "int") {
        throw std::runtime_error("Use of undeclared type " + node.type_identifier);
    }
    // add variable with default value
    // _named_values[node.identifier] = llvm::?
    // return it
}


void Visitor::LLVM::visit(const Parser::Nodes::AssignmentExpr &node) {
    // Compute left and right if needed
    node.lhs->accept(*this); auto lhs = _ret_value;
    //llvm::Value* rhs = nullptr;
    if(node.rhs) {
        throw std::runtime_error("Assignment not supported");
        //node.rhs->accept(*this); rhs = _ret_value;
    }
    _ret_value = lhs;
}

void Visitor::LLVM::visit(const Parser::Nodes::AdditiveExpr &node) {
    // Compute left and right if needed
    node.lhs->accept(*this); auto lhs = _ret_value;
    llvm::Value* rhs = nullptr;
    if(node.rhs) {
        node.rhs->accept(*this); rhs = _ret_value;
    }
    switch(node.op.id) {
        case Lexer::Token::Id::Plus:
            _ret_value = _builder.CreateAdd(lhs, rhs, "__addtmp");
            break;
        case Lexer::Token::Id::Minus:
            _ret_value = _builder.CreateSub(lhs, rhs, "__addtmp");
            break;
        case Lexer::Token::Id::None:
            _ret_value = lhs;
            break;
        default:
            throw std::runtime_error("Unexpected operator during addition operator");
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::MultiplicativeExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;
    llvm::Value* rhs = nullptr;
    if(node.rhs) {
        node.rhs->accept(*this); rhs = _ret_value;
    }
    switch(node.op.id) {
        case Lexer::Token::Id::Multiplication:
            _ret_value = _builder.CreateMul(lhs, rhs, "__multmp");
            break;
        case Lexer::Token::Id::Division:
            // todo for now its unsigned division, we can do signed division
            // but its kinda more comlpicated
            _ret_value = _builder.CreateUDiv(lhs, rhs, "__multmp");
            break;
        case Lexer::Token::Id::None:
            _ret_value = lhs;
            break;
        default:
            throw std::runtime_error("Unexpected operator during addition operator");
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::IntConstant &node) {
    _ret_value = llvm::ConstantInt::get(_context, llvm::APInt(64, uint64_t(node.value)));
}

void Visitor::LLVM::visit(const Parser::Nodes::Identifier &node) {
    // lookup variable
    llvm::Value *v = _named_values[node.symbol];
    if(!v) {
        throw std::runtime_error("Use of undeclared variable! " + node.symbol);
    }
    _ret_value = v;
}


//
// Function generation
//

void Visitor::LLVM::visit(const Parser::Nodes::FunctionProt &node) {
    if(node.type_identifier != "int") {
        throw std::runtime_error("Usage of unsupported return type " + node.type_identifier);
    }
    std::vector<llvm::Type*> args_types(node.arg_list.size());
    for(const auto& arg: node.arg_list) {
        if(arg->type_identifier != "int") {
            throw std::runtime_error("Usage of undeclared type " + arg->type_identifier);
        }
        // add int64 type. todo need more types
        args_types.push_back(llvm::Type::getInt64Ty(_context));
    }
    // return type is int. todo Later needs to use more than one type
    llvm::FunctionType* func_t = llvm::FunctionType::get(
            llvm::Type::getInt64Ty(_context),
            args_types,
            false);
    llvm::Function* func = llvm::Function::Create(
            func_t,
            llvm::Function::ExternalLinkage,
            node.identifier,
            _module.get());
}

void Visitor::LLVM::visit(const Parser::Nodes::FunctionDef &node) {

}
