#include "enter_electro_frame_driver.h"

#include "core.h"
#include "execution_exit.h"
#include "game.h"
#include "logic_frame.h"
#include "native_execution.h"

#include <cstdlib>
#include <lucent/log.h>

namespace spider::enterelectro {

EnterElectroFrameDriver::EnterElectroFrameDriver(Game &game, uint32_t entry, uint32_t presentPoint)
    : game_(game), execution_(game.core), entry_(entry), presentPoint_(presentPoint) {}

EnterElectroFrameDriver &EnterElectroFrameDriver::from(Core &core) {
  auto *driver =
      core.game ? dynamic_cast<EnterElectroFrameDriver *>(core.game->frameDriver.get()) : nullptr;
  if (!driver) {
    lucent::error("frame", "Spider-Man 2 submitFrame ran without its title FrameDriver");
    std::abort();
  }
  return *driver;
}

void EnterElectroFrameDriver::installOverrides() {
  installNativeOverride(game_.core, presentPoint_, "Spider-Man 2 submitFrame", submitFrame);
}

void EnterElectroFrameDriver::submitFrame(Core *core) {
  EnterElectroFrameDriver &driver = from(*core);
  if (!callOriginalOrPropagate(*core, driver.presentPoint_)) {
    return;
  }
  // The field this turn ends on presents it; the guest's own VBlank wait owns the cadence.
  driver.submitted_ = true;
}

void EnterElectroFrameDriver::stepFrame(Core &core, uint32_t frame) {
  submitted_ = false;
  const psx::cpu::ExecutionResult result =
      entered_ ? execution_.resumeAt(core.pc) : execution_.enter(entry_);
  entered_ = true;
  const bool fieldPassed =
      result.reason == psx::cpu::ExecutionExitReason::FrameBoundary ||
      (result.reason == psx::cpu::ExecutionExitReason::BudgetExhausted && result.cycles != 0);
  if (!fieldPassed) {
    lucent::error("frame", "Spider-Man 2 guest turn {} cannot continue", frame);
    reportExecutionResult(core, result, "Spider-Man 2");
    game_.presentation.commitUnpresented(&core);
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
  game_.presentation.commit(&core, 1, nullptr);
  // libpad polls on every VBlank; a submitted frame also starts the next logic frame, which
  // services the pad itself.
  if (submitted_) {
    psx::frame::LogicFrame::begin(game_, game_.timing.logicFrame + 1u);
  } else {
    game_.pad.serviceFrame();
  }
}

uint64_t EnterElectroFrameDriver::presentedFrames() const {
  return presentedFrames_;
}

uint64_t EnterElectroFrameDriver::waitedFields() const {
  return waitedFields_;
}

} // namespace spider::enterelectro
