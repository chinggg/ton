cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

add_executable(fuzz_vm fuzz_vm.cpp)
target_compile_options(fuzz_vm PUBLIC -fsanitize=fuzzer,address -fno-omit-frame-pointer -g)
target_link_libraries(fuzz_vm PUBLIC ton_crypto ton_validator tl_api -fsanitize=fuzzer,address -fno-omit-frame-pointer -g)