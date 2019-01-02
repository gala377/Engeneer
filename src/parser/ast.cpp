//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/ast.h>

void Parser::AST::accept(Parser::Visitor &v) {
    _root.accept(v);
}

Parser::Nodes::BaseParent& Parser::AST::root() {
    return _root; 
}