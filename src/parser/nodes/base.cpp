//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/nodes/base.h>


//
//  Base
//

Parser::Nodes::Base::Base(std::uint32_t depth): _depth(depth) {}

void Parser::Nodes::Base::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

const std::uint32_t &Parser::Nodes::Base::depth() const {
    return _depth;
}

void Parser::Nodes::Base::set_depth(std::uint32_t depth) {
    _depth = depth;
}

//
// BaseParent
//

void Parser::Nodes::BaseParent::add_child(Base *child) {
    add_child(std::unique_ptr<Base>(child));
}

void Parser::Nodes::BaseParent::add_child(std::unique_ptr<Base> &&child) {
    child->set_depth(_depth + 1);
    _children.emplace_back(std::move(child));
}

const Parser::Nodes::BaseParent::children_t& Parser::Nodes::BaseParent::children() const {
    return _children;
}

void Parser::Nodes::BaseParent::accept(Parser::Visitor &v) const {
    v.visit(*this);
    visit_children(v);
}

void Parser::Nodes::BaseParent::visit_children(Parser::Visitor &v) const {
    for(const auto& ch: _children) {
        ch->accept(v);
    }
}

std::unique_ptr<Parser::Nodes::Base> Parser::Nodes::BaseParent::take_child(Parser::Nodes::BaseParent::children_t::const_iterator it) {
    auto res = std::move(_children.at(it - _children.begin()));
    _children.erase(it);
    return res;
}

//
// BaseToken
//


Parser::Nodes::BaseToken::BaseToken(Lexer::Token tok): Base(0), _tok(std::move(tok)) {}

const Lexer::Token &Parser::Nodes::BaseToken::get_token() const {
    return _tok;
}

void Parser::Nodes::BaseToken::set_token(const Lexer::Token &tok) {
    _tok = tok;
}

void Parser::Nodes::BaseToken::accept(Parser::Visitor &v) const {
    v.visit(*this);
}
