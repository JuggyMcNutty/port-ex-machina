# Port Ex Machina

A native, controller-first launcher for Deus Ex, and the work to run the game
behind it on devices it was never made for -- structured so that a new device
is a small port, not a fork of the project.

The name swaps the god in *deus ex machina*, "the god from the machine", for
ports: the project is written with Claude, an AI coding agent, and each port
takes the game to another machine.

You supply your own Deus Ex game files. None are included.

## Two halves

**The launcher** takes the place of `System/DeusEx.exe` -- the Unreal Engine 1
`Launch` module, the ~250 KB bootstrap shell that decides whether to ask
anything, writes the config the engine reads, manages the crash-detection
sentinel and starts the engine. It keeps that binary's contract, documented in
[`docs/re/`](docs/re/) and reverse-engineered before any of this was written,
but its screens are a tabbed home screen driven by the pad (Play, Video,
Controls, System) with settings the engine actually reads.
[`docs/DESIGN.md`](docs/DESIGN.md) records every divergence and why.

**The engine** is [Surreal Engine](https://github.com/dpjudas/SurrealEngine), an
open-source UE1 reimplementation that recognises this exact build. It is not
vendored here: [`engine-patches/`](engine-patches/) holds the fork's patches and
the reason they must stay in a fork, and `scripts/engine.sh` clones, checks and
builds it.

## Ports

| Port | For | Built | Status |
|---|---|---|---|
| [`linux-x86_64`](ports/linux-x86_64/) | desktop Linux | natively | the engine runs the game on the development PC; also the build for tests and `dxl-shots` |
| [`linux-aarch64`](ports/linux-aarch64/) | aarch64 devices with an ordinary distro | cross (launcher) or natively | the launcher cross-builds; not yet run on a device |
| [`trimui-smartpro`](ports/trimui-smartpro/) | TrimUI Smart Pro, spruceOS | cross | runs: the intro at ~30 FPS, Liberty Island's opening fight at 6.7 FPS (7.9 at 853×480; CPU-bound, the target is ~20) |
| [`android`](ports/android/) | Android | -- | planned: what it needs is in its README |

A port is a directory of the few things that differ for one device -- its
toolchain, where SDL2 comes from, a device profile, packaging, how to deploy.
[`docs/PORTING.md`](docs/PORTING.md) is the contract and the checklist for a
new one.

## Quick start

```sh
scripts/dx.sh test                         # unit tests (host build)
scripts/engine.sh fetch                    # once: the engine fork, into engine/SurrealEngine

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

## Layout

```
agent.md             session handoff: state, open decisions, gotchas
CMakeLists.txt       the launcher; CMakePresets.json has one preset per port
src/core/            the launcher contract and settings -- C11, no SDL, no device facts
src/platform/        the device profile (target.h), the hand-over to the game (launch.h),
                     posix/: exec and the crash-isolated GPU probe
src/ui/              SDL2 frontend: ui.c widgets, screens.c session + rows, tab_*.c, remap.c
src/app.c            the launcher's sequence, shared by both front ends
src/main.c           deusex-launcher;  src/cli_main.c: dxl-cli, the same contract with no display
tests/               host unit tests -- no display needed
tools/               shots.c (dxl-shots: every screen as .bmp), probes/ (device probes)
engine-patches/      the Surreal Engine fork's patches, and why they stay in a fork
docs/                DESIGN.md, PORTING.md, re/ (the reverse-engineering spec)
scripts/             dx.sh, engine.sh, check-abi.sh, check-docs.sh, host-tools.sh, lib/common.sh,
                     sample-report.py (CPU samples from a device without perf)
ports/common/        what every port ships unless it overrides it: run-game.sh, defaults
ports/<port>/        one device: see docs/PORTING.md

build/               (ignored) build/<port>/{launcher,engine,app}
deps/                (ignored) fetched toolchains and sysroots
engine/              (ignored) the engine fork, a separate git clone
gamefiles/           (ignored) your Deus Ex install, for running on this machine and for test_gamefiles
reference/           (ignored) the 1112f SDK, the DeusExe launcher source, IDA and ini backups
```

## Reading order

1. [`agent.md`](agent.md) -- where things stand, what is open, what cost time.
2. [`docs/DESIGN.md`](docs/DESIGN.md) -- what the launcher does and why:
   the settings files the engine really reads, the screens, controller support.
3. [`docs/PORTING.md`](docs/PORTING.md) -- how ports work.
4. The port you are working on: `ports/<port>/README.md`.
5. [`engine-patches/README.md`](engine-patches/README.md) -- every engine change.
6. [`docs/re/`](docs/re/) -- the original binary, when a behaviour's origin matters.

## License

[zlib](LICENSE), for everything in this repository, the engine patches
included. Surreal Engine has its own licences, in its `LICENSE.md`.

Deus Ex belongs to its owners; this project is not affiliated with or endorsed
by them.
