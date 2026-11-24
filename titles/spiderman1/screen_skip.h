#pragma once

#include <cstdint>

class Core;

namespace spider::spider1 {

// Movie_play 0x8002AA0C runs while Movie_playing is set and stops after Movie_ended, set at the
// last frame.
inline constexpr std::uint32_t moviePlaying = 0x800B4E84u;
inline constexpr std::uint32_t movieEnded = 0x800B4E80u;
// Overlay_load 0x8001B990 links a descriptor per loaded NAME.bin at [gp+0x5C8]: +4 code base, +8
// name hash (FUN_8005F180), +0x2C next.
inline constexpr std::uint32_t overlayList = 0x800B4DBCu;
inline constexpr std::uint32_t overlayBase = 4u;
inline constexpr std::uint32_t overlayHash = 8u;
inline constexpr std::uint32_t overlayNext = 0x2Cu;
inline constexpr std::uint32_t shellHash = 0xBA6D4D02u;
// The shell's legal screen (shell + 0x17EEC) shows its still, then Field_wait(240) with no pad
// check.
inline constexpr std::uint32_t legalWaitReturn = 0x17F48u;

// Start ends the screens the player does not control through their own terminals.
class ScreenSkip {
public:
  // Once per delivered movie field, before the guest resumes.
  void movieField(Core &core);
  // Once per field of a held Field_wait: true when Start ends the rest of it.
  bool endsHeldWait(Core &core, std::uint32_t returnPc);

private:
  bool startPressed(Core &core);
  static bool isLegalWait(Core &core, std::uint32_t returnPc);

  std::uint16_t heldBefore_ = 0;
};

} // namespace spider::spider1
