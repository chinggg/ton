#include <iostream>

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"
#include "boc.pb.h"

#include "vm/boc.h"
#include "vm/cells/CellBuilder.h"
#include "vm/cells/CellSlice.h"
#include "vm/cp0.h"
#include "vm/vm.h"

DEFINE_BINARY_PROTO_FUZZER(const BagOfCells& message) {
    vm::BagOfCells boc;
    boc.deserialize(message.SerializeAsString());
    auto code = boc.get_root_cell(0);
    // auto code = vm::std_boc_deserialize(message.SerializeAsString()).move_as_ok();
    vm::init_op_cp0();
    vm::GasLimits gas{300000, 300000};
    td::Ref<vm::Stack> stack{true};
    vm::VmState vm{std::move(code), std::move(stack), gas, 1, vm::Ref<vm::Cell>{}, vm::VmLog::Null()};
    vm.run();
}
