#include "spiderman1/draw_replay.h"

#include "spiderman1/guest_layout.h"
#include "spiderman1/session.h"

#include "core.h"
#include "native_dispatch.h"

namespace spider::spider1 {
namespace {

constexpr psx::present::TransformRule kCameraRule{};
constexpr std::array<psx::present::MotionField, psx::present::TransformRule::kWords> kCameraFields{{
    {.offset = cameraRotation},
    {.offset = cameraRotation + 4u},
    {.offset = cameraRotation + 8u},
    {.offset = cameraRotation + 12u},
    {.offset = cameraRotation + 16u},
    {.offset = cameraPosition},
    {.offset = cameraPosition + 4u},
    {.offset = cameraPosition + 8u},
}};
const std::array<psx::present::MotionTrack, 1> kTracks{{
    {.name = "camera",
     .address = currentCamera,
     .fields = kCameraFields,
     .rule = &kCameraRule,
     .records = {.throughPointer = true}},
}};

} // namespace

DrawReplay::DrawReplay()
    : section_(poses_), sources_{&objects_}, frame_(section_, kTracks, sources_) {}

void DrawReplay::registerOverrides(Core &core) {
  psx::cpu::installNativeOverride(
      core, renderWalk, "Spider-Man 1 draw replay sample", renderWalkOverride);
  PoseBlend::registerOverrides(core);
}

void DrawReplay::renderWalkOverride(Core *core) {
  DrawReplay &replay = session(*core).drawReplay;
  if (!replay.frame().replaying()) {
    // The replays blend from the poses the last real draw fetched.
    replay.poses().openPass();
    replay.frame().beforeDraw(*core);
  }
  // A walk that outlives the turn resumes in the next one; the sample is already taken.
  (void)psx::cpu::callOriginalOrPropagate(*core, renderWalk);
}

} // namespace spider::spider1
