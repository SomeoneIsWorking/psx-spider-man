// pose_parts.h — an articulated record's pose: one 0x18-byte entry per part of its bank, blended
// part by part.
#pragma once

#include <cstdint>
#include <span>
#include <vector>

class Core;

namespace spider::spider1 {

// Parts the record's bank declares; 0 or more than posePartLimit when it has no pose to read.
[[nodiscard]] std::uint32_t posePartCount(Core &core, std::uint32_t record);
// A pose in guest memory: its first part's address and its part count.
struct PoseAt {
  std::uint32_t address = 0;
  std::uint32_t parts = 0;
};
[[nodiscard]] std::vector<std::uint16_t> readPose(Core &core, PoseAt pose);
// Each part `t` of the way: its rotation along the shorter arc, its translation straight.
[[nodiscard]] std::vector<std::uint16_t>
blendPose(std::span<const std::uint16_t> from, std::span<const std::uint16_t> to, float t);

} // namespace spider::spider1
