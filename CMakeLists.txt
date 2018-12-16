cmake_minimum_required(VERSION 3.10)
project(LittleTypeLibrary)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES "main.cpp")

set(HEADERS_LPL
    "lpl/and.h"
    "lpl/cat.h"
    "lpl/checker.h"
    "lpl/compl.h"
    "lpl/dec_inc.h"
    "lpl/evaluation.h"
    "lpl/if.h"
    "lpl/lpl.h"
    "lpl/stringify.h")

set(HEADERS_LTL_TUPLE
    "ltl/tuple/algorithm.h"
    "ltl/tuple/argument.h"
    "ltl/tuple/template.h"
    "ltl/tuple/tuple_t.h")

set(HEADERS_LTL_STRONG_TYPE
    "ltl/strong_type/strong_type_t.h")

set(HEADERS_LTL
    "ltl/bool_t.h"
    "ltl/decay_reference_wrapper.h"
    "ltl/is_valid.h"
    "ltl/number_t.h"
    "ltl/overloader.h"
    "ltl/tuple.h"
    "ltl/type_t.h"
    "ltl/trait.h"
    "ltl/strong_type.h"
    ${HEADERS_LTL_TUPLE} ${HEADERS_LTL_STRONG_TYPE})


if(MSVC)
	add_compile_options(/experimental:preprocessor /permissive-)
endif(MSVC)

add_executable(LittleTypeLibrary ${SOURCES} ${HEADERS_LPL} ${HEADERS_LTL})
