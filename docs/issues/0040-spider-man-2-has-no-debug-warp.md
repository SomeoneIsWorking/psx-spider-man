# 0040 — Spider-Man 2 has no debug warp

Status: open (2026-11-03). Owner: `titles/spiderman2/runtime.h` (`debugMenu_`, an empty psxport `TitleDebugMenu`).

## Observed
In Spider-Man 2, `warp ?` answers `not a debug verb` and the Debug tab says the title declares no debug routes.

## Route the game offers
Spider-Man 1's warp is the level script's change-level exit over FUN_80018800's level table (`spider1::LevelWarp`,
names at `0x800974A4`). Spider-Man 2 is not yet shown to carry the same table or exit; the next step is to find its
change-level command and level-name table in its executable, then supply a warp catalogue (level → start) through
psxport `TitleDebugMenu::warp`.
