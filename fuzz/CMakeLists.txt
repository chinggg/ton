cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(NO_FUZZING_FLAGS "-Wno-shorten-64-to-32 -Wno-implicit-int-conversion")
set(PROTOBUF_PROTOC_TARGET protoc)
option(LIB_PROTO_MUTATOR_TESTING "Enable test building" OFF)
option(LIB_PROTO_MUTATOR_WITH_ASAN "Enable address sanitizer" ON)
add_subdirectory(libprotobuf-mutator)

# set(CMAKE_CXX_STANDARD 17)  # make protobuf compile
add_executable(fuzz_vm fuzz_vm.cpp)
target_compile_options(fuzz_vm PUBLIC -fsanitize=fuzzer,address -fno-omit-frame-pointer -g -fstandalone-debug)
target_link_libraries(fuzz_vm PUBLIC ton_crypto -fsanitize=fuzzer,address -fno-omit-frame-pointer -g)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(FUZZING_FLAGS "-fsanitize=fuzzer-no-link")
set(FUZZING_FLAGS_BINARY "-fsanitize=fuzzer")

# Generate Protobuf sources and headers
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS boc.proto)
# set(PROTO_SRCS boc.pb.cc)

# Add executable target
add_executable(fuzz_vm_proto ${PROTO_SRCS} fuzz_vm_proto.cpp)

# Link necessary libraries
target_include_directories(fuzz_vm_proto PUBLIC libprotobuf-mutator)
target_link_libraries(fuzz_vm_proto ${PROTOBUF_LIBRARIES} protobuf-mutator-libfuzzer ton_crypto)

# Set compile and link flags
set_target_properties(fuzz_vm_proto PROPERTIES COMPILE_FLAGS ${FUZZING_FLAGS})
set_target_properties(fuzz_vm_proto PROPERTIES LINK_FLAGS ${FUZZING_FLAGS_BINARY})

# foreach(fuzz_vm_proto)
#   add_executable(${fuzzer} ${fuzzer}.cc)
#   target_link_libraries(${fuzzer}
#     # fuzzer-example-proto
#     protobuf-mutator-libfuzzer
#     ${LIB_PROTO_MUTATOR_FUZZER_LIBRARIES})
#   set_property(TARGET ${fuzzer}
#                PROPERTY COMPILE_FLAGS ${FUZZING_FLAGS})
#   set_property(TARGET ${fuzzer}
#                PROPERTY LINK_FLAGS ${FUZZING_FLAGS_BINARY})
# endforeach(fuzzer)
