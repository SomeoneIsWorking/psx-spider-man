#include "spiderman2/runtime.h"

#include "game.h"
#include "spiderman2/frame_driver.h"
#include "spiderman2/guest_facts.h"
#include "spiderman2/overlay_loader.h"

#include <lucent/log.h>

namespace spider::enterelectro {

Runtime::Runtime()
    : spider::Runtime(
          psx::hle::StockVSync({.entry = vsyncAddress, .queryCounter = libetcVblankCountAddress}),
          padBuffers,
          guestDmaCallbackTable) {}

void Runtime::registerOverrides(Game &game) {
  spider::Runtime::registerOverrides(game);
  OverlayLoader::install(game.core);
  game.frameDriverAs<FrameDriver>().installOverrides();
}

std::unique_ptr<psx::game::FrameDriver> Runtime::createFrameDriver(Game &game) {
  return std::make_unique<FrameDriver>(game);
}

void Runtime::reportRun(Core &core) const {
  const FrameDriver &driver = core.game->frameDriverAs<FrameDriver>();
  lucent::info("frame",
               "Spider-Man 2 run: {} presented turn(s), {} waited turn(s), {} field(s)",
               driver.presentedFrames(),
               driver.waitedFields(),
               driver.fields());
}

} // namespace spider::enterelectro
