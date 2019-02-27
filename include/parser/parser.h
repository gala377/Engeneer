//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_PARSER_H
#define TKOM2_PARSER_H

#include <memory>
#include <optional>

#include <lexer/lexer.h>

#include <parser/ast.h>
#include <parser/nodes/base.h>
#include <parser/nodes/concrete.h>
#include <exception/handler.h>
#include <exception/handling_mixin.h>

namespace Parser {

    class Parser: public Exception::HandlingMixin {
    public:
        Parser() = delete;
        explicit Parser(Lexer::Source::Base& s);
        explicit Parser(
                Lexer::Source::Base& s,
                Exception::Handler& excp_handler);

        AST parse();
    protected:
        Lexer::Lexer _lexer;
        // Parsers

        // End
        std::unique_ptr<Nodes::End> parse_end_of_file();
        // Top Level
        std::unique_ptr<Nodes::TopLevelDecl> parse_top_level_decl();
        std::unique_ptr<Nodes::GlobVariableDecl> parse_glob_var_decl();

        // Function
        std::unique_ptr<Nodes::FunctionDecl> parse_func_decl();
        std::unique_ptr<Nodes::FunctionProt> parse_func_header();

        // Statement
        std::unique_ptr<Nodes::Statement> parse_statement();
        std::unique_ptr<Nodes::CodeBlock> parse_code_block();
        std::unique_ptr<Nodes::VariableDecl> parse_var_decl();

        // Expressions
        std::unique_ptr<Nodes::Expression> parse_expr();

        // Binary
        std::unique_ptr<Nodes::AssignmentExpr> parse_assig_expr();
        std::unique_ptr<Nodes::AdditiveExpr> parse_add_expr();
        std::unique_ptr<Nodes::MultiplicativeExpr> parse_mult_expr();

        // Unary
        std::unique_ptr<Nodes::UnaryExpr> parse_unary_expr();
        std::unique_ptr<Nodes::NegativeExpr> parse_negative_expr();

        // Postfix
        std::unique_ptr<Nodes::PostfixExpr> parse_postfix_expr();
        std::unique_ptr<Nodes::CallExpr> parse_call_expr();
        std::unique_ptr<Nodes::IndexExpr> parse_index_expr();
        std::unique_ptr<Nodes::AccessExpr> parse_access_expr();

        // Primary
        std::unique_ptr<Nodes::PrimaryExpr> parse_prim_expr();
        std::unique_ptr<Nodes::Identifier> parse_ident();
        std::unique_ptr<Nodes::ParenthesisExpr> parse_parenthesis();

        // Const
        std::unique_ptr<Nodes::Constant> parse_const();
        std::unique_ptr<Nodes::IntConstant> parse_int();
        std::unique_ptr<Nodes::StringConstant> parse_string();


        // helper parsers
        using arg_list_t = std::vector<std::unique_ptr<Nodes::GlobVariableDecl>>;
        arg_list_t parse_func_arg_list();

        std::unique_ptr<Nodes::AdditiveExpr> parse_single_add_expr();
        std::unique_ptr<Nodes::MultiplicativeExpr> parse_single_mult_expr();

        // Token parsers
        std::optional<Lexer::Token> parse_token(Lexer::Token::Id id);
        std::optional<std::string> parse_type();
        std::function<std::optional<Lexer::Token>(Parser*)> make_tok_parser(Lexer::Token::Id id);

        // Helper functions
        template<typename Ret, typename ...Ts>
        std::unique_ptr<Ret> one_of(Ts &&... ts) {
            std::unique_ptr<Ret> res = nullptr;
            ( run_parser<std::unique_ptr<Ret>>(ts, res) || ...);
            return std::move(res);
        }

        template<typename Ret, typename ...Ts>
        std::optional<Ret> one_of_op(Ts &&... ts) {
            std::optional<Ret> res = std::nullopt;
            ( run_parser<std::optional<Ret>>(ts, res) || ...);
            return std::move(res);
        }

        template<typename Ret, typename F>
        bool run_parser(F f, Ret& result) {
            result = std::move(std::bind(f, this)());
            return (bool)result;
        }

        template<typename Parser, typename Func>
        void fold(Parser&& p, Func&& f) {
            auto wrap_p = std::move(std::bind(p, this));
            for(auto res = wrap_p(); res; res = wrap_p()) {
                f(std::move(res));
            }
        }

        template<typename Parser, typename Func>
        void parse_or(Parser p, Func f) {
            auto wrap_p = std::move(std::bind(p, this));
            if(auto res = wrap_p(); !res) {
                f(std::move(res));
            }
        }
    };

}

#endif //TKOM2_PARSER_H
