# Auto Modellista

*Auto Modellista* is an Xbox racing game. This project turns your own
copy of the Xbox disc into a game that runs on Windows, a Mac or a Steam
Deck. No emulator, and no disc in a drive once it is built.

![A red Viper GTS down the main straight at Suzuka, 16:9 at render scale 3](docs/images/hero.png)

<!-- xbr:readme:download -->
> [!TIP]
> **Get the builder:** download the wizard for your computer from the
> [latest release](https://github.com/apexJCL/automodellista-recomp/releases/latest),
> point it at your own disc image, and it builds and installs the game for you.
>
> | Your computer | Download |
> |---|---|
> | Windows 10 or 11 (x86-64) | `automodellista-wizard-<version>-windows-x64.zip` |
> | Mac (Apple silicon) | `automodellista-wizard-<version>-macos.zip` |
> | Steam Deck or Linux PC | `automodellista-wizard-<version>-linux-x64.tar.gz` |
<!-- /xbr:readme:download -->

**No game is included.** You need your own copy of Auto Modellista; everything the
builder makes from it stays on your machines.

<!-- xbr:readme:release -->
> [!IMPORTANT]
> **Supported game version.** This port is built for one release of
> Auto Modellista only:
>
> | | |
> |---|---|
> | Title | auto modellista |
> | Title ID | `0x4343000E` |
> | Region | region-free |
> | Version | 2 |
> | `default.xbe` sha256 | `581b44ace4726240610ab506147fc4277f9f07ab478f314efb047fb3b5a59378` |
>
> The wizard checks your copy on its Dump page before it builds; by hand,
> `./automodellista fingerprint /path/to/your/disc.iso` says the same. Another
> region or version is **not supported**: it warns "another release", and
> the game may crash or misbehave. A modded copy warns too.
<!-- /xbr:readme:release -->

## How the builder works

<!-- xbr:readme:wizard -->
- Checks your computer and fetches the build tools it needs, each download
  checked against a known checksum. It sends nothing of yours.
- Checks your disc image or game folder, and says before building when it
  is another release or a modded copy.
- Builds an installer for this computer, or for another one: a Windows setup
  program, a Mac app (`.dmg`) or a Steam Deck bundle that adds the game to
  Steam.
- Lets you pick the basic options before building:
  - Render scale
  - Start fullscreen
- Makes small updates for a device that already has a build, and reuses
  most of the work when you build again.
<!-- /xbr:readme:wizard -->

Step by step, with the SmartScreen and Gatekeeper prompts, and the by-hand
route: [docs/playing.md](docs/playing.md#build-the-game).

## Status

**From the title screen to the race.** The menus, the course, car and
tuning screens, and a race on the grid run with sound, at 4:3 or
widescreen. A full season is not checked yet.

| You play on | Graphics | State |
|---|---|---|
| Windows 10 or 11 | Direct3D 11 | Builds; not yet run on native Windows. |
| Steam Deck, Linux (Proton) | Direct3D 11 (DXVK) | Plays from the title to a race. |
| Mac (Apple silicon) | Metal | Plays from the title to a race, built from source; the Mac app is not tried by hand yet. |

Top known issues (all of them: [docs/playing.md](docs/playing.md#known-issues)):

- Native Windows is not checked yet; the Steam Deck is.
- On a wide screen the tachometer and the speed stay beside the middle
  of the picture; the rest of the race HUD moves to the edges.
- Play past the first races is not checked yet: expect crashes, and
  report them.

## Showcase

![The Viper on the grid at Suzuka, smoke from its rear tyres, the race clock already running](docs/images/still-grid.png)

![Second gear off the line, past the BATTLAX boards](docs/images/still-launch.png)

![192 km/h toward the first corner, speed lines across the sky](docs/images/still-speed.png)

### The in-game menu

F10 on the keyboard (Guide+A, or Back+L3 held on a pad) opens a menu over
the game while it keeps running. Its pages: Render (scale, aspect,
filtering), Shaders, Mods, Stats, Controls, Saves and Files. Most settings
apply at once; Save keeps what you changed.

![The in-game menu's Render page over the race at 16:9](docs/images/menu.png)

### Mods and enhancements

Every mod is off, or at the Xbox's setting, unless the table says
otherwise. Open the in-game menu, go to the page in the table and change
the option; most apply at once, and Save keeps them for the next start.
The wizard also asks for the render scale before it builds.

| Mod | Menu page | What it does | Default |
|---|---|---|---|
| **Render scale** | Render | Draws the 3D at 1 to 4 times the Xbox's 640x480: sharper, slower. | 1 |
| **Aspect ratio** | Render | Widescreen: 16:9, 16:10 or 21:9 show more at the sides instead of stretching the picture (from the next start). | 4:3 |
| **Anisotropic filtering** | Render | Sharper ground and walls seen at a slant: 2, 4, 8 or 16. | 0 |
| **Overscan crop** | Render | Crops the border an old TV hid, and zooms the rest (4:3 only). | 0 |
| **2D on guest grid** | Render | Keeps 2D art on the Xbox's pixel grid at render scale 2 and up, so tiled menus and HUD show no seams. | off |
| **Scaling filter** | Render | How the picture is stretched to the window: nearest is crisp, linear smooth, integer whole multiples only. | nearest |
| **HUD margin** | Render | Pulls the HUD in from the screen edges on a wide picture (0 to 64). | 0 |
| **Profile** | Shaders | A shader over the picture: crt-lottes, crt-lottes-fast or crt-hyllian for an old TV's look, fsr to upscale. | Off |
| **HUD at the screen edges** | Mods | On a wide screen, the race HUD's position, lap times, lap count and course map move out to the edges. | on |

#### Render scale

The Xbox drew every game at 640x480. Render scale 3 draws the 3D at
1920x1440; Render > Render scale changes it while you play.

![The Viper and the rev counter at render scale 1, as on the Xbox, and at render scale 3: the jagged edges on the car body and the road lines go smooth](docs/images/pair-scale.png)

#### Aspect ratio

Render > Aspect ratio > 16:9 keeps the game's height of view and shows
more at the sides; it takes effect at the next start.

![The straight at 4:3 and at 16:9, where the wider view shows more of the track at both sides](docs/images/pair-aspect.png)

#### Anisotropic filtering

Render > Anisotropic filtering > 16 keeps a texture sharp where it runs
away from the camera, as a road or a floor does.

![The grid's road markings at render scale 3 with anisotropic filtering 0, as on the Xbox, and 16: the far road markings and skid marks stay sharp instead of fading into blur](docs/images/pair-aniso.png)

#### Overscan crop

A still shows only a zoom. Its origin: a TV of the time hid a band around
the picture, so console games kept their HUD away from the border, and
some left junk there. Overscan crop cuts that band and fills the window
with the rest. It works at 4:3 only.

#### 2D on guest grid

Above render scale 1 the 3D gets sharper, but 2D art made of tiles (menus,
the HUD, fonts) can show thin seams where the tiles meet, because each tile
now samples between the Xbox's pixels. This keeps 2D on the Xbox's own
pixel grid, as at scale 1: no seams, blockier 2D art.

*No picture here: auto modellista draws its menus and race HUD as whole
sprites with no tiled edges, so at render scale 3 they look the same with
the option off and on. The option is there for games whose 2D art is built
from tiles that open seams at higher scales.*

#### Scaling filter

The Xbox drew for a TV; a window is larger, so the picture is stretched to
fit. Nearest keeps every pixel sharp, linear blends them for a softer
picture, and integer scales by whole multiples only, with a border, so
every pixel is the same size.

![The grid at Suzuka stretched to a full screen with the nearest filter, as on the Xbox, and a second later with linear](docs/images/pair-filter.png)

#### HUD margin

With HUD at the screen edges on a wide picture, the HUD sits at the very
edge of the screen. HUD margin pulls it back in, in steps of 4 of the
Xbox's pixels, for a TV that crops its border or for a picture that reads
better nearer the middle.

![The grid at 16:9 with HUD margin 0, and with 32, the position and the lap count pulled in from the edges](docs/images/pair-hud-margin.png)

#### Profile

Shaders > Profile > crt-lottes draws the picture like an old TV, with
scanlines and a shadow mask; crt-hyllian is another look, and fsr is
AMD's upscaler instead. The Shaders page tunes each one, and saves a chain of them as a
profile of your own.

![The grid without a shader and with crt-lottes: scanlines and a shadow mask over the whole picture](docs/images/pair-crt.png)

#### HUD at the screen edges

The game placed its race HUD for a 4:3 TV. On a wide screen, Mods > HUD
at the screen edges moves the position, the lap times, the lap count and
the course map out to the edges; the tachometer and the speed stay beside
the middle. It does nothing at 4:3, and is on by default because at 4:3 it draws the
stock frame.

![The grid at 16:9 with the HUD in the 4:3 middle, and with the position, times, lap count and course map at the edges](docs/images/pair-hud-edges.png)

## Build requirements and common gotchas

- About 15 GB free for the build, plus room for the installer.
- A Windows 10 or 11 PC, an Apple silicon Mac, or a Steam Deck or Linux PC
  (in Desktop Mode on the Deck).
- An internet connection for the first build (the tools download once).
- Your copy is the supported version (the box above).

The failures players hit most:

- **"Another release":** your disc is another region or revision than the
  one this port is made from. The build may stop, or the game misbehave.
- **Mac: "the app is damaged" or cannot be opened:** that is Gatekeeper.
  Right-click the app and pick Open, once.
- **Steam Deck: the first launch shows only a notice for a few minutes:**
  it is downloading Proton; the notice closes when the game starts.

The rest: [docs/playing.md](docs/playing.md#troubleshooting).

## Report a bug

[Open a bug report](https://github.com/apexJCL/automodellista-recomp/issues/new?template=bug_report.yml):
the form asks for what we need (your game version, the build version, where
you play, your settings and the logs). Never attach game files, a disc
image, a save or a built bundle. Problems with the wizard or the `./automodellista`
commands themselves go to
[xboxrecomp-cli's issues](https://github.com/apexJCL/xboxrecomp-cli/issues).

## More documentation

| Document | For |
|---|---|
| [docs/playing.md](docs/playing.md) | building by hand, installing, the options, the in-game menu, known issues, troubleshooting |
| [docs/developers.md](docs/developers.md) | how the port works, building from source, the toolchain and its pins, contributing |
| [CHANGELOG.md](CHANGELOG.md) | what changed for players in each release |

**Sister project.** [BLiNX 2: Masters of Time & Space](https://github.com/apexJCL/blinx2-recomp)
is recompiled with the same toolkit and build wizard, and is the furthest along.

## Legal

This repository contains **no game data and no game code**: no game files,
assets or saves, and no lifted (recompiled) game code. `src/recomp/gen/` is
generated on your machine by the pipeline and is never committed.

To build it you need your own legally obtained copy of Auto Modellista. The game and its assets belong to their respective owners; this
project is not affiliated with them.
A bundle made by `automodellista package` contains your copy of the game and code
generated from it: it is for your own machines only, and is not yours to
redistribute.

### License and credits

MIT, see [LICENSE](LICENSE). Parts of `src/main.c`, `src/recomp_manual.c`
and `CMakeLists.txt` started from xboxrecomp's MIT-licensed new-game
templates; the license file carries that notice too. The toolkit that the
build links in statically carries LGPL-2.1 APU code extracted from xemu: if
you distribute a built binary, read [NOTICE](NOTICE) first.

Built on [xboxrecomp](https://github.com/sp00nznet/xboxrecomp) by sp00nz,
through the [apexJCL/xboxrecomp](https://github.com/apexJCL/xboxrecomp) fork, and on
[xboxrecomp-cli](https://github.com/apexJCL/xboxrecomp-cli).
