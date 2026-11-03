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
Effect on the selector: Enter Electro's panel never shows a picture or its attract demo.
