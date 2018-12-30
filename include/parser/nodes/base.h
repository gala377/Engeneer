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
        using children_t = std::vector<std::unique_ptr<Base>>;

        virtual void add_child(Base* child);
        virtual void add_child(std::unique_ptr<Base>&& child);

        virtual const children_t& children() const;

        // todo this kinda works, but i doubt it will in the long run
        virtual void visit(Parser::Visitor& v) const;
    protected:
        children_t _children;
    };

}

#endif //TKOM2_BASE_H
