#pragma once

#include "execution_exit.h"
#include "game_runtime.h"
#include "spiderman1/level_warp.h"
#include "spiderman1/screen_skip.h"

#include <cstdint>
#include <functional>
#include <string>

class Core;
class Game;

namespace spider::spider1 {

// Retail main as host turns: each step runs the guest to a measured boundary, services its fields
// and one presentation fence, and leaves the guest to resume at its own PC. The field wait
// FUN_8005E748 is native: the gameplay loop's frame wait delivers the retail frame, max(2, K + 1)
// fields.
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
  // Arms a level arrival; a later step raises it through the guest's own level route.
  std::string armWarp(std::string level);

private:
  // A stop at a measured boundary, serviced as one display field; false means it is not one.
  bool serviceBoundary(Core &core, const psx::cpu::ExecutionResult &result);
  void serviceResetGraphVsync(Core &core);
  void serviceMovieVsync(Core &core);
  // The fields a native field wait owes, then the guest resumes at its return.
  void serviceFieldWait(Core &core, std::uint32_t continuation);
  // One field for a wait the guest cannot yield itself: StGetNext not ready or a spent turn budget.
  void deliverWaitField(Core &core, std::uint32_t continuation);
  // One field of a load floor: delivered and fenced, never presented.
  void deliverLoadField(Core &core, std::uint32_t continuation);
  void deliverField(Core &core);
  void commitFields(Core &core, std::uint32_t fields);
  void completeVsync(Core &core, std::uint32_t returnValue);

  static void captureVsyncCallback(Core *core);
  static void initializeCd(Core *core);
  static void startGpuDmaTimeout(Core *core);
  static void waitFields(Core *core);
  void holdField(Core &core);

  Game &game_;
  ExecuteSlice execute_;
  LevelWarp warp_;
  ScreenSkip skip_;
  std::uint32_t vsyncCallback_ = 0;
  std::uint32_t movieFieldCount_ = 0;
  // Fields the pending native field wait delivers; 0 while none is pending.
  std::uint32_t waitFields_ = 0;
  // The pending wait ends a gameplay frame, which presents once; any other wait shows every field
  // it holds.
  bool frameWait_ = false;
  // Fields such a wait still holds, one per step.
  std::uint32_t heldFields_ = 0;
  // Fields delivered since the last field wait returned: the gameplay frame's work so far.
  std::uint32_t frameFields_ = 0;
};

} // namespace spider::spider1
