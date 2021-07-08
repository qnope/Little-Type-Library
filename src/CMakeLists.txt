add_library(LTL INTERFACE)

add_subdirectory(lpl)
add_subdirectory(ltl)

set(CMAKE_CXX_EXTENSIONS false)

#useful for IDEs
set_property(
    TARGET LTL
    PROPERTY INTERFACE_SOURCES)

target_include_directories(LTL INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

string(TOLOWER "${CMAKE_BUILD_TYPE}" LTL_BUILD_TYPE)

if(MSVC)
    target_compile_options(LTL INTERFACE /Zc:preprocessor /Zc:__cplusplus /permissive- /bigobj)
elseif(LTL_CODE_COVERAGE AND LTL_BUILD_TYPE MATCHES debug)
    message("Build with code coverage")
    target_compile_options(LTL INTERFACE -fprofile-arcs -ftest-coverage)
    target_link_options(LTL INTERFACE -fprofile-arcs -ftest-coverage)

endif()

if(NOT MSVC)
    include(CheckCXXCompilerFlag)

    check_cxx_compiler_flag(-fcoroutines has_fcoroutines)
    check_cxx_compiler_flag(-fconcepts has_fconcepts)

    if(has_fcoroutines)
        target_compile_options(LTL INTERFACE $<$<COMPILE_FEATURES:cxx_std_20>:-fcoroutines>)
    endif()

    if(has_fconcepts)
        target_compile_options(LTL INTERFACE $<$<COMPILE_FEATURES:cxx_std_20>:-fconcepts>)
    endif()
endif()

add_library(LTL::LTL ALIAS LTL)
