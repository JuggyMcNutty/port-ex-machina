# Port Ex Machina

## A modern, cross-platform launcher for Deus Ex with efforts to port to other platforms.

# You supply your own game assets. None are included.

## What it is

**The launcher** A reverse engineered modern launcher for the deus ex engine:
[deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher). Its `main`
recreates the original `DeusEx.exe` launcher almost 1:1; each port is a branch
of it, holding that port's configs and additions.

**The engine** is [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine), our
fork of [Surreal Engine](https://github.com/dpjudas/SurrealEngine), an
open-source UE1 re-implementation, carrying the changes our platforms and Deus
Ex need -- with its own roadmap and docs.

**The RE** of the original binaries, `DeusEx.exe` and the game's DLLs, is
[dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info).

**The ports** linux-x86_64 is the base target with linux-aarch64, android, and platform specific builds.

**This repository** is the workspace that puts them together: the scripts
that fetch the others and build, stage, deploy and profile a port, the pins
naming which commits of the engine and the launcher it builds, and where
things stand.

## Ports

| Port | For | Status |
|---|---|---|
| [`linux-x86_64`](https://github.com/JuggyMcNutty/deusex-launcher/tree/linux-x86_64/ports/linux-x86_64) | desktop Linux; **the base** | runs the game; where the tests, `dxl-shots` and engine validation run |
| [`linux-aarch64`](https://github.com/JuggyMcNutty/deusex-launcher/tree/linux-aarch64/ports/linux-aarch64) | aarch64 devices with an ordinary distro | Builds not tested |
| [`trimui-smartpro`](https://github.com/JuggyMcNutty/deusex-launcher/tree/trimui-smartpro/ports/trimui-smartpro) | TrimUI Smart Pro, spruceOS | builds and runs; performance work in progress |
| [`android`](https://github.com/JuggyMcNutty/deusex-launcher/tree/android/ports/android) | Android | planned |
| [`x360`](https://github.com/JuggyMcNutty/deusex-launcher/tree/x360/ports/x360) | Xbox 360 | planned |

## Quick start

Each repository gets a folder of its own in one parent folder, so clone this
one into an empty folder; `fetch` puts the others beside it
([layout](#layout)).

```sh
mkdir deusex && cd deusex                  # the parent folder: any name
git clone https://github.com/JuggyMcNutty/port-ex-machina.git && cd port-ex-machina
scripts/dx.sh fetch                        # once: the engine, the launcher's port branches, the RE
scripts/dx.sh test                         # unit tests (the base port's build)
scripts/engine.sh status                   # how far upstream is past the fork

scripts/dx.sh deps   <port>                # toolchains and sysroot, if the port needs them
scripts/dx.sh build  <port>                # launcher, then engine
scripts/dx.sh stage  <port>                # build/<port>/app: exactly what ships
scripts/dx.sh run    <port>                # native ports
scripts/dx.sh deploy <port>                # device ports: builds and stages first
scripts/dx.sh profile <port>               # device ports: a frame-time profile (the engine's profiling hooks on)

scripts/dx.sh check                        # drift guards: every repository's docs, the pins, ports, ABI

scripts/recreation.sh build                # the original launcher recreated (the launcher's main), and the engine
scripts/recreation.sh install [<GameDir>]  # into the game's System/, beside DeusEx.exe
scripts/recreation.sh check [<GameDir>]    # run end to end on a copy, on a private display
scripts/recreation.sh run [<args>]         # System/DeusEx, as a player starts it
```

Put the game files where the port's `launcher.ini` says (`GameDir`); a
`linux-x86_64` app staged here points at the `gamefiles/` beside the
repositories.

## Documentation

| Doc | Read it for |
|---|---|
| [`agent.md`](agent.md) | where things stand: decisions, open items, what is next |
| [`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md) | how to work here: the base port, the repositories, cold start, commits, docs rules, gotchas |
| [`docs/PORTING.md`](docs/PORTING.md) | how ports work -- a branch of the launcher each -- and how to add one |
| [the launcher's README](https://github.com/JuggyMcNutty/deusex-launcher) | `main`, the original recreated, and the branches |
| [`LAUNCHER.md`](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md), on `linux-x86_64` | the launcher the ports run: the settings files the engine really reads, the screens, controller support, what it changes from the original |
| a port's `ports/<id>/README.md`, on its branch | one device: status, what differs from linux-x86_64, measurements, what was verified |
| VibeEngine's [`vibe/docs/`](https://github.com/JuggyMcNutty/VibeEngine/tree/deusex/vibe/docs) | the engine: how it is kept, run, profiled, and what it changes ([`ENGINE.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md)); the reimplementation roadmap ([`ROADMAP.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ROADMAP.md)); what it lacks of the original ([`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md)); working on it ([`DEVELOPMENT.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md)) |
| [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info) | the original binaries: `DeusEx.exe`, where the launcher began, and the game's DLLs -- what they do |

## Layout

Every repository is a folder of its own in one parent folder, none inside
another, beside what none of them owns. The parent is not versioned itself.

```
deusex/                  the parent folder (any name)
  port-ex-machina/       this repository:
    agent.md               where things stand (session handoff)
    ENGINE-PIN.txt         the engine's version: VibeEngine's repository, branch and commit
    LAUNCHER-PIN.txt       the launcher's: deusex-launcher's repository, and each port's commit
    docs/                  DEVELOPMENT, PORTING
    scripts/               dx.sh (the entry point), engine.sh, launcher.sh, recreation.sh,
                           check-docs.sh, lib/common.sh (shared with the ports' and the
                           engine's own scripts)
    tools/probes/          device probes (docs/PORTING.md)
  VibeEngine/            the engine, a clone (scripts/engine.sh)
  deusex-launcher/       the launcher: main/, and a worktree per port (scripts/launcher.sh)
  dx-reverse-info/       the RE, a clone (scripts/dx.sh fetch)
  gamefiles/             your Deus Ex install: for running on this machine, test_gamefiles
                         and the IDA databases
  reference/             the 1112f SDK, the DeusExe launcher source, IDA and ini backups,
                         the original's saves
  deps/                  fetched toolchains and sysroots
  build/                 build/<port>/{engine,app}, and the engine tools' own
```

A path in these docs that is not a repository's own is the parent folder's:
`VibeEngine/vibe/tools`, `gamefiles/System`.

## License

[zlib](LICENSE), for everything in this repository, the launcher and the RE,
and for our commits in the engine fork. Surreal Engine has its own licences,
in its `LICENSE.md`.

Deus Ex belongs to its owners; this project is not affiliated with or endorsed
by them.
