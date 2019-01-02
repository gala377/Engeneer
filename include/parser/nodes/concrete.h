//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_CONCRETE_H
#define TKOM2_CONCRETE_H

#include <variant>
#include <parser/nodes/base.h>
#include <parser/types/basic.hpp>

namespace Parser::Nodes {

    class Program: public Base {};

    class End: public Base {};

    class TopLevelDecl: public Base {};

    class VarGlobalDecl: public TopLevelDecl {

    };

    class FuncDeclaration: public TopLevelDecl {
        
    };

    class VariableDecl: public Base {
    public:
        enum class Type {
            int_t, float_t, string_t, char_t, struct_t
        };
        std::string symbol; 
        Type type; 
        Types::basic_t value; 
    };
}

#endif //TKOM2_CONCRETE_H
