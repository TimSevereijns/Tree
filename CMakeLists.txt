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
    source/tree.h
    source/tree_utils.h)

set(SOURCE_DIR
    source)

set(THIRD_PARTY
    third-party/catch2/single_include
    third-party/stopwatch/source)

set (TESTS
    tests/unit_tests.cpp)

set(BENCHMARK
    benchmark/main.cpp
    benchmark/drive_scanner.cpp
    benchmark/drive_scanner.h
    benchmark/file_info.h
    benchmark/scoped_handle.cpp
    benchmark/scoped_handle.h
    benchmark/thread_safe_queue.h
    benchmark/win_hack.h)

include_directories(${SOURCE_DIR} ${THIRD_PARTY})

add_executable(tests ${SOURCES} ${TESTS})
set_target_properties(tests PROPERTIES LINKER_LANGUAGE CXX)

if (UNIX)
    target_link_libraries(tests stdc++ ${CONAN_LIBS})
endif (UNIX)

add_executable(benchmark ${SOURCES} ${BENCHMARK})
set_target_properties(benchmark PROPERTIES LINKER_LANGUAGE CXX)

if (UNIX)
    target_link_libraries(benchmark stdc++ ${CONAN_LIBS})
endif (UNIX)