//
// Created by igor on 30.09.18.
//

#include <boost/test/unit_test.hpp>

#include <sstream>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include <lexer/source.h>

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

    const std::string test_file_name = "source_test_tmp.txt";
    const std::string example_message = "Example message\nAnother sentence";
};

BOOST_FIXTURE_TEST_SUITE(source_tests, ExampleSource)

    BOOST_AUTO_TEST_CASE(new_source_returns_correct_character)
    {
        auto s = Lexer::Source(test_file_name);
        char ch;
        ch << s;
        BOOST_CHECK_EQUAL(ch, 'E');
    }

    BOOST_AUTO_TEST_CASE(after_multiple_reads_next_character_is_read_correctly)
    {
        auto s = Lexer::Source(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Examp"); ++i) {
            ch << s;
        }
        ch << s;
        BOOST_CHECK_EQUAL(ch, 'l');
    }

    BOOST_AUTO_TEST_CASE(new_source_peeks_correct_character)
    {
        auto s = Lexer::Source(test_file_name);
        char ch = s.peek();
        BOOST_CHECK_EQUAL(ch, 'x');
    }

    BOOST_AUTO_TEST_CASE(after_multiple_reads_next_character_is_peeked_correctly)
    {
        auto s = Lexer::Source(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Examp"); ++i) {
            ch << s;
        }
        ch << s;
        BOOST_CHECK_EQUAL(ch, 'e');
    }

    BOOST_AUTO_TEST_CASE(whitespace_is_read_correctly)
    {
        auto s = Lexer::Source(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Example"); ++i) {
            ch << s;
        }
        ch << s;
        BOOST_CHECK_EQUAL(ch, ' ');

    }

    BOOST_AUTO_TEST_CASE(newline_is_read_correctly)
    {
        auto s = Lexer::Source(test_file_name);
        char ch;
        for(unsigned int i = 0; i < strlen("Example message"); ++i) {
            ch << s;
        }
        ch << s;
        BOOST_CHECK_EQUAL(ch, '\n');

    }


BOOST_AUTO_TEST_SUITE_END()