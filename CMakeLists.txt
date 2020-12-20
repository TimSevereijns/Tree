cmake_minimum_required(VERSION 3.8)

enable_language(CXX)

project(tree)

include(conan/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

if (UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror -Wpedantic --coverage")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -Werror -Wpedantic")
endif (UNIX)

if (WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /bigobj")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W4 /bigobj")
endif (WIN32)

set(SOURCES
    tests/unit_tests.cpp
    source/tree.h
    source/tree_utils.h)

set(SOURCE_DIR
    source)

set(THIRD_PARTY
    third-party/catch2/single_include)

set(BENCHMARK
    benchmark)

include_directories(${SOURCE_DIR} ${THIRD_PARTY})

add_executable(tests ${SOURCES})

if (UNIX)
    target_link_libraries(tests stdc++ ${CONAN_LIBS})
endif (UNIX)

add_executable(benchmark ${SOURCES} ${BENCHMARK})

if (UNIX)
    target_link_libraries(benchmark stdc++ ${CONAN_LIBS})
endif (UNIX)