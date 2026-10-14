#pragma once

#include "game_runtime.h"
#include "guest_execution.h"

#include <cstdint>

class Core;
class Game;

namespace spider::enterelectro {

// One host step is one budgeted guest turn, one display field and one presentation fence. The turn
// ends at a libetc VSync wait or when a field's worth of cycles is spent; it presents if the guest
// called submitFrame inside it.
class EnterElectroFrameDriver final : public FrameDriver {
public:
  EnterElectroFrameDriver(Game &game, uint32_t entry, uint32_t presentPoint);

  void installOverrides();
  void stepFrame(Core &core, uint32_t frame) override;

  [[nodiscard]] uint64_t presentedFrames() const;
  [[nodiscard]] uint64_t waitedFields() const;

private:
  static void submitFrame(Core *core);
  static EnterElectroFrameDriver &from(Core &core);

  Game &game_;
  GuestExecution execution_;
  uint32_t entry_;
  uint32_t presentPoint_;
  bool entered_ = false;
  bool submitted_ = false;
  uint64_t presentedFrames_ = 0;
  uint64_t waitedFields_ = 0;
};

} // namespace spider::enterelectro
