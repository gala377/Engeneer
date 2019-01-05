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
    ParserFunc Success(Nodes::Base* ret);

    // Returns parser that always fail
    ParserFunc Fail(ParserFunc p);

    // Makes parser that succeeds if encounters a specified token
    ParserFunc TokenParser(const Lexer::Token::Id& id);



}

#endif //TKOM2_PARSERS_H
