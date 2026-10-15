#pragma once

#include "runtime/runtime.h"

namespace spider::enterelectro {

// Spider-Man 2: Enter Electro (SLUS_013.78). Its own crt0 runs from the executable entry under the
// frame driver.
class Runtime final : public spider::Runtime {
public:
  Runtime();

  void registerOverrides(Game &game) override;
  std::unique_ptr<psx::game::FrameDriver> createFrameDriver(Game &game) override;
  void reportRun(Core &core) const override;
};

} // namespace spider::enterelectro
