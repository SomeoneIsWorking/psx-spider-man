#include "spiderman1/pose_blend.h"

#include "spiderman1/guest_layout.h"
#include "spiderman1/pose_parts.h"
#include "spiderman1/session.h"

#include "core.h"
#include "guest_call.h"
#include "native_dispatch.h"

#include <algorithm>
#include <lucent/log.h>

namespace spider::spider1 {

void PoseBlend::openPass() {
  drawn_.swap(drawing_);
  drawing_.clear();
}

void PoseBlend::restore(Core &core) {
  for (auto it = written_.rbegin(); it != written_.rend(); ++it) {
    core.mem_w16(it->first, it->second);
  }
  written_.clear();
}

void PoseBlend::report() const {
  lucent::info("replay",
               "Spider-Man 1 poses: {} drawn blended, {} drawn as the later tick",
               blended_,
               unpaired_);
}

const PoseBlend::Pose *PoseBlend::drawnPose(std::uint32_t record) const {
  const auto found = std::ranges::find(drawn_, record, &Pose::record);
  return found == drawn_.end() ? nullptr : &*found;
}

void PoseBlend::fetch(Core &core, std::optional<float> replaying) {
  const std::uint32_t record = core.r[psx::cpu::kA0];
  psx::cpu::callOriginalToReturn(
      core, poseFetch, psx::cpu::ExecutionBudget::currentTurn(core), "Spider-Man 1 pose");
  const std::uint32_t pose = core.r[psx::cpu::kV0];
  const std::uint32_t parts = posePartCount(core, record);
  if (parts == 0u || parts > posePartLimit) {
    return;
  }
  Pose later{.record = record,
             .animation = core.mem_r16(record + objectAnimation),
             .bank = core.mem_r8(record + objectBankByte),
             .halves = readPose(core, {.address = pose, .parts = parts})};
  if (!replaying) {
    if (std::ranges::find(drawing_, record, &Pose::record) == drawing_.end()) {
      drawing_.push_back(std::move(later));
    }
    return;
  }
  const float t = *replaying;
  const Pose *earlier = drawnPose(record);
  if (earlier == nullptr || earlier->animation != later.animation || earlier->bank != later.bank ||
      earlier->halves.size() != later.halves.size()) {
    ++unpaired_;
    return;
  }
  ++blended_;
  const std::vector<std::uint16_t> blended = blendPose(earlier->halves, later.halves, t);
  for (std::uint32_t i = 0; i < blended.size(); ++i) {
    written_.emplace_back(pose + 2u * i, later.halves[i]);
    core.mem_w16(pose + 2u * i, blended[i]);
  }
}

void PoseBlend::fetchOverride(Core *core) {
  DrawReplay &replay = session(*core).drawReplay;
  replay.poses().fetch(*core, replay.frame().replaying());
}

void PoseBlend::registerOverrides(Core &core) {
  psx::cpu::installNativeOverride(core, poseFetch, "Spider-Man 1 pose blend", fetchOverride);
}

} // namespace spider::spider1
