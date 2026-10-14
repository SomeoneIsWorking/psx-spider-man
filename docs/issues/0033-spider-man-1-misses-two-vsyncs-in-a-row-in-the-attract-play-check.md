---
id: 33
title: Spider-Man 1 misses two display fields in a row in the attract play check
status: open
symptom: the windowed player build's 3700-frame play check reports a longest miss run of 2, breaking RESTRUCTURE §0's "never 2 missed frames in a row"
state_items: S019
tags: pacing,frame-time,play-check
created: 2026-10-14
updated: 2026-10-14
---

## Observed

Windowed player build (Clang, Release), `PSXPORT_NATIVE_FRAMES=3700`, title and attract content only
(no replay or warp route reaches gameplay). Host load average 2.6–3.0.

| run | p50 ms | p99 ms | worst ms | frames over 1.5× | longest miss run | audio reprimes/drops |
|---|---|---|---|---|---|---|
| before M6 | 16.75 | 22.00 | 501.08 | 8 (0.22%) | 2 | 0/0 |
| after M6 | 16.75 | 22.50 | 520.13 | 8 (0.22%) | 2 | 0/0 |

Expected: no run of two missed fields. The miss share (0.22%) is inside the 0.5% limit; the run of 2
and the ~500 ms worst frame are not. The same result before and after M6 means the M6 catalog and
runtime change did not introduce it.

## Not yet known

Which frames miss, and whether the ~500 ms worst frame is the same event. Next step: rerun with the
per-frame time log and match the long frames to the guest state (movie start, mode switch, CD read)
at those fences.
