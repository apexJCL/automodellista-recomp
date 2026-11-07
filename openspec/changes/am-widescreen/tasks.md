# Tasks: Auto Modellista widescreen

## 1. Hooks and frame glue
- [x] 1.1 `game.toml` `[mods]`: the twelve 2D entries and the wrapper
  0x22B20, with `no_st0`; `analyze` writes `config/hook_functions.json`;
  `recomp`. (The level-0 slots on `Begin`, `DrawVertices` and
  `DrawIndexedVertices` served the mapping and were dropped, design D4.)
- [x] 1.2 `src/recomp_manual.c`: `xbox_mod_frame()` after `D3DDevice_Swap`
  (sub_002077A0).

## 2. The hud mod
- [x] 2.1 `src/hud_mod.c`: the draw table (design D2), the quad, the
  write-back and the restore (design D3); registered in `src/main.c`.
- [x] 2.2 `RECOMP_HUD` in `src/env/recomp_env_game.h`, `docs/env.md`.
- [x] 2.3 `packaging/config.default.toml`: `display.hud_aspect`,
  `display.hud_margin`, `mods."hud.enable" = "on"`.
- [x] 2.4 `CMakeLists.txt`: the source and the embedded table.

## 3. The table
- [x] 3.1 Level-1 capture on the PC (D3D11) over the menus and a race.
- [x] 3.2 `hud-map propose`, review, then the sites and groups in
  `config/hud.toml` (design D4); the two-sided call (design D5).
- [x] 3.3 `scripts/test_hud_table.py`.

## 4. Checks
- [x] 4.1 16:9 on the PC: the race HUD at the edges, the menus whole.
- [x] 4.2 4:3 and 16:9 with the mod off against main's build: the same
  frames at 4:3 (the stock check).
- [x] 4.3 Metal: one Mac build (839b0a1), 4:3 with the mod off and 16:9
  with the mod on (design, Results). The game has no golden yet and the
  Mac had no build of main, so the 4:3 check is a look, not a byte match.
- [x] 4.4 Gates: the Windows build, `uv run pytest scripts`, ruff.

## 5. Follow-ups
- [ ] 5.1 The gauge cluster at the right edge (design D6): a toolkit
  clip-space x offset for a camera-space model's draws, then a table row
  for sub_000230C0's model and the speed digits (0x00022518).
