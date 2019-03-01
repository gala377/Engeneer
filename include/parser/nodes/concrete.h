//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_CONCRETE_H
#define TKOM2_CONCRETE_H

#include <optional>

#include <parser/nodes/base.h>
#include <lexer/token.h>

namespace Parser::Nodes {

    // End
    class End: public Base {
    public:
        void accept(Parser::Visitor &v) const override;
    };


    // Program
    class Program: public BaseParent {
    public:
        using BaseParent::BaseParent;

        void accept(Parser::Visitor &v) const override;
    };


    // Top Level
    class TopLevelDecl: public Base {
    public:
        void accept(Parser::Visitor &v) const override;
    };

    class GlobVariableDecl: public TopLevelDecl {
    public:
        GlobVariableDecl(const std::string& symbol, const std::string& type_symbol);

        std::string identifier;
        std::string type_identifier;

        void accept(Parser::Visitor &v) const override;
    };

    // Function
    class FunctionDecl: public TopLevelDecl {
    public:
        void accept(Parser::Visitor &v) const override;
    };

    class FunctionProt: public FunctionDecl {
    public:
        FunctionProt(
                const std::string &identifier,
                const std::string &type_identifier,
                std::vector<std::unique_ptr<VariableDecl>> &&arg_list);

        std::string identifier;
        std::string type_identifier;
        std::vector<std::unique_ptr<VariableDecl>> arg_list;

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

    // struct
    class StructDecl: public TopLevelDecl {
    public:
        StructDecl(
            const std::string& identifier,
            std::vector<std::unique_ptr<VariableDecl>>&& members,
            std::vector<std::unique_ptr<FunctionDef>>&& methods,
            const std::optional<std::string>& wrapped_struct = std::nullopt);

        std::string identifier;
        std::vector<std::unique_ptr<VariableDecl>> members;
        std::vector<std::unique_ptr<FunctionDef>> methods;
        std::optional<std::string> wrapped_struct;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };


    // Statement
    class Statement: public Base {
    public:
        void accept(Parser::Visitor &v) const override;
    };

    class CodeBlock: public BaseParent {
    public:
        using BaseParent::BaseParent;

        void accept(Parser::Visitor &v) const override;
    };

    class VariableDecl: public Statement {
    public:
        VariableDecl(const std::string& symbol, const std::string& type_symbol);

        std::string identifier;
        std::string type_identifier;

        void accept(Parser::Visitor &v) const override;
    };


    // Expressions
    class Expression: public Statement {
    public:
        void accept(Parser::Visitor &v) const override;
    };


    //Binary
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

    // todo make constructors for binary
    // expr so they pass the operator by themselves
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


    // Unary Expr
    // todo each unary its own class?
    class UnaryExpr: public Expression {
    public:
        UnaryExpr(const Lexer::Token& op, std::unique_ptr<Expression>&& rhs);

        Lexer::Token op;
        std::unique_ptr<Expression> rhs;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class NegativeExpr: public UnaryExpr {
    public:
        NegativeExpr(std::unique_ptr<Expression>&& rhs);

        void accept(Parser::Visitor &v) const override;
    };


    // Postfix
    class PostfixExpr: public Expression {
        // todo finish postfix parsing and refactoring
    public:
        PostfixExpr(std::unique_ptr<Expression>&& lhs);

        std::unique_ptr<Expression> lhs;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class IndexExpr: public PostfixExpr {
    public:
        IndexExpr(
                std::unique_ptr<Expression>&& lhs,
                std::unique_ptr<Expression>&& index_expr);

        std::unique_ptr<Expression> index_expr;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class CallExpr: public PostfixExpr {
    public:
        CallExpr(
                std::unique_ptr<Expression>&& lhs,
                std::vector<std::unique_ptr<Expression>>&& args);

        std::vector<std::unique_ptr<Expression>> args;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    // todo should it be like that
    // and compiler says if something is wrong
    // or should we catch this on parsing stage?
    // Or should access be postfix like in c?
    class AccessExpr: public PostfixExpr {
    public:
        AccessExpr(
                std::unique_ptr<Expression>&& lhs,
                std::unique_ptr<Expression>&& rhs);

        std::unique_ptr<Expression> rhs;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    // Primary
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

    class ParenthesisExpr: public PrimaryExpr {
    public:
        explicit ParenthesisExpr(std::unique_ptr<Expression>&& expr);

        std::unique_ptr<Expression> expr;

        void set_depth(std::uint32_t depth) override;
        void accept(Parser::Visitor &v) const override;
    };


    // Consts
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

}

#endif //TKOM2_CONCRETE_H