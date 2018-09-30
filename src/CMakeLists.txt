add_subdirectory(lexer)

add_executable(main main.cpp)
target_link_libraries(main
        Lexer
        #Parser
        #Interpreter
        )

