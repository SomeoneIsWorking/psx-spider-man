#include "spiderman1/load_floor.h"

#include "core.h"
#include "execution_control.h"
#include "native_dispatch.h"

namespace spider::spider1 {

void LoadFloor::install(Core &core) {
  psx::cpu::installNativeOverride(core, padRead, "Spider Pad_read", readPad);
}

bool LoadFloor::polled(const Core &core, std::uint32_t guestPc) {
  return guestPc == padRead && core.pc == padRead && core.r[31] == bootLoadFloorReturn;
}

void LoadFloor::readPad(Core *core) {
  const std::uint32_t caller = core->r[31];
  if (!psx::cpu::callOriginalOrPropagate(*core, padRead) || caller != bootLoadFloorReturn) {
    return;
  }
  psx::cpu::requestExecutionExit(*core,
                                 {psx::cpu::ExecutionExitReason::CooperativeYield,
                                  padRead,
                                  0,
                                  "Spider-Man 1 boot load floor polled through one field"});
}

} // namespace spider::spider1
