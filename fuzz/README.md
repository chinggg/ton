## Ton Fuzz Targets

### Fuzz VM

构造 vm，并执行 `vm.run()`，试图发现 crash

```cmake
add_executable(fuzz_vm fuzz_vm.cpp)
target_compile_options(fuzz_vm PUBLIC -fsanitize=fuzzer,address -fno-omit-frame-pointer -g)
target_link_libraries(fuzz_vm PUBLIC ton_crypto -fsanitize=fuzzer,address -fno-omit-frame-pointer -g)
```

### Structure-Aware Fuzzing with Protobuf

构造 vm 需要 code: Cell，原本 fuzz target 从 buf 直接序列化，怀疑效率不高

现在定义 BagOfCells (`crypto/tl/boc.tlb`) 的 protobuf 格式，仿照 https://github.com/google/libprotobuf-mutator/blob/master/examples/libfuzzer/libfuzzer_bin_example.cc，`DEFINE_BINARY_PROTO_FUZZER(const BagOfCells& message)`

这样输入的是 BagOfCells 格式的 protobuf message，反序列化为 ton 中的 vm::BagOfCells 类，再 get_root_cell() 获得 code 传入 target

### Fuzz VM by mutating Fift ASM

尝试生成/变异 Fift 汇编代码，调用 fift::compile_asm 编译为 boc，最终调用 vm.run()

### Fuzz coverage report HTML

```
git clone https://github.com/vanhauser-thc/libfuzzer-cov
cd ton
./b.sh && ninja -C build fuzz_vm
../libfuzzer-cov/cov-build.sh ./b.sh build_cov && ninja -C build_cov fuzz_vm
./build/fuzz_vm fuzz/seeds/
../libfuzzer-cov/cov-generate.sh ../ton/fuzz/seeds/ ./build_cov/fuzz_vm
```