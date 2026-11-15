#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

class Core;

namespace spider::spider1 {

// A level arrival of the warp catalogue: the exit the level script's change-level command (0xB5 in
// FUN_8005C7EC) raises, the name in 0x800A568C and selector 3 in 0x800B4F34, with the attract demo
// flag cleared. Applied only at FUN_8002C174's frame wait, after the draw, so no frame draws with
// the new name.
class LevelWarp {
public:
  // FUN_80018800's level table: 20-byte rows, the name pointer at +4, ended by an empty name.
  static constexpr std::uint32_t kLevelTable = 0x800974A4u;
  static constexpr std::uint32_t kLevelTableRowBytes = 20u;
  static constexpr std::uint32_t kLevelTableRows = 34u;
  // The change-level command copies at most this many characters, then a terminator.
  static constexpr std::size_t kNameCapacity = 9u;
  static constexpr std::uint32_t kLevelSelector = 3u;

  // The level table's names in row order, read from guest memory; the table ends at its first empty
  // name.
  static int tableCount(Core &core);
  static std::string tableName(Core &core, int row);
  // Arms a name from the level table; the returned text answers the client.
  std::string arm(std::string name);
  // Raises the armed request when the guest resumes from the gameplay loop's frame wait with no
  // exit of its own pending; returns the level applied.
  std::optional<std::string> apply(Core &core);

private:
  std::optional<std::string> armed_;
};

} // namespace spider::spider1
