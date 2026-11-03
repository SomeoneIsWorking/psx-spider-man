#include "spiderman2/runtime.h"

#include "game.h"
#include "spiderman2/frame_driver.h"
#include "spiderman2/guest_facts.h"

#include <lucent/log.h>

namespace spider::enterelectro {

Runtime::Runtime()
    : spider::Runtime(
          psx::hle::StockVSync({.entry = vsyncAddress, .queryCounter = libetcVblankCountAddress}),
          padBuffers),
      dmaCallbacks_(guestDmaCallbackTable) {}

void Runtime::registerOverrides(Game &game) {
  spider::Runtime::registerOverrides(game);
  dmaCallbacks_.bind(game);
  game.frameDriverAs<FrameDriver>().installOverrides();
}

std::unique_ptr<psx::game::FrameDriver> Runtime::createFrameDriver(Game &game) {
  return std::make_unique<FrameDriver>(game);
}

GuestAddressRange Runtime::guestCodeModuleWindow() const {
  return overlayHeapArena;
}

void Runtime::reportRun(Core &core) const {
  const FrameDriver &driver = core.game->frameDriverAs<FrameDriver>();
  lucent::info("frame",
               "Spider-Man 2 run: {} presented field(s), {} waited field(s)",
               driver.presentedFrames(),
               driver.waitedFields());
}

} // namespace spider::enterelectro
