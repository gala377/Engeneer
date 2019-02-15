//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/parser.h>
#include <lexer/source/string.h>
#include <visitor/log.h>

BOOST_AUTO_TEST_SUITE()

void check_ast_equal(const std::string& input, const std::string& output) {
    Lexer::Source::String s{input};
    Visitor::Stringify v;
    Parser::Parser p{s};
    auto ast = p.parse();
    ast.accept(v);
    BOOST_CHECK_EQUAL(v.repr(), output);
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
    std::string input{
            R"(
let test int;
)"};
    std::string output{
            R"(Program
--------GlobVarDecl: int test
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(top_level_many_variable_decl_without_initialization) {
    std::string input{
            R"(
let test int;
let test2 string;
let test3 UnknownIdentifier;
)"};
    std::string output{
            R"(Program
--------GlobVarDecl: int test
--------GlobVarDecl: string test2
--------GlobVarDecl: UnknownIdentifier test3
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(top_level_function_declaration_without_arguments_and_body) {
        std::string input{
                R"(
int test();
string test2 ( ) ;
UnknownIdentifier test3 ();
)"};
        std::string output{
                R"(Program
--------FuncHeader: int test()
--------FuncHeader: string test2()
--------FuncHeader: UnknownIdentifier test3()
)"};
        check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(top_level_function_declaration_without_arguments_and_body_with_empty_variable_declarations) {
        std::string input{
                R"(
int test();
let var1 int; let var2 string;
string test2 ( ) ;
UnknownIdentifier test3 ();
let var_4 char;
)"};
        std::string output{
                R"(Program
--------FuncHeader: int test()
--------GlobVarDecl: int var1
--------GlobVarDecl: string var2
--------FuncHeader: string test2()
--------FuncHeader: UnknownIdentifier test3()
--------GlobVarDecl: char var_4
)"};
        check_ast_equal(input, output);
    }


    BOOST_AUTO_TEST_CASE(top_level_func_decl_with_single_argument) {
    std::string input{
        R"(
int test(a double);
)"};
    std::string output{
        R"(Program
--------FuncHeader: int test(double a, )
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(top_level_func_decl_with_multiple_arguments) {
    std::string input{
        R"(
int test (a double, b int, c char);
)"};
    std::string output{
        R"(Program
--------FuncHeader: int test(double a, int b, char c, )
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(top_level_func_decl_with_single_arg_and_trailing_comma_throws) {
    std::string input{
        R"(
    int test (a double,);
)"};
    ast_assert_throw<std::runtime_error>(input);
}

BOOST_AUTO_TEST_CASE(func_def_with_single_var_decl) {
    std::string input{
        R"(
void test() {
    let a int;
})"};
    std::string output{
        R"(Program
--------FuncDef
----------------FuncHeader: void test()
----------------CodeBlock
------------------------VarDecl: int a
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_SUITE(assignment_operator_on_already_declared_varaible) {
    std::string input{
        R"(
void test() {
    let a int;
    a = 5;
})"};
    std::string output{
        R"(Program
--------FuncDef
----------------FuncHeader: void test()
----------------CodeBlock
------------------------VarDecl: int a
------------------------Assignment: =
--------------------------------Identifier: a 
--------------------------------ConstInt: 5 
)"};
    check_ast_equal(input, output);
}

// BOOST_AUTO_TEST_SUITE(assignment_operator_on_var_decl) {
//     std::string input{
//         R"(
// void test() {
//     let a int = 5;
// })"};
//     std::string output{
//         R"(Program
// --------FuncDef
// ----------------FuncHeader: void test()
// ----------------CodeBlock
// ------------------------Assignment: =
// --------------------------------VarDecl: int a 
// --------------------------------ConstInt: 5 
// )"};
//     check_ast_equal(input, output);
// }


BOOST_AUTO_TEST_SUITE_END()