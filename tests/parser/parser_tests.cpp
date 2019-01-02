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
    Visitor::Stringify v;
    Parser::Parser p{s};
    auto ast = p.parse();
    ast.accept(v);
    BOOST_CHECK_EQUAL("Program\n", v.repr());
}

// todo failing variable decl

BOOST_AUTO_TEST_CASE(top_level_variable_decl_without_initialization) {
    std::string test_string{
            R"(
let test int;
)"};
    std::string expected{
            R"(Program
--------VarDecl: int test
)"};
    Lexer::Source::String s{test_string};
    Visitor::Stringify v;
    Parser::Parser p{s};
    auto ast = p.parse();
    ast.accept(v);
    BOOST_CHECK_EQUAL(v.repr(), expected);
}

BOOST_AUTO_TEST_CASE(top_level_many_variable_decl_without_initialization) {
    std::string test_string{
            R"(
let test int;
let test2 string;
let test3 UnknownIdentifier;
)"};
    std::string expected{
            R"(Program
--------VarDecl: int test
--------VarDecl: string test2
--------VarDecl: UnknownIdentifier test3
)"};

    Lexer::Source::String s{test_string};
    Visitor::Stringify v;
    Parser::Parser p{s};
    auto ast = p.parse();
    ast.accept(v);
    BOOST_CHECK_EQUAL(v.repr(), expected);
}

BOOST_AUTO_TEST_SUITE_END()