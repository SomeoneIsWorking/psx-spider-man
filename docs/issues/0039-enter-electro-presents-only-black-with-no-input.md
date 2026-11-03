# 0039 — Enter Electro presents only black when left with no input

Status: fixed (2026-11-03). Owner: `titles/spiderman2/runtime.*` (`spider::enterelectro::Runtime`).

## Observed
Spider-Man 2 run alone, hidden, unpaced, sink 3840x2160, ires 0, fps60 1, no input, 150 s
(`spiderman_port scratch/assets/spiderman2/SLUS_013.78`, `scratch/picker2/solo2`): 5060 steps, every shot black.
As the focused selector panel (`scratch/picker2/walk`, `right_*.png`) it stays black for 16472 steps (about 9.6e9
guest instructions) while the panel counts as ready. Spider-Man 1 in the same run reaches its menu and a "DEMO"
level.

## Open
Issue 0036 records a Marvel logo once; it is not known whether this is a regression or whether the logo needs input.
The recorded frontier names the likely owner: `docs/re-frontier.md` EE-04, the intro STR stream after the logo
waits in `FUN_80093548` for a libstr ring slot that never fills while a CD interrupt (I_STAT&I_MASK 0x004) goes
unclaimed. Not re-traced on HEAD in the 2026-11-03 picker pass; next step is a hang sample of the guest PC in the
selector walk to confirm it is that wait.
Effect on the selector: Enter Electro's panel never shows a picture or its attract demo.

## Cause (traced 2026-11-03, `scratch/picker3/sm2*`)
The intro STR player `FUN_8003035C` spins at `0x8003067C` in `FUN_80030D78` -> `FUN_80093548` for a ring slot in
state 2. Its stream read (Setloc 36:51:32, Setmode 0xE0, ReadS) delivers frame 1's nine sectors into the ring at
`0x8015167C`; on the last one `StCdInterrupt` (`FUN_80091A34`) sets `0x800CE444` = 1 and arms DMA3's interrupt, and
the DMA3 completion callback `0x800986F4` (libapi table `0x800C12B8`, channel 3) is what marks the frame ready and
clears the flag. `spider::enterelectro::Runtime::registerOverrides` bound no `StockDmaCallbacks`, so psxport's
`Hle::irqPoll` read the empty native registry, acknowledged the completion and ran nothing: every later sector was
skipped and the ring never filled.

## Fix
`Runtime` binds `psx::hle::StockDmaCallbacks(0x800C12B8)` (libapi `DMACallback` `0x8009D174`, word for word
SLUS_008.75's `0x8009152C`). The movie then plays, and the title's next screen jumps into an overlay it read to
`0x80150300` through its own libcd, which no code image owned (`ambiguous code-image identity`); `Runtime` now
declares heap 1 `[0x800CF0EC, 0x801FE000)` (bounds at `0x800A8B94`, set by `FUN_800773E8`) as its
`guestCodeModuleWindow`, so the CD transfer into it activates the module residency.

## Proof
`spiderman_port scratch/assets/spiderman2/SLUS_013.78`, build/picker3, hidden, unpaced, sink 3840x2160, ires 0,
fps60 1, no input: `scratch/picker3/sm2l/sheet.png` (15 shots, 10 s apart): the Activision logo, the intro STR
movie, the PRESS START title, the attract DEMO, the title again; 877 presented fields in 8327 steps, clean exit.

## Left open
In the DEMO level only actors and the caption draw; the city geometry is black (`sm2l/sheet.png` row 3). Not traced.
