//
// Created by igor on 05.01.19.
//

#ifndef TKOM2_PARSERS_H
#define TKOM2_PARSERS_H

#include <memory>
#include <functional>


#include <lexer/lexer.h>
#include <parser/nodes/base.h>
#include <parser/combinators.hpp>

namespace Parser {

    using parse_input_t = Lexer::Lexer&;
    using parse_res_t = std::unique_ptr<Nodes::Base>;
    using ParserFunc = std::function<parse_res_t(parse_input_t)>;


    // Returns parser that returns given node
    auto Success(Nodes::Base* ret) {
        return [ret](Lexer::Lexer &l) -> parse_res_t {
            return std::unique_ptr<Nodes::Base>(ret);
        };
    }

    // Returns parser that always fail
    template <typename T>
    auto Fail(const T& p) {
        return [](Lexer::Lexer &l) -> parse_res_t {
            return std::unique_ptr<Nodes::Base>{nullptr};
        };
    }

    // Makes parser that succeeds if encounters a specified token
    auto TokenParser(const Lexer::Token::Id& id)  {
        return [&id](Lexer::Lexer &l) -> parse_res_t {
            auto tok = l.curr_token();
            if (tok.id != id) {
                return {nullptr};
            }
            l.next_token();
            return std::make_unique<Nodes::BaseToken>(tok);
        };
    }

    // parses variable declaration
    auto VarDeclParser() {
        return [](Lexer::Lexer& l) {
            auto res =Combinators::return_left(
                    Combinators::combine(
                        Combinators::return_right(
                            TokenParser(Lexer::Token::Id::Let),
                            TokenParser(Lexer::Token::Id::Identifier)),
                        TokenParser(Lexer::Token::Id::Identifier),
                        [](std::unique_ptr<Nodes::BaseToken> l, std::unique_ptr<Nodes::BaseToken> r) {
                            return std::make_unique<Nodes::VariableDecl>(
                                    l->get_token().symbol,
                                    r->get_token().symbol);
                        }),
                    TokenParser(Lexer::Token::Id::Semicolon));
        };
    }
}

#endif //TKOM2_PARSERS_H
