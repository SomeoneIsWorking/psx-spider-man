#include "spiderman1/screen_cull.h"

#include "core.h"
#include "emit_memory.h"
#include "guest_call.h"
#include "native_dispatch.h"
#include "r3000.h"
#include "spiderman1/screen_window.h"

namespace spider::spider1 {
namespace {

using psx::gte::ProjectedVertex;

constexpr std::string_view owner = "Spider-Man 1 screen cull";

// Each pass writes (SXY, outcode << 16 | z) per vertex at [0x800B58F0] and (IR1, IR2) at
// [0x800B58F4].
constexpr std::uint32_t projectedVertices = 0x800B58F0u;
constexpr std::uint32_t viewVertices = 0x800B58F4u;
// The packed passes keep vertices flagged for reuse, growing down from [0x800B5940]; a
// flagged-reused vertex is found at the projected base + 0x1F38 minus its first word.
constexpr std::uint32_t reuseCursor = 0x800B5940u;
constexpr std::uint32_t reuseBase = 0x1F38u;
constexpr std::uint32_t projectedEnd = 0x1F8003D4u;
// Clip_outcodes ORs "a vertex nearer than 0x8B" into the word [0x800B58EC] points at.
constexpr std::uint32_t nearFlagPointer = 0x800B58ECu;
constexpr int nearFlagDepth = 0x8B;
constexpr std::uint32_t squareRoot = 0x80081438u;
constexpr std::uint32_t flaggedVertices = 0x1F800000u;
// The flagged passes cache vertices growing down from [0x800B5940] too; a reused one is found at
// 0x1F8003F8 minus its first word.
constexpr std::uint32_t flaggedReuseBase = 0x1F8003F8u;
constexpr std::uint32_t leftBit = 0x2000u;

constexpr std::uint32_t rtps = 0x4A180001u;
constexpr std::uint32_t sqr = 0x4AA00428u;
constexpr std::uint32_t gpf = 0x4B98003Du;
constexpr std::uint32_t gteIr0 = 8u;
constexpr std::uint32_t gteIr1 = 9u;
constexpr std::uint32_t gteIr2 = 10u;
constexpr std::uint32_t gteIr3 = 11u;
constexpr std::uint32_t gteMac1 = 25u;
constexpr std::uint32_t gteMac2 = 26u;
constexpr std::uint32_t gteFlag = 31u;
constexpr std::uint32_t emptyList = 0xFFu;
constexpr std::uint32_t allCodes = 0xFFFFu;
constexpr std::uint32_t allBits = 0xFFFFFFFFu;

int low(std::uint32_t word) {
  return static_cast<std::int16_t>(word & 0xFFFFu);
}

int high(std::uint32_t word) {
  return static_cast<std::int32_t>(word) >> 16;
}

std::uint32_t pack(int lowHalf, int highHalf) {
  return (static_cast<std::uint32_t>(lowHalf) & 0xFFFFu) |
         (static_cast<std::uint32_t>(highHalf) << 16);
}

ProjectedVertex project(Core &core, std::uint32_t vxy, std::uint32_t vz) {
  psx::cpu::Gte &gte = core.gte();
  gte.writeData(0u, vxy);
  gte.writeData(1u, vz);
  gte.execute(rtps);
  return gte.projected(2u);
}

void clearTranslation(Core &core) {
  for (std::uint32_t reg = 5u; reg <= 7u; ++reg) {
    core.gte().writeControl(reg, 0u);
  }
}

void finish(Core &core, std::uint32_t codes) {
  core.r[psx::cpu::kV0] = codes;
}

enum class Packing { Fixed12, Fixed16 };

// 0x8007B798 / 0x8007B9CC: 12.4 vertices (shifted down before RTPS) or 16-bit ones (IR shifted
// after).
void projectPacked(Core &core, Packing packing) {
  std::uint32_t list = core.r[psx::cpu::kA0];
  std::uint32_t count = core.r[psx::cpu::kA1];
  if (count == 0u) {
    finish(core, emptyList);
    return;
  }
  const ScreenBounds bounds = ScreenBounds::read(core);
  std::uint32_t out = core.mem_r32(projectedVertices);
  std::uint32_t view = core.mem_r32(viewVertices);
  std::uint32_t reuse = core.mem_r32(reuseCursor);
  const std::uint32_t reused = out + reuseBase;
  std::uint32_t codes = allCodes;
  psx::cpu::Gte &gte = core.gte();
  const psx::present::EmitMemory memory(core);
  for (; count != 0u; --count, list += 8u, out += 8u, view += 8u) {
    const std::uint32_t first = core.mem_r32(list);
    const std::uint32_t second = core.mem_r32(list + 4u);
    const std::uint32_t flags = second >> 16;
    std::uint32_t vxy = first;
    std::uint32_t vz = second;
    if (packing == Packing::Fixed12) {
      vxy = pack(low(first) >> 4, static_cast<std::int32_t>(first) >> 20);
      vz = (second & 0xFFFF0000u) | (static_cast<std::uint32_t>(low(second) >> 4) & 0xFFFFu);
    }
    const ProjectedVertex vertex = project(core, vxy, vz);
    const std::uint32_t sxy = vertex.word();
    if ((flags & 2u) != 0u) {
      const std::uint32_t from = reused - first;
      memory.storeVertex(out, memory.loadVertex(from));
      const std::uint32_t coded = core.mem_r32(from + 4u);
      core.mem_w32(out + 4u, coded);
      core.mem_w32(view, core.mem_r32(from + view - out));
      codes &= coded >> 16;
      continue;
    }
    const int shift = packing == Packing::Fixed16 ? 4 : 0;
    const auto ir1 = static_cast<std::int32_t>(gte.readData(gteIr1)) >> shift;
    const auto ir2 = static_cast<std::int32_t>(gte.readData(gteIr2)) >> shift;
    const int z = static_cast<std::int32_t>(gte.readData(gteIr3)) >> shift;
    const std::uint32_t code = ScreenBounds::paired(bounds.outcode(low(sxy), high(sxy), z, true));
    codes &= code;
    const std::uint32_t coded = (code << 16) | (static_cast<std::uint32_t>(z) & 0xFFFFu);
    const std::uint32_t irs = pack(ir1, ir2);
    memory.storeVertex(out, vertex);
    core.mem_w32(out + 4u, coded);
    core.mem_w32(view, irs);
    if ((flags & 1u) != 0u) {
      memory.storeVertex(reuse, vertex);
      core.mem_w32(reuse + 4u, coded);
      core.mem_w32(reuse + view - out, irs);
      reuse -= 8u;
    }
  }
  core.mem_w32(reuseCursor, reuse);
  core.mem_w32(projectedEnd, out);
  finish(core, codes);
}

// Clip_outcodes' billboard arm (0x8007C090): each remaining entry names two projected vertices; a
// unit vector across them, scaled by the entry's second word, is projected under a flat rotation
// and offset by the first vertex.
void clipBillboards(Core &core,
                    const ScreenBounds &bounds,
                    std::uint32_t list,
                    std::uint32_t out,
                    std::uint32_t count,
                    std::uint32_t codes) {
  const std::uint32_t ra = core.r[psx::cpu::kRa];
  psx::cpu::Gte &gte = core.gte();
  const psx::present::EmitMemory memory(core);
  gte.writeControl(0u, 0x28F5u);
  gte.writeControl(1u, 0u);
  gte.writeControl(2u, 0x1000u);
  gte.writeControl(3u, 0u);
  gte.writeControl(4u, 0x1000u);
  gte.writeControl(24u, 0u);
  gte.writeControl(25u, 0u);
  const std::uint32_t base = core.mem_r32(projectedVertices);
  std::uint32_t root = 0;
  for (; count != 0u; --count, list += 8u, out += 8u) {
    const std::uint32_t pair = core.mem_r32(list);
    const std::uint32_t scale = core.mem_r32(list + 4u);
    const std::uint32_t from = base + (pair & 0xFFFFu);
    const std::uint32_t to = base + (pair >> 16);
    const ProjectedVertex originVertex = memory.loadVertex(from);
    const std::uint32_t origin = originVertex.word();
    const int originX = low(origin);
    const int originY = high(origin);
    const int z = low(core.mem_r32(from + 4u));
    const std::uint32_t end = core.mem_r32(to);
    const int across = low(end) - originX;
    const int down = originY - high(end);
    gte.writeData(gteIr1, static_cast<std::uint32_t>(across));
    gte.writeData(gteIr2, static_cast<std::uint32_t>(down));
    gte.execute(sqr);
    root = psx::cpu::callGuestNowResult(
        core, owner, squareRoot, gte.readData(gteMac1) + gte.readData(gteMac2));
    const auto length = static_cast<std::int32_t>(root);
    gte.writeData(gteIr1, static_cast<std::uint32_t>((down << 12) / length));
    gte.writeData(gteIr2, static_cast<std::uint32_t>((across << 12) / length));
    gte.writeData(gteIr0, scale);
    gte.execute(gpf);
    const std::uint32_t offset = pack(static_cast<std::int32_t>(gte.readData(gteMac1)),
                                      static_cast<std::int32_t>(gte.readData(gteMac2)));
    const ProjectedVertex corner = project(core, offset, static_cast<std::uint32_t>(z));
    const std::uint32_t sxy = corner.word();
    const int sx = low(sxy) + originX;
    const int sy = high(sxy) + originY;
    // The corner lies its projected offset from the first vertex's place.
    GtePlace place = originVertex.place();
    place.x += corner.place().x;
    place.y += corner.place().y;
    const std::uint32_t code = ScreenBounds::paired(bounds.outcode(sx, sy, z, true));
    codes &= code;
    memory.storeVertex(out, ProjectedVertex(pack(sx, sy), place));
    core.mem_w32(out + 4u, (code << 16) | (static_cast<std::uint32_t>(z) & 0xFFFFu));
  }
  const std::uint32_t centre = core.mem_r32(core.mem_r32(screenWindow) + 0x10u);
  gte.writeControl(24u, centre << 16);
  gte.writeControl(25u, (centre >> 16) << 16);
  core.mem_w32(projectedEnd, root);
  core.r[psx::cpu::kRa] = ra;
  finish(core, codes);
}

// The AND of a flagged list with bit 13 taken from the widened left edge rather than the sign.
class FlaggedCodes {
public:
  explicit FlaggedCodes(std::uint32_t initial) : codes_(initial) {}
  void add(const ScreenBounds &bounds, std::uint32_t word) {
    codes_ &= word;
    allLeft_ = allLeft_ && bounds.leftOf(word);
  }
  [[nodiscard]] std::uint32_t result() const {
    return (codes_ & ~leftBit) | (allLeft_ ? leftBit : 0u);
  }

private:
  std::uint32_t codes_;
  bool allLeft_ = true;
};

// A 16-bit half shifted down by 4, as the 12.4 passes read their words.
std::uint32_t fixed12Low(std::uint32_t word) {
  return (static_cast<std::uint32_t>(static_cast<std::int32_t>(word << 16) >> 4)) >> 16;
}

// 0x8007BBD4 / 0x8007BD04: as projectPacked, writing flagged words to the scratchpad.
void projectFlagged(Core &core, Packing packing) {
  std::uint32_t list = core.r[psx::cpu::kA0];
  std::uint32_t count = core.r[psx::cpu::kA1];
  if (count == 0u) {
    finish(core, 0x60000000u);
    return;
  }
  const ScreenBounds bounds = ScreenBounds::read(core);
  const psx::present::EmitMemory memory(core);
  std::uint32_t reuse = core.mem_r32(reuseCursor);
  std::uint32_t out = flaggedVertices;
  FlaggedCodes codes(0x60006000u);
  for (; count != 0u; --count, list += 8u, out += 8u) {
    std::uint32_t vxy = core.mem_r32(list);
    std::uint32_t vz = core.mem_r32(list + 4u);
    if (packing == Packing::Fixed12) {
      vxy = ((static_cast<std::uint32_t>(static_cast<std::int32_t>(vxy) >> 20)) << 16) |
            fixed12Low(vxy);
      vz = (vz & 0xFFFF0000u) | fixed12Low(vz);
    }
    const ProjectedVertex vertex = project(core, vxy, vz);
    const std::uint32_t flags = vz >> 16;
    if ((flags & 2u) != 0u) {
      const std::uint32_t from = flaggedReuseBase - core.mem_r32(list);
      const ProjectedVertex reused = memory.loadVertex(from);
      memory.storeVertex(out, reused);
      core.mem_w16(out + 6u, core.mem_r16(from + 6u));
      codes.add(bounds, reused.word());
      continue;
    }
    auto z = core.gte().readData(gteIr3);
    if (packing == Packing::Fixed16) {
      z = static_cast<std::uint32_t>(static_cast<std::int32_t>(z) >> 4);
    }
    const ProjectedVertex word(bounds.flagged(vertex.word(), z), vertex.place());
    if ((flags & 1u) != 0u) {
      memory.storeVertex(reuse, word);
      core.mem_w16(reuse + 6u, static_cast<std::uint16_t>(z));
      reuse -= 8u;
    }
    memory.storeVertex(out, word);
    core.mem_w16(out + 6u, static_cast<std::uint16_t>(z));
    codes.add(bounds, word.word());
  }
  core.mem_w32(reuseCursor, reuse);
  finish(core, codes.result());
}

} // namespace

void ScreenCull::install(Core &core) {
  psx::cpu::installNativeOverride(core, projectPlain, "Spider project plain", plain);
  psx::cpu::installNativeOverride(core, projectPacked12, "Spider project packed 12.4", packed12);
  psx::cpu::installNativeOverride(core, projectPacked16, "Spider project packed 16", packed16);
  psx::cpu::installNativeOverride(
      core, projectUntranslated, "Spider project untranslated", untranslated);
  psx::cpu::installNativeOverride(core, clipOutcodes, "Spider Clip_outcodes", clip);
  psx::cpu::installNativeOverride(
      core, projectFlaggedOffset, "Spider project flagged offset", flaggedOffset);
  psx::cpu::installNativeOverride(core, projectFlagged12, "Spider project flagged 12.4", flagged12);
  psx::cpu::installNativeOverride(core, projectFlagged16, "Spider project flagged 16", flagged16);
}

void ScreenCull::flagged12(Core *core) {
  projectFlagged(*core, Packing::Fixed12);
}

void ScreenCull::flagged16(Core *core) {
  projectFlagged(*core, Packing::Fixed16);
}

// 0x8007F278: an untranslated list moved by the offsets at a2 (x, y, z words).
void ScreenCull::flaggedOffset(Core *corePtr) {
  Core &core = *corePtr;
  std::uint32_t list = core.r[psx::cpu::kA0];
  std::uint32_t count = core.r[psx::cpu::kA1];
  const std::uint32_t offsets = core.r[psx::cpu::kA2];
  const std::uint32_t offsetX = core.mem_r32(offsets) & 0xFFFFu;
  const std::uint32_t offsetY = core.mem_r32(offsets + 4u) << 16;
  const std::uint32_t offsetZ = core.mem_r32(offsets + 8u);
  clearTranslation(core);
  if (count == 0u) {
    finish(core, allBits);
    return;
  }
  const ScreenBounds bounds = ScreenBounds::read(core);
  const psx::present::EmitMemory memory(core);
  std::uint32_t out = flaggedVertices;
  FlaggedCodes codes(allBits);
  for (; count != 0u; --count, list += 8u, out += 8u) {
    const std::uint32_t first = core.mem_r32(list);
    const std::uint32_t vxy = ((first + offsetX) & 0xFFFFu) | ((first + offsetY) & 0xFFFF0000u);
    const ProjectedVertex vertex = project(core, vxy, core.mem_r32(list + 4u) + offsetZ);
    const std::uint32_t z = core.gte().readData(gteIr3);
    const ProjectedVertex word(bounds.flagged(vertex.word(), z), vertex.place());
    memory.storeVertex(out, word);
    core.mem_w16(out + 6u, static_cast<std::uint16_t>(z));
    codes.add(bounds, word.word());
  }
  finish(core, codes.result());
}

void ScreenCull::plain(Core *corePtr) {
  Core &core = *corePtr;
  std::uint32_t list = core.r[psx::cpu::kA0];
  std::uint32_t count = core.r[psx::cpu::kA1];
  if (count == 0u) {
    finish(core, emptyList);
    return;
  }
  const ScreenBounds bounds = ScreenBounds::read(core);
  std::uint32_t out = core.mem_r32(projectedVertices);
  std::uint32_t view = core.mem_r32(viewVertices);
  std::uint32_t codes = allCodes;
  psx::cpu::Gte &gte = core.gte();
  const psx::present::EmitMemory memory(core);
  for (; count != 0u; --count, list += 8u, out += 8u, view += 8u) {
    const ProjectedVertex vertex = project(core, core.mem_r32(list), core.mem_r32(list + 4u));
    const std::uint32_t sxy = vertex.word();
    const auto z = static_cast<std::int32_t>(gte.readData(gteIr3));
    core.mem_w32(view,
                 pack(static_cast<std::int32_t>(gte.readData(gteIr1)),
                      static_cast<std::int32_t>(gte.readData(gteIr2))));
    const std::uint32_t code = ScreenBounds::paired(bounds.outcode(low(sxy), high(sxy), z, true));
    codes &= code;
    memory.storeVertex(out, vertex);
    core.mem_w32(out + 4u, (code << 16) | (static_cast<std::uint32_t>(z) & 0xFFFFu));
  }
  core.mem_w32(projectedEnd, out);
  finish(core, codes);
}

void ScreenCull::packed12(Core *core) {
  projectPacked(*core, Packing::Fixed12);
}

void ScreenCull::packed16(Core *core) {
  projectPacked(*core, Packing::Fixed16);
}

void ScreenCull::untranslated(Core *corePtr) {
  Core &core = *corePtr;
  std::uint32_t list = core.r[psx::cpu::kA0];
  std::uint32_t count = core.r[psx::cpu::kA1];
  if (count == 0u) {
    finish(core, emptyList);
    return;
  }
  clearTranslation(core);
  const ScreenBounds bounds = ScreenBounds::read(core);
  std::uint32_t out = core.mem_r32(projectedVertices);
  std::uint32_t codes = allCodes;
  const psx::present::EmitMemory memory(core);
  for (; count != 0u; --count, list += 8u, out += 8u) {
    const ProjectedVertex vertex = project(core, core.mem_r32(list), core.mem_r32(list + 4u));
    const std::uint32_t sxy = vertex.word();
    const auto z = static_cast<std::int32_t>(core.gte().readData(gteIr3));
    const std::uint32_t code = ScreenBounds::paired(bounds.outcode(low(sxy), high(sxy), z, false));
    codes &= code;
    memory.storeVertex(out, vertex);
    core.mem_w32(out + 4u, (code << 16) | 0x7FFFu);
  }
  finish(core, codes);
}

void ScreenCull::clip(Core *corePtr) {
  Core &core = *corePtr;
  std::uint32_t list = core.r[psx::cpu::kA0];
  std::uint32_t count = core.r[psx::cpu::kA1];
  if (count == 0u) {
    finish(core, emptyList);
    return;
  }
  const std::uint32_t offsets = core.r[psx::cpu::kA2];
  const std::uint32_t offsetX = core.mem_r32(offsets);
  const std::uint32_t offsetY = core.mem_r32(offsets + 4u);
  const std::uint32_t offsetZ = core.mem_r32(offsets + 8u);
  clearTranslation(core);
  const ScreenBounds bounds = ScreenBounds::read(core);
  std::uint32_t out = core.mem_r32(projectedVertices);
  std::uint32_t view = core.mem_r32(viewVertices);
  std::uint32_t codes = allCodes;
  std::uint32_t nearFlag = 0;
  psx::cpu::Gte &gte = core.gte();
  const psx::present::EmitMemory memory(core);
  for (; count != 0u; --count, list += 8u, out += 8u, view += 8u) {
    const std::uint32_t first = core.mem_r32(list);
    const std::uint32_t second = core.mem_r32(list + 4u);
    const std::uint32_t vxy =
        pack(static_cast<int>(first + offsetX), high(first) + static_cast<std::int32_t>(offsetY));
    const ProjectedVertex vertex = project(core, vxy, second + offsetZ);
    const std::uint32_t sxy = vertex.word();
    if ((high(second) & 0x10) != 0) {
      clipBillboards(core, bounds, list, out, count, codes);
      return;
    }
    const auto z = static_cast<std::int32_t>(gte.readData(gteIr3));
    core.mem_w32(view,
                 pack(static_cast<std::int32_t>(gte.readData(gteIr1)),
                      static_cast<std::int32_t>(gte.readData(gteIr2))));
    nearFlag |= z < nearFlagDepth ? 1u : 0u;
    std::uint32_t code = bounds.outcode(low(sxy), high(sxy), z, true);
    // A projection the GTE flagged as erroneous drops its screen bits and is marked 0x40.
    if (static_cast<std::int32_t>(gte.readControl(gteFlag)) < 0) {
      code = ((code | 0x40u) >> 4) << 4;
    }
    code = ScreenBounds::paired(code);
    codes &= code;
    memory.storeVertex(out, vertex);
    core.mem_w32(out + 4u, (code << 16) | (static_cast<std::uint32_t>(z) & 0xFFFFu));
  }
  core.mem_w32(projectedEnd, out);
  core.mem_w32(core.mem_r32(nearFlagPointer), nearFlag);
  finish(core, codes);
}

} // namespace spider::spider1
