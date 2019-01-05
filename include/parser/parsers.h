//
// Created by igor on 05.01.19.
//

#ifndef TKOM2_PARSERS_H
#define TKOM2_PARSERS_H

#include <memory>
#include <functional>

#include <lexer/lexer.h>
#include <parser/nodes/base.h>

namespace Parser {

    using parse_input_t = Lexer::Lexer&;
    using parse_res_t = std::unique_ptr<Nodes::Base>;
    using ParserFunc = std::function<parse_res_t(parse_input_t)>;

    using node_parse_res_t = std::unique_ptr<Nodes::Base>;
    using tok_parse_res_t = std::optional<Lexer::Token>;

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
    };

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

}

#endif //TKOM2_PARSERS_H
