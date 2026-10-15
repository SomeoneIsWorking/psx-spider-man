// Field clock and CdInit owners against a bare Core.

#include "spiderman1/cd_initialization.h"
#include "spiderman1/field_clock.h"
#include "spiderman1/guest_layout.h"
#include "spiderman1/platform_facts.h"

#include "core.h"
#include "stub_game_runtime.h"
#include "testutil.h"

#include <cstdint>
#include <memory>

using namespace spider::spider1;

namespace {

// Resident image over main RAM in physical terms; nothing executes the bytes.
constexpr uint32_t kResidentStart = 0x00010000u;
constexpr uint32_t kResidentBytes = 0x00200000u;

// A Core with the resident image active, over a runtime whose virtuals the owners never read.
class TestCore {
public:
  // Core is 2 MB of RAM, so it lives on the heap.
  explicit TestCore(const char *name) : core_(std::make_unique<Core>(runtime_)) {
    core_->imageCatalog().activate(name, {kResidentStart, kResidentStart + kResidentBytes}, 1u);
  }

  Core &core() {
    return *core_;
  }

private:
  psx::test::StubGameRuntime runtime_;
  std::unique_ptr<Core> core_;
};

void test_field_clock_measures_since_the_previous_field() {
  TestCore fixture("field clock");
  Core &core = fixture.core();
  // A null counter pointer reads as zero, as the retail load does.
  core.mem_w32(horizontalCounterPointer, 0u);
  CHECK_EQ(FieldClock::horizontalCounter(core), 0u);
  CHECK_EQ(FieldClock::vsyncReturnValue(core), 0u);

  // Returns the advance since the last field, truncated to 16 bits.
  core.mem_w32(horizontalCounterPointer, 0x800A0000u);
  core.mem_w32(0x800A0000u, 1000u);
  core.mem_w32(horizontalCounterBaseline, 900u);
  CHECK_EQ(FieldClock::horizontalCounter(core), 1000u);
  CHECK_EQ(FieldClock::vsyncReturnValue(core), 100u);

  // The 16-bit wrap holds both ways: counter wrapped, and baseline ahead of counter.
  core.mem_w32(0x800A0000u, 5u);
  core.mem_w32(horizontalCounterBaseline, 0xFFFFu);
  CHECK_EQ(FieldClock::vsyncReturnValue(core), 6u);
  core.mem_w32(0x800A0000u, 0xFFF0u);
  core.mem_w32(horizontalCounterBaseline, 0x0010u);
  CHECK_EQ(FieldClock::vsyncReturnValue(core), 0xFFE0u);
}

void test_field_clock_rebases_only_after_the_value_was_read() {
  TestCore fixture("field clock rebase");
  Core &core = fixture.core();
  core.mem_w32(horizontalCounterPointer, 0x800A0000u);
  core.mem_w32(0x800A0000u, 4242u);
  core.mem_w32(horizontalCounterBaseline, 4000u);
  core.mem_w32(libetcVblankCountAddress, 77u);

  const uint32_t returnValue = FieldClock::vsyncReturnValue(core);
  CHECK_EQ(returnValue, 242u);
  FieldClock::recordDeliveredField(core);
  // Recording rebases the counter; the value is read before the record.
  CHECK_EQ(core.mem_r32(lastVsyncField), 77u);
  CHECK_EQ(core.mem_r32(horizontalCounterBaseline), 4242u);
  CHECK_EQ(FieldClock::vsyncReturnValue(core), 0u);
  CHECK_EQ(FieldClock::fieldCount(core), 77u);
}

void test_cd_initialization_installs_every_callback_slot() {
  // Slot half against real guest RAM: four slots plus the success result.
  TestCore fixture("cd init");
  Core &core = fixture.core();
  CdInitialization::installCallbackSlots(core);
  CHECK_EQ(core.mem_r32(cdSyncCallbackSlot), cdSyncCallback);
  CHECK_EQ(core.mem_r32(cdReadyCallbackSlot), cdReadyCallback);
  CHECK_EQ(core.mem_r32(cdEventCallbackSlot), cdEventCallback);
  CHECK_EQ(core.mem_r32(cdEventUnusedSlot), 0u);
  CHECK_EQ(core.r[2], 1u);
  // A second run writes the same constants.
  CdInitialization::installCallbackSlots(core);
  CHECK_EQ(core.mem_r32(cdSyncCallbackSlot), cdSyncCallback);
  CHECK_EQ(core.mem_r32(cdReadyCallbackSlot), cdReadyCallback);
  CHECK_EQ(core.mem_r32(cdEventCallbackSlot), cdEventCallback);
  CHECK_EQ(core.mem_r32(cdEventUnusedSlot), 0u);
}

} // namespace

int main() {
  RUN(field_clock_measures_since_the_previous_field);
  RUN(field_clock_rebases_only_after_the_value_was_read);
  RUN(cd_initialization_installs_every_callback_slot);
  return pt_summary();
}
