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
#include <parser/functions.hpp>


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
        // todo rename
        std::unique_ptr<Nodes::End> parse_end_of_file();
        std::unique_ptr<Nodes::VariableDecl> parse_var_decl();
        std::unique_ptr<Nodes::FunctionDecl> parse_func_decl();

        // Token parsers
        std::optional<Lexer::Token> parse_token(Lexer::Token::Id id);
        std::optional<std::string> parse_type();

        // todo maybe later
//        template <typename T>
//        T parenthesis_parse(std::function<std::unique_ptr<Nodes::Base>(void)>);
        // todo combine
        // combine_two_left_left
        // combine two left right
        // combine and pass to the next
        // parse_many
        // parse_one_many
        // parce_once

        // returns if is one of 
        template <typename ...Args>
        std::optional<Lexer::Token> one_of(Args... args) {
            auto tok = _lexer.curr_token();
            if((tok.id == args) || ...) {
                return {tok};
            }
            return {};
        }

        // returns if is none of 
        template <typename ...Args>
        std::optional<Lexer::Token> none_of(Args... args) {
            auto tok = _lexer.curr_token();
            if((tok.id != args) && ...) {
                return {tok};
            }
            return {};
        }
    };

}

#endif //TKOM2_PARSER_H
