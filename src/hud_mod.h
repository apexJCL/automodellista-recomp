/*
 * hud_mod.h - the hud mod: Auto Modellista's 2D at the wide frame's edges
 * under Hor+ (openspec change am-widescreen).
 *
 * Without the layer, or with a toolkit before mod API v6, hud_mod is NULL
 * and no draw is ever touched.
 */
#ifndef AM_HUD_MOD_H
#define AM_HUD_MOD_H

/* The table entry main.c registers before xbox_enhance_init; NULL when
 * the mod API v6 is not built. */
const void *hud_mod(void);

#endif
