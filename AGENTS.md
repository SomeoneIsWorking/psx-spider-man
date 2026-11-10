# Spider-Man port

Both Neversoft PSX Spider-Man games as native PC products; remaining guest instructions run through
psxport's Lightrec integration. Execution policy: the `dynarec-port`, `dynarec-runtime` and
`dynarec-overrides` skills. Also read [`../AGENTS.md`](../AGENTS.md) and
[`external/psxport/AGENTS.md`](external/psxport/AGENTS.md). The product-execution plan is
[`docs/migration.md`](docs/migration.md).

## How the product starts

Both titles are one product: `spiderman_port` runs psxport's `psx::host::ProductHost` over
`spider::Catalog` (`game/product/catalog.*`, identities from each `titles/<title>/title.json`)
through the framework main. Zero arguments open the in-window picker over the provisioned titles; one
executable argument runs that title directly. Boot is the host's generic `TitleSession` sequence. Each
title's runtime derives from `spider::Runtime` (`game/runtime/runtime.*`: VSync, pad buffers, Record
presentation, aspect policy). Spider-Man 1's `spider1::Runtime` runs crt0 and points the first turn at
retail main in `startProgram`, binds stock libgte/libcd/DMA callbacks in `registerOverrides`, and steps
with `spider1::FrameDriver::stepFrame`; Spider-Man 2's `enterelectro::FrameDriver` enters its crt0 and
runs one budgeted guest turn and one presentation fence per step.

To reach a Spider-Man 1 level, send `warp <level>` (a level table name such as `l1a2_t`; `warp ?`
lists them) on the debug server; `spider1::LevelWarp` raises it through the guest's own level route.

## Spider-Man first discriminator

The first migration checkpoint is the recorded `dem1` route, reached through Lightrec with nonzero
translated blocks and the existing native owners active. The build-derived `FUN_8002AA0C`
movie-fiber body is replaced by resumable runtime guest execution: the title field owner suspends and
resumes at the three authenticated STR VSync return PCs without rewriting the guest body. This
checkpoint proves wiring only; representative interactive gameplay, native and original-call
dispatch, invalidation, oracle comparison and bounded-fallback evidence are still required.

## Enter Electro

Enter Electro (`SLUS_013.78`) stays behind Spider-Man. Preserve its measured identity, entry and
present point (`docs/re-frontier.md` EE-00..EE-04); do not infer any Spider-Man address or behavior
from lineage similarity.

## Rendering

Native render producers consume pre-GTE game state. Guest GTE/OT/packet output can remain an
explicit, non-interpolated whole-frame debt path, never native-producer input.
