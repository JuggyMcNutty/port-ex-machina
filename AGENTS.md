# Port Ex Machina -- agent guide

Deus Ex (UE1) on modern platforms: our own launcher, our fork of Surreal Engine (VibeEngine), and
what the original binaries do, in four repositories side by side in one parent folder
([layout](README.md#layout); which doc holds what: [the docs table](README.md#documentation)).
This repository is the workspace: it fetches the others, builds, stages, deploys and profiles
the ports, and pins the commits they build ([the commands](README.md#quick-start)). Before
changing anything, read [`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md).

This file is edited in place: change what changed, delete what is done, never append a log --
git is the record. Every session loads it, so keep it small.

## Rules

- **Commit as JuggyMcNutty** (`11588877+JuggyMcNutty@users.noreply.github.com`), never the
  machine's global identity ([commits](docs/DEVELOPMENT.md#commits); another developer's
  identity is an open decision).
- **Never commit the game's files**: no ini, `.int`, package, IDA database or capture. All four
  repositories are public. The RE is behaviour in our own words, never decompiled code.
- **Push or move a pin only with the owner's go-ahead.** A change is committed
  in its own repository with the docs it affects, pushed, then pinned here
  (`scripts/engine.sh pin`, `scripts/launcher.sh pin <port>`) and the pin committed.
- **Nothing goes upstream**: the fork has diverged from upstream, and upstream's rules do not
  apply here.
- **Launcher branches**: `main` is the working base; each port branch is its own variant of it
  and takes `main`'s changes by merge ([the branches](docs/PORTING.md#the-branches)).
  `README.md` never flows from `main`: a merge always keeps the port branch's own README.
- **Done** means `scripts/dx.sh test` (linux-x86_64's unit tests) and `scripts/dx.sh check`
  (the drift guards) pass, and every cross port that ships -- today the Smart Pro -- builds
  warning-free. An engine change is also proven against the original by a scripted run of both
  engines
  ([scripted runs](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#scripted-runs-of-both-engines)).
- **Docs** hold the current state only, each fact in one place ([docs](docs/DEVELOPMENT.md#docs)).
- **Ask the owner** when you are unsure what the proper path forward is.

## Status

| Area | State |
|---|---|
| linux-x86_64 | The development platform. Launcher and engine build natively and run the game; the tests, `dxl-shots` and the engine harness run here. |
| linux-aarch64 | The launcher cross-builds. It has not run on a device yet. |
| trimui-smartpro | The game runs under Vulkan or OpenGL ES (chosen in the Video tab), at 853×480 by default, short of its ~20 FPS target. Measured when the owner chooses: [its Performance](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#performance). |
| android | Planned: [its README](https://github.com/JuggyMcNutty/deusex-launcher/blob/android/ports/android/README.md) is the plan. |
| x360 | Planned; nothing worked out. |
| The engine | [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine), branch `deusex`, pinned by `ENGINE-PIN.txt`. The reimplementation milestones are done, from crashes through multiplayer; what still differs from the original is [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md). |
| The launcher's `main` | The original `DeusEx.exe` recreated almost 1:1: launch sequence, wizard, splash, message boxes. `scripts/recreation.sh` builds, installs, checks and runs it. |
| The RE | [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info) documents every binary read so far ([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)). |

## Decided

- **The project**: a modern, cross-platform launcher for Deus Ex of our own, on our own engine
  fork. The original `DeusEx.exe` was reverse-engineered as a starting point, not a contract:
  the ports' launcher departs from it where that serves the ports, and the launcher's `main`
  keeps the original recreated beside it. The launcher's `main` is the working base, each port
  a variant of it ([`docs/PORTING.md`](docs/PORTING.md)); linux-x86_64 is where the project is
  developed and tested.
- **Four repositories**, each a folder of its own in one parent folder: this workspace;
  [deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher) (`main` the recreation, a
  branch per port); [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine) (the engine and
  everything about it, under `vibe/`); [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info)
  (the original binaries only).
- **The engine fork** is pinned and does not merge upstream: the two differ at the core, the
  script VM first; a fix of upstream's worth having is ported by hand
  ([how it is kept](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#how-it-is-kept)).
- **The engine first.** The porting work -- the next ports, the Smart Pro's own -- waits until the
  engine is more stable and has more of the game. The engine's speed against the original's is
  worked on: the desktop first, measured as the harness measures both engines
  ([measuring both engines](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#measuring-both-engines));
  the Smart Pro when the owner chooses. The open decisions wait for the owner unless one blocks
  the work.
- **Smart Pro target: ~20 FPS at Liberty Island's level start** (~50 ms a frame), for when its
  performance work resumes. Every trade-off for it is accepted, and deep script-VM work is in
  scope. Re-measure after each change, profiling on the device (`SAMPLE=1`): the desktop's
  proportions are not the device's. What is left:
  [where a frame goes](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#where-a-frame-goes).
- **Smart Pro default: 853×480** (`Performance.RenderScale` 0.6666667 in the port's
  `engine-settings.json.default`). There the frame is the CPU's, so engine speed-ups buy frames
  directly; at the panel's 1280×720 the GPU holds Vulkan's frame, and the GL driver's draw calls
  hold OpenGL ES's. 640×360 is out: 480 lines is the least Deus Ex's menus fit.
- **Renderers on aarch64 devices**: the goal is Vulkan, OpenGL ES and software, all selectable.
  The Smart Pro has the first two; a software renderer would have to be written, as Surreal
  Engine has none.
- **Reading the original**: the game's DLLs are read when a need comes up and documented in
  dx-reverse-info. Only if needed: `SoftDrv.dll`, `WinDrv.dll` beyond its flags, the owner's
  `ALAudio.dll`, `OpenGLDrv.dll` (its gamma ramp is read; the look is judged against
  `D3DDrv`). Never: `Editor.dll`, `Window.dll`'s code, the Glide, Metal and SGL drivers,
  `Setup.exe`, the GOG DLL, `RGalaxy.dll`.
- **The harness runs on the desktop**: every scripted run of either engine draws on the desktop's
  Xwayland, never a hidden display. This machine is a dedicated development device, and runs on
  different displays do not compare.
- **Multiplayer**: the fork joins the original's servers, live ones included (joining public
  servers is allowed), and hosts as the original does but for two choices: a listen server held
  to 144 frames a second, and `VibePlayer` for a name the player never chose (single player keeps
  the game's). Co-op is for later. A fork server on the
  public master servers' lists waits for the playtests: neither engine announces a server unless
  its uplink's `DoUplink` is set, which the game's `DeusEx.ini` does not set, and a master then
  queries the server's port, which this machine's NAT keeps from the internet
  ([multiplayer](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#multiplayer)).

## Open decisions

The owner's, each waiting until the owner takes it up.

- **The next ports**: a cross-built engine for linux-aarch64 (a sysroot with the engine's
  libraries, as the Smart Pro has); Android, starting with an in-process hand-over (its README).
- **`main`'s window icons into the port branches**: merging `main` into the port branches
  conflicts in `CMakeLists.txt` and `tests/test_gamefiles.c`, and keeps each branch's own README.
- **Another developer's commit identity**: JuggyMcNutty, or their own.
- **Detail textures**: the original, run here under Wine, draws none, `D3DDrv` and `OpenGLDrv`
  alike, its frames the same with the setting on or off, though `D3DDrv` has a three-pass one
  ([detail textures](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/d3ddrv-dll.md#detail-textures));
  the fork draws its own in one pass, which darkens Liberty Island's pier 2.3%, to 1.6% under
  `D3DDrv`'s. Keep the fork's, draw `D3DDrv`'s as read, or draw none
  ([the menus' settings](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-menus-settings)).

## Open items

Known defects, each linked to the doc that owns its code (the last line's have no other home):

- `main` drops the command line's quotes
  ([main's known defects](https://github.com/JuggyMcNutty/deusex-launcher/blob/main/README.md#known-defects)).
- The ports' launcher: the game gets the launcher's words, the lock is let go at the hand-over,
  no `RELAUNCH` reaches it, the Resolution row is Vulkan's only, the CPU-mode test is loose
  ([LAUNCHER.md's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md#known-defects)).
- Recorded only here: `main`'s `launch.c` keeps its own copies of `policy.c`'s bypass and splash
  rules; of `policy.c`'s, only the ports' launcher uses the bypass, and nothing the splash.

Unverified, each a known risk:

- Distant AI, which the Smart Pro runs with: far NPCs fighting with it on is untried
  ([what it throttles](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#settings-the-launcher-exposes)).
- The name walk's cost on the Smart Pro's collections: unmeasured
  ([housekeeping](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#housekeeping-not-seen-directly)).
- A menu dragged by its title bar and the Alt accelerators under a real mouse and keyboard:
  proven only with a timeline's input, which the original cannot take here
  ([the UI](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-ui)).
- A dedicated server's movers: drawing no frame, it never files one in the BSP again, as the
  original's code has it; no run proves it
  ([implemented, not as the original](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#implemented-not-as-the-original)).

Where the engine still differs from the original (each in
[`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md)):
06_HongKong_WanChai_Underworld's floor under its disco lights is 0.35-0.57 of `D3DDrv`'s, why
unread; a spotlight's pool on a wall sits half a texel off the original's; a line wholly inside
the level's solid meets nothing, and lines across two of DXMP_Silo's movers meet the mover where
the original's meet the level; at 20 frames a second a gull flown level into a wall stays put,
where the original's slides some 18 units along it; at Liberty Island's start the player stands
0.05 units higher, the original's first frame taking in the whole load; a net client's player
lands 2.4 units low at its spawn, which its server corrects; a collection deletes only the names
made for objects, keeping the rest for the session
([housekeeping](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#housekeeping-not-seen-directly)).
