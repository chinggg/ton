cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

add_executable(fuzz_vm fuzz_vm.cpp)
target_compile_options(fuzz_vm PUBLIC -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
target_include_directories(fuzz_vm PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(fuzz_vm PUBLIC ton_crypto -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)