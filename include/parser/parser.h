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


        // helper parsers
        using arg_list_t = std::vector<std::unique_ptr<Nodes::GlobVariableDecl>>;
        arg_list_t parse_func_arg_list();

        // Token parsers
        std::optional<Lexer::Token> parse_token(Lexer::Token::Id id);
        std::optional<std::string> parse_type();
    };

}

#endif //TKOM2_PARSER_H
