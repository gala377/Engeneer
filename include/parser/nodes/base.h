//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_PARSER_NODES_BASE_H
#define TKOM2_PARSER_NODES_BASE_H

#include <vector>
#include <memory>

#include <parser/visitor.h>

namespace Parser {
    class Visitor;
}

namespace Parser::Nodes {

    class Base {
    public:
        Base() = default;
        explicit Base(std::uint32_t depth);

        virtual const std::uint32_t& depth() const;
        virtual void set_depth(std::uint32_t depth);

        // todo this kinda works, but i doubt it will in the long run
        virtual void accept(Parser::Visitor &v) const;

    protected:
        std::uint32_t _depth{0};
    };

    class BaseParent: public Base {
    public:
        using Base::Base;

        using child_t = Nodes::Base;
        using children_t = std::vector<std::unique_ptr<child_t>>;

        virtual void add_child(std::unique_ptr<child_t> &&child);

        virtual const children_t &children() const;

        void accept(Parser::Visitor &v) const override;

    protected:
        children_t _children;

        virtual void visit_children(Parser::Visitor &v) const;
    };
}

#endif //TKOM2_BASE_H
