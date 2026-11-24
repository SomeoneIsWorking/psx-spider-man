#pragma once

#include <cstdint>

class Core;

namespace spider::spider1 {

// View_publish(camera, window, ot): publishes the window and frustum, then draws the object lists.
// Natively its side planes span the widened screen while H and OFX keep the retail window's.
inline constexpr std::uint32_t viewPublish = 0x80075D0Cu;

class ViewPublish {
public:
  static void install(Core &core);

private:
  static void publish(Core *core);
};

} // namespace spider::spider1
