#pragma once

#include <cstdint>

class Core;

namespace spider::spider1 {

// Boot_init 0x8006BF9C polls Pad_read until 300 fields pass: a black floor under its loads.
inline constexpr std::uint32_t padRead = 0x8006B514u;
inline constexpr std::uint32_t bootLoadFloorReturn = 0x8006C304u;

// Each floor poll is one field, delivered unpresented so the floor never holds the picture.
class LoadFloor {
public:
  static void install(Core &core);
  // A turn that stopped at a floor poll.
  static bool polled(const Core &core, std::uint32_t guestPc);

private:
  static void readPad(Core *core);
};

} // namespace spider::spider1
