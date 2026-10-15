#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

class Core;

namespace spider::spider1 {

// `warp <level>`: the exit the level script's change-level command (0xB5 in FUN_8005C7EC) raises,
// the name in 0x800A568C and selector 3 in 0x800B4F34, with the attract demo flag cleared. Applied
// only while FUN_8002C174 waits for its field after drawing, so no frame draws with the new name.
class LevelWarp {
public:
  // FUN_80018800's level table: 20-byte rows, the name pointer at +4, ended by an empty name.
  static constexpr std::uint32_t kLevelTable = 0x800974A4u;
  static constexpr std::uint32_t kLevelTableRowBytes = 20u;
  static constexpr std::uint32_t kLevelTableRows = 34u;
  // The change-level command copies at most this many characters, then a terminator.
  static constexpr std::size_t kNameCapacity = 9u;
  static constexpr std::uint32_t kLevelSelector = 3u;
  // FUN_8005E748, the guest's field wait, and its two `jal` returns inside FUN_8002C174.
  static constexpr std::uint32_t kFieldWaitBegin = 0x8005E748u;
  static constexpr std::uint32_t kFieldWaitEnd = 0x8005E77Cu;
  static constexpr std::array<std::uint32_t, 2> kLoopFieldWaitReturns{0x8002C274u, 0x8002C294u};

  // Parses one command line; the returned text answers the client.
  std::string arm(Core &core, std::string_view line);
  // Raises the armed request when the guest stands in the gameplay loop's field wait with no exit
  // of its own pending; returns the level applied.
  std::optional<std::string> apply(Core &core);

private:
  static bool inLevelTable(Core &core, std::string_view name);

  std::optional<std::string> armed_;
};

} // namespace spider::spider1
