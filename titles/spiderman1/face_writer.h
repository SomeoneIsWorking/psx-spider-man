#pragma once

#include <cstdint>

class Core;

namespace spider::spider1 {

// 0x8007F394: writes the faces of a flagged vertex list (ScreenCull's flagged passes) as GPU
// packets into the ordering table, each with its fog overlay (0x8007CD44). Natively a face is
// rejected against the widened left edge rather than its words' sign bits.
inline constexpr std::uint32_t writeFlaggedFaces = 0x8007F394u;

class FaceWriter {
public:
  static void install(Core &core);

private:
  static void write(Core *core);
};

} // namespace spider::spider1
