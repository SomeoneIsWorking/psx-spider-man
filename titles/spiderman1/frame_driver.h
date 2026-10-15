#pragma once

#include "execution_exit.h"
#include "game_runtime.h"
#include "spiderman1/level_warp.h"

#include <cstdint>
#include <functional>
#include <string>

class Core;
class Game;

namespace spider::spider1 {

// Retail main as host turns: each step runs the guest to a measured boundary, services it as one
// display field and one presentation fence, and leaves the guest to resume at its own PC.
class FrameDriver final : public psx::game::FrameDriver {
public:
  using ExecuteSlice = std::function<psx::cpu::ExecutionResult(Core &, std::uint32_t)>;

  // `execute` runs one guest turn from an address; the product passes `executeHostTurn`.
  FrameDriver(Game &game, ExecuteSlice execute);

  static psx::cpu::ExecutionResult executeHostTurn(Core &core, std::uint32_t address);

  void installOverrides();
  // Points the first turn at retail main with crt0's return address.
  void beginGuest(Core &core);
  void stepFrame(Core &core, std::uint32_t frame) override;
  // Arms a `warp <level>` request; a later step raises it through the guest's own level route.
  std::string armWarp(Core &core, const char *line);
  static FrameDriver &from(Core &core);

private:
  // A stop at a measured boundary, serviced as one display field; false means it is not one.
  bool serviceBoundary(Core &core, const psx::cpu::ExecutionResult &result);
  void serviceResetGraphVsync(Core &core);
  void serviceMovieVsync(Core &core);
  // One field for a wait the guest cannot yield itself: StGetNext not ready or a spent turn budget.
  void deliverWaitField(Core &core, std::uint32_t continuation);
  void deliverField(Core &core);
  void commitField(Core &core);
  void completeVsync(Core &core, std::uint32_t returnValue);

  static void captureVsyncCallback(Core *core);
  static void initializeCd(Core *core);
  static void startGpuDmaTimeout(Core *core);

  Game &game_;
  ExecuteSlice execute_;
  LevelWarp warp_;
  std::uint32_t vsyncCallback_ = 0;
  std::uint32_t movieFieldCount_ = 0;
  std::uint32_t fieldsSinceCommit_ = 0;
};

} // namespace spider::spider1
