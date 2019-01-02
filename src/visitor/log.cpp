//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <visitor/log.h>

void Visitor::Stringify::visit(const Parser::Nodes::Base& node) {
    std::cout << "Unknown node strinfifying\n";
    stringify(node, "Unknown node");
}

void Visitor::Stringify::visit(const Parser::Nodes::Program& node) {
    std::cout << "Program node strinfifying\n";
    stringify(node, "Program");
}

std::string Visitor::Stringify::repr() {
    return _stream.str();
}

void Visitor::Stringify::stringify(const Parser::Nodes::Base &node, std::string &&as) {
    std::cout << "Adding margin\n";
    add_margin(node.depth());
    std::cout << "Writing to a stream\n";
    _stream << as << "\n";
}

void Visitor::Stringify::add_margin(std::uint32_t depth) {
    std::cout << "Going through depth\n";
    for(int i = 0; i < depth; ++i) {
        _stream << "\t";
    }
}
