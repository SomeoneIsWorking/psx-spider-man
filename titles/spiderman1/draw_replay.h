// draw_replay.h — Spider-Man 1's in-betweens: the gameplay draw section replayed with the camera
// and the object lists blended, sampled at FUN_8002BD5C's entry once the frame's logic has run.
#pragma once

#include "spiderman1/draw_section.h"
#include "spiderman1/object_motion.h"
#include "spiderman1/pose_blend.h"

#include "motion_set.h"
#include "replay_frame.h"

#include <array>

class Core;

namespace spider::spider1 {

class DrawReplay {
public:
  DrawReplay();

  static void registerOverrides(Core &core);

  [[nodiscard]] psx::present::ReplayFrame &frame() {
    return frame_;
  }
  [[nodiscard]] PoseBlend &poses() {
    return poses_;
  }
  [[nodiscard]] const PoseBlend &poses() const {
    return poses_;
  }

private:
  static void renderWalkOverride(Core *core);

  PoseBlend poses_;
  DrawSection section_;
  ObjectMotion objects_;
  std::array<psx::present::MotionSource *, 1> sources_;
  psx::present::ReplayFrame frame_;
};

} // namespace spider::spider1
