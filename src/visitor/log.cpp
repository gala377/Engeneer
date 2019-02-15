//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <visitor/log.h>

void Visitor::Stringify::visit(const Parser::Nodes::Base& node) {
    stringify(node, "Unknown node");
}

void Visitor::Stringify::visit(const Parser::Nodes::BaseParent& node) {
    stringify(node, "Unknown parent node");
}

void Visitor::Stringify::visit(const Parser::Nodes::Program& node) {
    stringify(node, "Program");
}

void Visitor::Stringify::visit(const Parser::Nodes::GlobVariableDecl &node) {
    stringify(node, "VarDecl: " + node.type_identifier + " " + node.identifier);
}

void Visitor::Stringify::visit(const Parser::Nodes::FunctionDecl &node) {
    std::string repr{"FuncDecl: " + node.type_identifier + " " + node.identifier + "("};
    for(const auto& arg: node.arg_list) {
        repr += arg->type_identifier + " " + arg->identifier + ", ";
    }
    repr += ")";
    stringify(node, std::move(repr));
}

void Visitor::Stringify::visit(const Parser::Nodes::Statement &node) {
    stringify(node, "Unknown statement node");
}

void Visitor::Stringify::visit(const Parser::Nodes::CodeBlock &node) {
    stringify(node, "CodeBlock");
}


std::string Visitor::Stringify::repr() {
    return _stream.str();
}

void Visitor::Stringify::stringify(const Parser::Nodes::Base &node, std::string &&as) {
    add_margin(node.depth());
    _stream << as << "\n";
}

void Visitor::Stringify::add_margin(std::uint32_t depth) {
    for(std::uint32_t i = 0; i < depth; ++i) {
        _stream << "--------";
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::VariableDecl &node) {
    stringify(node, "VarDecl: " + node.type_identifier + " " + node.identifier);
}

void Visitor::Stringify::visit(const Parser::Nodes::FunctionDef &node) {
    // todo, make this and then
    // todo make function definition parsing
    stringify(node, "FuncDef:");
}
