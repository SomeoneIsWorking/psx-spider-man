#include "enter_electro_runtime.h"

#include "enter_electro_frame_driver.h"
#include "enter_electro_guest_facts.h"

#include "game.h"

#include <cstdlib>
#include <lucent/log.h>

namespace spider::enterelectro {

void *EnterElectroRuntime::createContext(Core &) {
  return nullptr;
}

void EnterElectroRuntime::destroyContext(void *) {}

void EnterElectroRuntime::registerOverrides(Game &game) {
  auto *driver = dynamic_cast<EnterElectroFrameDriver *>(game.frameDriver.get());
  if (!driver) {
    lucent::error("overrides", "Spider-Man 2 override registration has no title frame driver");
    std::abort();
  }
  driver->installOverrides();
}

// The frame driver enters the guest on its first step.
void EnterElectroRuntime::bootInit(Core &) {}

std::unique_ptr<FrameDriver> EnterElectroRuntime::createFrameDriver(Game &game) {
  return std::make_unique<EnterElectroFrameDriver>(game, crt0Entry, submitFrameAddress);
}

const GuestProgramImage *EnterElectroRuntime::guestProgramImage() const {
  return &image_;
}

const PlatformHlePlan *EnterElectroRuntime::platformHlePlan() const {
  return &platformServices;
}

const GuestPadBufferLayout *EnterElectroRuntime::guestPadBufferLayout() const {
  return &padBuffers;
}

RenderCapabilities EnterElectroRuntime::renderCapabilities() const {
  // No native producer or temporal product.
  return RenderCapabilities::widescreenOnly();
}

bool EnterElectroRuntime::guestVramIsPicture(const Game &) const {
  // submitFrame draws the whole frame into the displayed VRAM page.
  return true;
}

void EnterElectroRuntime::reportRun(Core &core) const {
  const auto *driver =
      core.game ? dynamic_cast<const EnterElectroFrameDriver *>(core.game->frameDriver.get())
                : nullptr;
  if (!driver) {
    return;
  }
  lucent::info("frame",
               "Spider-Man 2 run: {} presented field(s), {} waited field(s)",
               driver->presentedFrames(),
               driver->waitedFields());
}

} // namespace spider::enterelectro
