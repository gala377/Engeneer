//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_LOG_H
#define TKOM2_LOG_H

#include <sstream>

#include <visitor/base.h>
#include <parser/nodes/concrete.h>

namespace Visitor {

    class Log: public Base {
    public:
        virtual void visit(const Parser::Nodes::Base& node) override;
        virtual void visit(const Parser::Nodes::Program& node) override;

        std::string repr();

    private:
        std::stringstream _stream;
    };

}

#endif //TKOM2_LOG_H
