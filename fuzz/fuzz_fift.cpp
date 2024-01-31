#include <cstdint>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>

#include "td/utils/buffer.h"
#include "ton/ton-types.h"
#include "crypto/block/block-auto.h"
#include "vm/boc.h"
#include "vm/cells/CellBuilder.h"
#include "vm/cells/CellSlice.h"
#include "vm/vm.h"
#include "vm/cp0.h"
#include "vm/dict.h"
#include "fift/utils.h"
#include "td/utils/tests.h"


#include "FuzzedDataProvider.h"


// namespace ton {

// namespace validator {

std::string run_vm(td::Ref<vm::Cell> cell) {
  vm::init_op_cp0();
  vm::DictionaryBase::get_empty_dictionary();

  class Logger : public td::LogInterface {
   public:
    void append(td::CSlice slice) override {
      res.append(slice.data(), slice.size());
    }
    std::string res;
  };
  static Logger logger;
  logger.res = "";
  td::set_log_fatal_error_callback([](td::CSlice message) { td::default_log_interface->append(logger.res); });
  vm::VmLog log{&logger, td::LogOptions::plain()};
  log.log_options.level = 4;
  log.log_options.fix_newlines = true;
  log.log_mask |= vm::VmLog::DumpStack;

  auto total_data_cells_before = vm::DataCell::get_total_data_cells();
  SCOPE_EXIT {
    auto total_data_cells_after = vm::DataCell::get_total_data_cells();
    ASSERT_EQ(total_data_cells_before, total_data_cells_after);
  };

  vm::Stack stack;
  try {
    vm::GasLimits gas_limit(1000, 1000);

    vm::run_vm_code(vm::load_cell_slice_ref(cell), stack, 0 /*flags*/, nullptr /*data*/, std::move(log) /*VmLog*/,
                    nullptr, &gas_limit);
  } catch (...) {
    LOG(FATAL) << "catch unhandled exception";
  }
  return logger.res;  // must be a copy
}

td::Ref<vm::Cell> to_cell(const unsigned char *buff, int bits) {
  return vm::CellBuilder().store_bits(buff, bits, 0).finalize();
}

void test_run_vm(td::Ref<vm::Cell> code) {
  auto a = run_vm(code);
  auto b = run_vm(code);
  ASSERT_EQ(a, b);
  // REGRESSION_VERIFY(a);
}

td::Ref<vm::Cell> hex2cell(td::Slice code_hex) {
  unsigned char buff[128];
  int bits = (int)td::bitstring::parse_bitstring_hex_literal(buff, sizeof(buff), code_hex.begin(), code_hex.end());
  printf("%s\n", buff);
  auto cell = to_cell(buff, bits);
  return cell;
}

td::Ref<vm::Cell> str2cell(std::string str) {
  return vm::CellBuilder().store_bytes(str).finalize();
}

td::BufferSlice writecell2boc(td::Ref<vm::Cell> code, std::string write2file = "") {
  auto boc = vm::std_boc_serialize(code).move_as_ok();
  printf("%s\n", boc.data());
  printf("%zu\n", boc.size());
  if (write2file != "") {
    int fd = open(write2file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd, boc.data(), boc.size());
    close(fd);
  }
  return boc;
}

std::string gen_fift_asm() {
  // TODO: generate random fift asm
  return R"A(
CONT:<{
1 INT
2 INT
3 INT
2 RETARGS
}>
CALLX
ADD
)A";
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    FuzzedDataProvider fdp(Data, Size);
    auto asm_code = gen_fift_asm();
    auto code = fift::compile_asm(asm_code).move_as_ok();
    auto data = str2cell(fdp.ConsumeRandomLengthString(127));

    // **** INIT VM ****
    vm::init_op_cp0();
    vm::GasLimits gas{300000, 300000};
    td::Ref<vm::Stack> stack{true};
    vm::VmState vm{std::move(code), std::move(stack), gas, 1, std::move(data), vm::VmLog::Null()};
    // auto c7 = prepare_vm_c7(gen_utime, gen_lt, td::make_ref<vm::CellSlice>(acc.addr->clone()), balance);
    // vm.set_c7(c7);  // tuple with SmartContra,ctInfo
    // **** RUN VM ****
    int exit_code = ~vm.run();
    return exit_code;
}