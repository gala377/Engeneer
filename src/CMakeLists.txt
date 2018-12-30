add_subdirectory(lexer)
add_subdirectory(parser)


add_executable(main main.cpp)
target_link_libraries(main
        Lexer
        Parser
        #Interpreter
        )

