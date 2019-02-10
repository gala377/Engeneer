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
        void visit(const Parser::Nodes::Base &node) override = 0;
        void visit(const Parser::Nodes::BaseParent& node) override;
        void visit(const Parser::Nodes::Program& node) override;
        void visit(const Parser::Nodes::VariableDecl& node) override;
        void visit(const Parser::Nodes::FunctionDecl& node) override;
        void visit(const Parser::Nodes::Statement &node) override;
        void visit(const Parser::Nodes::CodeBlock& node) override;

    };

}

#endif //TKOM2_BASE_H
