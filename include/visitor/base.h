//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_BASE_H
#define TKOM2_BASE_H

#include <parser/visitor.h>
#include <parser/nodes/concrete.h>

namespace Visitor {

    class Base: public Parser::Visitor {
    public:
        virtual void visit(const Parser::Nodes::Base& node) override = 0;
        virtual void visit(const Parser::Nodes::Program& node) override;
    };

}

#endif //TKOM2_BASE_H
