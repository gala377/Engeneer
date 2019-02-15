//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_LOG_H
#define TKOM2_LOG_H

#include <sstream>

#include <visitor/base.h>
#include <parser/nodes/concrete.h>

namespace Visitor {

    class Stringify: public Base {
    public:
        void visit(const Parser::Nodes::Base &node) override;
        void visit(const Parser::Nodes::BaseParent& node) override;
        void visit(const Parser::Nodes::Program& node) override;
        void visit(const Parser::Nodes::GlobVariableDecl& node) override;
        void visit(const Parser::Nodes::FunctionDecl &node) override;
        void visit(const Parser::Nodes::Statement &node) override;
        void visit(const Parser::Nodes::CodeBlock &node) override;
        void visit(const Parser::Nodes::VariableDecl &node) override;
        void visit(const Parser::Nodes::FunctionDef &node) override;

        std::string repr();

    protected:
        std::stringstream _stream;

        void add_margin(std::uint32_t depth);
        void stringify(const Parser::Nodes::Base& node, std::string&& as);
    };

}

#endif //TKOM2_LOG_H
