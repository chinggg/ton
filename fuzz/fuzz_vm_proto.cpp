#include <iomanip>
#include <iostream>

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"
#include "boc.pb.h"

#include "td/utils/int_types.h"
#include "vm/boc.h"
#include "vm/cells/CellBuilder.h"
#include "vm/cells/CellSlice.h"
#include "vm/cp0.h"
#include "vm/vm.h"

std::string stringToHex(const std::string& input) {
    std::ostringstream hexStream;
    hexStream << std::hex << std::setfill('0');
    for (unsigned char c : input) {
        hexStream << std::setw(2) << static_cast<int>(c);
    }
    return hexStream.str();
}

std::string fromhex(const std::string& hexstr) {
    std::string bytes;
    for (size_t i = 0; i < hexstr.length(); i += 2) {
        std::string byteString = hexstr.substr(i, 2);
        char byte = static_cast<char>(std::stoul(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string BoC2String(const boc::BagOfCells* boc) {
    std::stringstream ss;
    ss << std::hex 
    << boc->info().magic() 
    << boc->info().ref_byte_size()
    << boc->info().offset_byte_size()
    << boc->cell_count()
    << boc->root_count()
    << 0
    << boc->info().data_size();

    return fromhex(ss.str());
}

DEFINE_BINARY_PROTO_FUZZER(boc::TwoBoC& message) {
    td::int32 boc_idx = 0x68ff65f3, boc_idx_crc32c = 0xacc3a728, boc_generic = 0xb5ee9c72;
    auto code_msg = message.mutable_code();
    auto data_msg = message.mutable_data();
    std::cout << "magic: " << code_msg->info().magic() << std::endl;
    code_msg->mutable_info()->set_magic(boc_generic);
    data_msg->mutable_info()->set_magic(boc_generic);
    std::cout << "magic: " << code_msg->info().magic() << std::endl;

    vm::BagOfCells code_boc, data_boc;
    // auto code_str = code_msg->SerializeAsString();
    auto code_str = BoC2String(code_msg);
    std::cout << "code_str: " << code_str << std::endl;
    // vm::std_boc_deserialize checks only one cell, use boc::deserialize instead
    // read boc string as hex number
    auto code_res = code_boc.deserialize(code_str);
    if (code_res.is_error()) {
        return;
    }
    auto code = code_boc.get_root_cell();
    if (code.is_null()) {
        return;
    }

    auto data_str = data_msg->SerializeAsString();
    auto data_res = data_boc.deserialize(data_str);
    if (data_res.is_error()) {
        return;
    }
    auto data = data_boc.get_root_cell();
    if (data.is_null()) {
        return;
    }

    vm::init_op_cp0();
    vm::GasLimits gas{300000, 300000};
    td::Ref<vm::Stack> stack{true};
    vm::VmState vm{std::move(code), std::move(stack), gas, 1, std::move(data), vm::VmLog::Null()};
    vm.run();
}
