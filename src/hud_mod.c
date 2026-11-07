/*
 * hud_mod.c - the hud mod (hud_mod.h; openspec change am-widescreen).
 *
 * The game draws all its 2D (the race HUD, the menus, the fades) through
 * one small library at 0x10A300..0x10E720: each entry takes one command
 * struct (a line, a rectangle, a triangle or a textured quad, in 640x480
 * guest pixels with short or float coordinates) and draws it in immediate
 * mode, Begin, SetVertexData and End, with an XYZRHW vertex format (design
 * D2). No vertex array ever exists, so the pre hook on each entry builds
 * one: it copies the struct's x and u into a quad in the mod's guest block,
 * hands that to the toolkit's HUD layer, which finds the element by its
 * call site in config/hud.toml (embedded at build time) and moves, clips or
 * stretches it for the wide frame, and writes the result back into the
 * struct. The post puts the fields it wrote back, since the game keeps
 * some of the structs and draws them again (design D3).
 *
 * The layer decides everything: while it is not active (4:3, the mod off,
 * no table) the pre's first hud_apply returns at its first test and no
 * byte of guest memory changes, the mod's block included.
 */
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "hud_mod.h"
#if defined(RECOMP_ENV_HAVE_ENHANCE_KEYS) && defined(__has_include)
#  if __has_include("mod.h")
#    include "mod.h"
#    if XBOX_MOD_API_VERSION >= 6
#      define HUD_MOD 1
#    endif
#  endif
#endif

#ifdef HUD_MOD
#include "enhance_menu.h"
#include "hud_table.h"          /* config/hud.toml, from CMake */

/* One library entry: where each point's x and u live in its struct. Every
 * entry is cdecl with the struct's address as its one argument and draws
 * it before it returns (design D2). Rectangles name two corners, so their
 * x are two points; the blits name x and a width instead (xw). */
struct am_draw {
    uint32_t va;
    uint8_t f32;        /* float coordinates, else signed 16-bit */
    uint8_t xw;         /* x[0] is x and x[1] the width (s16 blits) */
    uint8_t n;          /* points: 2 (line, rectangle, blit) or 3 */
    uint8_t x[3];       /* byte offset of each point's x (xw: x, width) */
    int8_t u[3];        /* of each point's u, -1 none; xw: u0, u1 (-1: u0 + width) */
};

#define NO -1
static const struct am_draw k_draws[] = {
    /* s16 x0 y0 x1 y1, colour: a line, a rectangle, a gradient */
    { 0x0010E180u, 0, 0, 2, { 0x00, 0x04 }, { NO, NO, NO } },
    { 0x0010E1E0u, 0, 0, 2, { 0x00, 0x04 }, { NO, NO, NO } },
    { 0x0010E210u, 0, 0, 2, { 0x00, 0x04 }, { NO, NO, NO } },
    /* s16 x y w h, texture, u v in texels: 1:1, then scaled to u1 v1 */
    { 0x0010E270u, 0, 1, 2, { 0x00, 0x04 }, { 0x0c, NO, NO } },
    { 0x0010E2A0u, 0, 1, 2, { 0x00, 0x04 }, { 0x0c, 0x10, NO } },
    /* s16 three points, colour or three colours */
    { 0x0010E2D0u, 0, 0, 3, { 0x00, 0x04, 0x08 }, { NO, NO, NO } },
    { 0x0010E300u, 0, 0, 3, { 0x00, 0x04, 0x08 }, { NO, NO, NO } },
    /* f32 xyzw per point: a triangle, a textured one (uv pairs at 0x34) */
    { 0x0010E4B0u, 1, 0, 3, { 0x00, 0x10, 0x20 }, { NO, NO, NO } },
    { 0x0010E540u, 1, 0, 3, { 0x00, 0x10, 0x20 }, { 0x34, 0x3c, 0x44 } },
    /* f32 x0 y0 z w x1 y1: a rectangle, textured, textured with a colour */
    { 0x0010E690u, 1, 0, 2, { 0x00, 0x10 }, { NO, NO, NO } },
    { 0x0010E6C0u, 1, 0, 2, { 0x00, 0x10 }, { 0x20, 0x28, NO } },
    { 0x0010E6F0u, 1, 0, 2, { 0x00, 0x10 }, { 0x24, 0x2c, NO } },
};
#undef NO
#define N_DRAWS (sizeof k_draws / sizeof k_draws[0])

