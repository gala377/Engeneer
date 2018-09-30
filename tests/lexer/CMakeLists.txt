add_library(Lexer_tests
        source_tests.cpp)

add_executable(lexer_tests tests.cpp)
target_link_libraries(lexer_tests
        Lexer
        Lexer_tests
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        )