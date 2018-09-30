cmake_minimum_required(VERSION 3.11)

project(TKOM2)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -std=c++17 -o4")

include_directories(include)
add_subdirectory(src)
add_subdirectory(tests)

enable_testing()
add_test(NAME All COMMAND Test)