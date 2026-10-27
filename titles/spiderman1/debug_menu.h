// debug_menu.h — Spider-Man's debug menu: the level warp over the guest's own level table.
#pragma once

#include "title_debug_menu.h"

namespace spider::spider1 {

class DebugMenu final : public psx::game::TitleDebugMenu {
public:
  DebugMenu();
};

} // namespace spider::spider1
