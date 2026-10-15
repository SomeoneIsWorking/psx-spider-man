# Spider-Man 2: Enter Electro (`SLUS_013.78`)

`SLUS_013.78`, the USA disc boot executable/serial, is this title's canonical key. The launcher and
provisioner read `SYSTEM.CNF` from the selected disc on every run and refuse the Spider-Man 1 disc.
`spider::enterelectro::Runtime` derives from the shared `spider::Runtime`; it binds none of Spider-Man
1's addresses.

The manifest also owns the measured 786,432-byte executable identity and SHA-256
`dbe6c3f32337fe0fa7085519c728a75abf5d007b45ea0ba58178bcf84b72908a`. Provisioning checks fresh
media, and the host repeats the serial/size/PS-X EXE/SHA-256 check before constructing `Game`;
matching the filename alone is never sufficient.

The title is one panel of the Spider-Man catalog; its `FrameDriver` enters crt0 at the admitted header entry and runs one budgeted guest turn per host step.
A turn ends at a libetc VSync wait or when the field's cycle budget is spent, and commits exactly one
presentation fence for one display field. The field presents if the guest called
`submitFrame 0x8006BE98` inside the turn, otherwise it rescans the last frame; the guest's own
VBlank-counter wait sets the frame rate. The
Marvel logo presents; the intro STR stream then waits forever for its first sector (`EE-04` in
`docs/re-frontier.md`). No gameplay, native producer, widescreen culling or runtime-module support is claimed.
Enter Electro follows only after Spider-Man passes its representative-gameplay migration gate.
