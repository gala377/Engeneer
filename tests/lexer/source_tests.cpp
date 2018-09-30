//
// Created by igor on 30.09.18.
//

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(source_tests)

BOOST_AUTO_TEST_CASE(simple_running_test)
    {
        BOOST_CHECK_EQUAL(2, 2);
    }

    BOOST_AUTO_TEST_CASE(simple_failing_test)
    {
        BOOST_CHECK_EQUAL(1, 2);
    }

BOOST_AUTO_TEST_SUITE_END()