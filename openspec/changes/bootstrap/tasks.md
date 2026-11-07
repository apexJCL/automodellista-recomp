# Tasks

- [x] `xbr new` from default.xbe; `setup`, `analyze`, `recomp`, `build macos`
- [x] Seeds for the two thread entries the first run could not dispatch
  (0x001F0E2D, 0x001F7E8B)
- [x] Seeds for the race: the two switch helpers of the first race load
  (0x0003DAF0, 0x000E02A0) and the CRT `exp` kernel (0x001F2395)
- [x] Scripted headless Metal run: ESRB notice, Sofdec/ADX logo, intro FMV,
  title, PRESS START
- [x] Scripted run through the main menu, arcade, single race, course select
- [x] Course select: confirm a course (see design, "Course select"; the
  phantom scroll was the lifter's narrow mul/div)
- [x] Car select, then the first race frame; the car drives under a held
  throttle (headless Mac, 330 s, no crash, no ICALL failure)
- [x] Needs a toolkit at or past the lifter fix (`fix/lifter-narrow-muldiv`):
  `game.toml` pins the public blinx2-v0.14.0 (push round 20)
- [x] ADX: `RECOMP_GUEST_LOCK=1` and `guest_sched=1` as game defaults in
  `src/env/recomp_env_game.h` (6 of 6 headless Mac runs)
- [ ] guest_sched on the PC: a D3D11 run through the intro, the menus and a
  race, with no ADX hang or "adxm_goto_mwidle_border"
- [x] guest_sched on the Deck (Proton), later: the same run (the
  PC-built exe, 240 s from boot through the save prompt, car select and
  loading to a race on the grid and driving, 57 flips/s, no crash, no
  ADX error, 0 present mismatches)
- [ ] Hands-on session on the Mac with the user: menus, textures, FMV and
  ADX sound
- [x] `XBOXRECOMP_ENHANCE=ON` in `stock_cmake`, per workspace policy
- [ ] Windows build on the PC and a D3D11 run (the native build passes in
  build-all since the round-20 pins; the native D3D11 run is still to do)
- [ ] Golden scenarios once a race starts
