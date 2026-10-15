# Spider-Man (`SLUS_008.75`)

First migration target (USA). `SLUS_008.75`, the disc boot executable/serial, is its canonical
title key. `spider::spider1::Runtime` (`runtime.*`) owns its stock library bindings and image-aware
overrides. Its measured
749,568-byte executable SHA-256 is
`d2270e35581ba083d9441166e9a45ead4f869ab07e890f9a512ad7ee4cc0b15b`; both provisioning and native
boot authenticate that identity before using the cache.

The two titles are separate runtime images and stay title-isolated; what they share is the
`spider::Runtime` base in `game/runtime/`. Remaining guest code executes from this authenticated image
through Lightrec, and the guest's GP0 record is the presented picture (Record).

The first discriminator is both-logo completion followed by early `dem1`, with the unchanged retail
movie body suspended and resumed by the runtime executor. Representative interactive gameplay is
required before the generator and old corpus are deleted.

The product is hosted: `spider::Catalog` (built from both `title.json` files) runs this title under
psxport's `ProductHost` beside Enter Electro, and `Runtime::startProgram` runs the framework's audited
crt0 (`stackBias` -8, crt0 `0x800873CC`) before entering retail main.
