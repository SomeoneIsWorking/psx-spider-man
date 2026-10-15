---
id: 34
title: "At 16:9 Spider-Man 1 culls polygons wholly inside the margins, so the margins hold only polygons that straddle the 4:3 edge"
status: open
symptom: attract demo at 16:9 (684x240 Record canvas) draws geometry into both margins, but it stops in jagged edges and black shows beyond them
state_items: S008
tags: spiderman1,widescreen,culling,record
created: 2026-10-15
updated: 2026-10-15
---

Reproduce: `spiderman_port` headless with `aspect=1`, `pick spiderman1`, then `warp l1a2_t` and a shot in gameplay
(`scratch/restructure/smoke/sm1-169-final/f9000.ppm`), or a shot at frame 13500 (the Lizard
attract demo). The canvas is 684 wide and the retail centre 256 is kept, so the canvas widening works; the
polygons in the margins are those the guest submitted because they cross x 0 or x 511.

Cause: `FUN_8007C2AC` (the world vertex transform and outcode pass) reads the window from `*DAT_800b5918`:
u16 +0 right, +4 left, +2 bottom, +6 top, +8 near z, +10 far z, each zero-extended and compared signed
against the projected SX/SY. A polygon is rejected when every vertex shares an outcode bit. With the retail
centre the left margin is screen x -86..-1, and a zero-extended left bound cannot go below 0, so a polygon
wholly in the left margin is always rejected. The right bound can move to 597 as a u16, but `FUN_80075D0C`
derives OFX as `(u16[0] + u16[2]) >> 1` and the frustum planes from the same span, so widening only the right
bound moves the centre (OFX 342), which is the slide Record must not show.

Proper fix: a native `FUN_8007C2AC` with a sign-extended left bound, and the publication from `FUN_80075D0C`
widened symmetrically (planes from the 684 span, OFX kept at 256), both bound in `spider1::Runtime`. The
deleted `Spider1Widescreen` widened right-only on the Gte path (issue 0025); it cannot be reused on Record.
