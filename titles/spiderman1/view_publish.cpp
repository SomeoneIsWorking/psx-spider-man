#include "spiderman1/view_publish.h"

#include "core.h"
#include "guest_call.h"
#include "native_dispatch.h"
#include "r3000.h"
#include "spiderman1/screen_window.h"

#include <array>

namespace spider::spider1 {
namespace {

constexpr std::string_view owner = "Spider-Man 1 View_publish";

// gp-relative words View_publish reads and writes.
constexpr std::uint32_t gpFieldCounterLast = 0x1160u;
constexpr std::uint32_t gpFieldCounterPrevious = 0x10ECu;
constexpr std::uint32_t gpFieldChanges = 0x10E8u;
constexpr std::uint32_t gpFarRamp = 0x1110u;
constexpr std::uint32_t gpFarRampValue = 0x1108u;
constexpr std::uint32_t gpFar = 0x1104u;
constexpr std::uint32_t gpFarRampStep = 0x1114u;
constexpr std::uint32_t gpFarStep = 0x1118u;
constexpr std::uint32_t gpCamera = 0x1128u;
constexpr std::uint32_t gpWindow = 0x1124u;
constexpr std::uint32_t gpOrderingEntry = 0x1150u;
constexpr std::uint32_t gpAreaWidth = 0x1134u;
constexpr std::uint32_t gpAreaHeight = 0x1138u;
constexpr std::uint32_t gpVerticalScale = 0x113Cu;
constexpr std::uint32_t gpHorizontalScale = 0x1140u;
constexpr std::uint32_t gpCameraNegated = 0x112Cu;
constexpr std::uint32_t gpEyeHeight = 0x1132u;

constexpr std::uint32_t fieldCounterCopy = 0x800B546Cu;
constexpr std::uint32_t drawBuffer = 0x800B54A8u;
constexpr std::uint32_t packetCursor = 0x800B54B0u;
constexpr std::uint32_t cameraHeight = 0x800B4FF4u;
// Ten planes of (nx, ny, nz, d) halfwords; the first six rotated into the camera at rotatedPlanes.
constexpr std::uint32_t planes = 0x800AA8DCu;
constexpr std::uint32_t rotatedPlanes = 0x800AA92Cu;
constexpr std::uint32_t rotatedPlanesSecond = 0x800AA94Cu;

constexpr std::uint32_t setDrawArea = 0x800824F8u;
constexpr std::uint32_t squareRoot = 0x80081438u;
constexpr std::uint32_t setGeomScreen = 0x8008BF14u;
constexpr std::uint32_t setGeomOffset = 0x8008BF24u;
constexpr std::uint32_t transposeRotation = 0x800814FCu;
constexpr std::uint32_t drawObjectList = 0x8007ABDCu;
constexpr std::uint32_t drawObjectListLast = 0x8007A764u;
constexpr std::array<std::uint32_t, 2> objectListWords{0x800B5650u, 0x800B5654u};
constexpr std::array<std::uint32_t, 2> objectListBytes{0x800B545Cu, 0x800B53CCu};

constexpr std::uint32_t mvmvaRotate = 0x4A486012u;
constexpr std::uint32_t frameSize = 0x38u;
constexpr std::uint32_t rectOffset = 0x10u;

void writePlane(Core &core, std::uint32_t index, int nx, int ny, int nz, int d) {
  const std::uint32_t plane = planes + index * 8u;
  core.mem_w16(plane, static_cast<std::uint16_t>(nx));
  core.mem_w16(plane + 2u, static_cast<std::uint16_t>(ny));
  core.mem_w16(plane + 4u, static_cast<std::uint16_t>(nz));
  core.mem_w16(plane + 6u, static_cast<std::uint16_t>(d));
}

std::uint32_t root(Core &core, std::uint32_t value) {
  return psx::cpu::callGuestNowResult(core, owner, squareRoot, value);
}

} // namespace

void ViewPublish::install(Core &core) {
  psx::cpu::installNativeOverride(core, viewPublish, "Spider View_publish", publish);
}

void ViewPublish::publish(Core *corePtr) {
  Core &core = *corePtr;
  const std::uint32_t camera = core.r[psx::cpu::kA0];
  const std::uint32_t window = core.r[psx::cpu::kA1];
  const std::uint32_t orderingEntry = core.r[psx::cpu::kA2];
  const std::uint32_t gp = core.r[psx::cpu::kGp];
  const std::uint32_t ra = core.r[psx::cpu::kRa];
  const std::uint32_t sp = core.r[psx::cpu::kSp] - frameSize;
  core.r[psx::cpu::kSp] = sp;

  const std::uint32_t counter = core.mem_r32(fieldCounterCopy);
  const std::uint32_t last = core.mem_r32(gp + gpFieldCounterLast);
  core.mem_w32(gp + gpFieldCounterPrevious, last);
  core.mem_w32(gp + gpFieldCounterLast, counter);
  if (last != counter) {
    core.mem_w32(gp + gpFieldChanges, core.mem_r32(gp + gpFieldChanges) + 1u);
  }
  const auto ramp = static_cast<std::int32_t>(core.mem_r32(gp + gpFarRamp));
  if (ramp > 0) {
    core.mem_w32(gp + gpFarRamp, static_cast<std::uint32_t>(ramp - 1));
    core.mem_w32(gp + gpFarRampValue,
                 core.mem_r32(gp + gpFarRampValue) + core.mem_r32(gp + gpFarRampStep));
    core.mem_w32(gp + gpFar, core.mem_r32(gp + gpFar) + core.mem_r32(gp + gpFarStep));
  }

  // The draw area packet, linked at the OT entry.
  const std::uint32_t packet = core.mem_r32(packetCursor);
  const std::uint32_t rect = sp + rectOffset;
  core.mem_w32(gp + gpCamera, camera);
  core.mem_w32(gp + gpWindow, window);
  core.mem_w32(gp + gpOrderingEntry, orderingEntry);
  core.mem_w16(rect, 0u);
  core.mem_w16(window + 10u, core.mem_r16(gp + gpFar));
  core.mem_w16(rect + 2u, core.mem_r16(core.mem_r32(drawBuffer) + 2u));
  core.mem_w16(rect + 4u, core.mem_r16(gp + gpAreaWidth));
  core.mem_w16(rect + 6u, core.mem_r16(gp + gpAreaHeight));
  psx::cpu::callGuestNow(core, owner, setDrawArea, packet, rect);
  const std::uint32_t entry = core.mem_r32(gp + gpOrderingEntry);
  core.mem_w32(packet, (core.mem_r32(packet) & 0xFF000000u) | (core.mem_r32(entry) & 0x00FFFFFFu));
  core.mem_w32(entry, (core.mem_r32(entry) & 0xFF000000u) | (packet & 0x00FFFFFFu));

  // H, OFX and OFY from the retail window.
  const std::uint32_t right = core.mem_r16(window);
  const std::uint32_t bottom = core.mem_r16(window + 2u);
  const std::uint32_t left = core.mem_r16(window + 4u);
  const std::uint32_t top = core.mem_r16(window + 6u);
  const std::uint32_t width = right - left;
  const auto horizontalScale = static_cast<std::int32_t>(core.mem_r32(gp + gpHorizontalScale));
  const auto lens = static_cast<std::int32_t>(core.mem_r16(window + 12u));
  const std::int32_t projected = ((static_cast<std::int32_t>(width) >> 1) << 12) / lens;
  const auto h = static_cast<std::uint16_t>((projected << 12) / horizontalScale);
  core.mem_w16(window + 16u,
               static_cast<std::uint16_t>(static_cast<std::int32_t>(left + right) >> 1));
  core.mem_w16(window + 18u,
               static_cast<std::uint16_t>(static_cast<std::int32_t>(top + bottom) >> 1));
  core.mem_w16(window + 14u, h);

  writePlane(core, 0u, 0, 0, -0x1000, core.mem_r16(window + 10u));
  writePlane(core, 1u, 0, 0, 0x1000, -static_cast<int>(core.mem_r16(window + 8u)));
  // The horizontal planes span the canvas; the vertical ones keep the retail width they scale.
  const std::uint32_t wideWidth =
      width + 2u * static_cast<std::uint32_t>(ScreenBounds::margin(core));
  const auto slopePair = [&](std::uint32_t scale, std::uint32_t span) {
    const std::uint32_t across = h * scale;
    const std::uint32_t along = span << 11;
    const std::uint32_t length =
        root(core, (along >> 12) * (along >> 12) + (across >> 12) * (across >> 12));
    return std::array<int, 2>{static_cast<int>(across / length), static_cast<int>(along / length)};
  };
  core.mem_w32(packetCursor, packet + 0xCu);
  const std::array<int, 2> vertical = slopePair(core.mem_r32(gp + gpVerticalScale), width);
  writePlane(core, 2u, 0, vertical[0], vertical[1], 0);
  writePlane(core, 3u, 0, -vertical[0], vertical[1], 0);
  const std::array<int, 2> horizontal = slopePair(core.mem_r32(gp + gpHorizontalScale), wideWidth);
  writePlane(core, 4u, horizontal[0], 0, horizontal[1], 0);
  writePlane(core, 5u, -horizontal[0], 0, horizontal[1], 0);
  const auto halfPair = [&](std::uint32_t span) {
    const auto half = static_cast<std::int32_t>(span) >> 1;
    const auto halfWord = static_cast<std::uint32_t>(half);
    const std::uint32_t length =
        root(core, halfWord * halfWord + static_cast<std::uint32_t>(h) * h);
    return std::array<int, 2>{static_cast<int>((static_cast<std::uint32_t>(h) << 12) / length),
                              static_cast<int>((halfWord << 12) / length)};
  };
  const std::array<int, 2> heightPlane = halfPair(bottom - top);
  writePlane(core, 6u, 0, heightPlane[0], heightPlane[1], 0);
  writePlane(core, 7u, 0, -heightPlane[0], heightPlane[1], 0);
  const std::array<int, 2> widthPlane = halfPair(wideWidth);
  writePlane(core, 8u, widthPlane[0], 0, widthPlane[1], 0);
  writePlane(core, 9u, -widthPlane[0], 0, widthPlane[1], 0);

  // The first six planes into the camera's rotation.
  psx::cpu::Gte &gte = core.gte();
  for (std::uint32_t word = 0; word < 5u; ++word) {
    gte.writeControl(word, core.mem_r32(camera + 0x34u + word * 4u));
  }
  for (std::uint32_t index = 0; index < 3u; ++index) {
    for (const auto [from, to] :
         {std::array<std::uint32_t, 2>{planes + index * 8u, rotatedPlanes + index * 6u},
          std::array<std::uint32_t, 2>{planes + 0x18u + index * 8u,
                                       rotatedPlanesSecond + index * 6u}}) {
      gte.writeData(0u, core.mem_r32(from));
      gte.writeData(1u, core.mem_r32(from + 4u));
      gte.execute(mvmvaRotate);
      core.mem_w16(to, static_cast<std::uint16_t>(gte.readData(9u)));
      core.mem_w16(to + 2u, static_cast<std::uint16_t>(gte.readData(10u)));
      core.mem_w16(to + 4u, static_cast<std::uint16_t>(gte.readData(11u)));
    }
  }

  psx::cpu::callGuestNow(core, owner, setGeomScreen, core.mem_r16(window + 14u));
  psx::cpu::callGuestNow(
      core, owner, setGeomOffset, core.mem_r16(window + 16u), core.mem_r16(window + 18u));
  psx::cpu::callGuestNow(core, owner, transposeRotation, camera + 0x34u, camera + 0x54u);
  const auto verticalScale = static_cast<std::int32_t>(core.mem_r32(gp + gpVerticalScale));
  for (std::uint32_t axis = 0; axis < 3u; ++axis) {
    const std::uint32_t at = axis * 2u;
    core.mem_w16(
        camera + 0x74u + at,
        static_cast<std::uint16_t>((core.mem_r16s(camera + 0x54u + at) * horizontalScale) >> 12));
    core.mem_w16(
        camera + 0x7Au + at,
        static_cast<std::uint16_t>((core.mem_r16s(camera + 0x5Au + at) * verticalScale) >> 12));
    core.mem_w16(camera + 0x80u + at, core.mem_r16(camera + 0x60u + at));
  }
  for (const std::uint32_t list : objectListWords) {
    psx::cpu::callGuestNow(core, owner, drawObjectList, core.mem_r32(list));
  }
  for (const std::uint32_t list : objectListBytes) {
    psx::cpu::callGuestNow(core, owner, drawObjectList, core.mem_r8(list));
  }
  psx::cpu::callGuestNow(core, owner, drawObjectListLast, core.mem_r8(objectListBytes[0]));

  const std::uint32_t published = core.mem_r32(gp + gpCamera);
  for (std::uint32_t axis = 0; axis < 3u; ++axis) {
    core.mem_w16(gp + gpCameraNegated + axis * 2u,
                 static_cast<std::uint16_t>(
                     -static_cast<std::int32_t>(core.mem_r16(published + 0x3Au + axis * 2u))));
  }
  core.mem_w16(
      gp + gpEyeHeight,
      static_cast<std::uint16_t>((static_cast<std::int32_t>(core.mem_r32(cameraHeight)) >> 12) -
                                 static_cast<std::int32_t>(core.mem_r16(published + 8u))));
  core.r[psx::cpu::kSp] = sp + frameSize;
  core.r[psx::cpu::kRa] = ra;
}

} // namespace spider::spider1
