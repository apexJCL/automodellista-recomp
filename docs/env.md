# Environment variables

The runtime reads every variable through one table, `RECOMP_ENV_KEYS` in the
toolkit's `src/platform/recomp_env.h`, once at startup; the toolkit's own keys
are documented there (`RECOMP_TRACE=help` or `RECOMP_DEBUG=help` prints the
whole table). This page lists what this game adds in
`src/env/recomp_env_game.h`, which the toolkit table pulls in when the
`recomp_env` target defines `RECOMP_ENV_HAVE_GAME_KEYS` (CMakeLists.txt sets
it), and the toolkit keys whose meaning here depends on this game's HUD
table.

## Toolkit defaults this game changes

`RECOMP_ENV_GAME_DEFAULTS` in `src/env/recomp_env_game.h` sets these toolkit
keys when neither the variable nor its old spelling is set; setting the
variable still wins (`=0` is the A/B). `RECOMP_VBLANK` is the exception: as in
BLiNX 1 and KUF, `src/main.c` sets it before the runtime starts, and an
explicit value still wins.

| Name | Default here | Why |
|---|---|---|
| `RECOMP_GUEST_LOCK` | `1` | CRI's ADX manager keeps its threads apart by thread priority and SuspendThread alone, which host threads running side by side break: the boot froze in the ADX lock, or the mwidle wake gave up ("1060102 adxm_goto_mwidle_border") and the game halted. One guest thread at a time is the console's one CPU. Alone it still hung 3 of 6 headless Mac runs |
| `RECOMP_DEBUG=guest_sched` | `1` | with the lock, the guest CPU follows the title's thread priorities and counts another thread's suspend: 6 of 6 headless Mac runs passed. A Deck (Proton) run of the PC-built exe reached the race with no ADX error; the native PC D3D11 run is a task in `openspec/changes/bootstrap/tasks.md` |
| `RECOMP_VBLANK` | `paced` | the 60 Hz vblank schedule. Since toolkit v0.16 `1` is the plain tick, which ran at about 42 Hz on the Mac and slowed play and the race-start audio; `=0` turns the vblank thread off |

## Config

| Name | Meaning | Read at |
|---|---|---|
| `RECOMP_HUD` | `mods."hud.enable"`: `off` or `on` (default, `packaging/config.default.toml`); enhancements layer only. `on` turns on the `hud` mod (`src/hud_mod.c`, openspec change am-widescreen): on a wide `display.aspect` the race HUD moves to the frame's edges as `config/hud.toml` says. At 4:3 the mod inits and its hooks run but change nothing, its guest block included. Set by env, the setting is read-only on the Mods page | `src/hud_mod.c` |

## Toolkit keys the HUD mod reads

These rows are the toolkit's (`src/platform/recomp_env.h`); what they do
here depends on `config/hud.toml`.

| Name | Tier | Meaning here |
|---|---|---|
| `RECOMP_DISPLAY_ASPECT` | config | `display.aspect`: `4:3` (stock), `16:9`, `16:10`, `21:9`. Hor+ widens the course; anything wider than 4:3 activates the `hud` mod's table |
| `RECOMP_DISPLAY_HUD_ASPECT` | config | `display.hud_aspect`: `display` (default, the frame's edges) or a `W:H` from 4:3 up to the display aspect, where the left and right elements stop |
| `RECOMP_DISPLAY_HUD_MARGIN` | config | `display.hud_margin`: guest pixels, 0 (default) to 64, the edge elements move back in |
| `hud` | trace | `[hud.site]` per listed site, `[hud.unlisted]` for 2D draws past the 4:3 columns that no row names, `[hud.skew]` and `[hud.skip]` for calls the layer refused |
| `hud_capture`, `hud_capture_at` | debug | the mapping capture behind `./automodellista hud-map`: `hud_capture=DIR,hud_capture_at=FROM:COUNT`. The game's 2D has no vertex array, so its calls record through the mod's hooks (level 1), with the mod on: the call and its chain, no box (the mod's block is above the 64 MB the capture reads). With the mod off the hooks do nothing and those calls do not record |
