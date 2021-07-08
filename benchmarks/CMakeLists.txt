set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY "https://github.com/google/benchmark.git"
        GIT_TAG v1.5.5
        UPDATE_DISCONNECTED YES)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(Benchmarks17 benchmarks.cpp)
target_compile_features(Benchmarks17 PRIVATE cxx_std_17)
target_link_libraries(Benchmarks17 PRIVATE LTL::LTL benchmark)

if(LTL_TEST_CPP20)
    add_executable(Benchmarks20 benchmarks.cpp)
    target_compile_features(Benchmarks20 PRIVATE cxx_std_20)
    target_link_libraries(Benchmarks20 PRIVATE LTL::LTL benchmark)
endif()
