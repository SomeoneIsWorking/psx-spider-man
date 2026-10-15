---
id: 36
title: "Enter Electro's 16:9 picture is unverified past the Marvel logo"
status: open
symptom: at 16:9 the Marvel logo is centred on the 684x240 Record canvas with black margins; no 3D frame exists to show whether the guest's culls leave the margins empty
state_items: S016
tags: spiderman2,widescreen,record
created: 2026-10-15
updated: 2026-10-15
---

Reproduce: `spiderman_port` headless with `aspect=1`, `pick spiderman2`, shot at frame 300. The logo is the
4:3 picture at x 86 with black margins, which is correct for a 2D screen. Nothing past it presents: the intro
STR stream waits for its first sector forever (EE-04 in `docs/re-frontier.md`), so no world frame exists.

Gap: the title's projection and cull owners are not reverse-engineered. Next, after EE-04: find the
SetGeomOffset/SetGeomScreen callers and the vertex outcode pass, as issue 0034 did for Spider-Man 1.
