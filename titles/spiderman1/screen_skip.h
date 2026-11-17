#pragma once

#include <cstdint>

class Core;

namespace spider::spider1 {

// Movie_play 0x8002AA0C runs while Movie_playing is set and stops after Movie_ended, set at the
// last frame.
inline constexpr std::uint32_t moviePlaying = 0x800B4E84u;
inline constexpr std::uint32_t movieEnded = 0x800B4E80u;

// Start ends the screens the player does not control through their own terminals.
class ScreenSkip {
public:
  // Once per delivered movie field, before the guest resumes.
  void movieField(Core &core);

private:
  bool startPressed(Core &core);

  std::uint16_t heldBefore_ = 0;
};

} // namespace spider::spider1
