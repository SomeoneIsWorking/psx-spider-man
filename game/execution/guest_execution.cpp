#include "execution/guest_execution.h"

#include "core.h"

#include <lucent/log.h>

namespace spider {

bool reportExecutionResult(const Core &core,
                           const psx::cpu::ExecutionResult &result,
                           std::string_view owner) {
  if (result.returned() || result.reason == psx::cpu::ExecutionExitReason::ProcessExit) {
    return true;
  }
  lucent::error("executor",
                "{} stopped at guest PC 0x{:08X}, ra=0x{:08X}, a0=0x{:08X}: {} ({})",
                owner,
                result.guestPc,
                core.r[31],
                core.r[4],
                psx::cpu::executionExitName(result.reason),
                result.detail);
  return false;
}

} // namespace spider
