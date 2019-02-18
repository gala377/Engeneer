//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_CONCRETE_H
#define TKOM2_CONCRETE_H

#include <variant>
#include <parser/nodes/base.h>
#include <parser/types/basic.hpp>
#include <lexer/token.h>

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

    class FunctionDecl: public TopLevelDecl {};

    class FunctionProt: public FunctionDecl {
    public:
        FunctionProt(
                const std::string &identifier,
                const std::string &type_identifier,
                std::vector<std::unique_ptr<GlobVariableDecl>> &&arg_list);

        std::string identifier;
        std::string type_identifier;
        // todo change to VariableDecl its local not global
        std::vector<std::unique_ptr<GlobVariableDecl>> arg_list;

        void accept(Parser::Visitor &v) const override;
    };

    class CodeBlock: public BaseParent {
    public:
        using BaseParent::BaseParent;

        void accept(Parser::Visitor &v) const override;
    };

    class FunctionDef: public FunctionDecl {
    public:
        FunctionDef(std::unique_ptr<FunctionProt>&& decl, std::unique_ptr<CodeBlock>&& body);

        std::unique_ptr<FunctionProt> declaration;
        std::unique_ptr<CodeBlock> body;

        void set_depth(std::uint32_t depth) override;
        void accept(Parser::Visitor &v) const override;
    };

    //
    // Expressions
    //

    class Expression: public Statement {
    public:
        void accept(Parser::Visitor &v) const override;
    };

    class BinaryExpr: public Expression {
    public:
        BinaryExpr(
                std::unique_ptr<Expression>&& lhs,
                const Lexer::Token& op,
                std::unique_ptr<Expression>&& rhs);

        std::unique_ptr<Expression> lhs;
        std::unique_ptr<Expression> rhs;
        Lexer::Token op;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class AssignmentExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor &v) const override;
    };

    // todo should + and - be the same
    // or should they be subclasses of AdditiveExpr?
    class AdditiveExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor &v) const override;
    };

    // todo division and multiplication subclasses?
    class MultiplicativeExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor &v) const override;
    };

    // todo each unary its own class?
    class UnaryExpr: public Expression {
    public:
        UnaryExpr(const Lexer::Token& op, std::unique_ptr<Expression>&& rhs);

        Lexer::Token op;
        std::unique_ptr<Expression> rhs;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    // todo
    // A postfix can be a function call ()
    // or a ++ -- [] (indexing)
    // or access (. ->) so more though is needed here
    // for now no postfix
    //    class PostfixExpr: public Expression{
    //    public:
    //        void accept(Parser::Visitor &v) const override;
    //    };

    class PrimaryExpr: public Expression {
    public:
        void accept(Parser::Visitor &v) const override;
    };

    class Identifier: public PrimaryExpr {
    public:
        explicit Identifier(const std::string& symbol);

        std::string symbol;

        void accept(Parser::Visitor &v) const override;
    };

    class Constant: public PrimaryExpr {
    public:
        void accept(Parser::Visitor &v) const override;
    };

    // todo should I actually have o value here?
    // or maybe just a symbol
    class IntConstant: public Constant {
    public:
        explicit IntConstant(int value);

        int value;

        void accept(Parser::Visitor &v) const override;
    };

    class StringConstant: public Constant {
    public:
        explicit StringConstant(const std::string& value);

        std::string value;

        void accept(Parser::Visitor &v) const override;
    };

    class ParenthesisExpr: public PrimaryExpr {
    public:
        explicit ParenthesisExpr(std::unique_ptr<Expression>&& expr);

        std::unique_ptr<Expression> expr;

        void set_depth(std::uint32_t depth) override;
        void accept(Parser::Visitor &v) const override;
    };

}

#endif //TKOM2_CONCRETE_H
