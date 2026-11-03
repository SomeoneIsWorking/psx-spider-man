#include "spiderman1/debug_menu.h"

#include "spiderman1/frame_driver.h"
#include "spiderman1/level_warp.h"

#include "core.h"
#include "game.h"

#include <lucent/log.h>

#include <vector>

namespace spider::spider1 {

DebugMenu::DebugMenu() {
  warp({
      .levels = {"level"},
      .places =
          [](Core &core) {
            std::vector<psx::game::WarpPlace> levels;
            for (int row = 0; row < LevelWarp::tableCount(core); ++row) {
              std::string name = LevelWarp::tableName(core, row);
              levels.push_back({
                  .name = name,
                  .arrive =
                      [name](Core &target) {
                        return target.game->frameDriverAs<FrameDriver>().armWarp(name);
                      },
              });
            }
            return levels;
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
