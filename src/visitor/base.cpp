//
// Created by igor on 30.12.18.
//

#include <visitor/base.h>

void Visitor::Base::visit(const Parser::Nodes::Program &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}

void Visitor::Base::visit(const Parser::Nodes::VariableDecl &node) {
    visit(static_cast<const Parser::Nodes::Base&>(node));
}
