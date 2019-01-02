//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_CONCRETE_H
#define TKOM2_CONCRETE_H

#include <variant>
#include <parser/nodes/base.h>
#include <parser/types/basic.hpp>

namespace Parser::Nodes {

    class Program: public BaseParent {
    public:
        using BaseParent::BaseParent;

        void accept(Parser::Visitor &v) const override;
    };

    class End: public Base {};

    class TopLevelDecl: public Base {};

    class VariableDecl: public TopLevelDecl {
    public:
        VariableDecl(const std::string& symbol, const std::string& type_symbol);

        std::string identifier;
        std::string type_identifier;

        void accept(Parser::Visitor &v) const override;
    };

    class FunctionDecl: public TopLevelDecl {
    public:
        FunctionDecl(
                const std::string &identifier,
                const std::string &type_identifier,
                const std::vector<VariableDecl> &arg_list);

        std::string identifier;
        std::string type_identifier;
        std::vector<VariableDecl> arg_list;

        void accept(Parser::Visitor &v) const override;
    };
}

#endif //TKOM2_CONCRETE_H
