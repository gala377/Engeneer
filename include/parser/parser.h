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

namespace Parser {

    class Parser {
    public:
        Parser() = delete;
        explicit Parser(Lexer::Source::Base& s);

        AST parse();
    private:
        Lexer::Lexer _lexer;

        // Syntax parsers
        std::unique_ptr<Nodes::TopLevelDecl> parse_top_level_decl();
        std::unique_ptr<Nodes::End> parse_end_of_file();
        std::unique_ptr<Nodes::GlobVariableDecl> parse_glob_var_decl();
        
        std::unique_ptr<Nodes::FunctionDecl> parse_func_decl();
        std::unique_ptr<Nodes::FunctionHeader> parse_func_header();

        std::unique_ptr<Nodes::CodeBlock> parse_code_block();
        std::unique_ptr<Nodes::Statement> parse_statement();
        std::unique_ptr<Nodes::VariableDecl> parse_var_decl();


        // Expressions
        std::unique_ptr<Nodes::Expression> parse_expr();
        std::unique_ptr<Nodes::AssignmentExpr> parse_assig_expr();
        std::unique_ptr<Nodes::AdditiveExpr> parse_add_expr();
        std::unique_ptr<Nodes::MultiplicativeExpr> parse_mult_expr();

        // todo parse unary

        std::unique_ptr<Nodes::PrimaryExpr> parse_prim_expr();
        std::unique_ptr<Nodes::Identifier> parse_ident();
        std::unique_ptr<Nodes::Constant> parse_const();
        std::unique_ptr<Nodes::IntConstant> parse_int();
        std::unique_ptr<Nodes::StringConstant> parse_string();
        std::unique_ptr<Nodes::ParenthesisExpr> parse_parenthesis();

        // helper parsers
        using arg_list_t = std::vector<std::unique_ptr<Nodes::GlobVariableDecl>>;
        arg_list_t parse_func_arg_list();


        // Token parsers
        std::optional<Lexer::Token> parse_token(Lexer::Token::Id id);
        std::optional<std::string> parse_type();

        // Helper functions
        template<typename Ret, typename ...Ts>
        std::unique_ptr<Ret> one_of(Ts &&... ts) {
            std::unique_ptr<Ret> res = nullptr;
            ( run_parser<Ret>(ts, res) || ...);
            return std::move(res);
        }

        template<typename Ret, typename F>
        bool run_parser(F f, std::unique_ptr<Ret> &result) {
            result = std::move(std::bind(f, this)());
            return (bool)result;
        }
    };

}

#endif //TKOM2_PARSER_H
