#include "spiderman1/pose_parts.h"

#include "spiderman1/guest_layout.h"

#include "core.h"
#include "gte_control.h"
#include "motion_set.h"

namespace spider::spider1 {
namespace {

constexpr std::uint32_t kPartHalves = posePartBytes / 2u;
// Halfwords 0..8 the rotation, 9..11 the translation.
constexpr std::uint32_t kRotationHalves = 9u;

// One part `t` of the way: its rotation along the shorter arc, its translation straight.
void blendPart(const std::uint16_t *from, const std::uint16_t *to, float t, std::uint16_t *out) {
  const auto rotation = [](const std::uint16_t *halves) {
    psx::gte::RotationWords words{};
    for (std::uint32_t i = 0; i < kRotationHalves; ++i) {
      words[i / 2u] |= static_cast<std::uint32_t>(halves[i]) << (16u * (i % 2u));
    }
    return words;
  };
  const psx::gte::RotationWords turned =
      psx::present::rotationBetween(rotation(from), rotation(to), t);
  for (std::uint32_t i = 0; i < kRotationHalves; ++i) {
    out[i] = static_cast<std::uint16_t>(turned[i / 2u] >> (16u * (i % 2u)));
  }
  for (std::uint32_t i = kRotationHalves; i < kPartHalves; ++i) {
    out[i] = static_cast<std::uint16_t>(psx::present::lerpInt(
        static_cast<std::int16_t>(from[i]), static_cast<std::int16_t>(to[i]), t));
  }
}

} // namespace

std::uint32_t posePartCount(Core &core, std::uint32_t record) {
  const std::uint32_t bank = core.mem_r8(record + objectBankByte);
  return core.mem_r16(bankPartTables + bank * bankStride + bankPartCount);
}

std::vector<std::uint16_t> readPose(Core &core, PoseAt pose) {
  std::vector<std::uint16_t> halves(static_cast<std::size_t>(pose.parts) * kPartHalves);
  for (std::uint32_t i = 0; i < halves.size(); ++i) {
    halves[i] = core.mem_r16(pose.address + 2u * i);
  }
  return halves;
}

std::vector<std::uint16_t>
blendPose(std::span<const std::uint16_t> from, std::span<const std::uint16_t> to, float t) {
  std::vector<std::uint16_t> blended(to.size());
  for (std::size_t at = 0; at + kPartHalves <= to.size(); at += kPartHalves) {
    blendPart(&from[at], &to[at], t, &blended[at]);
  }
  return blended;
}

} // namespace spider::spider1
