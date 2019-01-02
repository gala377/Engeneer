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
        std::unique_ptr<Nodes::Base> parse_top_level_decl();
        // todo rename
        std::unique_ptr<Nodes::Base> parse_end_of_file();

        std::unique_ptr<Nodes::VariableDecl> parser_var_decl();

        // Token parsers
        std::optional<Lexer::Token> parse_token(Lexer::Token::Id id);
    };

}

#endif //TKOM2_PARSER_H
