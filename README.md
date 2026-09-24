# Port Ex Machina

## A modern, cross-platform launcher for Deus Ex with efforts to port to other platforms.

# You supply your own game assets. None are included.

## What it is

**The launcher** A reverse engineered modern launcher for the deus ex engine.

**The engine** is a fork of [Surreal Engine](https://github.com/dpjudas/SurrealEngine),
an open-source UE1 re-implementation, with patches we develop targeting different platforms.

**The ports** linux-x86_64 is the base target with linux-aarch64, android, and platform specific builds.

## Ports

| Port | For | Status |
|---|---|---|
| [`linux-x86_64`](ports/linux-x86_64/) | desktop Linux; **the base** | runs the game; where the tests, `dxl-shots` and engine validation run |
| [`linux-aarch64`](ports/linux-aarch64/) | aarch64 devices with an ordinary distro | Builds not tested |
| [`trimui-smartpro`](ports/trimui-smartpro/) | TrimUI Smart Pro, spruceOS | builds and runs; performance work in progress |
| [`android`](ports/android/) | Android | planned |
| [`xbox360`](ports/x360/) | Xbox 360 | planned |

## Quick start

```sh
scripts/dx.sh test                         # unit tests (the base port's build)
scripts/engine.sh fetch                    # once: the engine fork, into engine/SurrealEngine
scripts/engine.sh status                   # how far upstream is past the pin (upgrade: docs/ENGINE.md)

scripts/dx.sh deps   <port>                # toolchains and sysroot, if the port needs them
scripts/dx.sh build  <port>                # launcher, then engine
scripts/dx.sh stage  <port>                # build/<port>/app: exactly what ships
scripts/dx.sh run    <port>                # native ports
scripts/dx.sh deploy <port>                # device ports: builds and stages first
scripts/dx.sh profile <port>               # device ports: a frame-time profile (scripts/engine.sh perf on)

scripts/dx.sh check                        # drift guards: docs, engine patches, ports, ABI
```

Put the game files where the port's `launcher.ini` says (`GameDir`); a
`linux-x86_64` app staged in this workspace points at `gamefiles/`.

## Documentation

| Doc | Read it for |
|---|---|
| [`agent.md`](agent.md) | where things stand: decisions, open items, what is next |
| [`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md) | how to work here: the base port, cold start, commits, docs rules, gotchas |
| [`docs/LAUNCHER.md`](docs/LAUNCHER.md) | the launcher: the settings files the engine really reads, the screens, controller support |
| [`docs/ENGINE.md`](docs/ENGINE.md) | the engine fork: how it is pinned and upgraded, running and profiling it, every patch |
| [`docs/PORTING.md`](docs/PORTING.md) | how ports work, and how to add one |
| `ports/<id>/README.md` | one device: status, what differs from linux-x86_64, measurements, what was verified |
| [`docs/re/`](docs/re/) | the original binaries: `DeusEx.exe`, where the launcher began, and the game's DLLs -- what their natives do, for the engine |

## Layout

```
agent.md             where things stand (session handoff)
CMakeLists.txt       the launcher; CMakePresets.json has one preset per port
src/core/            the launcher's core and settings -- C11, no SDL, no device facts
src/platform/        the device profile (target.h), the hand-over to the game (launch.h),
                     posix/: exec and the crash-isolated GPU probe
src/ui/              SDL2 frontend: ui.c widgets, screens.c session + rows, tab_*.c, remap.c
src/app.c            the launcher's sequence, shared by both front ends
src/main.c           deusex-launcher;  src/cli_main.c: dxl-cli, the same sequence with no display
tests/               host unit tests -- no display needed
tools/               shots.c (dxl-shots: every screen as .bmp), probes/ (device probes),
                     ida/ (IDA types from the game's script), natives_audit.py
                     (the original's natives against the engine fork's)
engine-patches/      the engine fork as patches over a pinned upstream commit
docs/                LAUNCHER, ENGINE, PORTING, DEVELOPMENT; re/ (the original DeusEx.exe and DLLs)
scripts/             dx.sh, engine.sh, check-abi.sh, check-docs.sh, host-tools.sh, lib/common.sh,
                     sample-report.py (CPU samples from a device without perf)
ports/common/        the base app every port ships unless it overrides it: run-game.sh, defaults
ports/<port>/        one device: see docs/PORTING.md

build/               (ignored) build/<port>/{launcher,engine,app}
deps/                (ignored) fetched toolchains and sysroots
engine/              (ignored) the engine fork, a separate git clone
gamefiles/           (ignored) your Deus Ex install, for running on this machine and for test_gamefiles
reference/           (ignored) the 1112f SDK, the DeusExe launcher source, IDA and ini backups
```

## License

[zlib](LICENSE), for everything in this repository, the engine patches
included. Surreal Engine has its own licences, in its `LICENSE.md`.

Deus Ex belongs to its owners; this project is not affiliated with or endorsed
by them.
