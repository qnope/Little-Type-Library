cmake_minimum_required(VERSION 3.10)
project(LittleTypeLibrary)

set(SOURCES "main.cpp")

set(HEADERS_RANGE
	ltl/Range/Range.h
	ltl/Range/NullableFunction.h
	ltl/Range/BaseIterator.h
    ltl/Range/AsPointer.h
    ltl/Range/Filter.h
    ltl/Range/Map.h
    ltl/Range/Taker.h
    ltl/Range/Zip.h
    ltl/Range/Value.h
    ltl/Range/Join.h
    ltl/Range/enumerate.h
    ltl/Range/DefaultView.h
    )

set(HEADERS
    ltl/ltl.h
    ltl/Tuple.h
    ltl/StrongType.h
    ltl/algos.h
    ltl/stream.h
    ltl/functional.h
    ltl/operator.h
    ltl/concept.h
    ltl/VariantUtils.h
    ltl/crtp.h
    ltl/invoke.h
    ltl/traits.h
    ltl/tuple_algos.h
    ltl/optional_type.h
    ltl/condition.h
	ltl/TypedTuple.h
    lpl/lpl.h
    ${HEADERS_RANGE}
    )

if(MSVC)
        add_compile_options(/experimental:preprocessor /permissive- /d2cgsummary)
endif(MSVC)

set(CMAKE_CXX_STANDARD 17)

add_executable(LittleTypeLibrary ${SOURCES} ${HEADERS})