/* The quad the layer reads ([layout] in config/hud.toml): {x, y, u, v}
 * f32, four vertices. Two points go in as p0 p1 p0 p1 and three as p0 p1
 * p2 p2, so the bounds and the clip are the struct's own. */
#define Q_STRIDE 16u
#define Q_VERTS  4u
static const uint8_t k_order[2][Q_VERTS] = { { 0, 1, 0, 1 }, { 0, 1, 2, 2 } };

/* Calls that draw on both sides of the screen: the race HUD's label loop
 * (sub_00022DB0) draws its 22 static pieces through one call, the position
 * and the lap times' labels on the left and the lap count on the right
 * (design D5). The table names such a call as "left", and a quad right of
 * the middle goes to the layer mirrored about it, so the layer moves and
 * clips it as a left piece, and comes back mirrored again: at the right
 * edge. By the call's return address. */
static const uint32_t k_sides[] = { 0x00022E31u };
#define CANVAS_W 640.0f

enum { SET_ENABLE, SET_N };

static const char *const k_offon[] = { "off", "on", NULL };

/* On by default, as BLiNX 2's and KUF's: at 4:3 the layer is inactive and
 * the game runs as stock, and on a wide screen a HUD left in the 4:3
 * middle is what nobody wants (the user, 2026-10-18). The key is
 * mods."hud.enable"; display.hud_aspect and display.hud_margin, which say
 * where the edges are, belong to the toolkit. */
static const struct xbox_mod_setting k_set[SET_N] = {
    /* name, key, env, aliases, label, help, unit, kind, live, flags,
     * def, lo, hi, step, choices */
    [SET_ENABLE] = { "enable", NULL, "RECOMP_HUD", NULL, "HUD at the screen edges",
      "On a wide screen the race HUD moves out to the edges, as the game"
      " had it on a 4:3 TV. Does nothing at 4:3.",
      NULL, XBOX_SET_ENUM, ENHANCE_LIVE, XBOX_SET_ENABLE, 1, 0, 0, 0, k_offon },
};

static const struct xbox_mod_desc k_desc = {
    XBOX_MOD_DESC_VERSION, sizeof k_desc, "HUD at the screen edges",
    "Keeps the HUD at the edges of a wide screen", NULL, "1",
    k_set, SET_N, NULL, 0
};

static const struct xbox_mod_api *s_api;
static const struct xbox_mod s_mod;

/* The quad lives in the mod's guest block, which two guest threads could
 * fill at once; the lock holds only from the fill to the read back. */
static uint32_t s_quad;
static char s_lock;

/* The fields the pre wrote (each point's x, or a blit's x and width, and
 * each u), as the caller left them, put back by the post of the same call
 * (matched by esp and frame; the entries never nest nor span a frame).
 * Only those: the drawer may have no business with the rest, but the game
 * owns it. */
#define SAVE_FIELDS 6
/* A row has at most three points, each an x and a u. */
_Static_assert(SAVE_FIELDS >= sizeof k_draws[0].x + sizeof k_draws[0].u,
               "a draw row can write more fields than the save holds");
static _Thread_local struct {
    uint32_t esp, va, frame;
    unsigned n;
    struct { uint8_t off, len; uint8_t b[4]; } f[SAVE_FIELDS];
} t_save;

/* Bumped once a guest frame (on_frame). A save from another frame is a
 * call that returned without its post: never put back, as the layer's
 * own save epoch. */
static uint32_t s_frame;

