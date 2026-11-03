#pragma once

#include "guest_address_range.h"
#include "stock_pad_buffers.h"

#include <cstdint>

namespace spider::enterelectro {

// submitFrame: ResetGraph(1), PutDispEnv(db+0x5C), PutDrawEnv(db), DrawOTag(db->ot+0x3FFC); the
// only caller of DrawOTag 0x8008E574.
inline constexpr std::uint32_t submitFrameAddress = 0x8006BE98u;

// libetc VSync 0x80091604..0x8009177B; VSync(-1) returns the VBlank count at 0x800C082C.
inline constexpr std::uint32_t vsyncAddress = 0x80091604u;
inline constexpr std::uint32_t libetcVblankCountAddress = 0x800C082Cu;

// libapi DMACallback 0x8009D174 (SLUS_008.75's 0x8009152C, word for word) keeps channel ch's
// callback at this base + 4*ch; FUN_8009CFF4 dispatches it.
inline constexpr std::uint32_t guestDmaCallbackTable = 0x800C12B8u;

// Heap 1 of the allocator FUN_8007032C (bounds at 0x800A8B94, set by FUN_800773E8): heap 0 is
// [0x800CF0DC, 0x800CF0EC), heap 1 runs to the stack guard 0x801FE000. The title reads its overlays
// into it through its own libcd (FUN_8001E8E4 enters one at 0x80150300); physical addresses.
inline constexpr GuestAddressRange overlayHeapArena{0x000CF0ECu, 0x001FE000u};

// PadInitDirect 0x800960AC from FUN_80076050; FUN_80076498 copies both 0x22-byte records each
// frame.
inline constexpr psx::input::StockPadBuffers::Layout padBuffers{
    .slot0Buffer = 0x800B2CC0u,
    .slot1Buffer = 0x800B2CE2u,
};

} // namespace spider::enterelectro
