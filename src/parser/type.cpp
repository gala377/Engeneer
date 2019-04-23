//
// Created by rafalgal on 07.03.19.
//

#include "parser/nodes/concrete.h"
#include "parser/visitor.h"
#include <memory>
#include <parser/type.h>
#include <visitor/base.h>

const Parser::Nodes::Identifier &Parser::Types::SimpleType::identifier() const {
    if(!ident) {
        throw std::runtime_error("Dereference a null identifier of a type");
    }
    return *ident;
}

std::unique_ptr<Parser::Types::BaseType> Parser::Types::SimpleType::copy() const {
    return std::make_unique<SimpleType>(
        std::make_unique<Nodes::Identifier>(ident->symbol));
}


const Parser::Nodes::Identifier &Parser::Types::ComplexType::identifier() const {
    // todo really doesn't make any sense
    return underlying_type->identifier();
}

Parser::Types::ComplexType::ComplexType(
        bool is_const,
        bool is_ptr,
        std::unique_ptr<BaseType>&& to_type): 
    BaseType(),
    is_const{is_const},
    is_ptr{is_ptr},
    underlying_type{std::move(to_type)} {}

std::unique_ptr<Parser::Types::BaseType> Parser::Types::ComplexType::copy() const {
    return std::make_unique<ComplexType>(
        is_const,
        is_ptr,
        underlying_type->copy());
}

Parser::Types::SimpleType::SimpleType(std::unique_ptr<Parser::Nodes::Identifier> &&ident):
    ident(std::move(ident)) {}

const Parser::Nodes::Identifier &Parser::Types::ArrayType::identifier() const {
    // todo really doesn't make any sense
    return underlying_type->identifier();
}

std::unique_ptr<Parser::Types::BaseType> Parser::Types::ArrayType::copy() const {
    return std::make_unique<ArrayType>(
        size,
        underlying_type->copy());
}

Parser::Types::ArrayType::ArrayType(
    const uint32_t &size,
    std::unique_ptr<Parser::Types::BaseType> &&underlying_type):
    size(size),
    underlying_type(std::move(underlying_type)) {}

const Parser::Nodes::Identifier &Parser::Types::FunctionType::identifier() const {
    // todo really doesn't make any sense
    return return_type->identifier();
}

Parser::Types::FunctionType::FunctionType(
        std::unique_ptr<Parser::Types::BaseType> return_t,
        std::vector<std::unique_ptr<Parser::Types::BaseType>> arg_t):
        return_type(std::move(return_t)), argument_types(std::move(arg_t)) {}

std::unique_ptr<Parser::Types::BaseType> Parser::Types::FunctionType::copy() const {
    std::vector<std::unique_ptr<BaseType>> arg_t;
    for(auto& arg: argument_types) {
        arg_t.emplace_back(arg->copy());
    }
    return std::make_unique<FunctionType>(
        return_type->copy(),
        std::move(arg_t));
}