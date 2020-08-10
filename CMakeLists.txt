cmake_minimum_required(VERSION 3.8)
project(LittleTypeLibrary)

if(MSVC)
        add_compile_options(
            /Zc:preprocessor
            /permissive-
            /d2cgsummary
            /Wv:18
            /MT)
        message(Debug)
        add_compile_options(/Zi /DEBUG:FULL)
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
    include/ltl/movable_any.h
    include/ltl/Tuple.h
    include/ltl/StrongType.h
    include/ltl/algos.h
    include/ltl/stream.h
    include/ltl/functional.h
    include/ltl/operator.h
    include/ltl/concept.h
    include/ltl/VariantUtils.h
    include/ltl/crtp.h
    include/ltl/invoke.h
    include/ltl/traits.h
    include/ltl/tuple_algos.h
    include/ltl/optional_type.h
    include/ltl/condition.h
    include/ltl/TypedTuple.h
    include/lpl/lpl.h
    ${HEADERS_RANGE}
    )

enable_testing()
add_subdirectory(googletest)

add_library(LTL INTERFACE)
target_include_directories(LTL INTERFACE include/)


add_executable(LittleTypeLibrary "test.cpp" ${HEADERS})
target_link_libraries(LittleTypeLibrary
    LTL
    gtest_main)
add_test(NAME test COMMAND LittleTypeLibrary)
