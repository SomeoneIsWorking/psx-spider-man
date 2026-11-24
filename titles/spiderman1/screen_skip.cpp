#include "spiderman1/screen_skip.h"

#include "core.h"
#include "game.h"
#include "host_input.h"

#include <lucent/log.h>

namespace spider::spider1 {

bool ScreenSkip::startPressed(Core &core) {
  const auto held = static_cast<std::uint16_t>(~core.game->pad.buttons & psx::input::kButtonStart);
  const bool pressed = (held & ~heldBefore_) != 0u;
  heldBefore_ = held;
  return pressed;
}

void ScreenSkip::movieField(Core &core) {
  if (!startPressed(core) || core.mem_r32(moviePlaying) == 0u || core.mem_r32(movieEnded) != 0u) {
    return;
  }
  core.mem_w32(movieEnded, 1u);
  lucent::info("skip", "Spider-Man 1 movie ends at its last-frame terminal");
}

bool ScreenSkip::isLegalWait(Core &core, std::uint32_t returnPc) {
  for (std::uint32_t entry = core.mem_r32(overlayList); entry != 0u;
       entry = core.mem_r32(entry + overlayNext)) {
    if (core.mem_r32(entry + overlayHash) == shellHash) {
      return returnPc == core.mem_r32(entry + overlayBase) + legalWaitReturn;
    }
  }
  return false;
}

bool ScreenSkip::endsHeldWait(Core &core, std::uint32_t returnPc) {
  if (!startPressed(core) || !isLegalWait(core, returnPc)) {
    return false;
  }
  lucent::info("skip", "Spider-Man 1 legal screen ends at its wait's own field count");
  return true;
}

} // namespace spider::spider1
