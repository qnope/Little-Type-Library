cmake_minimum_required(VERSION 3.8)
project(LittleTypeLibrary)

if(MSVC)
        add_compile_options(
            /Zc:preprocessor
            /permissive-
            /d2cgsummary
            /Wv:18
            /MT)
endif(MSVC)

set(CMAKE_CXX_STANDARD 17)

set(HEADERS_RANGE
    include/ltl/Range/Map.h
    include/ltl/Range/Zip.h
    include/ltl/Range/Join.h
    include/ltl/Range/Range.h
    include/ltl/Range/Taker.h
    include/ltl/Range/Value.h
    include/ltl/Range/Filter.h
    include/ltl/Range/GroupBy.h
    include/ltl/Range/actions.h
    include/ltl/Range/AsPointer.h
    include/ltl/Range/enumerate.h
    include/ltl/Range/DefaultView.h
    include/ltl/Range/BaseIterator.h
    include/ltl/Range/NullableFunction.h
    )

set(HEADERS
    include/ltl/ltl.h
    include/lpl/lpl.h
    include/ltl/algos.h
    include/ltl/crtp.h
    include/ltl/Tuple.h
    include/ltl/stream.h
    include/ltl/invoke.h
    include/ltl/traits.h
    include/ltl/concept.h
    include/ltl/expected.h
    include/ltl/operator.h
    include/ltl/condition.h
    include/ltl/StrongType.h
    include/ltl/TypedTuple.h
    include/ltl/functional.h
    include/ltl/tuple_algos.h
    include/ltl/movable_any.h
    include/ltl/VariantUtils.h
    include/ltl/optional_type.h
    ${HEADERS_RANGE}
    )

enable_testing()
add_subdirectory(googletest)

option(LTL_BUILD_TESTS "To generate test for Little Type Library" ON)

add_library(LTL INTERFACE)
target_include_directories(LTL INTERFACE include/)

if(LTL_BUILD_TESTS)
    add_executable(LittleTypeLibrary "test.cpp" ${HEADERS})

    string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
    string(TOLOWER ${CMAKE_CXX_COMPILER} compiler)
    if(MSVC AND ${build_type} STREQUAL "debug")
        message("using MSVC to build in full debug")
        target_compile_options(LittleTypeLibrary PUBLIC /Zi /DEBUG:FULL)
    endif()

    if(${compiler} MATCHES ".*clang.*")
        message("using clang to build LTL")
        target_compile_options(LittleTypeLibrary PUBLIC -ftime-trace)
    endif()

    target_link_libraries(LittleTypeLibrary
        LTL
        gtest_main)
    add_test(NAME test COMMAND LittleTypeLibrary)
endif()
