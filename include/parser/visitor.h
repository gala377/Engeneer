//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_VISITATOR_H
#define TKOM2_VISITATOR_H

#include <parser/nodes/base.h>

namespace Parser {

    namespace Nodes {
        class Base;
        class BaseParent;
        class Program;
        class VariableDecl;
        class FunctionDecl;
    }

    class Visitor {
    public:
        virtual void visit(const Nodes::Base& node) = 0;
        virtual void visit(const Nodes::BaseParent& node) = 0;
        virtual void visit(const Nodes::Program& node) = 0;
        virtual void visit(const Nodes::VariableDecl& node) = 0;
        virtual void visit(const Nodes::FunctionDecl& node) = 0;
    };
}

#endif //TKOM2_VISITATOR_H
