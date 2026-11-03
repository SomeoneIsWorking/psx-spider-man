#include "spiderman2/overlay_loader.h"

#include "core.h"
#include "execution_control.h"
#include "guest_code_module.h"
#include "native_dispatch.h"
#include "spiderman2/guest_facts.h"

#include <lucent/log.h>

#include <cstdint>
#include <span>

namespace spider::enterelectro {

void OverlayLoader::install(Core &core) {
  psx::cpu::installNativeOverride(core, overlayRelocator, "Spider-Man 2 overlay relocator", relocated);
}

void OverlayLoader::relocated(Core *core) {
  const std::uint32_t module = core->r[5] & 0x1FFFFFFFu;
  if (!psx::cpu::callOriginalOrPropagate(*core, overlayRelocator)) {
    return;
  }
  const std::uint32_t bytes = core->mem_r32(0x80000000u | (module - heapBlockHeaderBytes + 4u)) >> 4;
  if (bytes == 0u || module + bytes > sizeof core->ram) {
    lucent::error("code-module", "overlay at 0x{:08X} has a {}-byte heap block outside guest RAM", module, bytes);
    psx::cpu::requestExecutionExit(*core,
                                   {.reason = psx::cpu::ExecutionExitReason::Fault,
                                    .guestPc = core->pc,
                                    .detail = "overlay heap block outside guest RAM"});
    return;
  }
  const psx::code_module::Digest content =
      psx::code_module::digest(std::span<const std::uint8_t>(core->ram + module, bytes));
  const auto identity = psx::code_module::activate(*core, "overlay", {module, module + bytes}, content);
  lucent::info("code-module",
               "overlay 0x{:08X}..0x{:08X} relocated: image {} generation {}, SHA-256 {}",
               0x80000000u | module,
               0x80000000u | (module + bytes),
               identity.id,
               identity.generation,
               content.hex);
}

} // namespace spider::enterelectro
