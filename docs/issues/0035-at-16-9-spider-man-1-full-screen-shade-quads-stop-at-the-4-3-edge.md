---
id: 35
title: "At 16:9 Spider-Man 1's full-screen shade quads cover only the 4:3 picture"
status: open
symptom: attract demo at 16:9, frame 14000, a translucent darkening covers x 86..597 and the margins stay bright, with visible edges at both joins
state_items: S008
tags: spiderman1,widescreen,2d,record
created: 2026-10-15
updated: 2026-10-15
---

Reproduce: `spiderman_port` headless with `aspect=1`, `pick spiderman1`, shot at frame 14000
(`scratch/restructure/smoke/sm1-169-attract/f14000.png`). The darkened span is exactly the
4:3 picture, so the quad is authored to the 512-wide retail screen and Record places it there.

Not yet traced: which guest function emits the quad and whether it is the transition wipe
(the deleted `Spider1TransitionWipe` named one retail wipe). Next: watch
the GP0 record at frame 14000 for a 512-wide semi-transparent primitive, decompile its emitter, and widen the
quad there (x -86, w 684) under the title's aspect policy.
