#include "spiderman1/level_warp.h"

#include "spiderman1/guest_layout.h"

#include "core.h"

#include <algorithm>
#include <lucent/log.h>
#include <utility>

namespace spider::spider1 {
namespace {

// A guest name of at most the change-level command's capacity.
std::string readGuestName(Core &core, std::uint32_t address) {
  std::string text;
  for (std::size_t index = 0; index < LevelWarp::kNameCapacity; ++index) {
    const auto byte = static_cast<char>(core.mem_r8(address + static_cast<std::uint32_t>(index)));
    if (byte == '\0') {
      break;
    }
    text.push_back(byte);
  }
  return text;
}

} // namespace

bool LevelWarp::inLevelTable(Core &core, std::string_view name) {
  for (std::uint32_t row = 0; row < kLevelTableRows; ++row) {
    const std::uint32_t namePointer = core.mem_r32(kLevelTable + row * kLevelTableRowBytes + 4u);
    const std::string rowName = readGuestName(core, namePointer);
    if (rowName.empty()) {
      return false;
    }
    if (rowName == name) {
      return true;
    }
  }
  return false;
}

std::string LevelWarp::arm(Core &core, const psx::debug::CommandArgs &args) {
  const std::string_view name = args.word(1);
  if (name.empty()) {
    return "usage: warp <level> (a level table name such as l1a2_t)";
  }
  if (name.size() > kNameCapacity || !inLevelTable(core, name)) {
    return lucent::format("refused: {} is not in the level table at 0x{:08X}", name, kLevelTable);
  }
  armed_ = std::string(name);
  return lucent::format("ok: warp armed for {}", name);
}

std::optional<std::string> LevelWarp::apply(Core &core) {
  if (!armed_ || core.pc < kFieldWaitBegin || core.pc >= kFieldWaitEnd ||
      std::ranges::find(kLoopFieldWaitReturns, core.r[31]) == kLoopFieldWaitReturns.end() ||
      core.mem_r32(primaryModeState) != 0) {
    return std::nullopt;
  }
  std::string name = std::move(*armed_);
  armed_.reset();
  for (std::size_t index = 0; index <= name.size(); ++index) {
    const char byte = index < name.size() ? name[index] : '\0';
    core.mem_w8(levelName + static_cast<std::uint32_t>(index), static_cast<std::uint8_t>(byte));
  }
  core.mem_w32(primaryModeState, kLevelSelector);
  // The level is played, not the attract demo the warp may have interrupted.
  core.mem_w32(demoPlayback, 0);
  lucent::info("spider1-warp", "level {} requested", name);
  return name;
}

} // namespace spider::spider1
