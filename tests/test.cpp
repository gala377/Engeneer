//
// Created by igor on 30.09.18.
//


#define BOOST_TEST_MODULE All_Tests
#include <boost/test/unit_test.hpp>
#include "lexer/source_tests.cpp"

BOOST_AUTO_TEST_CASE( my_test )
{
    BOOST_CHECK_EQUAL(1, 1);
}