static void count_frame(void *user)
{
    (void)user;
    __atomic_add_fetch(&s_frame, 1u, __ATOMIC_RELAXED);
}

static uint8_t *g(uint32_t va);

static void save_field(uint32_t st, unsigned off, unsigned len)
{
    unsigned k = t_save.n;

    if (k >= SAVE_FIELDS)
        return;
    t_save.n = k + 1u;
    t_save.f[k].off = (uint8_t)off;
    t_save.f[k].len = (uint8_t)len;
    memcpy(t_save.f[k].b, g(st + off), len);
}

static uint8_t *g(uint32_t va)
{
    return (uint8_t *)(s_api->mem + va);
}

static float rd_x(const struct am_draw *e, uint32_t st, unsigned off)
{
    if (e->f32) {
        float f;
        memcpy(&f, g(st + off), 4);
        return f;
    }
    int16_t s;
    memcpy(&s, g(st + off), 2);
    return (float)s;
}

static void wr_x(const struct am_draw *e, uint32_t st, unsigned off, float v)
{
    if (e->f32) {
        memcpy(g(st + off), &v, 4);
        return;
    }
    long r = lrintf(v);
    int16_t s = (int16_t)(r < INT16_MIN ? INT16_MIN : r > INT16_MAX ? INT16_MAX : r);
    memcpy(g(st + off), &s, 2);
}

static void pre_draw(struct xbox_mod_call *c, void *user)
{
    const struct am_draw *e = user;
    struct xbox_hud_call d = { 0, 0, 0, 0, 0 };
    float px[3], py[3], pu[3];
    uint32_t st, ret;
    unsigned i, len;
    int kind, mirror = 0;

    /* A save left by a pre whose post never ran (the guest unwound past
     * it) must not be put back by a later post at the same esp. */
    t_save.n = 0;
    /* With the mod off, a mapping capture alone calls the hooks: the mod
     * then does nothing, so the run stays as without it (no block, no
     * write). Map with the mod on (design D4). */
    if (s_api->hud_capturing(&s_mod))
        return;
    /* The cheap test first: with n = 0 the layer answers whether it would
     * act (the mod on, the table, a wide aspect) and records nothing. */
    if (s_api->hud_apply(c, &d) < 0)
        return;
    st = XBOX_MOD_ARG32(s_api, c, 0);
    if (!st)
        return;
    if (!s_quad) {
        /* The block is in the contiguous arena (0x8xxxxxxx), its own
         * storage, not a view of the title's 64 MB: the layer reads it
         * there. A capture reads boxes only below the 64 MB top, so its
         * level-1 lines from this mod carry no box (design D3). */
        uint32_t va = s_api->guest_alloc(&s_mod, 4096u, 4096u);
        if (!va)
            return;
        s_quad = va;
    }

    for (i = 0; i < e->n; i++) {
        px[i] = rd_x(e, st, e->x[i]);
        py[i] = rd_x(e, st, e->x[i] + (e->f32 ? 4u : 2u));
        pu[i] = e->u[i] >= 0 ? rd_x(e, st, (unsigned)e->u[i]) : 0.0f;
    }
    if (e->xw) {
        px[1] += px[0];                         /* the right edge */
        py[1] += py[0];                         /* the bottom one */
        pu[1] = e->u[1] >= 0 ? pu[1] : pu[0] + px[1] - px[0];
    }

    /* Right of the middle: the piece's centre past 320. One centred on it
     * exactly stays a left piece, as the table's group says; none of the
     * loop's pieces sits there. Only p0 and p1 are tested: the listed
     * calls are blits, two points. */
    memcpy(&ret, g(c->esp), 4);
    for (i = 0; i < sizeof k_sides / sizeof k_sides[0]; i++)
        if (ret == k_sides[i])
            mirror = px[0] + px[1] > CANVAS_W;
    if (mirror)
        for (i = 0; i < e->n; i++)
            px[i] = CANVAS_W - px[i];

    while (__atomic_test_and_set(&s_lock, __ATOMIC_ACQUIRE))
        ;
    for (i = 0; i < Q_VERTS; i++) {
        unsigned p = k_order[e->n - 2u][i];
        float v[4] = { px[p], py[p], pu[p], 0.0f };
        memcpy(g(s_quad + i * Q_STRIDE), v, sizeof v);
    }
    d.vtx_va = s_quad;
    d.n = Q_VERTS;
    kind = s_api->hud_apply(c, &d);
    if (kind >= 0) {
        /* Each point's first vertex holds its result. */
        for (i = 0; i < e->n; i++) {
            memcpy(&px[i], g(s_quad + i * Q_STRIDE), 4);
            memcpy(&pu[i], g(s_quad + i * Q_STRIDE + 8u), 4);
        }
    }
    __atomic_clear(&s_lock, __ATOMIC_RELEASE);
    if (kind < 0)
        return;
    if (mirror)
        for (i = 0; i < e->n; i++)
            px[i] = CANVAS_W - px[i];

    t_save.esp = c->esp;
    t_save.va = st;
    t_save.frame = __atomic_load_n(&s_frame, __ATOMIC_RELAXED);
    len = e->f32 ? 4u : 2u;
    for (i = 0; i < e->n; i++) {
        save_field(st, e->x[i], len);
        if (e->u[i] >= 0)
            save_field(st, (unsigned)e->u[i], len);
    }
    if (e->xw) {
        wr_x(e, st, e->x[0], px[0]);
        wr_x(e, st, e->x[1], px[1] - px[0]);
    } else {
        for (i = 0; i < e->n; i++)
            wr_x(e, st, e->x[i], px[i]);
    }
    for (i = 0; i < e->n; i++)
        if (e->u[i] >= 0)
            wr_x(e, st, (unsigned)e->u[i], pu[i]);
}

