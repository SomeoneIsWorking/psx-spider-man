#include "spiderman1/runtime.h"

#include "crt0_boot.h"
#include "game.h"
#include "guest_code_module.h"
#include "spiderman1/frame_driver.h"
#include "spiderman1/platform_facts.h"
#include "spiderman1/session.h"

namespace spider::spider1 {

Runtime::Runtime()
    : spider::Runtime(
          psx::hle::StockVSync({.entry = vsyncAddress, .queryCounter = libetcVblankCountAddress}),
          padBuffers),
      libgte_(libgteEntries), libcd_(libcdEntries, libcdWorkArea, 0),
      dmaCallbacks_(guestDmaCallbackTable) {}

void Runtime::startProgram(Core &core) {
  psx::game::Crt0Boot{crt0Image}.run(core);
  core.game->frameDriverAs<FrameDriver>().beginGuest(core);
}

void Runtime::registerOverrides(Game &game) {
  spider::Runtime::registerOverrides(game);
  libgte_.bind(game);
  libcd_.bind(game);
  dmaCallbacks_.bind(game);
  game.frameDriverAs<FrameDriver>().installOverrides();
  session(game.core).stream.install();
}

std::unique_ptr<psx::game::TitleSessionState> Runtime::createSession(Game &game) {
  return std::make_unique<Session>(game.core);
}

std::unique_ptr<psx::game::FrameDriver> Runtime::createFrameDriver(Game &game) {
  return std::make_unique<FrameDriver>(game, FrameDriver::executeHostTurn);
}

void Runtime::stockCdReadLanded(Core &core, const psx::cd::StockReadLanding &landing) {
  psx::code_module::publishStockReadLanding(core, landing, overlayHeapArena);
}

} // namespace spider::spider1
