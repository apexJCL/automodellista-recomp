# Design notes

- **Game:** Auto Modellista (Capcom), title 0x4343000E, XDK 5558 (D3D8,
  D3DX8, XGRAPHC, DSOUND, XONLINES, XVOICE). default.xbe is 4.2 MB with 20
  sections, including CRI Sofdec/ADX (PSGSFD*), DOLBY and XPP. Its 183
  kernel imports are all bridged. Analysis finds 16,965 functions with no
  translation failures, 47 unresolved call stubs and 37 flag-fallback sites.
- **Fence:** `xbr new` finds the D3D8 1.0.5558 fence line by itself (device
  0x0021DBC8, fence wait 0x00209540). No override is needed.
- **Seeds:** five indirect-call targets the analysis did not reach, all
  seeds with `"observed": true`. 0x001F0E2D is the thread start routine that
  the CRT's `_beginthreadex` hands to PsCreateSystemThreadEx. 0x001F7E8B is
  the worker threads' callback, whose address is taken at 0x001F4739.
  0x0003DAF0 and 0x000E02A0 are byte switches on `[arg+0x34]` through a jump
  table, called while the first race loads; without them a wild write
  crashed the load. 0x001F2395 is the CRT's x87 `exp` kernel, reached only
  through the `_trandisp` table entry for "exp" at 0x003FFCB2, called in the
  race.
- **Guest CPU defaults:** `RECOMP_GUEST_LOCK=1` and `guest_sched=1` are this
  game's defaults (`src/env/recomp_env_game.h`, `docs/env.md`). CRI's ADX
  manager keeps its threads apart by priority and SuspendThread alone, and
  with host threads side by side its mwidle wake gave up
  ("1060102 adxm_goto_mwidle_border"). The lock alone hung 3 of 6 headless
  Mac runs; the lock with guest_sched passed 6 of 6.
- **Runs:** headless Metal runs with
  `RECOMP_DEBUG=watchdog=N,fb_dump=...,pad_script=@file` and
  `SDL_AUDIODRIVER=dummy`. The intros are timed by the wall clock (the
  Sofdec movies) but the pad script by flips, so a script that works on an
  idle Mac lands its presses elsewhere when the Mac is loaded. The
  `@open:<name>` anchors are the way to make scripts replay.
- **Xbox Live entry:** on the main menu, "Xbox Live" launches
  `d:\dashupdate.xbe` (launch data type 1, value 7), and the title exits
  through HalReturnToFirmware(2). This is the stock dashboard update path, not
  a bug. A later change hides the entry or answers it in the game.
- **Course select (answered):** the cursor moving on its own was a lifter
  bug, not input. The toolkit lifted one-operand 8- and 16-bit
  `mul`/`imul`/`div`/`idiv` as the 32-bit form, which writes EDX. The menu
  auto-repeat `sub_00109670` loads DL with the repeat delay, runs `imul bl`
  (0x1096A5) and reads DL back; with EDX overwritten the delay was 0 and
  every button not held repeated on every frame (repeat mask 0x8D29A8 read
  0xFFFFFF). The toolkit fix is branch `fix/lifter-narrow-muldiv`; with it
  the menus hold still and a scripted run confirms course, car and settings
  and reaches the race. This game needs a toolkit with that fix; the
  `game.toml` pin moves at the next round.
