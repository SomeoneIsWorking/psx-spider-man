#include "spiderman1/frame_driver.h"

#include "spiderman1/cd_initialization.h"
#include "spiderman1/field_clock.h"
#include "spiderman1/guest_layout.h"
#include "spiderman1/platform_facts.h"
#include "spiderman1/stream_driver.h"

#include "core.h"
#include "execution/guest_execution.h"
#include "execution_control.h"
#include "execution_services.h"
#include "game.h"
#include "host_turn.h"
#include "logic_frame.h"
#include "guest_call.h"
#include "native_dispatch.h"

#include <cstdlib>
#include <lucent/log.h>
#include <utility>

namespace spider::spider1 {

FrameDriver::FrameDriver(Game &game, ExecuteSlice execute)
    : game_(game), execute_(std::move(execute)) {}

psx::cpu::ExecutionResult FrameDriver::executeHostTurn(Core &core, std::uint32_t address) {
  return psx::cpu::dispatchGuestUntilExitNow(core, address);
}

void FrameDriver::installOverrides() {
  // Stock GPU DMA timeout arm; its VSync(-1) just reads the title field count.
  psx::cpu::installNativeOverride(
      game_.core, gpuDmaTimeoutStart, "Spider GPU DMA timeout", startGpuDmaTimeout);
  psx::cpu::installNativeOverride(game_.core, cdInitAddress, "Spider CdInit", initializeCd);
  game_.platform_hle.bindStock(vsyncCallbackRegistration, captureVsyncCallback);
}

void FrameDriver::beginGuest(Core &core) {
  core.r[31] = crt0MainReturn;
  core.pc = gameMainEntry;
}

std::string FrameDriver::armWarp(std::string level) {
  return warp_.arm(std::move(level));
}

void FrameDriver::stepFrame(Core &core, std::uint32_t) {
  warp_.apply(core);
  const psx::cpu::ExecutionResult result = execute_(core, core.pc);
  // A spent cycle budget is a field of guest time: deliver it; the next turn resumes there.
  if (result.reason == psx::cpu::ExecutionExitReason::BudgetExhausted && result.cycles != 0) {
    deliverWaitField(core, result.guestPc);
    return;
  }
  if (!serviceBoundary(core, result)) {
    reportExecutionResult(core, result, "Spider-Man 1");
    game_.presentation.commitUnpresented();
    game_.run.requestEnd();
  }
}

bool FrameDriver::serviceBoundary(Core &core, const psx::cpu::ExecutionResult &result) {
  if (result.reason == psx::cpu::ExecutionExitReason::CooperativeYield &&
      result.guestPc == stGetNextAddress && core.pc == result.guestPc) {
    // StGetNext answered "not ready": the field that passes; VSync state is untouched.
    if (core.r[2] == 0) {
      return false;
    }
    deliverWaitField(core, core.r[31]);
    return true;
  }
  if (result.reason != psx::cpu::ExecutionExitReason::FrameBoundary || core.r[4] != 0) {
    return false;
  }
  // The resume address must be one of the authenticated `jal` return PCs: a leaf's entry would
  // re-request the boundary forever.
  if (core.r[31] != resetGraphVsyncReturn && !isMovieFieldReturn(core.r[31])) {
    return false;
  }
  if (result.guestPc != core.r[31] || core.pc != core.r[31]) {
    return false;
  }
  if (core.r[31] == resetGraphVsyncReturn) {
    serviceResetGraphVsync(core);
  } else {
    serviceMovieVsync(core);
  }
  return true;
}

void FrameDriver::serviceResetGraphVsync(Core &core) {
  // ResetGraph's early VSync(0) runs before anything is drawn, so the fence is unpresented.
  const std::uint32_t returnValue = FieldClock::vsyncReturnValue(core);
  deliverField(core);
  completeVsync(core, returnValue);
  game_.presentation.commitUnpresented();
  core.pc = core.r[31];
  lucent::info("frame", "Spider-Man 1 completed pre-main ResetGraph field at 0x{:08X}", core.pc);
}

void FrameDriver::serviceMovieVsync(Core &core) {
  // The retail STR player stopped at libetc VSync; deliver that field, resume at the return PC.
  const std::uint32_t returnPc = core.r[31];
  const std::uint32_t returnValue = FieldClock::vsyncReturnValue(core);
  deliverField(core);
  if (core.executionControl().pending()) {
    lucent::error("str",
                  "Spider-Man 1 field callback exited before completing the retail STR field");
    std::abort();
  }
  completeVsync(core, returnValue);
  commitField(core);
  ++movieFieldCount_;
  core.pc = returnPc;
  lucent::info(
      "str", "Spider-Man 1 resumed retail STR field {} at 0x{:08X}", movieFieldCount_, core.pc);
}

void FrameDriver::deliverWaitField(Core &core, std::uint32_t continuation) {
  deliverField(core);
  if (core.executionControl().pending()) {
    lucent::error("frame", "Spider-Man 1 field callback exited during a direct field wait");
    std::abort();
  }
  commitField(core);
  core.pc = continuation;
}

void FrameDriver::deliverField(Core &core) {
  core.mem_w32(libetcVblankCountAddress, core.mem_r32(libetcVblankCountAddress) + 1u);
  game_.spu_audio.frame();
  if (vsyncCallback_ != 0) {
    const R3000 saved = static_cast<const R3000 &>(core);
    if (!psx::cpu::dispatchGuestOrPropagate(core, vsyncCallback_, "vsync-callback")) {
      return;
    }
    static_cast<R3000 &>(core) = saved;
  }
  core.hostTurn.onDisplayField(core);
}

void FrameDriver::commitField(Core &core) {
  game_.presentation.commit(&core, 1);
  // The next logic frame starts after presenting, so its OT attributes survive the fence.
  psx::frame::LogicFrame::begin(game_, game_.timing.logicFrame + 1u);
}

void FrameDriver::completeVsync(Core &core, std::uint32_t returnValue) {
  FieldClock::recordDeliveredField(core);
  core.r[2] = returnValue;
}

void FrameDriver::captureVsyncCallback(Core *core) {
  FrameDriver &driver = core->game->frameDriverAs<FrameDriver>();
  if (core->r[4] != driver.vsyncCallback_) {
    lucent::info("frame",
                 "Spider-Man 1 registered field callback 0x{:08X} (was 0x{:08X})",
                 core->r[4],
                 driver.vsyncCallback_);
  }
  driver.vsyncCallback_ = core->r[4];
  core->r[2] = 0;
}

void FrameDriver::initializeCd(Core *core) {
  CdInitialization::install(*core);
}

void FrameDriver::startGpuDmaTimeout(Core *core) {
  // SLUS_008.75 0x80083C60 minus its VSync(-1) query at 0x80083C68.
  const std::uint32_t deadline = core->mem_r32(libetcVblankCountAddress) + gpuDmaTimeoutFieldBudget;
  core->r[2] = deadline;
  core->mem_w32(gpuDmaTimeoutDeadline, deadline);
  core->mem_w32(gpuDmaTimeoutPollCount, 0);
  psx::cpu::accountGuestInstructions(*core, gpuDmaTimeoutRetailInstructions);
}

} // namespace spider::spider1
