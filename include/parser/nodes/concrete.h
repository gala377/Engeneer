//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_CONCRETE_H
#define TKOM2_CONCRETE_H

#include <variant>
#include <parser/nodes/base.h>
#include <parser/types/basic.hpp>

namespace Parser::Nodes {

    class Program: public Base {
    public:
        using Base::Base;

        void accept(Parser::Visitor &v) const override;
    };

    class End: public Base {};

    class TopLevelDecl: public Base {};

    class VarGlobalDecl: public TopLevelDecl {
    public:
        void accept(Parser::Visitor &v) const override;

        const std::string& symbol() const;
        const Types::basic_t& value() const;

    protected:
        std::string _symbol;
        Types::basic_t _value;
    };

    class FuncDeclaration: public TopLevelDecl {
        
    };

    class VariableDecl: public Base {
    public:
        VariableDecl(const std::string& symbol, const std::string& type_symbol);

        enum class Type {
            int_t, bool_t, float_t, string_t, char_t, struct_t
        };

        void accept(Parser::Visitor &v) const override;

        const std::string& symbol() const;
        const std::string& type_symbol() const;
        const Type& type() const;

    protected:
        std::string _symbol;
        std::string _type_symbol;
        Type _type;
    };
}

#endif //TKOM2_CONCRETE_H
