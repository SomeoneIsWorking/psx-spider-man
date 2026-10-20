#pragma once

#include "execution_exit.h"

#include <string_view>

class Core;

namespace spider {

// Logs a turn that stopped without returning; true when it returned or the process exited.
bool reportExecutionResult(const Core &core,
                           const psx::cpu::ExecutionResult &result,
                           std::string_view owner);

} // namespace spider
