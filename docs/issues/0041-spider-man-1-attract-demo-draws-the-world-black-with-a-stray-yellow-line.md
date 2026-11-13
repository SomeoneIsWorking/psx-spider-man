---
id: 41
title: Spider-Man 1's attract demo draws the world black with a stray yellow line across the screen
status: open
symptom: the attract demo shows the gunmen, a debris pile and a red marker on black, with no level geometry, and one thin yellow line from the top edge to the bottom
state_items: S008,S019
tags: render,attract
created: 2026-11-13
updated: 2026-11-13
---

# 0041 — Spider-Man 1's attract demo draws the world black with a stray yellow line

## Observed
Player build (psx-spider-man `main` after the two-disc launcher fix) installed on a second machine with
psxport `tools/port/remote_install.py`, launched from its `play.sh` hidden in headless gamescope at 1280x720
(`PSXPORT_PRESENT_SINK=1280x720`), player settings `ires = 0`, `fps60 = true`. About 30 s after launch the
present (`pshot`) is black except one yellow line running from x≈705 at the top to x≈830 at the bottom. About
60 s after launch the attract demo shows two gunmen, a body, a blue debris pile and a red marker, all on black,
with the same yellow line. Frame: `scratch/ally-install/attract-60s.png` (untracked).

## Expected
The demo's level geometry (rooftops, walls, floor) drawn behind the characters, and no line across the screen.

## Reach
`./run.sh` with both discs configured opens the picker; with no input at all the product reached this
attract demo. Capture with the debug server's `pshot` about 60 s in. Not yet checked on
this PC's build, at 4:3, or with `fps60 = false`, so the cause may be the widescreen culling of 0034, the
60 fps replay, or neither.
