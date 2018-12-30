//
// Created by igor on 30.12.18.
//

#include <visitor/base.h>

void Visitor::Base::visit(const Parser::Nodes::Program &node) {
    // todo how to get Base overload to work?
    visit(static_cast<const Parser::Nodes::Base&>(node));
}