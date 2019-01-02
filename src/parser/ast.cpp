//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/ast.h>

void Parser::AST::accept(Parser::Visitor &v) {
    std::cout << "Root accepts visitor\n";
    _root.accept(v);
    std::cout << "Visitor is done in root\n";
}

Parser::Nodes::Base& Parser::AST::root() {
    return _root; 
}