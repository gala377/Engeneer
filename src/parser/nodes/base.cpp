//
// Created by igor on 30.12.18.
//

#include <parser/nodes/base.h>

void Parser::Nodes::Base::add_child(Nodes::Base *child) {
    _children.emplace_back(
            std::unique_ptr<Base>(child));
}

void Parser::Nodes::Base::add_child(std::unique_ptr<Nodes::Base>&& child) {
    _children.emplace_back(std::move(child));
}

const Parser::Nodes::Base::children_t& Parser::Nodes::Base::children() const {
    return _children;
}

void Parser::Nodes::Base::visit(Parser::Visitor &v) const {
    v.visit(*this);
    for(const auto& ch: _children) {
        v.visit(*ch);
    }
}
