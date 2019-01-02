//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/nodes/base.h>

Parser::Nodes::Base::Base(std::uint32_t depth): _depth(depth) {}

void Parser::Nodes::Base::add_child(Nodes::Base *child) {
    add_child(std::unique_ptr<Base>(child));
}

void Parser::Nodes::Base::add_child(std::unique_ptr<Nodes::Base>&& child) {
    child->_depth = _depth + 1;
    _children.emplace_back(std::move(child));
}

const Parser::Nodes::Base::children_t& Parser::Nodes::Base::children() const {
    return _children;
}

const std::uint32_t &Parser::Nodes::Base::depth() const {
    return _depth;
}

void Parser::Nodes::Base::accept(Parser::Visitor &v) const {

    v.visit(*this);

    visit_children(v);
}

void Parser::Nodes::Base::visit_children(Parser::Visitor &v) const {
    for(const auto& ch: _children) {
        ch->accept(v);
    }
}
