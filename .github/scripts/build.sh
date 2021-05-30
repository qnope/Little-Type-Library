set -e
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLTL_TEST_CPP20=${GENERATE_CPP20_TESTS} -DLTL_BUILD_TESTS=ON
cmake --build build --config Release -j 8
cd build/Tests/
ctest -C Release -VV
