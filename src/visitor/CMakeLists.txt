add_library(Visitors
        log.cpp base.cpp llvm.cpp type.cpp)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM INCLUDE: ${LLVM_INCLUDE_DIRS}")


include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader)


message(STATUS "LLVM LIBS: ${llvm_libs}")
target_link_libraries(Visitors ${llvm_libs})