//
// Created by igor on 05.01.19.
//

#include <parser/parsers.h>

using namespace Parser;

ParserFunc Success(Nodes::Base* ret) {
    return [ret](Lexer::Lexer& l) -> std::unique_ptr<Nodes::Base> {
        return std::unique_ptr<Nodes::Base>(ret);
    };
}

// Returns parser that always fail
ParserFunc Fail(ParserFunc p) {
    return [](Lexer::Lexer& l) { return std::unique_ptr<Nodes::Base>{nullptr}; };
}

// Makes parser that succeeds if encounters a specified token
ParserFunc TokenParser(const Lexer::Token::Id& id) {
    return [&id](Lexer::Lexer& l) -> parse_res_t {
        auto tok = l.curr_token();
        if(tok.id != id) {
            return {nullptr};
        }
        l.next_token();
        return std::make_unique<Nodes::BaseToken>(tok);
    };
}