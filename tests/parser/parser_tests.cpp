//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <boost/test/unit_test.hpp>

#include <parser/parser.h>
#include <lexer/source/string.h>
#include <exception/concrete.h>
#include <string>
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
    ast_assert_throw<Exception::Handler::Abort>(input);
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

BOOST_AUTO_TEST_CASE(assignment_operator_on_already_declared_varaible) {
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
------------------------AssignmentExpr: =
--------------------------------Identifier: a
--------------------------------IntConst: 5
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(check_expression_operator_precedess) {
    std::string input{
        R"(
void test() {
    2+2*2;
})"};
    std::string output{
        R"(Program
--------FuncDef
----------------FuncHeader: void test()
----------------CodeBlock
------------------------AdditiveExpr: +
--------------------------------IntConst: 2
--------------------------------MultiplicativeExpr: *
----------------------------------------IntConst: 2
----------------------------------------IntConst: 2
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(check_expression_operator_precedess_parenthesis) {
    std::string input{
        R"(
void test() {
    (2+2)*2;
})"};
    std::string output{
        R"(Program
--------FuncDef
----------------FuncHeader: void test()
----------------CodeBlock
------------------------MultiplicativeExpr: *
--------------------------------Parenthesis: ()
----------------------------------------AdditiveExpr: +
------------------------------------------------IntConst: 2
------------------------------------------------IntConst: 2
--------------------------------IntConst: 2
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(parsing_string_consts) {
    std::string input{
        R"(
void test() {
    "Hello World";
})"};
    std::string output{R"(Program
--------FuncDef
----------------FuncHeader: void test()
----------------CodeBlock
------------------------StringConst: Hello World
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(assignment_operator_on_var_decl_2){
    std::string input{
        R"(
void test() {
    let a int = 5;
})"};
    std::string output{
        R"(Program
--------FuncDef
----------------FuncHeader: void test()
----------------CodeBlock
------------------------VarDecl: int a
--------------------------------IntConst: 5
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(basic_memory_decl) {
    std::string input{R"(
    memory Heap {
        a int;

        void meth() {

        }

        void meth2();
    }
)"};
    std::string output{
        R"(Program
--------MemoryDecl: Heap
----------------VarDecl: int a
----------------FuncDef
------------------------FuncHeader: void meth()
------------------------CodeBlock
----------------FuncHeader: void meth2()
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(at_stmt_static_alloc) {
    std::string input{R"(
        void foo() {
            @Heap(1+1) let i int = 32;
        }
    )"};
    std::string output{R"(Program
--------FuncDef
----------------FuncHeader: void foo()
----------------CodeBlock
------------------------AtStmt: Heap
--------------------------------VarDecl: int i
----------------------------------------IntConst: 32
--------------------------------AdditiveExpr: +
----------------------------------------IntConst: 1
----------------------------------------IntConst: 1
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(at_stmt_static_alloc_without_init_expr) {
    std::string input{R"(
        void foo() {
            @Heap(foo) let i int;
        }
    )"};
    std::string output{R"(Program
--------FuncDef
----------------FuncHeader: void foo()
----------------CodeBlock
------------------------AtStmt: Heap
--------------------------------VarDecl: int i
--------------------------------Identifier: foo
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(at_stmt_dynamic_alloc) {
    std::string input{R"(
        void foo() {
            @Heap let i int = 32;
        }
    )"};
    std::string output{R"(Program
--------FuncDef
----------------FuncHeader: void foo()
----------------CodeBlock
------------------------AtStmt: Heap
--------------------------------VarDecl: int i
----------------------------------------IntConst: 32
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_CASE(at_stmt_dynamic_alloc_without_init_expr) {
    std::string input{R"(
        void foo() {
            @Heap let i int;
        }
    )"};
    std::string output{R"(Program
--------FuncDef
----------------FuncHeader: void foo()
----------------CodeBlock
------------------------AtStmt: Heap
--------------------------------VarDecl: int i
)"};
    check_ast_equal(input, output);
}

BOOST_AUTO_TEST_SUITE_END()