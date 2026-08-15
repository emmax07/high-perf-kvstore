cmake_minimum_required(VERSION 3.16)
project(HighPerfKVStore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

include_directories(include)

# Shared Core Engine Library
add_library(kvstore_lib 
    src/wal.cpp
    src/engine.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(kvstore_lib PRIVATE Threads::Threads)

# Main Demo Executable App
add_executable(kvstore_app src/main.cpp)
target_link_libraries(kvstore_app PRIVATE kvstore_lib)

# Unit & Integration Test Suite Executable
add_executable(kvstore_tests tests/test_kvstore.cpp)
target_link_libraries(kvstore_tests PRIVATE kvstore_lib)