# Port Ex Machina

Deus Ex (version 1112fm) on modern platforms: a launcher of our own, a fork of Surreal Engine
to run the game, and the reverse engineering of the original binaries behind both.

**You supply your own game files. None are included.**

## The workspace

This repository holds no launcher, engine or game code. It is the workspace that ties the
project's other three repositories together:

- `scripts/dx.sh fetch` clones them beside it, each a folder of its own in one parent folder
  ([layout](#layout)).
- Its scripts build each port (the launcher, then the engine), stage it, run it, and deploy it
  to a device and profile it there ([quick start](#quick-start)).
- `ENGINE-PIN.txt` and `LAUNCHER-PIN.txt` pin the exact engine and launcher commits the ports
  build. A change is committed and pushed in its own repository, then pinned here.
- `scripts/dx.sh check` guards all four repositories at once: every doc's paths and links, the
  pins, the ports' files and the ABI of what is staged.
- [`AGENTS.md`](AGENTS.md) and [`docs/`](docs/) say how the project is worked on, where it
  stands and what is open ([documentation](#documentation)).

## The repositories

| Repository | What it is | Here |
|---|---|---|
| [port-ex-machina](https://github.com/JuggyMcNutty/port-ex-machina) | this workspace | `port-ex-machina/` |
| [deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher) | the launcher. `main`, the working base, recreates the original `DeusEx.exe` almost 1:1; each port is a branch of it, its own variant | `deusex-launcher/main/` and a worktree per port; each port's commit pinned in `LAUNCHER-PIN.txt` |
| [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine) | the engine: our fork of [Surreal Engine](https://github.com/dpjudas/SurrealEngine), an open-source UE1 reimplementation, made to play Deus Ex as the original does and to run on our ports (branch `deusex`). The game's main menu shows the engine commit it was built from | `VibeEngine/`, pinned in `ENGINE-PIN.txt` |
| [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info) | what the original binaries do -- `DeusEx.exe` and the game's DLLs -- in our own words, and the IDA scripts that read them | `dx-reverse-info/`, not pinned |

## Ports

| Port | For | Status |
|---|---|---|
| [`linux-x86_64`](https://github.com/JuggyMcNutty/deusex-launcher/tree/linux-x86_64/ports/linux-x86_64) | desktop Linux; **the development platform** | runs the game; where the tests, `dxl-shots` and engine validation run |
| [`trimui-smartpro`](https://github.com/JuggyMcNutty/deusex-launcher/tree/trimui-smartpro/ports/trimui-smartpro) | TrimUI Smart Pro, spruceOS | runs the game under Vulkan or OpenGL ES, short of its ~20 FPS target; the engine first |
| [`linux-aarch64`](https://github.com/JuggyMcNutty/deusex-launcher/tree/linux-aarch64/ports/linux-aarch64) | aarch64 devices with an ordinary distro | the launcher cross-builds; not yet run on a device |
| [`android`](https://github.com/JuggyMcNutty/deusex-launcher/tree/android/ports/android) | Android | planned |
| [`x360`](https://github.com/JuggyMcNutty/deusex-launcher/tree/x360/ports/x360) | Xbox 360 | planned |

## Quick start

Clone this repository into an empty folder; `fetch` puts the others beside it.

```sh
mkdir deusex && cd deusex                  # the parent folder: any name
git clone https://github.com/JuggyMcNutty/port-ex-machina.git && cd port-ex-machina
scripts/dx.sh fetch                        # once: the engine, the launcher's port branches, the RE
scripts/dx.sh test [<port>]                # unit tests: linux-x86_64's, or a device branch's host build
scripts/engine.sh status                   # the pin and the fork's branch

scripts/dx.sh deps   <port>                # toolchains and sysroot, if the port needs them
scripts/dx.sh build  <port>                # launcher, then engine
scripts/dx.sh stage  <port>                # build/<port>/app: exactly what ships
scripts/dx.sh run    <port>                # native ports
scripts/dx.sh deploy <port>                # device ports: builds and stages first
scripts/dx.sh profile <port>               # device ports: a frame-time profile (deploy with the engine's profiling hooks on first)

scripts/dx.sh check                        # drift guards: every repository's docs, the pins, ports, ABI

scripts/recreation.sh build                # the original launcher recreated (the launcher's main), and the engine
scripts/recreation.sh install [<GameDir>]  # into the game's System/, beside DeusEx.exe
scripts/recreation.sh check [<GameDir>]    # run end to end on a copy, on a private display
scripts/recreation.sh run [<args>]         # System/DeusEx, as a player starts it
```

Put the game files where the port's `launcher.ini` says (`GameDir`); a `linux-x86_64` app
staged here points at the `gamefiles/` beside the repositories.

## Documentation

| Doc | Read it for |
|---|---|
| [`AGENTS.md`](AGENTS.md) | the rules, where things stand, the decisions, what is open (agents load it through `CLAUDE.md`) |
| [`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md) | how to work here: the repositories, dependencies, commits, docs, drift guards, gotchas |
| [`docs/PORTING.md`](docs/PORTING.md) | how ports work -- a branch of the launcher each -- and how to add one |
| [the launcher's README](https://github.com/JuggyMcNutty/deusex-launcher) | `main`, the original recreated; [linux-x86_64's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/README.md) introduces the ports' launcher |
| [`LAUNCHER.md`](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md), on `linux-x86_64` | the launcher the Linux ports run: the settings files the engine really reads, the screens, controller support, what it changes from the original |
| a port's `ports/<id>/README.md`, on its branch | one device: status, what differs from the branch it started from, measurements, what was verified |
| VibeEngine's [`vibe/docs/`](https://github.com/JuggyMcNutty/VibeEngine/tree/deusex/vibe/docs) | the engine: how it is kept, run, profiled and what the fork changes ([`ENGINE.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md)); where it still differs from the original ([`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md)); working on it, the harness ([`DEVELOPMENT.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md)) |
| [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info) | the original binaries: `DeusEx.exe`, where the launcher began, and the game's DLLs -- what they do |

## Layout

Every repository is a folder of its own in one parent folder, none inside another, beside what
none of them owns. The parent is not versioned itself.

```
deusex/                  the parent folder (any name)
  port-ex-machina/       this repository:
    AGENTS.md              rules, status, decisions, open items (CLAUDE.md imports it)
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
                         the original's saves and its wizard's captures
  deps/                  fetched toolchains and sysroots
  build/                 build/<port>/{engine,app}, build/main/ (the recreation's launcher
                         and its live check), and the engine tools' own
```

A path in these docs that is not a repository's own is the parent folder's:
`VibeEngine/vibe/tools`, `gamefiles/System`.

## License

[zlib](LICENSE), for everything in this repository, the launcher and the RE, and for our commits
in the engine fork. Surreal Engine has its own licences, in its `LICENSE.md`.

Deus Ex belongs to its owners; this project is not affiliated with or endorsed by them.
