//
// Created by igor on 30.12.18.
//

#include <parser/nodes/concrete.h>


/*
 *  Program
 */

void Parser::Nodes::Program::accept(Parser::Visitor &v) const {
    v.visit(*this);
    visit_children(v);
}


/*
 * VariableDecl
 */

void Parser::Nodes::VariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::VariableDecl::VariableDecl(
        const std::string& symbol,
        const std::string& type_symbol):
        identifier(symbol), type_identifier(type_symbol) {}


/*
 *  FunctionDecl
 */

void Parser::Nodes::FunctionDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::FunctionDecl::FunctionDecl(
        const std::string &identifier,
        const std::string &type_identifier,
        const std::vector<Parser::Nodes::VariableDecl> &arg_list) :
        identifier(identifier), type_identifier(type_identifier), arg_list(arg_list) {}
