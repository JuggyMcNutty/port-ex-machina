# Port Ex Machina -- agent guide

Deus Ex (UE1) on modern platforms:
+ our own launcher
+ our fork of Surreal Engine now called VibeEngine
+ reverse engineering docs

All in four repositories side by side
([the repositories](README.md#the-repositories)).

Which doc holds what:
[the docs table](README.md#documentation).

Before changing anything, read
[`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md).

This repository is the workspace: from it we fetch the
others, build, stage, deploy and profile the ports, and pin the commits they build
([the commands](README.md#quick-start)).

This file is edited in place: change what changed, delete what is done, never append a log --
git is the record. Every session loads it, so keep it small.

## Rules

- **Ask the owner** when you are unsure what the proper path forward is.
- **Commit as JuggyMcNutty** (`11588877+JuggyMcNutty@users.noreply.github.com`), never the
  machine's global identity ([commits](docs/DEVELOPMENT.md#commits)).
- **Never commit the game's files**: no ini, `.int`, package, IDA database or capture. All four
  repositories are public. The RE is behaviour in our own words, never decompiled code.
- **Push or move a pin only with the owner's go-ahead.** A change is committed in its own
  repository with the docs it affects, pushed, then pinned here (`scripts/engine.sh pin`,
  `scripts/launcher.sh pin <port>`) and the pin committed.
- **Nothing goes upstream**: the fork has diverged, and upstream's rules do not apply here
  ([how it is kept](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#how-it-is-kept)).
- **Launcher branches**: `main`, the original recreated as close to 1:1 as possible, and a
  branch per targeted platform, each tailored to it, so far that includes (`linux-x86_64`,
  `linux-aarch64`, `trimui-smartpro`). None merges another: a change crosses only by a
  deliberate cherry-pick ([the branches](docs/PORTING.md#the-branches)).
- **Done** means `scripts/dx.sh test` and `scripts/dx.sh check` pass, and every cross port that
  ships (the Smart Pro) builds warning-free. An engine change is also proven against the
  original by a scripted run of both engines
  ([scripted runs](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#scripted-runs-of-both-engines)).
- **Docs** hold the current state only, each fact in one place
  ([docs](docs/DEVELOPMENT.md#docs)).

## Status

Each port's: [the ports](README.md#engine-ports); linux-x86_64 is the main platform developed
for. The engine, [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine) on branch `deusex`,
pinned by `ENGINE-PIN.txt`, plays the game as the original does but for what
[`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md)
lists. The launcher's `main` is the original `DeusEx.exe` recreated almost 1:1;
`scripts/recreation.sh` builds, installs, checks and runs it. The RE,
[dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info), documents every binary read
([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).

## Decided

- **The project**: a cross-platform launcher for Deus Ex of our own, on our own engine fork. The
  original `DeusEx.exe` is the model, not a contract: each port's launcher departs from it where
  that serves the port, and the launcher's `main` keeps the original recreated beside it.
  linux-x86_64 is where the project is developed and tested ([porting](docs/PORTING.md)).
- **The engine is Deus Ex's alone**: no other game's code, nor upstream's platforms and tools
  that Deus Ex does not use
  ([how it is kept](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#how-it-is-kept));
  the D3D11 device, the Win32 code it needs and SDL3 are kept, neither built nor tested
  ([building](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/README.md#building-and-running)).
- **VibeEngine is developed first.** All porting work is deferred until the owner chooses.
- **Reading the original**: the game's DLLs are read when a need comes up and documented in
  dx-reverse-info. Only if needed: `SoftDrv.dll`, `WinDrv.dll` beyond its flags, the owner's
  `ALAudio.dll`, `OpenGLDrv.dll` beyond its gamma ramp (the look is judged against `D3DDrv`).
  Never: `Editor.dll`, `Window.dll`'s code, the Glide, Metal and SGL drivers, `Setup.exe`, the
  GOG DLL, `RGalaxy.dll`.
- **The harness runs on the desktop's display**, never a hidden one: this machine is a dedicated
  development device
  ([the display](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#the-display)).

## Open decisions

The owner's, each waiting until the owner takes it up.

- **Detail textures**: keep the fork's one-pass ones, draw `D3DDrv`'s three-pass ones as read,
  or draw none, as the original run here does
  ([the menus' settings](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-menus-settings),
  [detail textures](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/d3ddrv-dll.md#detail-textures)).
- **An idea from DXD3D9 2.0** (a third-party renderer in `reference/`, ideas only:
  [its terms](docs/DEVELOPMENT.md#the-repositories)): a 10-bit GL scene buffer, `GL_RGB10_A2`
  for RGBA8
  ([rendering](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#rendering)).

## To do

Decided work for later sessions, each deleted when it is done.

- **SDL3**: the engine's SDL3 window backend brought level with SDL2's, the fork's gamepad
  support first (`GetGamepadState`), then built and tested
  ([the gamepad](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#running-on-our-devices)).
- **The engine's own name**: the build named VibeEngine, not upstream's `SurrealEngine` (the
  executable, `SurrealEngine.pk3`, the settings folder).
- **One linux-aarch64 engine build for every aarch64 target**, the Smart Pro included (its
  launcher stays its own), with what differs between devices found at run time: every GPU fully
  supported (Snapdragon, Mali, PowerVR), on development boards the owner is acquiring; the
  engine's build-time device choices moved to run time (the Smart Pro's borderless fullscreen;
  X11 and Wayland, compiled out of its build); one glibc ceiling and sysroot for all
  ([running on our devices](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#running-on-our-devices)).
- **Windows as a build target**: the engine's kept Windows code built and tested
  ([building](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/README.md#building-and-running)),
  and a launcher branch with win32 versions of the launcher's OS dependencies
  ([other operating systems](docs/PORTING.md#other-operating-systems)).

## Open items

Known defects, each in the doc that owns its code:
[`main`'s](https://github.com/JuggyMcNutty/deusex-launcher/blob/main/README.md#known-defects)
and each port branch's `docs/LAUNCHER.md`
([linux-x86_64's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md#known-defects),
[linux-aarch64's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-aarch64/docs/LAUNCHER.md#known-defects),
[trimui-smartpro's](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/docs/LAUNCHER.md#known-defects)).
Where the engine still differs from the original: each section's "Differs" in
[`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md).

Unverified, a known risk the owner ranks low:

- A menu dragged by its title bar and the Alt accelerators under a real mouse and keyboard:
  proven only with a timeline's input, which the original cannot take here
  ([the UI](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-ui)).
