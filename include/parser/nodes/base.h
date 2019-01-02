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

        using children_t = std::vector<std::unique_ptr<Base>>;

        virtual void add_child(Base* child);
        virtual void add_child(std::unique_ptr<Base>&& child);

        virtual const children_t& children() const;
        virtual const std::uint32_t& depth() const;


        // todo this kinda works, but i doubt it will in the long run
        virtual void accept(Parser::Visitor &v) const;
    protected:
        children_t _children;
        std::uint32_t _depth{0};

        virtual void visit_children(Parser::Visitor &v) const;
    };

}

#endif //TKOM2_BASE_H
