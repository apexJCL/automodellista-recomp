/*
 * recomp_env_game.h - this game's own environment rows, added to the
 * toolkit's table (xboxrecomp src/platform/recomp_env.h, which includes this
 * file when RECOMP_ENV_HAVE_GAME_KEYS is defined; CMakeLists.txt sets it on
 * the recomp_env target). Inventory with meanings: docs/env.md.
 */
#ifndef RECOMP_ENV_GAME_H
#define RECOMP_ENV_GAME_H

/* Toolkit keys this game changes the default of; an explicit variable still
 * wins (recomp_env.h, RECOMP_ENV_GAME_DEFAULTS; =0 is the A/B).
 *
 * GUEST_LOCK and GUEST_SCHED: one guest thread runs guest code at a time, as
 * on the console's one CPU, and that CPU goes by the title's thread
 * priorities. CRI's ADX manager (adxm_*, 0x1AF4F0..0x1AFE10; its lock is
 * sub_001AF4F0, the counterpart of JSRF sub_0013B0A0 and BLiNX 2
 * sub_002ACBA0) keeps its threads apart by priority and SuspendThread alone:
 * the lock raises the caller's priority and resumes a busy thread, the
 * unlock suspends that thread from outside, and sub_001AF590 wakes the
 * "mwidle" thread by raising its priority and resuming it until that thread
 * clears 0x795840. On several host cores the threads run side by side and a
 * suspend from another thread stops nothing (POSIX), so the handshake raced:
 * the boot froze in the lock loop, or after Start at the title the wake gave
 * up after 3,000,000 tries ("1060102: Internal Error:
 * adxm_goto_mwidle_border") and the title's error callback halted the game.
 * The lock alone, FIFO and blind to priority, still hung 3 of 6 headless
 * runs; with guest_sched the guest CPU honours the priority and counts the
 * foreign suspend, and 6 of 6 passed. */
/* The hud mod's setting names its env row, and the mod API refuses a
 * setting whose row is missing (src/hud_mod.c). */
#define RECOMP_ENV_GAME_KEYS(X) \
    X(HUD, CONFIG, "RECOMP_HUD", NULL, "enhancements: mods.hud.enable, off|on (the HUD at the edges of a wide display.aspect; on by default)")

#define RECOMP_ENV_GAME_DEFAULTS(D) \
    D(GUEST_LOCK, "1") \
    D(GUEST_SCHED, "1")

#endif
