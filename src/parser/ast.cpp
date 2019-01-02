//
// Created by igor on 30.12.18.
//

#include <parser/ast.h>

void Parser::AST::accept(Parser::Visitor &v) {
    v.visit(_root);
}

Parser::Nodes::Base& Parser::AST::root() {
    return _root; 
}