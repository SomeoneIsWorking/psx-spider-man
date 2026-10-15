#pragma once

#include "execution/guest_execution.h"
#include "game_runtime.h"

#include <cstdint>

class Core;
class Game;

namespace spider::enterelectro {

// One host step is one budgeted guest turn, one display field and one presentation fence. The turn
// ends at a libetc VSync wait or when a field's worth of cycles is spent; it presents if the guest
// called submitFrame inside it.
class FrameDriver final : public psx::game::FrameDriver {
public:
  explicit FrameDriver(Game &game);

  void installOverrides();
  void stepFrame(Core &core, uint32_t frame) override;

  [[nodiscard]] uint64_t presentedFrames() const;
  [[nodiscard]] uint64_t waitedFields() const;

  // The driver of the Game `core` belongs to; aborts when it is another title's.
  static FrameDriver &from(Core &core);

private:
  static void submitFrame(Core *core);

  Game &game_;
  GuestExecution execution_;
  bool entered_ = false;
  bool submitted_ = false;
  uint64_t presentedFrames_ = 0;
  uint64_t waitedFields_ = 0;
};

} // namespace spider::enterelectro
