//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_PARSER_NODES_BASE_H
#define TKOM2_PARSER_NODES_BASE_H

#include <vector>
#include <memory>
#include <optional>

#include <parser/visitor.h>
#include <lexer/token.h>

namespace Parser {
    class Visitor;
}

namespace Parser::Nodes {

    class Base {
    public:
        Base() = default;
        virtual ~Base() = default;
        
        explicit Base(std::uint32_t depth);
        explicit Base(Lexer::Token::Span span);
        Base(Lexer::Token::Span span, std::uint32_t depth);

        virtual const std::uint32_t& depth() const;
        virtual void set_depth(std::uint32_t depth);

        virtual const Lexer::Token::Span& span() const; 
        virtual void set_span(Lexer::Token::Span span);

        virtual void accept(Parser::Visitor &v) const;

    protected:
        std::uint32_t _depth{0};
        std::optional<Lexer::Token::Span> _span;
    };

    class BaseParent: public Base {
    public:
        using Base::Base;

        using child_t = Nodes::Base;
        using children_t = std::vector<std::unique_ptr<child_t>>;

        virtual void add_child(std::unique_ptr<child_t> &&child);

        virtual const children_t &children() const;

        void set_depth(std::uint32_t depth) override;
        void accept(Parser::Visitor &v) const override;

        virtual void accept_children(Parser::Visitor &v) const;
    protected:
        children_t _children;

    };
}

#endif //TKOM2_BASE_H
