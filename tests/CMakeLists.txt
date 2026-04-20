include(GoogleTest)

add_executable(clipper-tests EXCLUDE_FROM_ALL
    PredicateTest.cpp
    OptionTest.cpp
    ClipperTest.cpp
)

target_link_libraries(clipper-tests PRIVATE
    ${PROJECT_NAME}
    GTest::gtest_main
)

set_target_properties(clipper-tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)


if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(clipper-tests PRIVATE -Wall -Wextra -Werror -pedantic-errors)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(clipper-tests PRIVATE /W4)
endif() 

gtest_discover_tests(clipper-tests)