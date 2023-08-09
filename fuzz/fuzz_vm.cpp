#include <sys/types.h>

#include "ton/ton-types.h"
#include "crypto/vm/vm.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    vm::VmState vm;
    vm.run();
    return 0;
}
