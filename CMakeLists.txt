cmake_minimum_required(VERSION 3.10)
project(LittleTypeLibrary)

set(SOURCES "main.cpp")

set(HEADERS
    ltl/ltl.h
    ltl/Tuple.h
    ltl/StrongType.h
    ltl/algos.h
    ltl/stream.h
    ltl/functional.h
    ltl/operator.h
    ltl/concept.h
    ltl/range.h
    ltl/DefaultView.h
    ltl/VariantUtils.h
    ltl/crtp.h
    ltl/traits.h
    ltl/tuple_algos.h
    lpl/lpl.h
    )

if(MSVC)
        add_compile_options(/experimental:preprocessor /permissive- /d2cgsummary)
endif(MSVC)

set(CMAKE_CXX_STANDARD 17)

add_executable(LittleTypeLibrary ${SOURCES} ${HEADERS})
