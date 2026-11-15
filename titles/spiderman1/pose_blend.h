// pose_blend.h — an articulated object's pose in a Spider-Man 1 in-between. FUN_80010614 builds the
// part matrices for the record's integer animation frame, so a replay draws the later tick's pose.
// Each real draw records the pose every record fetched; a replay's fetch hands back that pose t of
// the way to its own, part by part, written over the fetched entries until the section's steps end.
#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

class Core;

namespace spider::spider1 {

class PoseBlend {
public:
  // A real draw begins: the in-betweens replayed before the next one blend from the poses it
  // fetched.
  void openPass();
  static void registerOverrides(Core &core);
  // The replay's steps ended: the fetched entries hold their own words again.
  void restore(Core &core);
  // Run end: poses drawn blended, and drawn as the later tick because the record fetched no
  // matching pose last tick.
  void report() const;

private:
  struct Pose {
    std::uint32_t record = 0;
    std::uint32_t animation = 0;
    std::uint32_t bank = 0;
    std::vector<std::uint16_t> halves;
  };
  static void fetchOverride(Core *core);
  // `replaying` is the in-between's fraction, empty on a real draw.
  void fetch(Core &core, std::optional<float> replaying);
  [[nodiscard]] const Pose *drawnPose(std::uint32_t record) const;

  std::vector<Pose> drawn_;
  std::vector<Pose> drawing_;
  // Halfwords a replay overwrote, with their own value.
  std::vector<std::pair<std::uint32_t, std::uint16_t>> written_;
  std::uint64_t blended_ = 0;
  std::uint64_t unpaired_ = 0;
};

} // namespace spider::spider1
