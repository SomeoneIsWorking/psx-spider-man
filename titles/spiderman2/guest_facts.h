#pragma once

#include "stock_pad_buffers.h"

#include <cstdint>

namespace spider::enterelectro {

// submitFrame: ResetGraph(1), PutDispEnv(db+0x5C), PutDrawEnv(db), DrawOTag(db->ot+0x3FFC); the
// only caller of DrawOTag 0x8008E574.
inline constexpr std::uint32_t submitFrameAddress = 0x8006BE98u;

// libetc VSync 0x80091604..0x8009177B; VSync(-1) returns the VBlank count at 0x800C082C.
inline constexpr std::uint32_t vsyncAddress = 0x80091604u;
inline constexpr std::uint32_t libetcVblankCountAddress = 0x800C082Cu;

// PadInitDirect 0x800960AC from FUN_80076050; FUN_80076498 copies both 0x22-byte records each
// frame.
inline constexpr psx::input::StockPadBuffers::Layout padBuffers{
    .slot0Buffer = 0x800B2CC0u,
    .slot1Buffer = 0x800B2CE2u,
};

} // namespace spider::enterelectro
