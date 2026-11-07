# Design: Auto Modellista widescreen

Runs and captures are in the workspace's `runs/am-widescreen/` (never
published). Addresses are the US release's (title 0x4343000E).

## D1. Hor+, not native widescreen

The native mode (cat `native-widescreen`) needs a title that renders 16:9
itself when the dashboard says the TV is wide. Auto Modellista does not:

- `XGetVideoFlags` (the thunk sub_00179A1B) has one caller, sub_000F7C60,
  at boot. It ORs the widescreen bit into the present parameters' flags and
  stores 16/9 at 0x4330F4. No instruction reads 0x4330F4.
- The game's projections are built from a 4/3 aspect constant, whatever
  the flag says.
- The toolkit's kernel answers XC_VIDEO (0x03) in the wrong domain, so
  every game sees 0, and `display.widescreen` is not built yet.

A native mode would therefore show the 4:3 picture stretched. Hor+
(`display.aspect`) widens the projection the toolkit sees and gives the
right picture with no game code. Its own placement of 2D (centred, cut to
the 4:3 columns) is what the HUD layer then fixes.

## D2. The 2D library

All the game's 2D goes through one small library. Its twelve entries
(0x10E180..0x10E6F0) share one shape: return 0 while the flag at 0x445114
is set, else save the device's render state and set the 2D one
(sub_0010DFA0, its argument 1 when the entry is textured), call the drawer
with the one argument (a command struct), and put the state back
(sub_0010E0A0). The drawer sets the vertex format (sub_002041C0 with FVF
0x44, XYZRHW and DIFFUSE, or 0x144, with TEX1 too), calls the XDK's
`Begin`, then `SetVertexData2f(9, u, v)` and `SetVertexData4f(-1, x, y, z,
w)` per vertex, then `End`. No vertex array exists at any point: the
vertices go straight into the push buffer.

The command structs, by entry (offsets in bytes; s16 = signed 16-bit):

| Entry | Drawer | Struct | Points' x | u |
|---|---|---|---|---|
| 0x10E180 | 0x10A470 | line: s16 x0 y0 x1 y1, colour | 0, 4 | - |
| 0x10E1E0 | 0x10A5E0 | rectangle: s16 x0 y0 x1 y1, colour | 0, 4 | - |
| 0x10E210 | 0x10A700 | rectangle, four colours | 0, 4 | - |
| 0x10E270 | 0x10AE20 | blit 1:1: s16 x y w h, texture, s16 u v | x 0, w 4 | 0xC (u1 = u + w) |
| 0x10E2A0 | 0x10B120 | blit scaled: s16 x y w h, texture, s16 u0 v0 u1 v1 | x 0, w 4 | 0xC, 0x10 |
| 0x10E2D0 | 0x10B8F0 | triangle: s16 three points, colour | 0, 4, 8 | - |
| 0x10E300 | 0x10B9E0 | triangle, three colours | 0, 4, 8 | - |
| 0x10E4B0 | 0x10CB50 | triangle: f32 xyzw per point, colour | 0, 0x10, 0x20 | - |
| 0x10E540 | 0x10CE10 | textured triangle, uv pairs from 0x34 | 0, 0x10, 0x20 | 0x34, 0x3C, 0x44 |
| 0x10E690 | 0x10C2C0 | rectangle: f32 x0 y0 z w x1 y1, colour | 0, 0x10 | - |
| 0x10E6C0 | 0x10C3B0 | textured rectangle, f32 u0 v0 u1 v1 at 0x20 | 0, 0x10 | 0x20, 0x28 |
| 0x10E6F0 | 0x10C540 | textured rectangle, colour, uv at 0x24 | 0, 0x10 | 0x24, 0x2C |

The coordinates are 640x480 guest pixels, top-left, y down (the drawers
subtract 0.5 for the pixel centre; the struct holds whole pixels).

## D3. The mod builds the vertices

The HUD layer (toolkit `src/enhance/hud_anchor.c`) works on a vertex array
in guest memory. The mod makes one per call:

1. The pre hook on each entry first calls `hud_apply` with no vertices.
   That returns -1 at its first test while the layer is inactive (4:3, the
   mod off, no table), and the pre returns. No guest byte changes then:
   the mod's guest block is not even allocated.
