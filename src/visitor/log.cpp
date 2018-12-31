//
// Created by igor on 30.12.18.
//

#include <visitor/log.h>

void Visitor::Stringify::visit(const Parser::Nodes::Base& node) {
    stringify(node, "Unknown node");
}

void Visitor::Stringify::visit(const Parser::Nodes::Program& node) {
    stringify(node, "Program");
}

std::string Visitor::Stringify::repr() {
    return _stream.str();
}

void Visitor::Stringify::stringify(const Parser::Nodes::Base &node, std::string &&as) {
    add_margin(node.depth());
    _stream << as << "\n";
}

void Visitor::Stringify::add_margin(std::uint32_t depth) {
    for(int i = 0; i < depth; ++i) {
        _stream << "\t";
    }
}
