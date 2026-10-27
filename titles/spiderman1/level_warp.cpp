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

int LevelWarp::tableCount(Core &core) {
  int rows = 0;
  while (rows < static_cast<int>(kLevelTableRows) && !tableName(core, rows).empty()) {
    ++rows;
  }
  return rows;
}

std::string LevelWarp::tableName(Core &core, int row) {
  if (row < 0 || row >= static_cast<int>(kLevelTableRows)) {
    return {};
  }
  return readGuestName(
      core, core.mem_r32(kLevelTable + static_cast<std::uint32_t>(row) * kLevelTableRowBytes + 4u));
}

std::string LevelWarp::arm(std::string name) {
  armed_ = std::move(name);
  return lucent::format("ok: warp armed for {}", *armed_);
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
