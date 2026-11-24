#include "spiderman1/face_writer.h"

#include "core.h"
#include "emit_memory.h"
#include "native_dispatch.h"
#include "r3000.h"
#include "spiderman1/screen_window.h"

#include <algorithm>
#include <array>

namespace spider::spider1 {
namespace {

using psx::gte::ProjectedVertex;

constexpr std::uint32_t packetCursor = 0x800B54B0u;
constexpr std::uint32_t orderingTable = 0x800B5944u;
constexpr std::uint32_t colourTable = 0x800B5914u;
// The packet buffer's end; a face starts only below it less 0x64.
constexpr std::uint32_t packetEnd = 0x800B4FE8u;
constexpr std::uint32_t packetReserve = 0x64u;
// While [0x800B58BC] is set, each drawn face sets its bit (faces counted from the list's end) in
// the 64-bit mask at 0x800B5938.
constexpr std::uint32_t markFaces = 0x800B58BCu;
constexpr std::uint32_t faceMarks = 0x800B5938u;

constexpr std::uint32_t flaggedVertices = 0x1F800000u;
constexpr std::uint32_t mirrored = 0x1F8002D4u;
constexpr std::uint32_t fogColour = 0x1F800354u;
constexpr std::uint32_t fogNear = 0x1F800284u;
constexpr std::uint32_t fogShift = 0x1F800294u;
constexpr std::uint32_t fogDepth = 0x1F8002E4u;
constexpr std::uint32_t depthBiasLow = 0x1F8002C4u;
constexpr std::uint32_t depthBiasHigh = 0x1F8002B4u;
constexpr std::uint32_t depthBiasBoth = 0x1F800344u;

constexpr std::uint32_t nclip = 0x4B400006u;
constexpr std::uint32_t dpcs = 0x4A780010u;
constexpr std::uint32_t dpct = 0x4AF8002Au;
constexpr std::uint32_t gteRgbc = 6u;
constexpr std::uint32_t gteIr0 = 8u;
constexpr std::uint32_t gteSxy0 = 12u;
constexpr std::uint32_t gteSxyPush = 15u;
constexpr std::uint32_t gteRgb0 = 20u;
constexpr std::uint32_t gteRgb1 = 21u;
constexpr std::uint32_t gteRgb2 = 22u;
constexpr std::uint32_t gteMac0 = 24u;

// Bits of a face's first word.
constexpr std::uint32_t textured = 0x1u;
constexpr std::uint32_t triangle = 0x10u;
constexpr std::uint32_t textureWindow = 0x20u;
constexpr std::uint32_t semiTransparent = 0x40u;
constexpr std::uint32_t sides = 0xC0u;
constexpr std::uint32_t gouraud = 0x800u;
constexpr std::uint32_t biasLow = 0x2000u;
constexpr std::uint32_t biasHigh = 0x4000u;
// A face is off screen when its words AND to any of these (bit 13, left, is tested natively).
constexpr std::uint32_t offScreen = 0x6000C000u;

std::int32_t signedHalf(Core &core, std::uint32_t address) {
  return core.mem_r16s(address);
}

// The colour word, primitive code and packet length a face's body leaves.
struct Body {
  std::uint32_t colour = 0;
  std::uint32_t code = 0;
  std::uint32_t length = 0;
};

class FaceRun {
public:
  explicit FaceRun(Core &core)
      : core_(core), gte_(core.gte()), memory_(core), bounds_(ScreenBounds::read(core)),
        orderingTable_(core.mem_r32(orderingTable)), colours_(core.mem_r32(colourTable)),
        limit_(core.mem_r32(packetEnd) - packetReserve),
        packet_(core.mem_r32(packetCursor) & 0x00FFFFFFu) {}

  // False when the packet buffer is full.
  bool face(std::uint32_t face, std::uint32_t index);
  void finish() {
    core_.mem_w32(packetCursor, packet_);
    core_.r[psx::cpu::kV0] = packet_;
  }

private:
  Body flatBody(std::uint32_t face, std::uint32_t flags);
  Body gouraudBody(std::uint32_t face, std::uint32_t flags);
  void fog(std::uint32_t slot, std::uint32_t flags);
  void storeVertex(std::uint32_t at, std::size_t vertex) {
    memory_.storeVertex(packet_ + at, vertices_[vertex]);
  }