2. Otherwise it takes its guest block (`guest_alloc`, one page, allocated at
   the first active call) and writes a quad of four `{x, y, u, v}` f32
   vertices: two points as p0 p1 p0 p1, three as p0 p1 p2 p2. The bounds
   and the clip are then the struct's own. A blit's right edge is x + w,
   its bottom y + h, and a 1:1 blit's u1 is u + w. The layer moves x and u
   only; y is there for the capture's boxes. The block is in the
   contiguous arena (0x80EFD000 here), which the toolkit backs with its own
   storage, not a view of the title's 64 MB (`xbox_memory_layout.c`): the
   mod uses that address as it is. The capture reads boxes only below the
   64 MB top (`HUD_VA_TOP`), so its level-1 lines from this mod carry the
   call and its chain but no box.
3. `hud_apply` finds the element by the entry's return address (the
   caller's call site) and shifts, clips or stretches the quad.
4. The mod writes each point's new x and u back into the struct (rounded
   for s16), after saving those fields' bytes. The post of the same call
   (matched by esp; the entries never nest) puts back those fields only,
   since some structs live in the game's own data and are drawn again next
   frame. Each pre first drops any save left by a pre whose post never ran,
   and the save carries the guest frame its pre ran in (a count bumped by
   the mod's `on_frame`): a post in another frame puts nothing back, as the
   layer's own save epoch. The Mods page's switch from on to off between a
   pre and its post is safe only because the switch takes effect in
   `xbox_mod_frame`, on the thread the game draws its 2D from, so no entry
   is open then. If the game ever draws 2D from a second thread, revisit
   this.

A lock covers steps 2 to 4's read back, since the block is shared by the
guest threads. During a mapping capture with the mod off the hooks run for
the capture alone (`hud_capturing`): the pre returns at once, so nothing
is allocated or written and those calls do not record. Map with the mod
on (D4).

The game had no `xbox_mod_frame` call, which the mod API needs to count
frames and to hand the mods guest memory. `src/recomp_manual.c` adds it
after the XDK's `D3DDevice_Swap` (sub_002077A0, the same body as KUF's
sub_002C5680).

## D4. The table, from the captures

`./automodellista hud-map` over level-1 captures on the PC (D3D11, 16:9,
the mod on with an empty table; `runs/am-widescreen/cap-race2`, frames
5400..5489 of a race, and `cap-menu2`, 2400..3799, the menus). Those
captures had boxes because the mod then wrote its quad at the block's low
"alias", which is title RAM (0x00EFD000), a bug since fixed (D3). The
sites and kinds they gave were checked again on the fixed build by the
16:9 frames (Results). A new capture has no boxes; the table's rows then
come from the calls and their chains, and from frames.

- **The race HUD** is sub_000230C0. Its 2D comes from a few functions:
  sub_00022DB0 draws the course map (0x00022DEC), the lap label
  (0x00022E8E), 22 static pieces in one loop (0x00022E31, design D5) and,
  through sub_00022CE0, the cars on the map (0x00022D76). The lap times'
  digits are sub_00022B20, called six times from sub_00022DB0 (0x00022E61)
  and from the result screens (0x0007C121 and five more). The table makes
  it a `[[wrapper]]`, so the race's call names its pieces (and those of
  sub_00022A60 it calls) and the other callers stay unlisted.
- **Groups**: `left` (the position, the lap time labels and digits) and
  `right` (the lap count, the map and its dots).
- **The menus** all draw through one sprite list (sub_00124000, 0x00124100,
  26945 of 27203 calls), plus a full-canvas backdrop (0x00047F6E) and a
  cursor (0x000DDF53). They are screen-wide compositions, so they stay
  unlisted: centred in the 4:3 columns as Hor+ draws them, the backdrop
  widened by the toolkit's full-width rule. That is how they look at 4:3.

A level-0 capture on the XDK's `Begin` (0x205D40) showed every immediate
draw coming from the library's drawers, so no 2D is missed. One on
`DrawVertices` (0x205520) and `DrawIndexedVertices` (0x205870) found only
the mesh renderer (0x121EC0, 0x122540), the loading screen (0x104A30) and
no 2D. Those three hook slots were for the mapping only and are not kept.

