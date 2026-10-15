// Named SLUS_008.75 guest addresses the title's native owners read or write.
#pragma once

#include <cstdint>

namespace spider::spider1 {

// The current level name, beside the mode argument block at 0x800A5688.
inline constexpr uint32_t levelName = 0x800A568Cu;
// Gameplay loop FUN_8002C174 runs while zero; the value it stops on is main's jump table selector.
inline constexpr uint32_t primaryModeState = 0x800B4F34u;
// Non-zero while the level runs as an attract demo: FUN_80017AF4 draws the DEMO caption.
inline constexpr uint32_t demoPlayback = 0x800B5778u;

// VSync(0) clock: pointer to the horizontal counter, its rebased baseline, last VSync field, and
// the GPU status word the VSync tail reads.
inline constexpr uint32_t horizontalCounterPointer = 0x800B0FA4u;
inline constexpr uint32_t horizontalCounterBaseline = 0x800B0FA8u;
inline constexpr uint32_t lastVsyncField = 0x800B0FACu;
inline constexpr uint32_t gpuStatusWord = 0x800B0FA0u;

// Field-callback registration entry.
inline constexpr uint32_t vsyncCallbackRegistration = 0x8008B8CCu;
// Stock GPU DMA timeout arm; answered from the title field counter instead of guest libetc.
inline constexpr uint32_t gpuDmaTimeoutStart = 0x80083C60u;
inline constexpr uint32_t gpuDmaTimeoutDeadline = 0x800B0F64u;
inline constexpr uint32_t gpuDmaTimeoutPollCount = 0x800B0F68u;
inline constexpr uint32_t gpuDmaTimeoutFieldBudget = 240u;
// Instructions the retail timeout body would have run, for the executor budget.
inline constexpr uint32_t gpuDmaTimeoutRetailInstructions = 13u;

// Return address crt0's `jal main` at 0x80087438 leaves in $ra.
inline constexpr uint32_t crt0MainReturn = 0x80087440u;

} // namespace spider::spider1
