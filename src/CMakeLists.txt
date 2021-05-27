cmake_minimum_required(VERSION 3.14)

add_library(LTL INTERFACE)
target_sources(LTL INTERFACE
    lpl/lpl.h

    ltl/ltl.h
    ltl/algos.h
    ltl/concept.h
    ltl/condition.h
    ltl/crtp.h
    ltl/expected.h
    ltl/functional.h
    ltl/immutable.h
    ltl/invoke.h
    ltl/movable_any.h
    ltl/operator.h
    ltl/optional.h
    ltl/optional_type.h
    ltl/stream.h
    ltl/StrongType.h
    ltl/traits.h
    ltl/Tuple.h
    ltl/tuple_algos.h
    ltl/TypedTuple.h
    ltl/VariantUtils.h
    ltl/fast.h
    ltl/coroutine_helpers.h

    ltl/Range/actions.h
    ltl/Range/AsPointer.h
    ltl/Range/BaseIterator.h
    ltl/Range/DefaultView.h
    ltl/Range/enumerate.h
    ltl/Range/Filter.h
    ltl/Range/GroupBy.h
    ltl/Range/Join.h
    ltl/Range/Map.h
    ltl/Range/NullableFunction.h
    ltl/Range/Range.h
    ltl/Range/Repeater.h
    ltl/Range/Reverse.h
    ltl/Range/seq.h
    ltl/Range/Split.h
    ltl/Range/Taker.h
    ltl/Range/Value.h
    ltl/Range/Zip.h)

target_include_directories(LTL INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
if(MSVC)
    target_compile_options(LTL INTERFACE /Zc:preprocessor /Zc:__cplusplus /permissive- /bigobj)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(LTL INTERFACE  $<$<COMPILE_FEATURES:cxx_std_20>:-fconcepts -fcoroutines>)
else()
    target_compile_options(LTL INTERFACE  $<$<COMPILE_FEATURES:cxx_std_20>:-fcoroutines>)
endif()

add_library(LTL::LTL ALIAS LTL)
