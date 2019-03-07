//
// Created by igor on 17.02.19.
//

#include <visitor/llvm.h>
#include <parser/nodes/concrete.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <llvm/IR/Verifier.h>
#include <llvm/ADT/STLExtras.h>

// Base
void Visitor::LLVM::visit(const Parser::Nodes::Base &node) {
    throw std::runtime_error("Cannot compile base node!");
}


// End
// Program
void Visitor::LLVM::visit(const Parser::Nodes::Program &node) {
    // todo maybe init module here ?
    std::cout << "Program node\n";
    std::cout << "Program compilation started\n";

    node.accept_children(*this);
}


// Top Level
// Function
void Visitor::LLVM::visit(const Parser::Nodes::FunctionProt &node) {
    std::cout << "FunctionProt\n";
    if(node.type->identifier().symbol != "int") {
        throw std::runtime_error("Usage of unsupported return type " + node.type->identifier().symbol);
    }
    std::cout << "Compiling arg list\n";
    std::cout << "Arg list size is " << node.arg_list.size() << "\n";
    std::vector<llvm::Type*> args_types;
    for(const auto& arg: node.arg_list) {
        if(arg->type->identifier().symbol != "int") {
            throw std::runtime_error("Usage of undeclared type " + arg->type->identifier().symbol);
        }
        // add int64 type. todo need more types
        args_types.push_back(llvm::Type::getInt64Ty(_context));
    }
    // return type is int. todo Later needs to use more than one type
    std::cout << "Func arg types size: " << args_types.size() << "\n";
    std::cout << "Compiling func type\n";
    llvm::FunctionType* func_t = llvm::FunctionType::get(
            llvm::Type::getInt64Ty(_context),
            args_types,
            false);
    std::cout << "Compiling func header\n";
    llvm::Function* func = llvm::Function::Create(
            func_t,
            llvm::Function::ExternalLinkage,
            node.identifier,
            _module.get());

    if(!func) {
        throw std::runtime_error("Could not compile function!");
    }

    // todo its ugly, lets make it enumerate
    std::cout << "Setting func arg names\n";
    unsigned int i = 0;
    for(auto& arg: func->args()) {
        std::cout << "Setting name of arg: " << i << " " << node.arg_list[i]->identifier << "\n";
        arg.setName(node.arg_list[i++]->identifier);
    }
    std::cout << "Printing func IR\n";
    // todo its just temporary
    _ret_func = func;
    func->print(llvm::outs());
    std::cout << "\n";
}

void Visitor::LLVM::visit(const Parser::Nodes::FunctionDef &node) {
    std::cout << "FuncDef\n";
    // todo there is a bug with redefining function with different arg names

    auto func = _module->getFunction(node.declaration->identifier);
    if(!func) {
        node.declaration->accept(*this); func = _ret_func;
    }
    if(!func) {
        throw std::runtime_error("Could not compile func prototype! " + node.declaration->identifier);
    }
    if(!func->empty()) {
        throw std::runtime_error("Redefinition of func " + node.declaration->identifier);
    }
    auto basic_block = llvm::BasicBlock::Create(_context, "entry", func);
    _builder.SetInsertPoint(basic_block);

    // todo why do we clear it?
    _named_values.clear();
    for(auto& arg: func->args()) {
        _named_values[arg.getName()] = &arg;
    }

    // visit the body
    // todo actually should subclass codeblock to function body
    // todo no func body parsing for now
    node.body->accept(*this);


    if(_ret_value) {
        //_builder.CreateRet(_ret_value);
        // no return here
        llvm::verifyFunction(*func);
        _ret_func = func;
    } else {
        // error on body, remove function
        func->eraseFromParent();
        _ret_func = nullptr;
    }
    func->print(llvm::outs());
}


// Statement
void Visitor::LLVM::visit(const Parser::Nodes::CodeBlock &node) {
    _ret_value = nullptr;
    for(auto& ch: node.children()) {
        ch->accept(*this);
        if (_ret_value) {
            // does it work?
            // todo change it but we have no variables yet
            _builder.CreateRet(_ret_value);
        }
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::VariableDecl &node) {
    std::cout << "Var decl\n";
    llvm::Value *v = _named_values[node.identifier];
    if(v) {
        throw std::runtime_error("Redeclaration of a variable! " + node.identifier);
    }
    if(node.type->identifier().symbol != "int") {
        throw std::runtime_error("Use of undeclared type " + node.type->identifier().symbol);
    }
    // add variable with default value
    // _named_values[node.identifier] = llvm::?
    // return it
}


// Expression
// Binary
void Visitor::LLVM::visit(const Parser::Nodes::AssignmentExpr &node) {
    std::cout << "Assig expr\n";
    // Compute left and right if needed
    node.lhs->accept(*this); auto lhs = _ret_value;
    //llvm::Value* rhs = nullptr;
    if(node.rhs) {
        throw std::runtime_error("Assignment not supported");
        //node.rhs->accept(*this); rhs = _ret_value;
    }
    // todo temporary
    _ret_value = lhs;
//    lhs->print(llvm::errs());
}

void Visitor::LLVM::visit(const Parser::Nodes::AdditiveExpr &node) {
    // Compute left and right if needed
    std::cout << "Add expr\n";
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
    std::cout << "Mult expr\n";
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


// Unary
// Postfix
// Primary
void Visitor::LLVM::visit(const Parser::Nodes::Identifier &node) {
    std::cout << "IdentifierExpr\n";
    // lookup variable
    llvm::Value *v = _named_values[node.symbol];
    if(!v) {
        throw std::runtime_error("Use of undeclared variable! " + node.symbol);
    }
    _ret_value = v;
}

void Visitor::LLVM::visit(const Parser::Nodes::ParenthesisExpr &node) {
    node.expr->accept(*this);
}


// Consts
void Visitor::LLVM::visit(const Parser::Nodes::IntConstant &node) {
    std::cout << "ConstInt\n";
    _ret_value = llvm::ConstantInt::get(_context, llvm::APInt(64, uint64_t(node.value)));
}