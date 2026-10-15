// Spider-Man 2's FrameDriver through the shipping runtime and FrameLoopShell, over synthetic guests
// written at the title's own entry, submitFrame and VSync addresses.

#include "spiderman2/frame_driver.h"
#include "spiderman2/guest_facts.h"
#include "spiderman2/runtime.h"

#include "core.h"
#include "frame_loop_shell.h"
#include "game.h"
#include "testutil.h"
#include "typed_config.h"

#include <array>
#include <cstdint>
#include <memory>
#include <span>

namespace {

using spider::enterelectro::FrameDriver;

constexpr uint32_t rZero = 0;
constexpr uint32_t rT0 = 8;
constexpr uint32_t rT1 = 9;
constexpr uint32_t rT2 = 10;
constexpr uint32_t rA0 = 4;
constexpr uint32_t kSpinWord = 0x80100000u;
// The title.json header entry, where admission leaves pc.
constexpr uint32_t kExecutableEntry = 0x80093C68u;

constexpr uint32_t jal(uint32_t target) {
  return 0x0C000000u | ((target >> 2u) & 0x03FFFFFFu);
}
constexpr uint32_t addiu(uint32_t rt, uint32_t rs, int32_t imm) {
  return (0x09u << 26u) | (rs << 21u) | (rt << 16u) | (static_cast<uint32_t>(imm) & 0xFFFFu);
}
constexpr uint32_t lui(uint32_t rt, uint32_t imm) {
  return (0x0Fu << 26u) | (rt << 16u) | (imm & 0xFFFFu);
}
constexpr uint32_t lw(uint32_t rt, uint32_t base, int32_t offset) {
  return (0x23u << 26u) | (base << 21u) | (rt << 16u) | (static_cast<uint32_t>(offset) & 0xFFFFu);
}
constexpr uint32_t beq(uint32_t rs, uint32_t rt, int32_t wordOffset) {
  return (0x04u << 26u) | (rs << 21u) | (rt << 16u) | (static_cast<uint32_t>(wordOffset) & 0xFFFFu);
}
constexpr uint32_t jrRa() {
  return 0x03E00008u;
}
constexpr uint32_t nop() {
  return 0;
}

// submitFrame, then VSync(0), then a wait on a word nothing writes.
constexpr std::array<uint32_t, 8> kEntryProgram{
    jal(spider::enterelectro::submitFrameAddress),
    nop(),
    jal(spider::enterelectro::vsyncAddress),
    addiu(rA0, rZero, 0),
    lui(rT0, kSpinWord >> 16u),
    lw(rT0, rT0, 0),
    beq(rT0, rZero, -3),
    nop(),
};

// FUN_80031D40's frame: FUN_80069154(1) snapshots the VBlank counter and spins for the next
// increment, then submitFrame, then the next frame.
constexpr std::array<uint32_t, 9> kVblankWaitProgram{
    lui(rT0, kSpinWord >> 16u),
    lw(rT1, rT0, 0),
    lw(rT2, rT0, 0),
    beq(rT2, rT1, -2),
    nop(),
    jal(spider::enterelectro::submitFrameAddress),
    nop(),
    beq(rZero, rZero, -8),
    nop(),
};

void writeWords(Core &core, uint32_t address, std::span<const uint32_t> words) {
  for (uint32_t index = 0; index < words.size(); ++index) {
    core.mem_w32(address + index * 4u, words[index]);
  }
}

// Each step: (presented frames, waited fields) after it.
using StepCounts = std::array<std::array<uint64_t, 2>, 4>;

// Runs `program` from the executable entry for one step per row of `after`. With `vblankCounter`,
// the test plays the guest's VBlank callback: one increment of the counter word per delivered
// field.
void runSteps(std::span<const uint32_t> program, const StepCounts &after, bool vblankCounter) {
  spider::enterelectro::Runtime runtime;
  auto game = std::make_unique<Game>(runtime, psx::config::Config{});
  Core &core = game->core;
  // The pre-BSS resident span; BSS starts at 0x800C2AF4.
  auto image =
      core.imageCatalog().activate("Spider-Man 2 resident", {0x00010000u, 0x000C2AF4u}, 1u);
  writeWords(core, kExecutableEntry, program);
  core.pc = kExecutableEntry;
  constexpr std::array<uint32_t, 2> kSubmitFrame{jrRa(), nop()};
  writeWords(core, spider::enterelectro::submitFrameAddress, kSubmitFrame);
  core.mem_w32(kSpinWord, 0);
  runtime.registerOverrides(*game);
  psx::frame::FrameLoopShell shell;
  shell.prepareProduct(*game);
  auto &driver = dynamic_cast<FrameDriver &>(*game->frameDriver);

  for (uint32_t step = 0; step < after.size(); ++step) {
    const uint64_t fence = game->presentation.fence();
    shell.step(core, step + 1u);
    CHECK_EQ(game->presentation.fence(), fence + 1u);
    CHECK_EQ(driver.presentedFrames(), after[step][0]);
    CHECK_EQ(driver.waitedFields(), after[step][1]);
    CHECK(!game->run.shouldEnd());
    if (vblankCounter) {
      core.mem_w32(kSpinWord, core.mem_r32(kSpinWord) + 1u);
    }
  }
  CHECK(core.imageCatalog().deactivate(image));
}

// Every host step is one turn and exactly one presentation fence, whichever way the turn ends.
void test_every_step_is_one_turn_and_one_fence() {
  runSteps(kEntryProgram, StepCounts{{{1, 0}, {1, 1}, {1, 2}, {1, 3}}}, false);
}

// A guest frame that waits one VBlank costs one field, as on retail: submitFrame does not end the
// turn, so it adds no field of its own.
void test_vblank_paced_frame_costs_one_field() {
  runSteps(kVblankWaitProgram, StepCounts{{{0, 1}, {1, 1}, {2, 1}, {3, 1}}}, true);
}

} // namespace

int main() {
  RUN(every_step_is_one_turn_and_one_fence);
  RUN(vblank_paced_frame_costs_one_field);
  return psx::test::summary();
}
