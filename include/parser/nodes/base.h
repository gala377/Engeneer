//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_PARSER_NODES_BASE_H
#define TKOM2_PARSER_NODES_BASE_H

#include <vector>
#include <memory>

#include <parser/visitor.h>
#include <lexer/token.h>

namespace Parser {
    class Visitor;
}

namespace Parser::Nodes {

    class Base {
    public:
        Base() = default;
        explicit Base(std::uint32_t depth);

        virtual const std::uint32_t& depth() const;
        virtual void set_depth(std::uint32_t depth);

        // todo this kinda works, but i doubt it will in the long run
        virtual void accept(Parser::Visitor &v) const;

    protected:
        std::uint32_t _depth{0};
    };

    class BaseParent: public Base {
    public:
        using Base::Base;

        using children_t = std::vector<std::unique_ptr<Base>>;

        virtual void add_child(Base *child);
        virtual void add_child(std::unique_ptr<Base> &&child);
        virtual const children_t& children() const;
        virtual std::unique_ptr<Base> take_child(children_t::const_iterator it);

        // todo this kinda works, but i doubt it will in the long run
        void accept(Parser::Visitor &v) const override;
    protected:
        children_t _children;
        virtual void visit_children(Parser::Visitor &v) const;
    };

    class BaseToken: public Base {
    public:
        BaseToken(Lexer::Token tok);

        const Lexer::Token& get_token() const;
        void set_token(const Lexer::Token &tok);

        void accept(Parser::Visitor &v) const override;

    protected:
        Lexer::Token _tok;
    };
}

#endif //TKOM2_BASE_H
