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

void Parser::Nodes::FunctionDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

Parser::Nodes::FunctionDecl::FunctionDecl(
        const std::string &identifier,
        const std::string &type_identifier,
        std::vector<std::unique_ptr<Parser::Nodes::GlobVariableDecl>> &&arg_list) :
        identifier(identifier), type_identifier(type_identifier), arg_list(std::move(arg_list)) {}



/*
 *  CodeBlock
 */

void Parser::Nodes::CodeBlock::accept(Parser::Visitor &v) const {
    v.visit(*this);
    visit_children(v);
}


/*
 * FunctionDef
 */

Parser::Nodes::FunctionDef::FunctionDef(
        std::unique_ptr<Parser::Nodes::FunctionDecl> &&decl,
        std::unique_ptr<Parser::Nodes::CodeBlock> &&body) :
        declaration(std::move(decl)), body(std::move(body)) {}

void Parser::Nodes::FunctionDef::accept(Parser::Visitor &v) const {
    v.visit(*this);
    // todo: should I also visit FuncDecl and CodeBlock
    // todo: or should visitor deal with it itself
    v.visit(*declaration);
    v.visit(*body);
}