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
        virtual void visit(const Parser::Nodes::Base& node) override;
        virtual void visit(const Parser::Nodes::Program& node) override;

        std::string repr();

    private:
        std::stringstream _stream;

        void add_margin(std::uint32_t depth);
        void stringify(const Parser::Nodes::Base& node, std::string&& as);
    };

}

#endif //TKOM2_LOG_H
