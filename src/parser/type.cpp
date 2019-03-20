//
// Created by rafalgal on 07.03.19.
//

#include <parser/type.h>
#include <visitor/base.h>

const Parser::Nodes::Identifier &Parser::Types::SimpleType::identifier() const {
    if(!ident) {
        throw std::runtime_error("Dereference a null identifier of a type");
    }
    return *ident;
}

const Parser::Nodes::Identifier &Parser::Types::ComplexType::identifier() const {
    return underlying_type->identifier();
}

Parser::Types::SimpleType::SimpleType(std::unique_ptr<Parser::Nodes::Identifier> &&ident):
    ident(std::move(ident)) {}

const Parser::Nodes::Identifier &Parser::Types::ArrayType::identifier() const {
    return underlying_type->identifier();
}

Parser::Types::ArrayType::ArrayType(
    const uint32_t &size,
    std::unique_ptr<Parser::Types::BaseType> &&underlying_type):
    size(size),
    underlying_type(std::move(underlying_type)) {}
