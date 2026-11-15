// object_motion.h — the object lists' motion in a replay: each record's position and angles, and
// the pose its update precomputed, paired by its address, model and bank while it stays on a list.
#pragma once

#include "motion_set.h"

#include <array>
#include <cstdint>
#include <vector>

class Core;

namespace spider::spider1 {

class ObjectMotion final : public psx::present::MotionSource {
public:
  [[nodiscard]] const char *name() const override {
    return "objects";
  }
  void sample(Core &core) override;
  psx::present::MotionCount write(psx::present::GuestRedraw &redraw, float t) const override;

private:
  struct Object {
    std::uint32_t record = 0;
    // +0x4, +0x8, +0xC, then the angle words +0x10 and +0x14 (whose high half is the model), then
    // the bank word.
    std::array<std::uint32_t, 6> words{};
    // The precomputed pose's address and halfwords; none while the record fetches its pose in the
    // draw.
    std::uint32_t pose = 0;
    std::vector<std::uint16_t> halves;
  };
  [[nodiscard]] const Object *earlier(const Object &object) const;

  std::vector<Object> from_;
  std::vector<Object> to_;
};

} // namespace spider::spider1
