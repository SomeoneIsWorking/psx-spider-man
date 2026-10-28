# 0038 — The frame loop is not owned

Status: open (2026-10-28). Owner: `titles/spiderman1/frame_driver.cpp, titles/spiderman2/frame_driver.cpp`.

## What the code does
Spider-Man runs retail main `0x8002C354` and Enter Electro runs `0x80031F54` as guest turns; each turn ends at a libetc VSync (`0x80084BE0`, `0x80091604`) or a spent budget and is one field and one fence.

## Why it matters
The workspace rule (psxport `docs/workspace/WORKSPACE.md`, "Each title owns its frame loop") wants the frame driver to run the guest's per-frame update and draw and decide the cadence, replacing the guest's VSync waits. Until then the loop is on the fallback; it is not yet on `Timing::vsyncFields` either, since every guest VSync is answered with one field whatever its argument.

## What blocks it
Neither title's per-frame update and draw is identified inside main; Spider-Man 1's per-frame field count is not statically known (issue 0030). Next: decompile both mains' loops and drive their update/draw pairs.