## D5. One call, both sides

sub_00022DB0's loop (0x00022E10..0x00022E3B) draws 22 blits from a static
array at 0x33C950 through the one call at 0x00022E31: the position ("8th
/ 8") and the three time labels on the left, the lap count ("1 / 3") on
the right. The layer names an element by its call, so one kind would send
half of them the wrong way, and `proportional` stops short of the edge.

The mod handles it: `k_sides` in `src/hud_mod.c` lists such calls, the
table gives them a left group, and a quad whose middle is right of the
canvas's goes to the layer mirrored about the middle (x to 640 - x). The
layer then moves and clips it as a left piece, and the mod mirrors the
result back, which puts it at the right edge by the same pad. u stays with
its point, so a clip still cuts the right texels. A piece whose centre is
exactly the middle stays a left piece, as the group says (none of the
loop's pieces sits there). The test reads p0 and p1 only: the listed
calls are blits, two points.

## D6. The gauge cluster stays

The tachometer, its needle, the gear and AT are not 2D: sub_000230C0 sets
an identity view (sub_00105930, sub_001040B0 with 0x1A) and draws a model
in camera space (sub_0001C050, the game's object drawer), through
`DrawIndexedVertices`. Hor+ keeps a camera-space object's offset from the
middle, so at 16:9 the cluster sits where it does at 4:3 relative to the
centre, whole. Moving it would need a clip-space x offset on that model's
draws, which neither the HUD layer nor the mod API has; a mod-side shift of
the object in view space would need the projection's focal length, which
the game may change with the camera.

So the cluster stays, and its speed digits (sub_000224C0, 0x00022518),
which sit on the dial, stay unlisted with it. A toolkit follow-up (a
per-draw clip-space offset for camera-space HUD models) would let the
table move it; it is recorded in TASKS.

## Results (2026-11-04, the PC, native Windows, D3D11)

Runs of 130 s with `runs/am-release/race.txt` (the menus, then a race from
about guest frame 4800), `d3d11_dump` every 60th present; the exes built
on the PC against the local toolkit 09c6283f (posix-host/portability).
Frames and pictures are in `runs/am-widescreen/` (`v8-169-on`, `stock/`,
`captures/`). The v8 runs are this branch's exe after
the review's fixes (D3, sha256 e9f8cd97); the v7 runs and earlier captures were the build
that wrote its quad into title RAM, and v8 repeats them.

- **16:9, the mod on**: the position, the lap time labels and digits sit
  at the left edge; the lap count, the course map and its dots at the
  right one (the mirrored pieces of D5 included). The gauge cluster stays
  whole beside the middle (D6). The menus render as at 4:3, centred, with
  the backdrop across the frame.
- **Stock**: main's build (5cb9949's game code; da22090 changes only
  packaging and pins) against this branch's, at 4:3 with the mod on
  (13 of 26 frames differ) and at 16:9 with `RECOMP_HUD=off` (12 of 26):
  the boot frames are identical byte for byte, and the race frames differ
  only by timing (the lap clock a few frames apart, the course flyover's
  camera). One 4:3 menu frame (40) differs too, the main menu's animated
  backdrop at another phase, the menu itself the same. Two runs of main's
  build at 4:3 differ in 12 of 26 frames the same way, so that is the
  runs' own noise. The HUD's place in each frame is the same as main's.
- **Metal** (2026-11-05, the Mac, one build of 839b0a1, the same race
  script, `SDL_AUDIODRIVER=dummy`; `runs/am-widescreen/mac/`): at 16:9
  with the mod on, the race HUD sits at the edges as on D3D11 (the left
  column, the lap count and map at the right; the gauge cluster beside
  the middle, D6). At 4:3 with the mod off the race renders as stock,
  the HUD where main's D3D11 frames have it. The game has no golden
  scenarios yet and the Mac had no build of main, so this is a look,
  not a byte match against main; the byte match is D3D11's (above), and
  the mod's code is the same on both backends.
