add_library(Visitors
        log.cpp base.cpp llvm.cpp)

# todo how to make it look for llvm in Projects not in system?
find_package(LLVM REQUIRED CONFIG
        PATHS  ~/Projects/Cpp/llvm-project/build NO_DEFAULT_PATH)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
message(STATUS "LLVM INCLUDE: ${LLVM_INCLUDE_DIRS}")
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader)
message(STATUS "LLVM LIBS: ${llvm_libs}")
target_link_libraries(Visitors ${llvm_libs})