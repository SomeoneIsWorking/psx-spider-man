// Named SLUS_008.75 guest addresses the title's native owners read or write.
#pragma once

#include <array>
#include <cstdint>

namespace spider::spider1 {

// The current level name, beside the mode argument block at 0x800A5688.
inline constexpr uint32_t levelName = 0x800A568Cu;
// Gameplay loop FUN_8002C174 runs while zero; the value it stops on is main's jump table selector.
inline constexpr uint32_t primaryModeState = 0x800B4F34u;
// FUN_8005E748(n): spins until the field counter is n past its value at the call.
inline constexpr uint32_t fieldWait = 0x8005E748u;
inline constexpr uint32_t fieldCounter = 0x800B5468u;
// FUN_8002C174's two waits: the idle one only when no field passed during the work, then the
// frame wait before DrawSync and the submit. A frame lasts max(2, K + 1) fields.
inline constexpr uint32_t gameLoopIdleWaitReturn = 0x8002C274u;
inline constexpr uint32_t gameLoopFrameWaitReturn = 0x8002C294u;
inline constexpr uint32_t gameLoopFieldsPerFrame = 2u;

// The gameplay frame's draw: FUN_8002BD5C walks the world and the object lists into the OT
// FUN_800612B8 cleared, then FUN_8007F930(ot, 0x1000) drops its empty entries. The draw buffer
// pointer [gp+0xCB4] alternates between two buffers whose OT pointer is at +0x70; DrawOTag starts
// at its last entry.
inline constexpr uint32_t renderWalk = 0x8002BD5Cu;
inline constexpr uint32_t orderingTableCompact = 0x8007F930u;
inline constexpr uint32_t orderingTableEntries = 0x1000u;
inline constexpr uint32_t drawBuffer = 0x800B54A8u;
inline constexpr uint32_t drawBufferOrderingTable = 0x70u;
inline constexpr uint32_t orderingTableHeadOffset = 0x3FFCu;

// [gp+0x1128], the camera the object draw FUN_80076480 projects through: its rotation MATRIX's
// five words at +0x74 and its position at +0x4 (world units).
inline constexpr uint32_t currentCamera = 0x800B591Cu;
inline constexpr uint32_t cameraRotation = 0x74u;
inline constexpr uint32_t cameraPosition = 0x4u;

// The object lists FUN_8002BD5C draws through FUN_80076480, in its order; the player's record
// (0x800B5268) heads its own. A record links the next at +0x1C, holds its position (20.12) at
// +0x4, its three 12-bit angles at +0x10, its model index at +0x16 and its model bank at +0x1B.
inline constexpr std::array<uint32_t, 9> objectLists{0x800B566Cu,
                                                     0x800B5394u,
                                                     0x800B5268u,
                                                     0x800B526Cu,
                                                     0x800B5398u,
                                                     0x800B5234u,
                                                     0x800B4E2Cu,
                                                     0x800B53A0u,
                                                     0x800B4E6Cu};
inline constexpr uint32_t objectNext = 0x1Cu;
inline constexpr uint32_t objectPosition = 0x4u;
inline constexpr uint32_t objectAngles = 0x10u;
inline constexpr uint32_t objectAngleZModel = 0x14u;
inline constexpr uint32_t objectBank = 0x18u;
// FUN_80010614(record): an articulated record's pose, one 0x18-byte entry per part (a rotation
// MATRIX's nine halfwords, then its translation's three at +0x12), for animation +0x126 at
// integer frame +0x124. The bank's part table is the word at 0x800A0914 + bank * 0x40 and
// FUN_80077198 draws as many parts as the halfword at +0x24 of that bank entry.
inline constexpr uint32_t poseFetch = 0x80010614u;
// A record whose flag halfword +0 has bit 4 set skips the fetch: its update leaves the pose at the
// pointer +0x180.
inline constexpr uint32_t objectFlags = 0x0u;
inline constexpr uint32_t objectFlagPosePrecomputed = 0x4u;
inline constexpr uint32_t objectPrecomputedPose = 0x180u;
inline constexpr uint32_t objectAnimationFrame = 0x124u;
inline constexpr uint32_t objectAnimation = 0x126u;
inline constexpr uint32_t objectBankByte = 0x1Bu;
inline constexpr uint32_t bankPartTables = 0x800A0914u;
inline constexpr uint32_t bankStride = 0x40u;
inline constexpr uint32_t bankPartCount = 0x24u;
inline constexpr uint32_t posePartBytes = 0x18u;
// FUN_80077198 marks visible parts in two 32-bit masks.
inline constexpr uint32_t posePartLimit = 64u;
// Records a list holds at most; a longer walk is a broken link.
inline constexpr uint32_t objectListLimit = 1024u;
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
