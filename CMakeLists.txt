cmake_minimum_required(VERSION 3.10)

project(TKOM2)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -o4")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_CXX_COMPILER "/usr/local/bin/clang++")

include_directories(include)
add_subdirectory(src)
add_subdirectory(tests)

enable_testing()
add_test(NAME All COMMAND Test)