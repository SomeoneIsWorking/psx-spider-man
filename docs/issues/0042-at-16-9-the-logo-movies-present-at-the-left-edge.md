---
id: 42
title: "At 16:9 Spider-Man 1's logo movies present at the left edge of the screen"
status: open
symptom: the Activision, Neversoft and title-card movies fill sink columns 0..2864 of 3840 with black on the right, while the legal screen and menu sit centred at 483..3356
state_items: S008
tags: spiderman1,widescreen,movie,record
created: 2026-11-17
updated: 2026-11-17
---

# 0042 — At 16:9 the logo movies present at the left edge

## Observed
`spiderman_port` on SLUS_008.75, hidden, unpaced, sink 3840x2160, ires 0, fps60 1, aspect 3 (MatchSink), no
input, `pshot` every 100 frames (`scratch/ws/boot0/`, run on ally). Every movie present from frame 200 to 1600
has its picture in sink columns 0..2864: the 4:3 picture at the left with 960 columns of black on the right
(`b00700.png`). The legal screen (`b02100.png`) and the menu (`b03200.png`) span 483..3356, centred.

## Expected
The movie's 4:3 picture centred with equal black bars, as the legal screen and menu are.

## What differs
The retail STR player `FUN_8002AA0C` sets a 320x240 display (`PutDispEnv` at x 0) and only uploads decoded
frames (`LoadImage`); it draws no primitive. The run log announces the plan for it
(`[wide] native picture: aspect=3 native_width=320 render_width=428`), so the plan widens. A frame no record drew
has no margin canvas (`RecordRasterizer::present` falls to the VRAM image at the display rect) and the present
places that 320-wide picture as if it were the 428-wide view.

Not yet traced to a line: whether the canvas-less present should centre the display rect, or the plan should stay
4:3 for a frame that draws nothing. Owner is psxport's record present (`render/record_rasterizer.cpp`
`RecordRasterizer::present`, `render/record_present.cpp`); the title supplies nothing here.

The first menu present after the attract demo is placed the other way: `scratch/ws/skip1/s01900.png` (Start skip
run, frame 1900) shows the 4:3 menu at the right edge of the sink; the next shot (frame 1960) is centred.
