cmake_minimum_required(VERSION 3.14)

project("Little Type Library"
        VERSION 2.0.0
        LANGUAGES CXX)

option(LTL_BUILD_TESTS "Enable LTL tests" OFF)
option(LTL_GENERATE_DOCUMENTATION "Generate the ltl documentation" OFF)
option(LTL_TEST_CPP20 "Generate CPP20 samples" ON)
option(LTL_CODE_COVERAGE "Generate Code Coverage" ON)

add_subdirectory(src/)

if(LTL_BUILD_TESTS)
    add_subdirectory(Tests/)
    add_subdirectory(benchmarks/)
endif()

if(LTL_GENERATE_DOCUMENTATION)
    add_subdirectory(doc/)
endif()
