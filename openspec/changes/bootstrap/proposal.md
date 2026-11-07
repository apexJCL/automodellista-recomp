# Auto Modellista bootstrap on xbr

## Why
Auto Modellista (Capcom, 2003, title 0x4343000E) is the first game I bring up
the way a new porter would: `xbr new` from the disc, then the documented
`setup`, `analyze`, `recomp` and `build` steps, with no other game's repo
open. It tests how far the CLI's defaults and the toolkit carry an unseen
XDK 5558 title, and it collects the friction a porter meets on the way.
It is also a third D3D8 racing title next to Burnout 3 and SSX 3,
with CRI Sofdec/ADX movies and a DSP-mixed DSOUND path.

## What changes
- A game project made with `xbr new` (CLI d43db28, toolkit d683d5de).
- Game-side fixes only: `config/seed_functions.json`, `src/recomp_manual.c`
  and `game.toml`. `src/recomp/gen/` is never edited by hand.
- Scripted, headless Metal runs on the Mac with frame dumps, which reach the
  title and the menus as far as the course select.

## Pass bar
The bootstrap passes when a scripted run reaches the in-game menus with no
lifted-code failure, and the first change the CLI or the toolkit needs is
named with a diagnosis. Racing, sound and saves come in a later change.
