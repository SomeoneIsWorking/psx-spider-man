#include "runtime/runtime.h"

#include "game.h"

namespace spider {

Runtime::Runtime(const psx::hle::StockVSync &vsync, const psx::input::StockPadBuffers::Layout &pad)
    : vsync_(vsync), pad_(pad) {}

void Runtime::registerOverrides(Game &game) {
  vsync_.bind(game);
}

void Runtime::deliverPad(Core &core, const psx::input::PadDelivery &delivery) {
  pad_.deliver(core, delivery);
}

const GuestWidescreenProjection *Runtime::guestWidescreenProjection() const {
  return &aspectPolicy_;
}

} // namespace spider
