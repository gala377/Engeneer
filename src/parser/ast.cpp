//
// Created by igor on 30.12.18.
//

#include <parser/ast.h>

void Parser::AST::visit(Parser::Visitor &v) {
    v.visit(_root);
}
