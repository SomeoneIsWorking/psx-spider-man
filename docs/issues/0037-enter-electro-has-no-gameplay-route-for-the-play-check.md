---
id: 37
title: "Enter Electro has no route to gameplay, so the real-window play check cannot run on it"
status: open
symptom: after `pick spiderman2` the Marvel logo plays and the window then stays black for the rest of a 75 s run
state_items: S003, S019
tags: spiderman2,play-check,cd
created: 2026-10-16
updated: 2026-10-16
---

Reproduce: `scratch/play-check/play.py spiderman2 build/play-check/bin/spiderman_port none 75 boot` (real window,
`config/aspect_16x9.ini`). The run end reports `intervals=4506 missed=0 longest_miss_run=0`,
`audio: reprimes=0 drops=0` and no record presents. The picture is black from the logo's end to the run's end.
The log reports `pending I_STAT&I_MASK=0x004; no SysEnq element claimed it`.

Cause: the intro STR player `FUN_8003035C` waits forever for its first frame from the libstr ring at
`0x800CE44C`, and no CD stream feeds that ring (EE-04 in `docs/re-frontier.md`). Every later screen depends on it,
so a `warp` has no level loader to reach.

Gap: a warp in the Tomba 2 pattern needs EE-04 first: the CdInit callback install and the stream read, as
Spider-Man 1 did in issues 0024 and 0026. Then a level route traced from Enter Electro's own level loader.
