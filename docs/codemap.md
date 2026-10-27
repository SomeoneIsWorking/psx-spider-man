# Codemap

Where code lives in the Spider-Man port. Capability state is in `docs/project-state.md`, feature hops in
`docs/features.md`, the evidence chain in `docs/re-frontier.md`, atomic work in `docs/issues/`. A title's
identity is in `titles/<title>/title.json`; its measured addresses are in its facts headers.

Product path: `run.sh` → the framework main (`psxport_add_product`, target `spiderman_port`) →
`psx::host::ProductHost` over `spider::Catalog` (in-window picker; `pick spiderman1|spiderman2`) →
`TitleSession` → the title's `Runtime` → its `FrameDriver::stepFrame` on psxport's Lightrec executor.

## Placement rules

- `titles/spiderman1/` holds what is measured out of `SLUS_008.75`, in namespace `spider::spider1`;
  `titles/spiderman2/` holds `SLUS_013.78`, in `spider::enterelectro`.
- `game/` holds what both titles share, in namespace `spider` (`spider::render` under `game/render/`), and
  names no title address. A title differs from the shared owner by subclass and virtual override only.
- Includes name the owning directory: `"runtime/runtime.h"` under `game/`, `"spiderman1/runtime.h"` under
  `titles/`; psxport headers are bare.
- psxport (`external/psxport`, a link to the live checkout) owns the mechanism: the executor, override keys,
  stock libcd/libgte/VSync/pad/DMA classes, `Crt0Boot`, presentation. A title supplies addresses.

## Owners

| Owner | Path | What it owns |
|---|---|---|
| `spider::Runtime` | `game/runtime/runtime.*` | The shared `GameRuntime`: `StockVSync`, `StockPadBuffers`, Record, the stock aspect policy |
| `spider::reportExecutionResult` | `game/execution/guest_execution.*` | The report of a guest turn that stopped without returning; turns run through psxport's `dispatchGuestNow`/`dispatchGuestUntilExitNow` |
| `spider::Catalog`, `createCatalog` | `game/product/catalog.*` | Both identities and runtimes, provisioned under `scratch/assets` |
| `spider::render::*` | `game/render/` | Render contracts not linked by the product (`spider_render_contracts` and the `mesh_*` tests): frame envelope, libgpu env records, scene ids, mesh decode |
| `spider1::Runtime` | `titles/spiderman1/runtime.*` | `startProgram` (crt0, retail main), `StockLibgte`/`StockLibcd`/`StockDmaCallbacks` bindings, the session, overlay landings |
| `spider1::Session`, `session(core)` | `titles/spiderman1/session.*` | Per-Core state outside guest RAM: the `StreamDriver` |
| `spider1::FrameDriver` | `titles/spiderman1/frame_driver.*` | Retail main as host turns through an injectable `ExecuteSlice`: the boundary each turn stopped at (`ResetGraph` VSync, the retail STR player's three VSyncs, a dry `StGetNext`, a spent budget) as one field and one fence; the CdInit, DMA-timeout and VSyncCallback overrides |
| `spider1::DebugMenu` | `titles/spiderman1/debug_menu.*` | The title debug menu (psxport `TitleDebugMenu`): `warp <level>` over the guest level table's names |
| `spider1::LevelWarp` | `titles/spiderman1/level_warp.*` | `warp <level>`: the level script's change-level exit, raised from the gameplay loop's field wait |
| `spider1::StreamDriver` | `titles/spiderman1/stream_driver.*` | The `StGetNext` override: super-call, controller pump on a dry poll, the field that poll waited through |
| `spider1::CdInitialization` | `titles/spiderman1/cd_initialization.*` | The retail `CdInit` body: four callback slots and the CD interrupt arm |
| `spider1::FieldClock` | `titles/spiderman1/field_clock.*` | The field count and the `VSync(0)` answer from the guest's own words |
| facts | `titles/spiderman1/platform_facts.h`, `guest_layout.h` | Every named `SLUS_008.75` address, word and threshold |
| `enterelectro::Runtime` | `titles/spiderman2/runtime.*` | The `submitFrame` override, the frame driver, the run report |
| `enterelectro::FrameDriver` | `titles/spiderman2/frame_driver.*` | Enters crt0 at the admitted entry; one budgeted turn and one fence per host step, presented if `submitFrame` ran |
| facts | `titles/spiderman2/guest_facts.h` | `submitFrame`, VSync and its counter, pad buffers |
| tests | `tests/` | Each title's owners through the shipping sources, the catalog, mesh contracts, link isolation, the picker |
| tools | `tools/` | Launcher, provisioning, title catalog |

## The frame turn

| Hop | Owner |
|---|---|
| product spine | `ProductHost` → `TitleSession`: `admitExecutable` → `Runtime::registerOverrides` → `startProgram` |
| Spider-Man 1 step | `spider1::FrameDriver::stepFrame`: apply an armed warp, run one turn, service its stop (VSync, movie field, dry `StGetNext`, spent budget) as one field and one fence |
| Spider-Man 2 step | `enterelectro::FrameDriver::stepFrame`: one budgeted turn, one fence |
| presentation | psxport `FramePresenter::commit`; Record widens the canvas at 16:9 and keeps the retail centre |

## Input, CD, widescreen

| Concern | Owner |
|---|---|
| host pad | psxport `Pad`, serviced by `LogicFrame::begin` once per delivered field |
| guest pad buffers | `spider::Runtime::deliverPad` → `StockPadBuffers` |
| stock libcd | `StockLibcd` bound in `spider1::Runtime::registerOverrides`; CD reads are synchronous |
| CD interrupt | the guest's own handler `0x8008DA24` through Lightrec, armed by `CdInitialization` |
| overlay code | `spider1::Runtime::stockCdReadLanded` → `publishStockReadLanding` over `overlayHeapArena` |
| widescreen | `spider::Runtime::guestWidescreenProjection` (stock aspect policy); margin culls missing (issues 0034, 0035, 0036) |

## Where does it go?

| Responsibility | Owner |
|---|---|
| A title serial, hash or header word | `titles/<title>/title.json`, read through CMake definitions |
| A measured address | the title's facts header, at a name; `kUnattributed…` when the role is unknown |
| Behaviour only one title has | that title's `Runtime` override or session under `titles/<title>/` |
| A display field or presentation fence | the title's `FrameDriver`, and only there |
| A native override | `psx::cpu::installNativeOverride` from the owning module's install |
| A debug route to a level or screen | a verb in the title's `DebugMenu` (psxport `TitleDebugMenu`) the title's frame driver applies at a frame boundary through the guest's own route |
| Executor, presentation, stock library mechanics | psxport |
