#!/bin/bash
mkdir -p build && cd build
# export CC=afl-clang-fast
# export CXX=afl-clang-fast++
export CC=clang
export CXX=clang++
cmake -GNinja -S .. -B . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX=~/.local
# cmake --build . -j8
ninja
