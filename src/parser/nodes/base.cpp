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
