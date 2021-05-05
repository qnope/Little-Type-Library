cmake_minimum_required(VERSION 3.14)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(googletest
        GIT_REPOSITORY "https://github.com/google/googletest.git"
        GIT_TAG aee0f9d9b5b87796ee8a0ab26b7587ec30e8858e
        UPDATE_DISCONNECTED YES)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY "https://github.com/google/benchmark.git"
        GIT_TAG v1.5.0
        UPDATE_DISCONNECTED YES)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(Benchmarks17 benchmarks.cpp)
target_compile_features(Benchmarks17 PRIVATE cxx_std_17)
target_link_libraries(Benchmarks17 PRIVATE LTL::LTL benchmark)

if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    add_executable(Benchmarks20 benchmarks.cpp)
    target_compile_features(Benchmarks20 PRIVATE cxx_std_20)
    target_link_libraries(Benchmarks20 PRIVATE LTL::LTL benchmark)
endif()
