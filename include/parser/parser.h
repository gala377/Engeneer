//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_PARSER_H
#define TKOM2_PARSER_H

#include <lexer/lexer.h>
#include <parser/ast.h>

namespace Parser {

    class Parser {
    public:
        Parser() = delete;
        explicit Parser(Lexer::Source::Base& s);

        AST parse();
    private:
        Lexer::Lexer _lexer;
    };

}

#endif //TKOM2_PARSER_H
