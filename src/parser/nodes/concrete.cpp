//
// Created by igor on 30.12.18.
//

#include <parser/nodes/concrete.h>
#include <iostream>

/*
 *  Program
 */

void Parser::Nodes::Program::accept(Parser::Visitor &v) const {
    v.visit(*this);
    visit_children(v);
}

/*
 *  CodeBlock
 */

void Parser::Nodes::CodeBlock::accept(Parser::Visitor &v) const {
    v.visit(*this);
    visit_children(v);
}


/*
 * GlobVariableDecl
 */

void Parser::Nodes::GlobVariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::GlobVariableDecl::GlobVariableDecl(
        const std::string& symbol,
        const std::string& type_symbol):
        identifier(symbol), type_identifier(type_symbol) {}

/*
 * VariableDecl
 */

Parser::Nodes::VariableDecl::VariableDecl(
        const std::string& symbol,
        const std::string& type_symbol):
        identifier(symbol), type_identifier(type_symbol) {}

void Parser::Nodes::VariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

/*
 *  FunctionDecl
 */

void Parser::Nodes::FunctionHeader::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::FunctionHeader::FunctionHeader(
        const std::string &identifier,
        const std::string &type_identifier,
        std::vector<std::unique_ptr<Parser::Nodes::GlobVariableDecl>> &&arg_list) :
        identifier(identifier), type_identifier(type_identifier), arg_list(std::move(arg_list)) {}


/*
 * FunctionDef
 */

Parser::Nodes::FunctionDef::FunctionDef(
        std::unique_ptr<Parser::Nodes::FunctionHeader> &&decl,
        std::unique_ptr<Parser::Nodes::CodeBlock> &&body) :
        declaration(std::move(decl)), body(std::move(body)) {
    this->declaration->set_depth(_depth+1);
    this->body->set_depth(_depth+1);
}

void Parser::Nodes::FunctionDef::set_depth(std::uint32_t depth) {
    FunctionDecl::set_depth(depth);
    declaration->set_depth(_depth+1);
    body->set_depth(_depth+1);
}

void Parser::Nodes::FunctionDef::accept(Parser::Visitor &v) const {
    v.visit(*this);
    declaration->accept(v);
    body->accept(v);
}