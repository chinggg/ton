#include <sys/types.h>

#include "ton/ton-types.h"
#include "crypto/block/block-auto.h"
#include "vm/vm.h"
#include "vm/cp0.h"
#include "vm/dict.h"
#include "td/utils/tests.h"


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

void test_run_vm(td::Slice code_hex) {
  unsigned char buff[128];
  int bits = (int)td::bitstring::parse_bitstring_hex_literal(buff, sizeof(buff), code_hex.begin(), code_hex.end());
  // CHECK(bits >= 0);
  test_run_vm(to_cell(buff, bits));
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    test_run_vm({Data, Size});
    // test_run_vm("ABCBABABABA");
    return 0;
}