static void post_draw(struct xbox_mod_call *c, void *user)
{
    unsigned k;

    (void)user;
    if (!t_save.n || t_save.esp != c->esp
        || t_save.frame != __atomic_load_n(&s_frame, __ATOMIC_RELAXED)) {
        t_save.n = 0;
        return;
    }
    for (k = 0; k < t_save.n; k++)
        memcpy(g(t_save.va + t_save.f[k].off), t_save.f[k].b, t_save.f[k].len);
    t_save.n = 0;
}

/* The Mods page's switch. The layer and this mod's hooks take the owner's
 * state at the next frame (the toolkit's hud_frame), and no entry spans a
 * frame, so every struct is back by then: there is nothing left for the
 * mod to do. Registering it is what makes the switch live. */
static void on_enable(int on, void *user)
{
    (void)on;
    (void)user;
}

static int init(const struct xbox_mod_api *api)
{
    unsigned i;

    if (api->size < offsetof(struct xbox_mod_api, guest_alloc) + sizeof api->guest_alloc)
        return 1;   /* no v6 guest block: the HUD stays where Hor+ puts it */
    s_api = api;
    if (api->hud_table((const char *)k_hud_table, sizeof k_hud_table) < 0)
        return 1;   /* the layer said why */
    if (!api->on_frame(count_frame, NULL))
        return 1;   /* the frame table is full */
    for (i = 0; i < N_DRAWS; i++)
        if (!api->hook(k_draws[i].va, pre_draw, post_draw, 0, (void *)&k_draws[i]))
            return 1;
    api->on_enable(on_enable, NULL);
    return 0;
}

/* needs: the entries' hooks and the wide aspect the layer reads. */
static const struct xbox_mod s_mod = {
    "hud", init, NULL, &k_desc, XBOX_MOD_NEED_HOOK | XBOX_MOD_NEED_ASPECT, NULL
};

const void *hud_mod(void)
{
    return &s_mod;
}
#else /* no mod API v6: stock */
const void *hud_mod(void)
{
    return NULL;
}
#endif
