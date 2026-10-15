# Features — Spider-Man and Spider-Man 2: Enter Electro

Title hops only. Each row is where a title joins a psxport feature hop (`psxport F-X.n`, psxport
`docs/features.md`); a feature with no row runs on the framework alone. Shared rows are `game/`; a
title's own rows name `titles/<title>/`.

## F-BOOT

| hop | joins | owner |
|---|---|---|
| 1 | psxport F-BOOT.1 | `game/product/catalog.cpp:createCatalog` — both identities from each `title.json`, one runtime each |
| 2 | psxport F-BOOT.5 | `titles/spiderman1/runtime.cpp:Runtime::startProgram` — `Crt0Boot`, then points the first turn at retail main `0x8002C354` |
| 3 | psxport F-BOOT.6 | `game/runtime/runtime.cpp:Runtime::registerOverrides` — `StockVSync` at each title's VSync and counter |
| 4 | psxport F-BOOT.6 | `titles/spiderman1/runtime.cpp:Runtime::registerOverrides` — `StockLibgte`, `StockLibcd`, `StockDmaCallbacks`, CdInit, the GPU DMA timeout, `StGetNext` |
| 5 | psxport F-BOOT.6 | `titles/spiderman1/session.cpp:Session` — per-Core `StreamDriver` |
| 6 | psxport F-BOOT.6 | `titles/spiderman2/runtime.cpp:Runtime::registerOverrides` — the `submitFrame` override at `0x8006BE98` |

## F-TURN

| hop | joins | owner |
|---|---|---|
| 1 | psxport F-TURN.2 | `titles/spiderman1/frame_driver.cpp:FrameDriver::stepFrame` — one turn through the injected `ExecuteSlice` to a measured VSync, movie or stream boundary or a spent budget, one field, one fence |
| 2 | psxport F-TURN.2 | `titles/spiderman2/frame_driver.cpp:FrameDriver::stepFrame` — enters crt0 at the admitted entry, then one budgeted turn to a VSync wait, one fence, presented if `submitFrame` ran |
| 3 | psxport F-TURN.4 | `titles/spiderman2/runtime.cpp:Runtime::reportRun` — presented and waited fields |

## F-INPUT

| hop | joins | owner |
|---|---|---|
| 1 | psxport F-INPUT.1 | `game/runtime/runtime.cpp:Runtime::deliverPad` — `StockPadBuffers` at each title's libpad receive buffers |

## F-CD

| hop | joins | owner |
|---|---|---|
| 1 | psxport F-CD.1 | `titles/spiderman1/runtime.cpp:Runtime` — stock `CdRead`, `CdReadSync`, `CD_cw`, `CdSync`, work area `0x800B3B2C`/`30` |
| 2 | psxport F-CD.3 | `titles/spiderman1/runtime.cpp:Runtime::stockCdReadLanded` — overlay code read into heap 1 (`overlayHeapArena`) |

## F-RENDER

| hop | joins | owner |
|---|---|---|
| 1 | psxport F-RENDER.1 | `game/runtime/runtime.cpp:Runtime::renderCapabilities` — Record, no producers, guest VRAM is not the picture |

## F-WIDE

| hop | joins | owner |
|---|---|---|
| 1 | psxport F-WIDE.1 | `game/runtime/runtime.cpp:Runtime::guestWidescreenProjection` — the stock aspect policy; Record widens the canvas, the retail centre stays |
| 2 | psxport F-WIDE.2 | missing — Spider-Man 1's outcode pass culls polygons wholly in a margin (issue 0034); Enter Electro unverified (issue 0036) |

## F-DEBUG

| hop | joins | owner |
|---|---|---|
| 1 | psxport F-DEBUG.1 | `titles/spiderman1/runtime.cpp:Runtime::titleCommand` — `warp <level>`, a name from the level table at `0x800974A4` |
| 2 | psxport F-DEBUG.1 | `titles/spiderman1/level_warp.cpp:LevelWarp::apply` — level name and selector 3, the demo flag cleared, from `FUN_8002C174`'s field wait; retail main's level route loads it |
