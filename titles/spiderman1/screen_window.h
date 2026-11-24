#pragma once

#include <cstdint>

class Core;

namespace spider::spider1 {

// [gp+0x1124]: the screen window View_publish takes, u16 fields right, bottom, left, top, near,
// far, lens divisor, H, OFX, OFY.
inline constexpr std::uint32_t screenWindow = 0x800B5918u;

// The window's bounds as the culls compare them, widened on both sides by the record canvas margin.
struct ScreenBounds {
  int right = 0;
  int bottom = 0;
  int left = 0;
  int top = 0;
  int nearZ = 0;
  int farZ = 0;

  // The 6-bit outcode of one projected vertex: a vertex behind the eye inverts the four screen
  // bits.
  [[nodiscard]] std::uint32_t outcode(int sx, int sy, int z, bool testsFar) const;
  // An outcode with its inverse in the high byte, as the culls AND them.
  [[nodiscard]] static std::uint32_t paired(std::uint32_t code);
  // A vertex word as the packed passes store it: SXY with bit 14 / 30 set beyond right / bottom
  // and bit 15 / 31 from the far compare of view z; bits 13 and 29 are the coordinates' signs.
  [[nodiscard]] std::uint32_t flagged(std::uint32_t sxy, std::uint32_t z) const;
  // Whether a packed word's x lies left of the window, which retail reads as its bit 13.
  [[nodiscard]] bool leftOf(std::uint32_t word) const;

  static ScreenBounds read(Core &core);
  // Guest columns the canvas adds on each side; 0 at 4:3.
  static int margin(Core &core);
};

} // namespace spider::spider1
