#include "spiderman1/session.h"

#include "core.h"
#include "game.h"

#include <cstdlib>
#include <lucent/log.h>

namespace spider::spider1 {

Session::Session(Core &core) : stream(core) {}

Session &session(Core &core) {
  auto *found =
      core.game != nullptr ? dynamic_cast<Session *>(core.game->sessionState.get()) : nullptr;
  if (found == nullptr) {
    lucent::error("spider1", "a Spider-Man native owner ran on a Core with no Spider-Man session");
    std::abort();
  }
  return *found;
}

} // namespace spider::spider1
