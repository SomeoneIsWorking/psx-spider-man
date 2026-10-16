# Project state — spider1

`partial` = some subset demonstrated; `missing` = nothing product-facing exists; `blocked` = the
predecessor is missing; `verified` = the observable condition holds. Which reverse-engineering steps
are ground-truth-ready and which are not lives in `docs/re-frontier.md`.

| id | capability | state | evidence / gap |
|---|---|---|---|
| S001 | Launcher selects and authenticates one title, provisions its executable, launches the product | verified | `tools/run.py`, `tests/test_help_contract.py`, `tests/test_provision.py`, `tests/test_title_catalog.py`; both serials plus missing/unknown/ambiguous/mismatched inputs |
| S002 | Spider-Man reaches both intro movies and early `dem1` under its native owners | partial | Lightrec run plays the three retail logo movies (425 STR fields: ATVILOGO, LOGO, TTSLOGO) through the guest's own CD interrupt service and DMA3 frame callback, then the overlay loader's CdRead landings publish as images (issue 0031), the legal screen, the main menu and an attract demo (3D, "DEMO" caption) present at both aspects; the first attract demo is `dem1` (its six files `dem1.sfx/.vab/_g/_l/_o/_t` are read, issue 0025); the loop then plays `dem3` |
| S003 | Enter Electro executes through its measured crt0 and first game-owned-call boundary | partial | `SLUS_013.78` authenticated; `enterelectro::FrameDriver` runs crt0, main `0x80031F54` and the Marvel logo with one fence per step (600/600 at the cap); the intro STR stream waits forever for its first sector (EE-04) |
| S004 | Spider-Man platform services owned through measured native seams | partial | psxport's `StockLibgte` (SetGeomOffset/SetGeomScreen), `StockLibcd` (CdRead/CdReadSync/CD_cw/CdSync), `StockVSync` and `StockDmaCallbacks` bound in `spider1::Runtime::registerOverrides`, CdInit and the GPU DMA timeout as title overrides; retail CD command wait and full callback equivalence unverified |
| S005 | Render seam owns frame policy and a native frame envelope | partial | the owners in `spider::render` compile as `spider_render_contracts` and no product links them yet — an image-aware title render boundary is required |
| S006 | Named scenes have complete native display-list producers | missing | no scene has a producer; see issue 0013 |
| S007 | Animated mesh input, pose identity, and pre-GTE composition contracts | partial | `mesh_animated_vertex.*`, `mesh_pose_contract.*` decode and are unit-tested; no live corpus run has produced valid pose rows |
| S008 | Title-correct widescreen projection from active game state | partial | Record path with the stock aspect policy; the 684x240 canvas keeps the retail centre 256. The menu is the 4:3 picture at x 86 with black margins and the attract demo and its "DEMO" caption sit at their 4:3 place, not stretched or slid. Gaps: polygons wholly inside a margin are culled by the outcode pass `FUN_8007C2AC` (issue 0034); full-screen shade quads cover only the 4:3 picture (issue 0035) |
| S009 | True per-object interpolated 60 fps | missing | scope undecided: the guest's per-frame field count is neither statically nor dynamically established — issue 0030 |
| S010 | No whole-frame compatibility fallback on the shipping path | verified | the retired whole-guest-frame owner, its selector and its tests are absent; no tool guards against their return since `tools/source_policy.py` was deleted |
| S011 | Maintainer gates verify hermetic contracts and bounded product behavior | partial | `tools/verify.py` runs configure/build/CTest/boundary inspection on Linux x86-64; no release host is qualified |
| S012 | Input, memory card, and runtime module placement support gameplay | partial | pad receive-buffer layout proven by `spiderman1_runtime`; after `warp l1a2_t` a held Up runs Spider-Man forward on the rooftop (`scratch/restructure/smoke/sm1-warp-play/`); memory card and module placement not qualified |
| S013 | FMV and audio delivery through host-owned services | partial | first movie field presents; no durable A/V synchronization, audio not user-confirmed |
| S014 | Two-title isolation | verified | serial-keyed selection; `spider_title_link_isolation` finds no symbol shared between the two title libraries, and each title library sees only its own headers and manifest |
| S015 | First-party C++ passes Clang build, format, clang-tidy, and structure policy | verified | `cpp_policy` CTest over the real compile database |
| S016 | Enter Electro native producers, widescreen, and temporal interpolation | missing | no producer; Record with the stock aspect policy centres the Marvel logo at 16:9, and nothing past it presents (EE-04, issue 0036) |
| S017 | Remaining guest code runs through a per-`Core` Lightrec runtime with no selectable interpreter | partial | 352,889 translated blocks / 1,798,345 instructions, zero interpreter fallback; original-call title dispatch and module invalidation unproven |
| S018 | Spider-Man reaches `dem1` dynamically and resumes the retail movie player through host-owned field boundaries | partial | the STR ring, DMA3 frame callback and all 425 movie fields run on the guest's own CD interrupt service; a turn that exhausts its budget delivers one display field; the overlay loader (`FUN_8001B990`) reads `NAME.bin` into guest heap 1 through the stock CdRead and each read is published as an image by psxport's `publishStockReadLanding` (issue 0031); the run reaches the legal screen, menu and demo. the first demo is `dem1` by the guest's own disc reads (issue 0025) |
| S019 | Representative interactive gameplay conforms on each released host | missing | `warp <level>` reaches player-controlled Spider-Man 1 levels (`spider1::LevelWarp`), Spider-Man 2 none (EE-04); conformance is unmeasured: also needs invalidation, oracle comparison, and per-host frame-time budgets; the attract play check misses two fields in a row (issue 0033, not reproduced 2026-10-16); 2026-10-16 real-window play check: Spider-Man 1 misses 23% of presents after `warp l1a2_t` at auto internal resolution (psxport issue 0161), Spider-Man 2 has no route past the logo (issue 0037) |
| S020 | Load operations complete without loading-only waits; logos cancel through the recovered route | missing | no load operation has been classified yet |
| S021 | Spider-Man 1 runs through psxport's multi-title host (in-window picker) | partial | zero arguments open the picker over both titles (`spider::Catalog`), `pick spiderman1` boots the generic `TitleSession` and plays the logo movies, `session return` rebuilds the picker; the run continues through the legal screen, menu and demo (issue 0031). Spider-Man 2 is the catalog's second panel and boots through its own frame driver. A guest stop ends the session with exit status 0 |

