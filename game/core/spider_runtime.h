#pragma once

#include "executable_identity.h"
#include "game_runtime.h"

#include <string_view>

namespace spider {

// Process-lifetime seam for the two Neversoft Spider titles; owns no guest address.
class SpiderRuntime : public GameRuntime {
public:
  std::string_view serial() const;
  virtual const ExecutableIdentity &executableIdentity() const = 0;

protected:
  [[noreturn]] void refuseUnported(std::string_view boundary, std::string_view frontier) const;
};

} // namespace spider
