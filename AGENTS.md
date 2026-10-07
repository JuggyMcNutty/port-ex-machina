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
  warning-free but for one third-party warning (open items). An engine change is also proven
  against the original by a scripted run of both engines
  ([scripted runs](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#scripted-runs-of-both-engines)).
- **Docs** hold the current state only, each fact in one place ([docs](docs/DEVELOPMENT.md#docs)).
- **Ask the owner** when you are unsure what the proper path forward is.

## Status

| Area | State |
|---|---|
| linux-x86_64 | The development platform. Launcher and engine build natively and run the game; the tests, `dxl-shots` and the engine harness run here. |
| linux-aarch64 | The launcher cross-builds. It has not run on a device yet. |
| trimui-smartpro | The game runs under Vulkan or OpenGL ES (chosen in the Video tab), at 853×480 by default. Performance work is on hold. Numbers: [its Performance](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#performance). |
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
- **The engine first.** The porting work -- the Smart Pro's performance, the next ports -- waits
  until the engine is more stable and has more of the game. The open decisions wait for the
  owner unless one blocks the work.
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
- **Multiplayer**: the fork joins the original's servers, live ones included (joining public
  servers is allowed), and hosts as the original does. Co-op is for later. A fork server on the
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

## Open items

Known defects, each linked to the doc that owns its code (the last line's have no other home):

- `main` drops the command line's quotes, and its `ParseParam` is stricter than the original's
  ([main's known defects](https://github.com/JuggyMcNutty/deusex-launcher/blob/main/README.md#known-defects)).
- The ports' launcher: the game gets the launcher's words, the lock is let go at the hand-over,
  the Resolution row is Vulkan's only, the CPU-mode test is loose
  ([LAUNCHER.md's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md#known-defects)).
- The engine: a listen server at some 1,000 frames a second sending a client no unreliable call
  (a sound it hears); coronas taking every dynamic corona light, `GC.DrawActor` stamping no
  render time
  ([multiplayer](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#multiplayer),
  [coronas](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#coronas),
  [out of sight](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#out-of-sight)).
- Recorded only here: `main`'s `policy.c` keeps bypass and splash rules nothing live uses;
  dx-reverse-info's `types/launch.h` needs `<stddef.h>` to compile on its own; a clean Smart Pro
  build warns once, in the third-party `Thirdparty/resample/pffft.cpp` (it tests `__arm__`, not
  `__aarch64__`; the resampler is built without it, `R8B_PFFFT` 0); dx-reverse-info's
  `engine-dll.md` (traces) has movers block the BSP line test and its `galaxy-dll.md` (sounds
  behind walls) has them not, unread which holds where.

Unverified, each a known risk:

- The Vulkan device's scene shader without `darkClamp` is built but has never run: the
  comparisons ran the GL device.
- Jumping onto an NPC's head: the stomp and the bounce go through `SupportActor`; untried in play.
- Distant AI, which the Smart Pro runs with: whether far NPCs still behave is unjudged in play.
- Fractal textures' cost on the Smart Pro: unmeasured.
- The Smart Pro's frame with the Hor+ view (a third wider at 16:9): its Performance numbers
  predate it.
- The name walk's cost on the Smart Pro's collections (5 to 8 ms more on linux-x86_64): unmeasured.
- A live server correcting the client at a stop, with the traces and moves as the original's
  ([multiplayer](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#multiplayer)).
- linux-aarch64 on a real device.

Where the engine still differs from the original (each in
[`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md)):
Liberty Island's pier floor is 1.7-3% darker than `D3DDrv`'s; the light effects whose original
is unread keep the fork's shapes; at the level start 9 NPCs count as drawn where the original
counts 3; Terrorist10 moves farther than the original's, and Terrorist12 walks on from a ledge
where the original's stops being ticked; a pawn falling into water sinks deeper; the root's first
focus (the keypad's, none with no modal up) differs by the code, unchecked in a run
([the UI](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-ui));
a collection deletes only the names made for objects, keeping the rest for the session
([housekeeping](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#housekeeping-not-seen-directly)).
