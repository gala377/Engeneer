//
// Created by igor on 05.01.19.
//

#include <parser/combinators.hpp>
#include <parser/parsers.h>

#include <lexer/lexer.h>
#include <lexer/source/string.h>


BOOST_AUTO_TEST_SUITE()

    BOOST_AUTO_TEST_CASE(token_parser_parsers_tokens_the_proper_way) {
        Lexer::Source::String s("let var be 0");
        Lexer::Lexer l(s);
        auto res = Parser::TokenParser(Lexer::Token::Id::Let)(l);
        auto casted = dynamic_cast<Parser::Nodes::BaseToken *>(res.get());
        BOOST_CHECK_EQUAL(casted->get_token().symbol, "let");
    }

    BOOST_AUTO_TEST_CASE(token_parser_fail_the_proper_way) {
        Lexer::Source::String s("var be 0");
        Lexer::Lexer l(s);
        auto res = Parser::TokenParser(Lexer::Token::Id::Let)(l);
        BOOST_CHECK_EQUAL((bool) res, false);
    }


BOOST_AUTO_TEST_SUITE_END()