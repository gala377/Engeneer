//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/nodes/base.h>
#include <stdexcept>


//
//  Base
//

Parser::Nodes::Base::Base(std::uint32_t depth): _depth(depth), _span{std::nullopt} {}

Parser::Nodes::Base::Base(Lexer::Token::Span span): _depth(0), _span{std::move(span)} {}

Parser::Nodes::Base::Base(Lexer::Token::Span span, std::uint32_t depth): _depth(depth), _span{std::move(span)} {}

void Parser::Nodes::Base::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

const std::uint32_t &Parser::Nodes::Base::depth() const {
    return _depth;
}

void Parser::Nodes::Base::set_depth(std::uint32_t depth) {
    _depth = depth;
}

const Lexer::Token::Span& Parser::Nodes::Base::span() const {
    if(!_span) {
        throw std::runtime_error("Dereferencing emty span");
    }
    return *_span;
}

void Parser::Nodes::Base::set_span(Lexer::Token::Span span) {
    _span = std::optional{std::move(span)};
}

//
// BaseParent
//

void Parser::Nodes::BaseParent::add_child(std::unique_ptr<Base> &&child) {
    child->set_depth(_depth + 1);
    _children.emplace_back(std::move(child));
    if(_children.size() == 1) {
        set_span(_children[0]->span());
    }
}

const Parser::Nodes::BaseParent::children_t& Parser::Nodes::BaseParent::children() const {
    return _children;
}

void Parser::Nodes::BaseParent::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    for(auto& ch: _children) {
        ch->set_depth(_depth +1);
    }
}

void Parser::Nodes::BaseParent::accept(Parser::Visitor &v) const {
    v.visit(*this);
//    accept_children(v);
}

void Parser::Nodes::BaseParent::accept_children(Parser::Visitor &v) const {
    for(const auto& ch: _children) {
        ch->accept(v);
    }
}


