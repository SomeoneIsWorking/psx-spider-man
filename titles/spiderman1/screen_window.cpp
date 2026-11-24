#include "spiderman1/screen_window.h"

#include "core.h"
#include "game.h"
#include "guest_widescreen_projection.h"

namespace spider::spider1 {

std::uint32_t ScreenBounds::outcode(int sx, int sy, int z, bool testsFar) const {
  std::uint32_t code = (right < sx ? 1u : 0u) | (sx < left ? 2u : 0u) | (bottom < sy ? 4u : 0u) |
                       (sy < top ? 8u : 0u) | (z < nearZ ? 0x10u : 0u);
  if (testsFar && farZ < z) {
    code |= 0x20u;
  }
  if (z < 0) {
    code ^= 0xFu;
  }
  return code;
}

std::uint32_t ScreenBounds::paired(std::uint32_t code) {
  return code | ((code << 8) ^ 0xFF00u);
}

std::uint32_t ScreenBounds::flagged(std::uint32_t sxy, std::uint32_t z) const {
  constexpr std::uint32_t screenBits = 0xBFFFBFFFu;
  constexpr std::uint32_t depthBits = 0x7FFF7FFFu;
  // Retail subtracts window word 0 (bottom << 16 | right) from SXY as one word.
  const std::uint32_t edge =
      (static_cast<std::uint32_t>(bottom) << 16) | (static_cast<std::uint32_t>(right) & 0xFFFFu);
  const std::uint32_t screen = (sxy & screenBits) | ~((sxy - edge) | screenBits);
  const std::uint32_t biasedFar = static_cast<std::uint32_t>(farZ) + 0x80000000u;
  return (screen & depthBits) | ~((z - biasedFar) | depthBits);
}

bool ScreenBounds::leftOf(std::uint32_t word) const {
  return (static_cast<std::int32_t>(word << 18) >> 18) < left;
}

int ScreenBounds::margin(Core &core) {
  return psx::present::widePresentation(core)
             ? core.game->guestDisplay.plan().projectionHorizontalMargin
             : 0;
}

ScreenBounds ScreenBounds::read(Core &core) {
  const std::uint32_t window = core.mem_r32(screenWindow);
  const int margin = ScreenBounds::margin(core);
  return {.right = static_cast<int>(core.mem_r16(window)) + margin,
          .bottom = static_cast<int>(core.mem_r16(window + 2u)),
          .left = static_cast<int>(core.mem_r16(window + 4u)) - margin,
          .top = static_cast<int>(core.mem_r16(window + 6u)),
          .nearZ = static_cast<int>(core.mem_r16(window + 8u)),
          .farZ = static_cast<int>(core.mem_r16(window + 10u))};
}

} // namespace spider::spider1
