#pragma once

#include "game_runtime.h"
#include "guest_widescreen_projection.h"
#include "stock_pad_buffers.h"
#include "stock_vsync.h"

namespace spider {

// What both Spider-Man titles share at the framework seam: libetc VSync as the field boundary,
// libpad's receive buffers, and the Record picture whose 16:9 canvas carries the margins.
class Runtime : public psx::game::GameRuntime {
public:
  void registerOverrides(Game &game) override;
  void deliverPad(Core &core, const psx::input::PadDelivery &delivery) override;
  const GuestWidescreenProjection *guestWidescreenProjection() const override;

protected:
  Runtime(const psx::hle::StockVSync &vsync, const psx::input::StockPadBuffers::Layout &pad);

private:
  psx::hle::StockVSync vsync_;
  psx::input::StockPadBuffers pad_;
  GuestWidescreenProjection aspectPolicy_;
};

} // namespace spider
