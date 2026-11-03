# 0039 — Enter Electro presents only black when left with no input

Status: open (2026-11-03). Owner: not yet traced (`titles/spiderman2/`).

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
