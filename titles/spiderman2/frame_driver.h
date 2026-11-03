#pragma once

#include "game_runtime.h"

#include <cstdint>

class Core;
class Game;

namespace spider::enterelectro {

// One host step is one budgeted guest turn and one presentation fence. The turn ends at a libetc VSync wait or
// when a field's worth of cycles is spent; it delivers the fields the wait asked for or the guest clock reached,
// each clocking the SPU, and presents if the guest called submitFrame inside it.
class FrameDriver final : public psx::game::FrameDriver {
public:
  explicit FrameDriver(Game &game);

  void installOverrides();
  void stepFrame(Core &core, uint32_t frame) override;

  [[nodiscard]] uint64_t presentedFrames() const;
  [[nodiscard]] uint64_t waitedFields() const;
  [[nodiscard]] uint64_t fields() const;


private:
  static void submitFrame(Core *core);

  Game &game_;
  bool entered_ = false;
  bool submitted_ = false;
  uint64_t presentedFrames_ = 0;
  uint64_t waitedFields_ = 0;
  uint64_t fields_ = 0;
};

} // namespace spider::enterelectro
