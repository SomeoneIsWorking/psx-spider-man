#pragma once

#include "guest_pad_buffer_layout.h"
#include "platform_hle.h"

#include <cstdint>

namespace spider::enterelectro {

// SLUS_013.78 crt0, the PS-X EXE entry.
inline constexpr uint32_t crt0Entry = 0x80093C68u;

// submitFrame: ResetGraph(1), PutDispEnv(db+0x5C), PutDrawEnv(db), DrawOTag(db->ot+0x3FFC); the
// only caller of DrawOTag 0x8008E574.
inline constexpr uint32_t submitFrameAddress = 0x8006BE98u;

// libetc VSync 0x80091604..0x8009177B; VSync(-1) returns the VBlank count at 0x800C082C.
inline constexpr uint32_t vsyncAddress = 0x80091604u;
inline constexpr uint32_t vsyncEnd = 0x8009177Cu;
inline constexpr uint32_t libetcVblankCountAddress = 0x800C082Cu;

inline constexpr PlatformHlePlan platformServices{
    .vsyncAddress = vsyncAddress,
    .vsyncQueryCounterAddress = libetcVblankCountAddress,
    .windowLo = {vsyncAddress},
    .windowHi = {vsyncEnd},
};

// PadInitDirect 0x800960AC from FUN_80076050; FUN_80076498 copies both 0x22-byte records each
// frame.
inline constexpr GuestPadBufferLayout padBuffers{
    .slot0Buffer = 0x800B2CC0u,
    .slot1Buffer = 0x800B2CE2u,
};

} // namespace spider::enterelectro
