//
// Created by igor on 30.12.18.
//

#include <parser/parser.h>
#include <lexer/source/string.h>
#include <visitor/log.h>

BOOST_AUTO_TEST_SUITE()

BOOST_AUTO_TEST_CASE(empty_source_initializes_to_empty_program) {
    Lexer::Source::String s{""};
    Visitor::Log v{};

    Parser::Parser p{s};
    p.parse().visit(v);

    BOOST_CHECK_EQUAL("Program\n", v.repr());

}

BOOST_AUTO_TEST_SUITE_END()