#include "spiderman1/object_motion.h"

#include "spiderman1/guest_layout.h"
#include "spiderman1/pose_parts.h"

#include "core.h"

#include <algorithm>
#include <cstdlib>
#include <lucent/log.h>

namespace spider::spider1 {
namespace {

constexpr std::uint32_t kBankMask = 0xFF000000u;
constexpr std::uint32_t kModelMask = 0xFFFF0000u;

} // namespace

void ObjectMotion::sample(Core &core) {
  std::swap(from_, to_);
  to_.clear();
  for (const std::uint32_t list : objectLists) {
    std::uint32_t count = 0;
    for (std::uint32_t record = core.mem_r32(list); record != 0u;
         record = core.mem_r32(record + objectNext)) {
      if (++count > objectListLimit) {
        lucent::error("replay",
                      "Spider-Man 1 object list 0x{:08X} runs past {} records",
                      list,
                      objectListLimit);
        std::abort();
      }
      Object &object = to_.emplace_back(Object{.record = record});
      object.words = {core.mem_r32(record + objectPosition),
                      core.mem_r32(record + objectPosition + 4u),
                      core.mem_r32(record + objectPosition + 8u),
                      core.mem_r32(record + objectAngles),
                      core.mem_r32(record + objectAngleZModel),
                      core.mem_r32(record + objectBank)};
      const std::uint32_t parts = posePartCount(core, record);
      if ((core.mem_r16(record + objectFlags) & objectFlagPosePrecomputed) != 0u && parts != 0u &&
          parts <= posePartLimit) {
        object.pose = core.mem_r32(record + objectPrecomputedPose);
        object.halves = readPose(core, {.address = object.pose, .parts = parts});
      }
    }
  }
}

const ObjectMotion::Object *ObjectMotion::earlier(const Object &object) const {
  const auto found = std::ranges::find_if(from_, [&](const Object &candidate) {
    return candidate.record == object.record &&
           (candidate.words[4] & kModelMask) == (object.words[4] & kModelMask) &&
           (candidate.words[5] & kBankMask) == (object.words[5] & kBankMask);
  });
  return found == from_.end() ? nullptr : &*found;
}

psx::present::MotionCount ObjectMotion::write(psx::present::GuestRedraw &redraw, float t) const {
  using psx::present::Motion;
  psx::present::MotionCount count;
  for (const Object &object : to_) {
    ++count.drawn;
    const Object *from = earlier(object);
    if (from == nullptr) {
      ++count.unpaired;
      continue;
    }
    for (std::uint32_t axis = 0; axis < 3u; ++axis) {
      redraw.writeWord(
          object.record + objectPosition + 4u * axis,
          psx::present::blendedWord(Motion::Linear, from->words[axis], object.words[axis], t));
    }
    redraw.writeWord(
        object.record + objectAngles,
        psx::present::blendedWord(Motion::Angle12x2, from->words[3], object.words[3], t));
    redraw.writeWord(
        object.record + objectAngleZModel,
        psx::present::blendedWord(Motion::Angle12Low, from->words[4], object.words[4], t));
    if (object.pose == 0u) {
      continue;
    }
    if (from->pose != object.pose || from->halves.size() != object.halves.size()) {
      ++count.unpaired;
      continue;
    }
    const std::vector<std::uint16_t> blended = blendPose(from->halves, object.halves, t);
    for (std::uint32_t i = 0; i < blended.size(); ++i) {
      redraw.writeHalf(object.pose + 2u * i, blended[i]);
    }
  }
  return count;
}

} // namespace spider::spider1
