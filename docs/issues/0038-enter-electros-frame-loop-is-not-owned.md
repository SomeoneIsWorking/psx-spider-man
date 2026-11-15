# 0038 — Enter Electro's frame loop is not owned

Status: open (2026-10-28, Spider-Man 1 owned 2026-11-15). Owner: `titles/spiderman2/frame_driver.cpp`.

## What the code does
Enter Electro runs main `0x80031F54` as guest turns; each turn ends at a libetc VSync (`0x80091604`) or a spent budget and delivers `Timing::takeStepFields()` (the replaced wait's fields, or the edges a budget-ended turn crossed) and clocks the SPU once per field (issue 0039).

## Why it matters
The workspace rule (psxport `docs/workspace/WORKSPACE.md`, "Each title owns its frame loop") wants the frame driver to run the guest's per-frame update and draw and decide the cadence, replacing the guest's VSync waits. Until then the loop is on the fallback.

## What blocks it
The per-frame update and draw are not identified inside main, and nothing past the Marvel logo presents (EE-04). Next: decompile the main loop and own it the way `spider1::FrameDriver` owns `GameLoop_run`.