## Comparison baseline deltas

Each row is a user-visible delta against the original USA PSX releases run in a faithful emulator.

| delta | state | evidence / gap |
|---|---|---|
| Widescreen | partial | Record 684x240 canvas with the retail centre; margins hold only polygons that cross the 4:3 edge — issues 0034, 0035, 0036 |
| 60 fps interpolation | missing | scope undecided — issue 0030 |
| Loading removal | missing | — |
| Controls | partial | forced pad input walks the front-end; deterministic replay exists but a present index is still not content-stable — issue 0009 |
| Speed / cadence | partial | 59.71 presents/second is the host's `quota=1` pacer, not a measured retail rate; present pacer runs at 60 kHz — issue 0012 |
| Platforms | missing | Linux x86-64 asset-free build only; Windows, macOS x86-64/arm64 and Android arm64 have no qualified backend |

## Hosted platform coverage

| platform | state | gap |
|---|---|---|
| Linux x86-64 | partial | asset-free product composition gate passes; authenticated gameplay and release performance unverified |
| Windows x86-64 | missing | no qualified psxport/Lightrec product backend or Spider build contract |
| macOS x86-64 | missing | no qualified backend |
| macOS arm64 | missing | AArch64 emitter, executable-memory/ICache boundary, packaging, gameplay |
| Android arm64-v8a | missing | AArch64 backend, shared Android packaging/runtime, touch layer, device performance |

**Current focus:** S008 — a native outcode pass with a signed left bound and a symmetric frustum (issue 0034), then S019 conformance in levels reached by `warp <level>`.

