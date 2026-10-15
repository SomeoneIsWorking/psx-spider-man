#include "spiderman1/field_clock.h"

#include "spiderman1/guest_layout.h"
#include "spiderman1/platform_facts.h"

#include "core.h"

namespace spider::spider1 {

uint32_t FieldClock::fieldCount(Core &core) {
  return core.mem_r32(libetcVblankCountAddress);
}

uint32_t FieldClock::horizontalCounter(Core &core) {
  const uint32_t pointer = core.mem_r32(horizontalCounterPointer);
  return pointer != 0 ? core.mem_r32(pointer) : 0u;
}

uint32_t FieldClock::vsyncReturnValue(Core &core) {
  return (horizontalCounter(core) - core.mem_r32(horizontalCounterBaseline)) & 0xFFFFu;
}

void FieldClock::recordDeliveredField(Core &core) {
  core.mem_w32(lastVsyncField, core.mem_r32(libetcVblankCountAddress));
  core.mem_w32(horizontalCounterBaseline, horizontalCounter(core));
  // The retail VSync tail reads the GPU status word; the read is kept for its observable side
  // effect and the result is discarded.
  (void)core.mem_r32(gpuStatusWord);
}

} // namespace spider::spider1
