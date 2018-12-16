//
// Created by igor on 16.12.18.
//

#include <lexer/lexer.h>
#include <lexer/source/string.h>
#include <lexer/helpers.h>

// todo exceptions
// alpha in numeric value

// todo operators
// strings
// all keywords

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

    BOOST_AUTO_TEST_CASE(read_on_empty_source_returns_proper_value) {
        Lexer::Source::String s("");
        Lexer::Lexer l(s);
        auto tok = l.curr_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::End);
    }

    BOOST_AUTO_TEST_CASE(peek_on_empty_source_returns_proper_value) {
        Lexer::Source::String s("");
        Lexer::Lexer l(s);
        auto tok = l.peek();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::End);
    }

    BOOST_AUTO_TEST_CASE(integer_is_read_correctly) {
        Lexer::Source::String s("3437246");
        Lexer::Lexer l(s);
        auto tok = l.curr_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Integer);
        BOOST_CHECK_EQUAL(tok.symbol, "3437246");
    }

    BOOST_AUTO_TEST_CASE(expression_is_read_correctly) {
        Lexer::Source::String s("-3437246%3");
        Lexer::Lexer l(s);
        auto tok = l.curr_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Minus);
        BOOST_CHECK_EQUAL(tok.symbol, "-");

        tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Integer);
        BOOST_CHECK_EQUAL(tok.symbol, "3437246");

        tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Modulo);
        BOOST_CHECK_EQUAL(tok.symbol, "%");

        tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Integer);
        BOOST_CHECK_EQUAL(tok.symbol, "3");
    }

    BOOST_AUTO_TEST_CASE(double_character_operators_are_returned_correctly) {
        Lexer::Source::String s("!!===");
        Lexer::Lexer l(s);

        auto tok = l.curr_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Negation);
        BOOST_CHECK_EQUAL(tok.symbol, "!");

        tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Inequality);
        BOOST_CHECK_EQUAL(tok.symbol, "!=");

        tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Equality);
        BOOST_CHECK_EQUAL(tok.symbol, "==");
    }

    BOOST_AUTO_TEST_CASE(keywords_are_handled_correctly) {
        Lexer::Source::String s("if If return");
        Lexer::Lexer l(s);

        auto tok = l.curr_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::If);
        BOOST_CHECK_EQUAL(tok.symbol, "if");

        tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Identifier);
        BOOST_CHECK_EQUAL(tok.symbol, "If");

        tok = l.next_token();
        BOOST_CHECK_EQUAL(tok.id, Lexer::Token::Id::Return);
        BOOST_CHECK_EQUAL(tok.symbol, "return");
    }

BOOST_AUTO_TEST_SUITE_END()
