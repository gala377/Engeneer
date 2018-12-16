//
// Created by igor on 16.12.18.
//

#include <lexer/lexer.h>
#include <lexer/source/string.h>
#include <lexer/helpers.h>

BOOST_AUTO_TEST_SUITE()

    BOOST_AUTO_TEST_CASE(first_identifier_from_lexer_is_returned_correctly) {
        Lexer::Source::String s("hello world");
        Lexer::Lexer l(s);
        auto tok = l.curr_token();
        BOOST_CHECK_EQUAL(tok.symbol, "hello");
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Identifier);
    }

    BOOST_AUTO_TEST_CASE(second_identifier_from_lexer_is_returned_correctly) {
        Lexer::Source::String s("hello world");
        Lexer::Lexer l(s);
        auto tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.symbol, "world");
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Identifier);
    }

    BOOST_AUTO_TEST_CASE(read_on_end_returns_correctly) {
        Lexer::Source::String s("hello world");
        Lexer::Lexer l(s);
        l.next_token();
        auto tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::End);
    }

    BOOST_AUTO_TEST_CASE(peek_on_initialized_lexer_returns_correctly) {
        Lexer::Source::String s("hello world");
        Lexer::Lexer l(s);
        auto tok = l.peek();
        BOOST_CHECK_EQUAL(tok.symbol, "world");
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Identifier);
    }

    BOOST_AUTO_TEST_CASE(peek_on_end_returns_correctly) {
        Lexer::Source::String s("hello world");
        Lexer::Lexer l(s);
        l.next_token();
        auto tok = l.peek();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::End);
    }

BOOST_AUTO_TEST_SUITE_END()
