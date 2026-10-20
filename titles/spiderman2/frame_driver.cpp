#include "spiderman2/frame_driver.h"

#include "core.h"
#include "execution/guest_execution.h"
#include "execution_exit.h"
#include "game.h"
#include "guest_call.h"
#include "logic_frame.h"
#include "native_dispatch.h"
#include "spiderman2/guest_facts.h"

#include <cstdlib>
#include <lucent/log.h>

namespace spider::enterelectro {

FrameDriver::FrameDriver(Game &game) : game_(game) {}

void FrameDriver::installOverrides() {
  psx::cpu::installNativeOverride(
      game_.core, submitFrameAddress, "Spider-Man 2 submitFrame", submitFrame);
}

void FrameDriver::submitFrame(Core *core) {
  FrameDriver &driver = core->game->frameDriverAs<FrameDriver>();
  if (!psx::cpu::callOriginalOrPropagate(*core, submitFrameAddress)) {
    return;
  }
  // The field this turn ends on presents it; the guest's own VBlank wait owns the cadence.
  driver.submitted_ = true;
}

void FrameDriver::stepFrame(Core &core, uint32_t frame) {
  submitted_ = false;
  // The first turn enters crt0 at the executable entry admission left in pc.
  const psx::cpu::ExecutionResult result =
      entered_ ? psx::cpu::dispatchGuestUntilExitNow(core, core.pc) : psx::cpu::dispatchGuestNow(core, core.pc);
  entered_ = true;
  const bool fieldPassed =
      result.reason == psx::cpu::ExecutionExitReason::FrameBoundary ||
      (result.reason == psx::cpu::ExecutionExitReason::BudgetExhausted && result.cycles != 0);
  if (!fieldPassed) {
    lucent::error("frame", "Spider-Man 2 guest turn {} cannot continue", frame);
    reportExecutionResult(core, result, "Spider-Man 2");
    game_.presentation.commitUnpresented();
    game_.run.requestEnd();
    return;
  }
  if (submitted_) {
    ++presentedFrames_;
  } else {
    // No submit this field: the display rescans the last submitted frame.
    ++waitedFields_;
    lucent::debug("frame",
                  "Spider-Man 2 turn {} waited a field at 0x{:08X} ({})",
                  frame,
                  result.guestPc,
                  psx::cpu::executionExitName(result.reason));
  }
  game_.presentation.commit(&core, 1);
  // libpad polls on every VBlank; a submitted frame also starts the next logic frame, which
  // services the pad itself.
  if (submitted_) {
    psx::frame::LogicFrame::begin(game_, game_.timing.logicFrame + 1u);
  } else {
    game_.pad.serviceFrame();
  }
}

uint64_t FrameDriver::presentedFrames() const {
  return presentedFrames_;
}

uint64_t FrameDriver::waitedFields() const {
  return waitedFields_;
}

} // namespace spider::enterelectro
