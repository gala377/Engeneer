//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_PARSER_H
#define TKOM2_PARSER_H

#include <memory>
#include <optional>

#include <lexer/lexer.h>

#include <parser/ast.h>
#include <parser/type.h>
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

        std::vector<Nodes::FunctionProt*> _function_protos;
        std::vector<Nodes::FunctionDef*> _function_defs;
        std::vector<Nodes::GlobVariableDecl*> _glob_var_decls;
        std::vector<Nodes::StructDecl*> _structs_decls;

        void initialize_ast(AST& ast) const;
        // Parsers

        // End
        std::unique_ptr<Nodes::End> parse_end_of_file();
        // Top Level
        std::unique_ptr<Nodes::TopLevelDecl> parse_top_level_decl();
        std::unique_ptr<Nodes::GlobVariableDecl> parse_glob_var_decl();
        std::unique_ptr<Nodes::StructDecl> parse_struct_decl();

        // Function
        std::unique_ptr<Nodes::FunctionDecl> parse_func_decl();
        std::unique_ptr<Nodes::FunctionProt> parse_func_header();

        // Statement
        std::unique_ptr<Nodes::Statement> parse_statement();
        std::unique_ptr<Nodes::CodeBlock> parse_code_block();
        std::unique_ptr<Nodes::VariableDecl> parse_var_decl();
        std::unique_ptr<Nodes::BlockStmt> parse_block_stmt();
        std::unique_ptr<Nodes::IfStmt> parse_if_stmt();
        std::unique_ptr<Nodes::WhileStmt> parse_while_stmt();
        std::unique_ptr<Nodes::ReturnStmt> parse_return_stmt();
        std::unique_ptr<Nodes::BreakStmt> parse_break_stmt();
        std::unique_ptr<Nodes::ContinueStmt> parse_continue_stmt();

        // Expressions
        std::unique_ptr<Nodes::Expression> parse_expr();

        // Binary

        // Logical
        std::unique_ptr<Nodes::Expression> parse_logical_or_expr();
        std::unique_ptr<Nodes::Expression> parse_logical_and_expr();
        std::unique_ptr<Nodes::Expression> parse_inclusive_or_expr();
        std::unique_ptr<Nodes::Expression> parse_exclusive_or_expr();
        std::unique_ptr<Nodes::Expression> parse_and_expr();
        std::unique_ptr<Nodes::Expression> parse_equality_expr();
        std::unique_ptr<Nodes::Expression> parse_relational_expr();
        std::unique_ptr<Nodes::Expression> parse_shift_expr();

        // Arithmetic
        std::unique_ptr<Nodes::Expression> parse_assig_expr();
        std::unique_ptr<Nodes::Expression> parse_add_expr();
        std::unique_ptr<Nodes::Expression> parse_mult_expr();

        // Unary
        std::unique_ptr<Nodes::Expression> parse_unary_expr();
        std::unique_ptr<Nodes::Expression> parse_negative_expr();
        std::unique_ptr<Nodes::Expression> parse_negation_expr();

        // Postfix
        std::unique_ptr<Nodes::Expression> parse_postfix_expr();

        // Primary
        std::unique_ptr<Nodes::Expression> parse_prim_expr();
        std::unique_ptr<Nodes::Expression> parse_ident();
        std::unique_ptr<Nodes::Expression> parse_parenthesis();

        // Const
        std::unique_ptr<Nodes::Expression> parse_const();
        std::unique_ptr<Nodes::Expression> parse_int();
        std::unique_ptr<Nodes::Expression> parse_string();
        std::unique_ptr<Nodes::Expression> parse_float();


        // helper parsers
        template <typename T>
        using unique_vec = std::vector<std::unique_ptr<T>>;

        // Func helpers
        using arg_list_t = unique_vec<Nodes::VariableDecl>;
        std::optional<arg_list_t> parse_func_arg_list();

        // Struct helpers
        unique_vec<Nodes::Identifier> parse_wraps_decl();
        using struct_body_parse_res_t = std::tuple<unique_vec<Nodes::VariableDecl>, unique_vec<Nodes::FunctionDecl>>;
        struct_body_parse_res_t parse_struct_body();

        // Stmt helpers
        std::unique_ptr<Nodes::Statement> parse_semicolon_terminated_stmt();
        std::unique_ptr<Nodes::Statement> parse_code_block_terminated_stmt();

        // type parsers
        std::unique_ptr<Types::BaseType> parse_type();
        std::unique_ptr<Types::ComplexType> parse_complex_type();
        std::unique_ptr<Types::ArrayType> parse_array_type();
        std::unique_ptr<Types::SimpleType> parse_simple_type();

        // postfix helpers
        using call_args_t = unique_vec<Nodes::Expression>;
        std::optional<call_args_t> parse_call_parameters();
        std::unique_ptr<Nodes::Expression> parse_index_parameters();
        std::unique_ptr<Nodes::Identifier> parse_access_parameters();

        std::unique_ptr<Nodes::Expression> parse_call_expr(std::unique_ptr<Nodes::Expression>& lhs);
        std::unique_ptr<Nodes::Expression> parse_index_expr(std::unique_ptr<Nodes::Expression>& lhs);
        std::unique_ptr<Nodes::Expression> parse_access_expr(std::unique_ptr<Nodes::Expression>& lhs);

        // Token parsers
        std::optional<Lexer::Token> parse_token(Lexer::Token::Id id);
        std::optional<Lexer::Token> parse_relational_op();
        std::function<std::optional<Lexer::Token>(Parser*)> make_tok_parser(Lexer::Token::Id id);

        // Functional parsers
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

        template <typename T, typename Parser>
        std::unique_ptr<T> parse_cast(Parser&& p) {
            auto res = std::move(std::bind(p, this)());
            return std::unique_ptr<T>(
                    dynamic_cast<T*>(res.release()));
        }
    };

}

#endif //TKOM2_PARSER_H
