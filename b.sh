#!/bin/bash

# normal build: ./b.sh
# build with: coverage: ../libfuzzer-cov/cov-build.sh ./b.sh build_cov

builddir=${1:-"build"}
mkdir -p $builddir && cd $builddir
# export CC=afl-clang-fast
# export CXX=afl-clang-fast++
export CC=clang
export CXX=clang++
export CFLAGS="$CFLAGS -g"
export CXXFLAGS="$CXXFLAGS -g"
export ASAN_OPTIONS=detect_leaks=0
cmake -GNinja -S .. -B . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX=~/.local -DTON_USE_ASAN=ON
# cmake --build . -j8
# ninja validator-engine
