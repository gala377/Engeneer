add_executable(visitor_tests tests.cpp)
target_link_libraries(visitor_tests
    Lexer
    Parser
    Visitors
    Exception
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	stdc++fs
)