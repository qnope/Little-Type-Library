cmake_minimum_required(VERSION 3.10)
project(LittleTypeLibrary)

set(SOURCES "main.cpp")

set(HEADERS
    ltl/ltl.h
    lpl/lpl.h)


if(MSVC)
	add_compile_options(/experimental:preprocessor /permissive-)
endif(MSVC)

set(CMAKE_CXX_STANDARD 17)

add_executable(LittleTypeLibrary ${SOURCES} ${HEADERS})
