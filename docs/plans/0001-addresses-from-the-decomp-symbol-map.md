# 0001 — Guest addresses from the decomp symbol map

RE knowledge lives in the decomp repository; the port reads guest addresses from its symbol map as generated
constants, never a hand copy. Spider-Man's decomp is `external/spider-man-decomp` (MIT, ours); its symbol map is
`config/symbols/<image>.txt`, splat `symbol_addrs` format, one file per executable (`slus_008.75`, `slus_013.78`).
Every name the port's headers use was seeded into it on 2026-11-15.

## Today

- `titles/spiderman1/guest_layout.h`, `platform_facts.h`, `stream_driver.h`, `level_warp.h`;
  `titles/spiderman2/guest_facts.h`; `game/render/gpu_env.cpp`, `scene_id.h`, `mesh_transform.cpp`: about 90
  `inline constexpr` addresses, each a hand copy with its meaning in a comment.

## Generator

The shared psxport generator of Tomba! 2's plan (psx-tomba `docs/plans/0065-addresses-from-the-decomp-symbol-map.md`):
`tools/symbol_constants.py` and the CMake function `psxport_symbol_constants`, writing
`namespace spider::sym::<image>` headers under `build/`. Spider-Man adds no second parser.

## Order

1. Once the generator lands in psxport, generate both images' headers from `spiderman_port`'s CMake.
2. One header per commit, in the order listed under Today: each constant becomes a use of the generated name, or
   its name is added to the decomp map first (decomp commit, submodule bump in the same port change). Code sites
   that are not symbols (return addresses inside a function) are written as `<function> + 0xNN`.
3. Delete each emptied header. Build with Clang; boot both titles once at the end to show no binding moved.
