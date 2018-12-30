//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_AST_H
#define TKOM2_AST_H

#include <parser/nodes/base.h>
#include <parser/visitor.h>

namespace Parser {

    class AST {
    public:
        AST();

        void visit(Visitor& v);
    private:
        Nodes::Base _root;
    };

}

#endif //TKOM2_AST_H
