//
// Created by igor on 17.02.19.
//

#include <visitor/llvm.h>
#include <parser/nodes/concrete.h>

void Visitor::LLVM::visit(const Parser::Nodes::Base &node) {
    throw std::runtime_error("Cannot compile base node!");
}

void Visitor::LLVM::visit(const Parser::Nodes::IntConstant &node) {
    // todo for now 64 bits
    // todo isSigned = true when signed values are there
    auto value = llvm::ConstantInt::get(_context, llvm::APInt(64, uint64_t(node.value)));
    // return value
}

void Visitor::LLVM::visit(const Parser::Nodes::Identifier &node) {
    // lookup variable
    llvm::Value *v = _named_values[node.symbol];
    if(!v) {
        throw std::runtime_error("Use of undeclared variable! " + node.symbol);
    }
    // return value
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


