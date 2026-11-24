#pragma once

#include <cstdint>

class Core;

namespace spider::spider1 {

// The hand-written vertex passes that project a list (RTPS per vertex) and outcode it against the
// screen window. Natively they compare against the widened bounds, so a polygon in the canvas
// margin is kept.
inline constexpr std::uint32_t projectPlain = 0x8007B628u;
inline constexpr std::uint32_t projectPacked12 = 0x8007B798u;
inline constexpr std::uint32_t projectPacked16 = 0x8007B9CCu;
inline constexpr std::uint32_t projectUntranslated = 0x8007BE90u;
inline constexpr std::uint32_t clipOutcodes = 0x8007C2ACu;
// The packed passes write flagged words (ScreenBounds::flagged) to the scratchpad for the face
// writer; the AND they return carries bit 13 as "every vertex left of the window".
inline constexpr std::uint32_t projectFlaggedOffset = 0x8007F278u;
inline constexpr std::uint32_t projectFlagged12 = 0x8007BBD4u;
inline constexpr std::uint32_t projectFlagged16 = 0x8007BD04u;

class ScreenCull {
public:
  static void install(Core &core);

private:
  static void plain(Core *core);
  static void packed12(Core *core);
  static void packed16(Core *core);
  static void untranslated(Core *core);
  static void clip(Core *core);
  static void flaggedOffset(Core *core);
  static void flagged12(Core *core);
  static void flagged16(Core *core);
};

} // namespace spider::spider1
