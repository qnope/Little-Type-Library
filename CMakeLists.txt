cmake_minimum_required(VERSION 3.14)

project("Little Type Library"
        VERSION 2.0.0
        LANGUAGES CXX)

option(LTL_BUILD_TESTS "Enable LTL tests" OFF)
option(LTL_GENERATE_DOCUMENTATION "Generate the ltl documentation" OFF)
option(LTL_CODE_COVERAGE "Generate Code Coverage" ON)

add_subdirectory(src/)

if(LTL_BUILD_TESTS)
    if("${CMAKE_CXX_COMPILE_FEATURES}" MATCHES "cxx_std_20")
        set(LTL_TEST_CPP20 true CACHE INTERNAL "" )
    else()
        set(LTL_TEST_CPP20 false CACHE INTERNAL "" )
    endif()

    message("Build tests and benchmarks")
    add_subdirectory(Tests/)
    add_subdirectory(benchmarks/)
endif()

if(LTL_GENERATE_DOCUMENTATION)
    message("Build documentation")
    add_subdirectory(doc/)
endif()
