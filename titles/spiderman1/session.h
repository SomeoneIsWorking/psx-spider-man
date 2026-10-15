#pragma once

#include "game_runtime.h"
#include "spiderman1/stream_driver.h"

class Core;

namespace spider::spider1 {

// What one Core of Spider-Man owns outside guest RAM, created with its Game.
class Session final : public psx::game::TitleSessionState {
public:
  explicit Session(Core &core);

  StreamDriver stream;
};

// The session of the Game `core` belongs to; aborts when it has none or another title's.
Session &session(Core &core);

} // namespace spider::spider1
