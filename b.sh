#!/bin/bash
mkdir -p build && cd build
# export CC=afl-clang-fast
# export CXX=afl-clang-fast++
export CC=clang
export CXX=clang++
export CFLAGS="-g"
export CXXFLAGS="-g"
export ASAN_OPTIONS=detect_leaks=0
cmake -GNinja -S .. -B . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX=~/.local -DTON_USE_ASAN=ON
# cmake --build . -j8
# ninja
