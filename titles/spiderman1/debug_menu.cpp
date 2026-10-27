#include "spiderman1/debug_menu.h"

#include "spiderman1/frame_driver.h"
#include "spiderman1/level_warp.h"

#include "core.h"
#include "game.h"

namespace spider::spider1 {

DebugMenu::DebugMenu() {
  using psx::game::DebugCall;
  using psx::game::DebugParam;
  section("Level Warp");
  verb({
      .name = "warp",
      .summary = "Change to that level",
      .params = {DebugParam::word("level", {&LevelWarp::tableCount, &LevelWarp::tableName})},
      .run =
          [](const DebugCall &call) {
            call.reply = call.core.game->frameDriverAs<FrameDriver>().armWarp(
                std::string(call.values.word(0)));
          },
      .refusal =
          [](Core &core) {
            return core.game->frameDriverIf<FrameDriver>() != nullptr
                       ? std::string()
                       : "the game loop is not running";
          },
  });
}

} // namespace spider::spider1
