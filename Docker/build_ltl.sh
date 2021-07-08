#!/bin/sh

cd /home/dev
rm -rf Little-Type-Library
git clone https://github.com/qnope/Little-Type-Library

cd Little-Type-Library
git checkout Benchmarks

cmake -S . -B build -G "Ninja Multi-Config"

cd build
cmake --build . --config Debug -v -j 8
cmake --build . --config Release -v -j 8

cd Tests
valgrind ctest -C "Debug" -VV

cd ../benchmarks/Release
./Benchmarks17 && ./Benchmarks20