  Core &core_;
  psx::cpu::Gte &gte_;
  const psx::present::EmitMemory memory_;
  const ScreenBounds bounds_;
  const std::uint32_t orderingTable_;
  const std::uint32_t colours_;
  const std::uint32_t limit_;
  std::uint32_t packet_;
  std::array<ProjectedVertex, 4> vertices_{};
};

bool FaceRun::face(std::uint32_t face, std::uint32_t index) {
  const std::uint32_t indices = core_.mem_r32(face + 4u);
  std::array<std::uint32_t, 4> at{};
  std::uint32_t all = 0xFFFFFFFFu;
  bool allLeft = true;
  for (std::size_t k = 0; k < 4; ++k) {
    at[k] = flaggedVertices + ((indices >> (8u * k)) & 0xFFu) * 8u;
    vertices_[k] = memory_.loadVertex(at[k]);
    all &= vertices_[k].word();
    allLeft = allLeft && bounds_.leftOf(vertices_[k].word());
  }
  for (std::uint32_t k = 0; k < 3; ++k) {
    gte_.writeData(gteSxy0 + k, vertices_[k].word() << 2);
  }
  gte_.execute(nclip);
  std::uint32_t flags = core_.mem_r32(face);
  // Bit 31 of every word: all nearer than the far plane.
  if (static_cast<std::int32_t>(all) >= 0 || (all & offScreen) != 0u || allLeft) {
    return true;
  }
  const auto front = static_cast<std::int32_t>(gte_.readData(gteMac0));
  gte_.writeData(gteSxyPush, vertices_[3].word() << 2);
  gte_.execute(nclip);
  if ((flags & sides) == 0u) {
    return true;
  }
  const auto back = static_cast<std::int32_t>(gte_.readData(gteMac0));
  const bool visible =
      core_.mem_r16(mirrored) != 0u ? (front < 0 || back > 0) : (front > 0 || back < 0);
  if (!visible) {
    return true;
  }
  storeVertex(8u, 0);
  if (packet_ >= limit_) {
    return false;
  }
  std::int32_t depth = signedHalf(core_, at[0] + 6u);
  for (std::size_t k = 1; k < 4; ++k) {
    depth = std::max(depth, signedHalf(core_, at[k] + 6u));
  }
  const auto fogWord = static_cast<std::int32_t>(core_.mem_r32(fogColour));
  const std::int32_t excess = depth - signedHalf(core_, fogNear);
  const std::int32_t shift = signedHalf(core_, fogShift);
  std::uint32_t fogged = 0;
  if (excess > 0) {
    if (fogWord >= 0) {
      flags = (flags | 0x1C0u) ^ 0x100u;
    }
    fogged = shift < 0
                 ? static_cast<std::uint32_t>(excess >> (static_cast<std::uint32_t>(-shift) & 31u))
                 : static_cast<std::uint32_t>(excess) << (static_cast<std::uint32_t>(shift) & 31u);
  }
  gte_.writeData(gteIr0, fogged);
  core_.mem_w16(fogDepth, static_cast<std::uint16_t>(fogged));

  const Body body = (flags & gouraud) != 0u ? gouraudBody(face, flags) : flatBody(face, flags);
  core_.mem_w32(packet_ + 4u, body.colour | body.code | ((flags & semiTransparent) << 19));

  if ((flags & (biasLow | biasHigh)) != 0u) {
    const std::uint32_t bias = (flags & biasLow) == 0u    ? depthBiasLow
                               : (flags & biasHigh) == 0u ? depthBiasHigh
                                                          : depthBiasBoth;
    depth = std::max(depth + signedHalf(core_, bias), 0);
  }
  const auto order = static_cast<std::uint32_t>(depth);
  const std::uint32_t slot = (order < 0x4000u ? (order & 0xFFFCu) : 0x3FFCu) + orderingTable_;
  const std::uint32_t link = core_.mem_r32(slot);
  if ((flags & textureWindow) != 0u) {
    // The face gains a texture-window reset and is preceded by its own texpage packet.
    const std::uint32_t polygon = packet_;
    const std::uint32_t length = body.length + 0x03000000u;
    const std::uint32_t texpage = core_.mem_r32(face + 0x1Cu);
    core_.mem_w32(packet_, link | length);
    packet_ += (length >> 22) + 4u;
    core_.mem_w32(slot, packet_);
    core_.mem_w32(packet_ - 0xCu, 0u);
    core_.mem_w32(packet_ - 8u, 0xE2000000u);
    core_.mem_w32(packet_ - 4u, 0u);
    core_.mem_w32(packet_, polygon | 0x02000000u);
    core_.mem_w32(packet_ + 4u, texpage);
    core_.mem_w32(packet_ + 8u, 0u);
    packet_ += 0xCu;
  } else {
    core_.mem_w32(slot, packet_);
    core_.mem_w32(packet_, link | body.length);
    const std::uint32_t step = (body.length >> 22) + 4u;
    if ((flags & (semiTransparent | textured)) == semiTransparent) {
      // An untextured blended face is preceded by a texpage packet naming its blend mode.
      const std::uint32_t mode = packet_ + step;
      core_.mem_w32(mode + 4u, ((flags & 0x180u) >> 2) | 0xE1000200u);
      core_.mem_w32(mode, 0x01000000u | packet_);
      core_.mem_w32(slot, mode);
      packet_ = mode + 8u;
    } else {
      packet_ += step;
    }
  }
  if (signedHalf(core_, fogDepth) != 0 &&
      static_cast<std::int32_t>(core_.mem_r32(fogColour)) >= 0) {
    fog(slot, flags);
  }
  if (core_.mem_r32(markFaces) != 0u) {
    const std::uint32_t word = (index >> 5) == 0u ? faceMarks + 4u : faceMarks;
    core_.mem_w32(word, core_.mem_r32(word) | (1u << (index & 31u)));
  }
  return true;
}

Body FaceRun::flatBody(std::uint32_t face, std::uint32_t flags) {
  gte_.writeData(gteRgbc, core_.mem_r32(face + 8u));
  gte_.execute(dpcs);
  if ((flags & textured) == 0u) {
    storeVertex(0xCu, 1);
    storeVertex(0x10u, 2);
    const std::uint32_t colour = gte_.readData(gteRgb2);
    if ((flags & triangle) != 0u) {
      return {colour, 0x20000000u, 0x04000000u};
    }
    storeVertex(0x14u, 3);
    return {colour, 0x28000000u, 0x05000000u};
  }
  const std::uint32_t uv0 = core_.mem_r32(face + 0x10u);
  const std::uint32_t uv1 = core_.mem_r32(face + 0x14u) | ((flags & 0x180u) << 14);
  const std::uint32_t uv2 = core_.mem_r32(face + 0x18u);
  storeVertex(0x10u, 1);
  storeVertex(0x18u, 2);
  core_.mem_w32(packet_ + 0xCu, uv0);
  core_.mem_w32(packet_ + 0x14u, uv1);
  core_.mem_w32(packet_ + 0x1Cu, uv2);
  const std::uint32_t colour = gte_.readData(gteRgb2);
  if ((flags & triangle) != 0u) {
    return {colour, 0x24000000u, 0x07000000u};
  }
  storeVertex(0x20u, 3);
  core_.mem_w32(packet_ + 0x24u, uv2 >> 16);
  return {colour, 0x2C000000u, 0x09000000u};
}

Body FaceRun::gouraudBody(std::uint32_t face, std::uint32_t flags) {
  const std::uint32_t shades = core_.mem_r32(face + 8u);
  const auto shade = [&](unsigned shift) {
    return core_.mem_r32(colours_ + ((shades >> shift << 2) & 0x3FCu));
  };
  gte_.writeData(gteRgb0, shade(0));
  gte_.writeData(gteRgb1, shade(8));
  gte_.writeData(gteRgb2, shade(16));
  gte_.execute(dpct);
  if ((flags & textured) == 0u) {
    storeVertex(0x10u, 1);
    storeVertex(0x18u, 2);
    gte_.writeData(gteRgbc, shade(24));
    const std::uint32_t colour = gte_.readData(gteRgb0);
    core_.mem_w32(packet_ + 0xCu, gte_.readData(gteRgb1));
    core_.mem_w32(packet_ + 0x14u, gte_.readData(gteRgb2));
    gte_.execute(dpcs);
    if ((flags & triangle) != 0u) {
      return {colour, 0x30000000u, 0x06000000u};
    }
    storeVertex(0x20u, 3);
    core_.mem_w32(packet_ + 0x1Cu, gte_.readData(gteRgb2));
    return {colour, 0x38000000u, 0x08000000u};
  }
  const std::uint32_t uv0 = core_.mem_r32(face + 0x10u);
  const std::uint32_t uv1 = core_.mem_r32(face + 0x14u) | ((flags & 0x180u) << 14);
  const std::uint32_t uv2 = core_.mem_r32(face + 0x18u);
  storeVertex(0x14u, 1);
  storeVertex(0x20u, 2);
  core_.mem_w32(packet_ + 0xCu, uv0);
  core_.mem_w32(packet_ + 0x18u, uv1);
  core_.mem_w32(packet_ + 0x24u, uv2);
  gte_.writeData(gteRgbc, shade(24));
  const std::uint32_t colour = gte_.readData(gteRgb0) & 0x00FFFFFFu;
  core_.mem_w32(packet_ + 0x10u, gte_.readData(gteRgb1));
  core_.mem_w32(packet_ + 0x1Cu, gte_.readData(gteRgb2));
  gte_.execute(dpcs);
  if ((flags & triangle) != 0u) {
    return {colour, 0x34000000u, 0x09000000u};
  }
  storeVertex(0x2Cu, 3);
  core_.mem_w32(packet_ + 0x30u, uv2 >> 16);
  core_.mem_w32(packet_ + 0x28u, gte_.readData(gteRgb2));
  return {colour, 0x3C000000u, 0x0C000000u};
}

// 0x8007CD44: the face again in the fog colour, depth-cued by the fog depth and linked ahead of
// the face; a fog colour with bit 25 set blends it additively.
void FaceRun::fog(std::uint32_t slot, std::uint32_t flags) {
  const std::uint32_t fogWord = core_.mem_r32(fogColour);
  auto strength = static_cast<std::uint32_t>(signedHalf(core_, fogDepth));
  gte_.writeData(gteRgbc, fogWord);
  const std::uint32_t head = packet_;
  std::uint32_t words = 0;
  if (((fogWord >> 24) & 2u) != 0u) {
    core_.mem_w32(head + 4u, 0xE1000240u);
    core_.mem_w32(head + 8u, 0u);
    packet_ += 8u;
    words = 2;
  } else {
    strength = 0x1000u - strength;
  }
  gte_.writeData(gteIr0, strength);
  std::uint32_t link = core_.mem_r32(slot);
  core_.mem_w32(slot, head);
  gte_.execute(dpcs);
  for (std::uint32_t k = 0; k < 4; ++k) {
    storeVertex(8u + 4u * k, k);
  }
  const std::uint32_t isTriangle = (flags & triangle) >> 1;
  const std::uint32_t code = isTriangle ^ 0x28u;
  core_.mem_w32(packet_ + 4u, ((gte_.readData(gteRgb2) << 6) >> 6) | (code << 24));
  words += isTriangle != 0u ? 4u : 5u;
  link |= words << 24;
  core_.mem_w32(head, link);
  packet_ = head + (link >> 22) + 4u;
}

} // namespace

void FaceWriter::install(Core &core) {
  psx::cpu::installNativeOverride(core, writeFlaggedFaces, "Spider flagged face writer", write);
}

void FaceWriter::write(Core *corePtr) {
  Core &core = *corePtr;
  const std::uint32_t count = core.r[psx::cpu::kA2];
  if (count == 0u) {
    return;
  }
  std::uint32_t face = core.r[psx::cpu::kA0];
  FaceRun run(core);
  for (std::uint32_t index = count; index-- > 0u;) {
    const std::uint32_t size = core.mem_r32(face) >> 16;
    if (!run.face(face, index)) {
      break;
    }
    face += size;
  }
  run.finish();
}

} // namespace spider::spider1
