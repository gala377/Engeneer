find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

include_directories(${TKOM2_SOURCE_DIR}/include
					${Boost_INCLUDE_DIRS})

add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable(Test test.cpp)


add_subdirectory(lexer)

target_link_libraries(Test
		Lexer
		${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
		${Boost_SYSTEM_LIBRARY}
		stdc++fs
		)