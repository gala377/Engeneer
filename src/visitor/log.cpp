//
// Created by igor on 30.12.18.
//

#include <visitor/log.h>

void Visitor::Log::visit(const Parser::Nodes::Base& node) {
    _stream << "Unknown node\n";
}

void Visitor::Log::visit(const Parser::Nodes::Program& node) {
    _stream << "Program\n";
}

std::string Visitor::Log::repr() {
    return _stream.str();
}
