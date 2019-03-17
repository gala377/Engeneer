//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_CONCRETE_H
#define TKOM2_CONCRETE_H

#include <optional>

#include <parser/nodes/base.h>
#include <lexer/token.h>

namespace Parser::Types {
    class BaseType {
    public:
        virtual const Nodes::Identifier& identifier() const = 0;
    };
}

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
        GlobVariableDecl(
            std::unique_ptr<Identifier>&& identifier,
            std::unique_ptr<Types::BaseType> &&type,
            std::unique_ptr<Expression>&& init_expr = nullptr);

        std::unique_ptr<Identifier> identifier;
        std::unique_ptr<Types::BaseType> type;
        std::unique_ptr<Expression> init_expr;

        void set_depth(std::uint32_t depth) override;

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
                std::unique_ptr<Identifier>&& identifier,
                std::unique_ptr<Types::BaseType>&& type,
                std::vector<std::unique_ptr<VariableDecl>> &&arg_list);

        std::unique_ptr<Identifier> identifier;;
        std::unique_ptr<Types::BaseType> type;
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
            std::unique_ptr<Identifier>&& identifier,
            std::vector<std::unique_ptr<VariableDecl>>&& members,
            std::vector<std::unique_ptr<FunctionDecl>>&& methods,
            std::vector<std::unique_ptr<Identifier>>&& wrapped_structs);

        std::unique_ptr<Identifier> identifier;;
        std::vector<std::unique_ptr<VariableDecl>> members;
        std::vector<std::unique_ptr<FunctionDecl>> methods;
        std::vector<std::unique_ptr<Identifier>> wrapped_structs;

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
        VariableDecl(
            std::unique_ptr<Identifier>&& identifier,
            std::unique_ptr<Types::BaseType> &&type,
            std::unique_ptr<Expression>&& init_expr = nullptr);

        std::unique_ptr<Identifier> identifier;
        std::unique_ptr<Types::BaseType> type;
        std::unique_ptr<Expression> init_expr;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class BlockStmt: public Statement {
    public:
        explicit BlockStmt(std::unique_ptr<CodeBlock>&& body);

        std::unique_ptr<CodeBlock> body;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class IfStmt: public BlockStmt {
    public:
        IfStmt(
            std::unique_ptr<Expression>&& cond,
            std::unique_ptr<CodeBlock>&& body,
            std::unique_ptr<BlockStmt>&& else_clause = nullptr);

        std::unique_ptr<Expression> cond;
        std::unique_ptr<BlockStmt> else_clause;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class WhileStmt: public BlockStmt {
    public:
        WhileStmt(
            std::unique_ptr<Expression>&& cond,
            std::unique_ptr<CodeBlock>&& body);

        std::unique_ptr<Expression> cond;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class ReturnStmt: public Statement {
    public:
        explicit ReturnStmt(std::unique_ptr<Expression>&& expr);

        std::unique_ptr<Expression> expr;

        void set_depth(std::uint32_t depth) override;

        void accept(Parser::Visitor &v) const override;
    };

    class BreakStmt: public Statement {
    public:
        void accept(Parser::Visitor &v) const override;
    };

    class ContinueStmt: public Statement {
    public:
        void accept(Parser::Visitor& v) const override;
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

    // logical
    class LogicalOrExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor& v) const override;
    };

    class LogicalAndExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor& v) const override;
    };

    class InclusiveOrExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor &v) const override;
    };

    class ExclusiveOrExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor& v) const override;
    };

    class AndExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor &v) const override;
    };

    class EqualityExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor& v) const override;
    };

    class RelationalExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor& v) const override;
    };

    class ShiftExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor& v) const override;
    };

    class AssignmentExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor &v) const override;
    };

    class AdditiveExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor &v) const override;
    };

    class MultiplicativeExpr: public BinaryExpr {
    public:
        using BinaryExpr::BinaryExpr;

        void accept(Parser::Visitor &v) const override;
    };


    // Unary Expr
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
        NegativeExpr(const Lexer::Token& op, std::unique_ptr<Expression>&& rhs);

        void accept(Parser::Visitor &v) const override;
    };

    class NegationExpr: public UnaryExpr {
    public:
        NegationExpr(const Lexer::Token& op, std::unique_ptr<Expression>&& rhs);

        void accept(Parser::Visitor& v) const override;
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