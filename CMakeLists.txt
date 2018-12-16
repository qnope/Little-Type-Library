cmake_minimum_required(VERSION 3.10)
project(LittleTypeLibrary)

enable_language(CXX)

set(SOURCES "main.cpp")

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	add_compile_options(/experimental:preprocessor /permissive-)
endif(MSVC)

add_executable(LittleTypeLibrary ${SOURCES})