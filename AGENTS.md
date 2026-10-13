# Port Ex Machina -- agent guide

Deus Ex (UE1) on modern platforms: our own launcher, our fork of Surreal Engine (VibeEngine),
and what the original binaries do, in four repositories side by side
([the repositories](README.md#the-repositories); which doc holds what:
[the docs table](README.md#documentation)). This repository is the workspace: it fetches the
others, builds, stages, deploys and profiles the ports, and pins the commits they build
([the commands](README.md#quick-start)). Before changing anything, read
[`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md).

This file is edited in place: change what changed, delete what is done, never append a log --
git is the record. Every session loads it, so keep it small.

## Rules

- **Commit as JuggyMcNutty** (`11588877+JuggyMcNutty@users.noreply.github.com`), never the
  machine's global identity ([commits](docs/DEVELOPMENT.md#commits)).
- **Never commit the game's files**: no ini, `.int`, package, IDA database or capture. All four
  repositories are public. The RE is behaviour in our own words, never decompiled code.
- **Push or move a pin only with the owner's go-ahead.** A change is committed in its own
  repository with the docs it affects, pushed, then pinned here (`scripts/engine.sh pin`,
  `scripts/launcher.sh pin <port>`) and the pin committed.
- **Nothing goes upstream**: the fork has diverged, and upstream's rules do not apply here
  ([how it is kept](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#how-it-is-kept)).
- **Launcher branches**: `main` is the working base; each port branch is a variant of it and
  takes `main`'s changes by merge; the port branches share the ports' README, which never takes
  `main`'s ([the branches](docs/PORTING.md#the-branches)).
- **Done** means `scripts/dx.sh test` and `scripts/dx.sh check` pass, and every cross port that
  ships (the Smart Pro) builds warning-free. An engine change is also proven against the
  original by a scripted run of both engines
  ([scripted runs](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#scripted-runs-of-both-engines)).
- **Docs** hold the current state only, each fact in one place
  ([docs](docs/DEVELOPMENT.md#docs)).
- **Ask the owner** when you are unsure what the proper path forward is.

## Status

Each port's: [the ports](README.md#ports); linux-x86_64 is the development platform. The engine,
[VibeEngine](https://github.com/JuggyMcNutty/VibeEngine) on branch `deusex`, pinned by
`ENGINE-PIN.txt`, plays the game as the original does but for what
[`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md)
lists. The launcher's `main` is the original `DeusEx.exe` recreated almost 1:1;
`scripts/recreation.sh` builds, installs, checks and runs it. The RE,
[dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info), documents every binary read
([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).

## Decided

- **The project**: a cross-platform launcher for Deus Ex of our own, on our own engine fork. The
  original `DeusEx.exe` is the model, not a contract: the ports' launcher departs from it where
  that serves the ports, and the launcher's `main` keeps the original recreated beside it.
  linux-x86_64 is where the project is developed and tested ([porting](docs/PORTING.md)).
- **The engine first.** The porting work -- the next ports, the Smart Pro's own -- waits until
  the engine is more stable and has more of the game. The engine's speed against the original's
  is worked on: the desktop first, measured as the harness measures both engines
  ([measuring both engines](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#measuring-both-engines));
  the Smart Pro when the owner chooses. The open decisions wait for the owner unless one blocks
  the work.
- **Smart Pro target: ~20 FPS at Liberty Island's level start** (~50 ms a frame), for when its
  performance work resumes. Every trade-off for it is accepted but the AI's: every pawn thinks
  every frame: far pawns thinking less often make the AI behave oddly. Deep script-VM work is
  in scope. Re-measure after each change, profiling on the device (`SAMPLE=1`): the desktop's
  proportions are not the device's. Where its frame goes and what is left:
  [its README](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#where-a-frame-goes).
- **Smart Pro default: 853x480** (`Performance.RenderScale` 0.6666667 in the port's
  `engine-settings.json.default`): 480 lines is the least Deus Ex's menus fit, so 640x360 is
  out; at the panel's 1280x720 the GPU holds the frame.
- **Renderers on aarch64 devices**: the goal is Vulkan, OpenGL ES and software, all selectable.
  The Smart Pro has the first two; a software renderer would have to be written, as Surreal
  Engine has none.
- **Reading the original**: the game's DLLs are read when a need comes up and documented in
  dx-reverse-info. Only if needed: `SoftDrv.dll`, `WinDrv.dll` beyond its flags, the owner's
  `ALAudio.dll`, `OpenGLDrv.dll` beyond its gamma ramp (the look is judged against `D3DDrv`).
  Never: `Editor.dll`, `Window.dll`'s code, the Glide, Metal and SGL drivers, `Setup.exe`, the
  GOG DLL, `RGalaxy.dll`.
- **The harness runs on the desktop's display**, never a hidden one: this machine is a dedicated
  development device
  ([the display](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#the-display)).
- **Multiplayer**: the fork joins the original's servers, live ones included (joining public
  servers is allowed), and hosts as the original does but for two choices, a listen server held
  to 144 frames a second and `VibePlayer` for a name the player never chose
  ([multiplayer](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#multiplayer)).
  Co-op is for later. A fork server on the public master servers' lists waits for the playtests.

## Open decisions

The owner's, each waiting until the owner takes it up.

- **The next ports**: a cross-built engine for linux-aarch64 (a sysroot with the engine's
  libraries, as the Smart Pro has); Android, starting with an in-process hand-over
  ([its README](https://github.com/JuggyMcNutty/deusex-launcher/blob/android/ports/android/README.md)).
- **`main` into the port branches**: its window icons and its change to the shared
  command-line code (a `?` ends a flag's name too) wait for that merge, which conflicts in
  `CMakeLists.txt` and `tests/test_gamefiles.c` and keeps the ports' README.
- **Another developer's commit identity**: JuggyMcNutty, or their own.
- **Detail textures**: keep the fork's one-pass ones, draw `D3DDrv`'s three-pass ones as read,
  or draw none, as the original run here does
  ([the menus' settings](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-menus-settings),
  [detail textures](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/d3ddrv-dll.md#detail-textures)).
- **An idea from DXD3D9 2.0** (a third-party renderer in `reference/`, ideas only:
  [its terms](docs/DEVELOPMENT.md#the-repositories)): a 10-bit GL scene buffer, `GL_RGB10_A2`
  for RGBA8
  ([rendering](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#rendering)).

## Open items

Known defects, each in the doc that owns its code:
[`main`'s](https://github.com/JuggyMcNutty/deusex-launcher/blob/main/README.md#known-defects)
and
[the ports' launcher's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md#known-defects).
Where the engine still differs from the original: each section's "Differs" in
[`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md).

Unverified, a known risk the owner ranks low:

- A menu dragged by its title bar and the Alt accelerators under a real mouse and keyboard:
  proven only with a timeline's input, which the original cannot take here
  ([the UI](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-ui)).
