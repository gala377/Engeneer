//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_CONCRETE_H
#define TKOM2_CONCRETE_H

#include <variant>
#include <parser/nodes/base.h>
#include <parser/types/basic.hpp>

namespace Parser::Nodes {

    class TopLevelDecl: public Base {};

    class End: public Base {};

    class Program: public BaseParent {
    public:
        using BaseParent::BaseParent;

        void accept(Parser::Visitor &v) const override;
    };

    class Statement: public Base {
    public:
        Statement() = default;
    };

    class GlobVariableDecl: public TopLevelDecl {
    public:
        GlobVariableDecl(const std::string& symbol, const std::string& type_symbol);

        std::string identifier;
        std::string type_identifier;

        void accept(Parser::Visitor &v) const override;
    };

    class VariableDecl: public Statement {
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
                std::vector<std::unique_ptr<GlobVariableDecl>> &&arg_list);

        std::string identifier;
        std::string type_identifier;
        std::vector<std::unique_ptr<GlobVariableDecl>> arg_list;

        void accept(Parser::Visitor &v) const override;
    };

    class CodeBlock: public BaseParent {
    public:
        using BaseParent::BaseParent;

        void accept(Parser::Visitor &v) const override;
    };

    class FunctionDef: public TopLevelDecl {
    public:
        FunctionDef(std::unique_ptr<FunctionDecl>&& decl, std::unique_ptr<CodeBlock>&& body);

        std::unique_ptr<FunctionDecl> declaration;
        std::unique_ptr<CodeBlock> body;

        void accept(Parser::Visitor &v) const override;
    };
}

#endif //TKOM2_CONCRETE_H
