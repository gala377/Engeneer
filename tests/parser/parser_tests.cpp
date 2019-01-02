//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/parser.h>
#include <lexer/source/string.h>
#include <visitor/log.h>

BOOST_AUTO_TEST_SUITE()

BOOST_AUTO_TEST_CASE(empty_source_initializes_to_empty_program) {
    Lexer::Source::String s{""};
    Visitor::Stringify v{};

    std::cout << "Initializing parser\n";
    Parser::Parser p{s};
    std::cout << "Parsing\n";
    auto ast = p.parse();
    std::cout << "Running visitor\n";
    ast.accept(v);
    std::cout << "Aaaand done\n";
    BOOST_CHECK_EQUAL("Program\n", v.repr());

}

BOOST_AUTO_TEST_SUITE_END()