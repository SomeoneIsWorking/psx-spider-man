---
id: 34
title: "At 16:9 Spider-Man 1 culls polygons wholly inside the margins, so the margins hold only polygons that straddle the 4:3 edge"
status: open
symptom: attract demo at 16:9 (684x240 Record canvas) draws geometry into both margins, but it stops in jagged edges and black shows beyond them
state_items: S008
tags: spiderman1,widescreen,culling,record
created: 2026-10-15
updated: 2026-11-25
---

Reproduce: `spiderman_port` headless with `aspect=1`, `pick spiderman1`, then `warp l1a2_t` and a shot in gameplay
(`scratch/restructure/smoke/sm1-169-final/f9000.ppm`; still shown on `f1e488b`, hidden, `ires=0`, frame 9000), or a shot at frame 13500 (the Lizard
attract demo). The canvas is 684 wide and the retail centre 256 is kept, so the canvas widening works; the
polygons in the margins are those the guest submitted because they cross x 0 or x 511.

Cause: `FUN_8007C2AC` (the world vertex transform and outcode pass) reads the window from `*DAT_800b5918`:
u16 +0 right, +4 left, +2 bottom, +6 top, +8 near z, +10 far z, each zero-extended and compared signed
against the projected SX/SY. A polygon is rejected when every vertex shares an outcode bit. With the retail
centre the left margin is screen x -86..-1, and a zero-extended left bound cannot go below 0, so a polygon
wholly in the left margin is always rejected. The right bound can move to 597 as a u16, but `FUN_80075D0C`
derives OFX as `(u16[0] + u16[2]) >> 1` and the frustum planes from the same span, so widening only the right
bound moves the centre (OFX 342), which is the slide Record must not show.

State: `ViewPublish` (0x80075D0C) publishes side planes over the widened screen with H and OFX kept; `ScreenCull`
owns the five outcode passes and the three flagged-word passes, and `FaceWriter` owns `Faces_write_flagged`
(0x8007F394), all comparing against `ScreenBounds` widened by the canvas margin
(`titles/spiderman1/screen_{window,cull}.cpp`, `face_writer.cpp`, `view_publish.cpp`). At `warp l1a2_t`, 16:9, the
left wall and the fence now reach both window edges (`scratch/picker/sm/g2.png`, 2026-11-25).

Remaining: the large near floor still ends in a saw-tooth edge in the right margin: its subdivided triangles are
dropped (`scratch/picker/sm/g2_right.png`). The subdivision face writers 0x8007D978 / 0x8007DDC8 build their split
vertices' flagged words from retail window word 0 (spider-man-decomp `docs/notes/screen-cull.md`); they need native
bodies over `ScreenBounds` like `FaceWriter`.

Until it is fixed the README carries no widescreen gameplay stills or 4:3-vs-16:9 comparison.
