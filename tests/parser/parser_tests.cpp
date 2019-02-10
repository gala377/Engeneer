//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/parser.h>
#include <lexer/source/string.h>
#include <visitor/log.h>

BOOST_AUTO_TEST_SUITE()

void check_ast_equal(const std::string& test_string, const std::string& expected) {
    Lexer::Source::String s{test_string};
    Visitor::Stringify v;
    Parser::Parser p{s};
    auto ast = p.parse();
    ast.accept(v);
    BOOST_CHECK_EQUAL(v.repr(), expected);
}

template <typename Exception>
void ast_assert_throw(const std::string& input) {
    Lexer::Source::String s{input};
    Visitor::Stringify v;
    Parser::Parser p{s};
    BOOST_CHECK_THROW(p.parse(), Exception);
}

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
    check_ast_equal(test_string, expected);
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
    check_ast_equal(test_string, expected);
}

BOOST_AUTO_TEST_CASE(top_level_function_declaration_without_arguments_and_body) {
        std::string test_string{
                R"(
int test();
string test2 ( ) ;
UnknownIdentifier test3 ();
)"};
        std::string expected{
                R"(Program
--------FuncDecl: int test()
--------FuncDecl: string test2()
--------FuncDecl: UnknownIdentifier test3()
)"};
        check_ast_equal(test_string, expected);
}

BOOST_AUTO_TEST_CASE(top_level_function_declaration_without_arguments_and_body_with_empty_variable_declarations) {
        std::string test_string{
                R"(
int test();
let var1 int; let var2 string;
string test2 ( ) ;
UnknownIdentifier test3 ();
let var_4 char;
)"};
        std::string expected{
                R"(Program
--------FuncDecl: int test()
--------VarDecl: int var1
--------VarDecl: string var2
--------FuncDecl: string test2()
--------FuncDecl: UnknownIdentifier test3()
--------VarDecl: char var_4
)"};
        check_ast_equal(test_string, expected);
    }


    BOOST_AUTO_TEST_CASE(top_level_func_decl_with_single_argument) {
    std::string test_string{
        R"(
int test(a double);
)"};
    std::string expected{
        R"(Program
--------FuncDecl: int test(double a, )
)"};
    check_ast_equal(test_string, expected);
}

BOOST_AUTO_TEST_CASE(top_level_func_decl_with_multiple_arguments) {
    std::string test_string{
        R"(
int test (a double, b int, c char);
)"};
    std::string expected{
        R"(Program
--------FuncDecl: int test(double a, int b, char c, )
)"};
    check_ast_equal(test_string, expected);
}

BOOST_AUTO_TEST_CASE(top_level_func_decl_with_single_arg_and_trailing_comma_throws) {
    std::string test_string{
        R"(
    int test (a double,);
)"};
    ast_assert_throw<std::runtime_error>(test_string);
}

BOOST_AUTO_TEST_SUITE_END()