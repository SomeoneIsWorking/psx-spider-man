#pragma once

#include "execution_exit.h"

#include <cstdint>
#include <string_view>

class Core;

namespace spider {

// Guest execution on one Core under the current turn's budget.
class GuestExecution {
public:
  explicit GuestExecution(Core &core);

  psx::cpu::ExecutionResult enter(std::uint32_t address);
  psx::cpu::ExecutionResult resumeAt(std::uint32_t address);
  psx::cpu::ExecutionResult callOriginal(std::uint32_t address);

private:
  Core &core_;
};

// Runs `address` to its return; false when a bounded exit is propagating to the frame driver.
bool dispatchGuestOrPropagate(Core &core, std::uint32_t address);
// The overridden guest body at `address`; false when a bounded exit is propagating.
bool callOriginalOrPropagate(Core &core, std::uint32_t address);

bool reportExecutionResult(const Core &core,
                           const psx::cpu::ExecutionResult &result,
                           std::string_view owner);

} // namespace spider
