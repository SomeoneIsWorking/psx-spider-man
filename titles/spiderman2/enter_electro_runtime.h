#pragma once

#include "game_runtime.h"
#include "guest_program_image.h"

namespace spider::enterelectro {

class EnterElectroRuntime final : public GameRuntime {
public:
  void *createContext(Core &core) override;
  void destroyContext(void *context) override;
  void registerOverrides(Game &game) override;
  void bootInit(Core &core) override;
  std::unique_ptr<FrameDriver> createFrameDriver(Game &game) override;
  const GuestProgramImage *guestProgramImage() const override;
  const PlatformHlePlan *platformHlePlan() const override;
  const GuestPadBufferLayout *guestPadBufferLayout() const override;
  RenderCapabilities renderCapabilities() const override;
  bool guestVramIsPicture(const Game &game) const override;
  void reportRun(Core &core) const override;

private:
  // The guest's own crt0 runs from the executable entry under the frame driver.
  const GuestProgramImage image_ = {
      .bootEntry = BootEntry::DriverOwned,
      // Pre-BSS resident span (BSS starts at 0x800C2AF4), not the PS-X EXE payload extent.
      .residentText = {0x00010000u, 0x000C2AF4u},
  };
};

} // namespace spider::enterelectro
