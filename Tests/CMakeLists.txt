# ---- Tests ----

include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
        GIT_REPOSITORY "https://github.com/google/googletest.git"
        GIT_TAG aee0f9d9b5b87796ee8a0ab26b7587ec30e8858e
        UPDATE_DISCONNECTED YES)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(LTLtest17 test.cpp)
target_compile_features(LTLtest17 PRIVATE cxx_std_17)
target_link_libraries(LTLtest17 PRIVATE LTL::LTL gtest_main)
add_test(NAME LTLtest17 COMMAND LTLtest17)

if(LTL_TEST_CPP20)
    add_executable(LTLtest20 test.cpp)
    target_compile_features(LTLtest20 PRIVATE cxx_std_20)
    target_link_libraries(LTLtest20 PRIVATE LTL::LTL gtest_main)
    add_test(NAME LTLtest20 COMMAND LTLtest20)
endif()

