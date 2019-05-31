add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(visitor)
add_subdirectory(exception)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM INCLUDE: ${LLVM_INCLUDE_DIRS}")


include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader)


message(STATUS "LLVM LIBS: ${llvm_libs}")


add_executable(rgc main.cpp)
target_link_libraries(rgc
        Lexer
        Parser
        Visitors
        Exception
        ${llvm_libs}
        )

