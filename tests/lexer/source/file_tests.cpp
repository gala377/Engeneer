//
// Created by igor on 30.09.18.
//

#include <boost/test/unit_test.hpp>

#include <sstream>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include <lexer/source/file.h>

namespace fs = std::experimental::filesystem;

struct ExampleSource {
    ExampleSource() {
        std::ofstream outfile(test_file_name);
        outfile << example_message << std::endl;
        outfile.close();
    }

    ~ExampleSource() {
        try {
            fs::remove(test_file_name);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Could not delete temporary file " << test_file_name << "\n";
        }
    }

    const fs::path test_file_name = fs::path("./source_test_tmp.txt");
    const std::string example_message = "Example message\nAnother sentence";
};

// todo eof chars
// todo test on empty file
// todo tests on new line at the end of file

BOOST_FIXTURE_TEST_SUITE(source_tests, ExampleSource)

    BOOST_AUTO_TEST_CASE(new_source_returns_correct_character)
    {
        auto s = Lexer::Source::File(test_file_name);
        char ch;
        ch << s;
        BOOST_CHECK_EQUAL(ch, 'E');
    }

    BOOST_AUTO_TEST_CASE(after_multiple_reads_next_character_is_read_correctly)
    {
        auto s = Lexer::Source::File(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Examp"); ++i) {
            ch << s;
        }
        ch << s;
        BOOST_CHECK_EQUAL(ch, 'l');
    }

    BOOST_AUTO_TEST_CASE(new_source_peeks_correct_character)
    {
        auto s = Lexer::Source::File(test_file_name);
        char ch = s.peek();
        BOOST_CHECK_EQUAL(ch, 'x');
    }

    BOOST_AUTO_TEST_CASE(after_multiple_reads_next_character_is_peeked_correctly)
    {
        auto s = Lexer::Source::File(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Examp"); ++i) {
            ch << s;
        }
        ch << s;
        BOOST_CHECK_EQUAL(ch, 'l');
    }

    BOOST_AUTO_TEST_CASE(whitespace_is_read_correctly)
    {
        auto s = Lexer::Source::File(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Example"); ++i) {
            ch << s;
        }
        ch << s;
        BOOST_CHECK_EQUAL(ch, ' ');
    }

    BOOST_AUTO_TEST_CASE(newline_is_read_correctly)
    {
        auto s = Lexer::Source::File(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Example message"); ++i) {
            ch << s;
        }
        ch << s;
        BOOST_CHECK_EQUAL(ch, '\n');
    }

    BOOST_AUTO_TEST_CASE(after_peek_character_is_read_correctly) {
        auto s = Lexer::Source::File(test_file_name);
        s.peek();
        char ch;
        ch << s;
        BOOST_CHECK_EQUAL(ch, 'E');
    }

    BOOST_AUTO_TEST_CASE(multiple_peeks_return_correct_character) {
        auto s = Lexer::Source::File(test_file_name);
        for(unsigned int i = 0; i < 100; ++i) {
            BOOST_CHECK_EQUAL(s.peek(), 'x');
        }
    }

    BOOST_AUTO_TEST_CASE(initial_position_is_1_2) {
        auto s = Lexer::Source::File(test_file_name);
        auto [line, pos] = s.curr_source_position();
        BOOST_CHECK_EQUAL(line, 1);
        BOOST_CHECK_EQUAL(pos, 2);
    }

    BOOST_AUTO_TEST_CASE(after_some_reads_position_is_correct) {
        Lexer::Source::File s(test_file_name);
        for(unsigned int i = 0; i < strlen("Exam"); ++i) {
            char c;
            c << s;
        }
        auto [line, pos] = s.curr_source_position();
        BOOST_CHECK_EQUAL(line, 1);
        BOOST_CHECK_EQUAL(pos, 6);
    }

    BOOST_AUTO_TEST_CASE(line_and_position_are_correct_after_new_line) {
        auto s = Lexer::Source::File(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Example message\n"); ++i) {
            ch << s;
        }
        auto [line, pos] = s.curr_source_position();
        BOOST_CHECK_EQUAL(line, 2);
        BOOST_CHECK_EQUAL(pos, 2);
        BOOST_CHECK_EQUAL(s.curr_char(), 'A');
    }

    BOOST_AUTO_TEST_CASE(line_and_position_are_correct_before_new_line) {
        auto s = Lexer::Source::File(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Example message"); ++i) {
            ch << s;
        }
        auto [line, pos] = s.curr_source_position();
        BOOST_CHECK_EQUAL(line, 2);
        BOOST_CHECK_EQUAL(pos, 1);
        BOOST_CHECK_EQUAL(s.curr_char(), '\n');
    }

BOOST_AUTO_TEST_SUITE_END()