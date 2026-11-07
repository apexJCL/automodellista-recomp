# Changelog

What changed for players in each release, newest first, in the style of
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). A release tag's
section is its GitHub release notes: before tagging vYYYY.MM.N, add
`## [YYYY.MM.N] - YYYY-MM-DD` with the changes (Added, Changed, Fixed,
Removed). .github/workflows/release-wizard.yml stops when the tag has none,
and builds the wizard from game.toml's `[wizard]` table.

## [Unreleased]

**Auto Modellista, from the title screen to the race on PC, Steam Deck
and Mac.** The first public build: the menus, the course, car and tuning
screens, and a race on the grid, through Direct3D 11 (tested under Proton
on the Deck; native Windows pending) or Metal.

### Added

- **Windows, Steam Deck and Mac packages**: `package windows`,
  `package steamos` and `package macos` build AutoModellista with your own
  dump inside. Saves and settings live in `%LOCALAPPDATA%\AutoModellista`,
  `~/Games/AutoModellista` or `~/Library/Application Support/AutoModellista`;
  `AUTOMODELLISTA_DATA_DIR` moves them.
- **Options in `config.toml`**: render scale (up to 4x), anisotropic
  filtering, the present filter, fullscreen, and the toolkit's CRT and FSR
  shader presets. The game draws as stock until you set one.
- **Widescreen**: `display.aspect = "16:9"` (or 16:10, 21:9) in
  `config.toml` widens the course (Hor+), and the race HUD's position, lap
  times, lap count and course map move out to the screen's edges (the
  `hud` mod, on by default; `display.hud_aspect` and `display.hud_margin`
  say where the edges are). The tachometer and speed stay beside the
  middle. At 4:3 the game draws as stock.
- **An in-game menu**, on by default: F10, Guide+A or Back+L3 held for a
  moment opens it over the game. F3 shows the stats strip.
- **Game defaults that differ from the toolkit**, each for a measured
  reason (`docs/env.md`, `src/main.c`):
  - `RECOMP_VBLANK=paced`: the plain tick ran at about 42 Hz, so the game
    ran slow and the race start crackled. Paced gives 60 Hz.
  - `RECOMP_GUEST_LOCK=1` with the guest scheduler: CRI's ADX sound
    manager keeps its threads apart by priority, which host threads side
    by side break; without them the boot could hang in the ADX lock.

### For developers

- **Pins:** xboxrecomp-cli v0.22.0 and toolkit blinx2-v0.15.0, both
  public: no local `XBOXRECOMP_DIR` is needed. The toolkit brings the
  lifter fix the menus need (narrow mul/imul/div/idiv keep their 8- and
  16-bit registers; without it every menu scrolled on its own).
- **Present shaders come from the toolkit:** blinx2-v0.15.0 stages
  librashader and the `shaders/` presets beside every exe that links the
  enhancements layer, so the game's own `xboxrecomp_present_shaders` call
  is gone.
- **Seeds:** six function starts the analysis missed
  (`config/seed_functions.json`), among them the CRT thread start, the
  x87 exp kernel reached only through `_trandisp`, and the game options'
  type-dispatch thunk at 0x0008E880 (without it, changing the camera mode
  crashed).
