#include "spiderman1/runtime.h"

#include "crt0_boot.h"
#include "game.h"
#include "guest_code_module.h"
#include "spiderman1/frame_driver.h"
#include "spiderman1/platform_facts.h"
#include "spiderman1/session.h"

#include <cstring>

namespace spider::spider1 {

Runtime::Runtime()
    : spider::Runtime(
          psx::hle::StockVSync({.entry = vsyncAddress, .queryCounter = libetcVblankCountAddress}),
          padBuffers),
      libgte_(libgteEntries), libcd_(libcdEntries, libcdWorkArea, 0),
      dmaCallbacks_(guestDmaCallbackTable) {}

void Runtime::startProgram(Core &core) {
  psx::game::Crt0Boot{crt0Image}.run(core);
  FrameDriver::from(core).beginGuest(core);
}

void Runtime::registerOverrides(Game &game) {
  spider::Runtime::registerOverrides(game);
  libgte_.bind(game);
  libcd_.bind(game);
  dmaCallbacks_.bind(game);
  FrameDriver::from(game.core).installOverrides();
  session(game.core).stream.install();
}

std::unique_ptr<psx::game::TitleSessionState> Runtime::createSession(Game &game) {
  return std::make_unique<Session>(game.core);
}

std::unique_ptr<psx::game::FrameDriver> Runtime::createFrameDriver(Game &game) {
  return std::make_unique<FrameDriver>(game, FrameDriver::executeHostTurn);
}

bool Runtime::titleCommand(Core &core, const psx::game::TitleCommandLine &input, std::FILE *reply) {
  if (std::strcmp(input.command, "warp") != 0) {
    return false;
  }
  std::fprintf(reply, "%s\n", FrameDriver::from(core).armWarp(core, input.line).c_str());
  return true;
}

void Runtime::stockCdReadLanded(Core &core, const psx::cd::StockReadLanding &landing) {
  psx::code_module::publishStockReadLanding(core, landing, overlayHeapArena);
}

} // namespace spider::spider1
