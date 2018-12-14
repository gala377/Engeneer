add_executable(lexer_tests tests.cpp)
target_link_libraries(lexer_tests
        Lexer
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        stdc++fs
        )