// draw_section.h — Spider-Man 1's gameplay draw section: FUN_8002BD5C's walk into the cleared OT,
// then its compaction.
#pragma once

#include "spiderman1/pose_blend.h"

#include "replay_frame.h"

#include <array>
#include <cstdint>
#include <optional>

class Core;

namespace spider::spider1 {

class DrawSection final : public psx::present::ReplaySection {
public:
  explicit DrawSection(PoseBlend &poses) : poses_(poses) {}

  [[nodiscard]] std::optional<std::uint32_t> draw(Core &core,
                                                  psx::present::GuestRedraw &redraw) const override;
  [[nodiscard]] bool linked(std::uint32_t head) const override;
  // FUN_8002C174 waits for its fields between the draw and the submit.
  [[nodiscard]] bool submitsAfterWait() const override {
    return true;
  }

private:
  PoseBlend &poses_;
  // The two draw buffers' OT heads, as the section last linked them.
  mutable std::array<std::uint32_t, 2> heads_{};
};

} // namespace spider::spider1
