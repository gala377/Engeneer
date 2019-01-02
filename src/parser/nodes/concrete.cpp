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

//void Parser::Nodes::VarGlobalDecl::accept(Parser::Visitor &v) const {
//    v.visit(*this);
//    visit_children(v);
//}
//
//const std::string &Parser::Nodes::VarGlobalDecl::symbol() const {
//    return _symbol;
//}
//
//const Parser::Types::basic_t &Parser::Nodes::VarGlobalDecl::value() const {
//    return _value;
//}

void Parser::Nodes::VariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
    visit_children(v);
}

const std::string &Parser::Nodes::VariableDecl::symbol() const {
    return _symbol;
}

const std::string &Parser::Nodes::VariableDecl::type_symbol() const {
    return _type_symbol;
}

const Parser::Nodes::VariableDecl::Type &Parser::Nodes::VariableDecl::type() const {
    return _type;
}

Parser::Nodes::VariableDecl::VariableDecl(const std::string& symbol, const std::string& type_symbol):
    _symbol(symbol), _type_symbol(type_symbol) {
    if(_type_symbol == "int") {
        _type = Type::int_t;
    } else if(_type_symbol == "float") {
        _type = Type::float_t;
    } else if(_type_symbol == "string") {
        _type = Type::string_t;
    } else if(_type_symbol == "char") {
        _type = Type::char_t;
    } else if(_type_symbol == "bool") {
        _type = Type::bool_t;
    } else {
        _type = Type::struct_t;
    }
}
