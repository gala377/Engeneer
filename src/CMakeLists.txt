add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(visitor)
add_subdirectory(exception)

add_executable(main main.cpp)
target_link_libraries(main
        Lexer
        Parser
        Visitors
        Exception
        #Interpreter
        )

