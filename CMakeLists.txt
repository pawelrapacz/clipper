cmake_minimum_required(VERSION 3.15...4.0)
project(clipper-testing
        DESCRIPTION "Testing for clipper"
        LANGUAGES CXX)

set(
    CMAKE_CXX_STANDARD 20
    CMAKE_CXX_STANDARD_REQUIRED ON
    CMAKE_CXX_EXTENSIONS OFF
)

add_library(clipper INTERFACE)
target_include_directories(clipper INTERFACE include)
target_compile_options(clipper INTERFACE -Wall -Wextra -Wpedantic -Werror --pedantic-errors)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 6910c9d9165801d8827d628cb72eb7ea9dd538c5 # v1.16.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    tests
    tests/PredicateTest.cpp
    tests/OptionTest.cpp
    tests/ClipperTest.cpp
)

target_link_libraries(
    tests
    clipper
    GTest::gtest_main
)

target_compile_options(tests PRIVATE -g)

include(GoogleTest)
gtest_discover_tests(tests)