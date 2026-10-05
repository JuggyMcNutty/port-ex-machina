# Working on the project

How to work here, whatever the task. Where things stand and what is next is
[`../agent.md`](../agent.md). Working on the engine itself -- temporary debug
hooks, scripted runs of both engines, its gotchas -- is VibeEngine's
[`vibe/docs/DEVELOPMENT.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md).

## The base: linux-x86_64

Development happens on [linux-x86_64](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/ports/linux-x86_64/README.md): the
unit tests, `dxl-shots`, the engine's validation and the side-by-side checks of
engine changes all build and run there. Every other port starts from it
([`PORTING.md`](PORTING.md)). A change is clean when it builds and tests there
*and* builds warning-free for each cross port that ships -- the Smart Pro's GCC
9.3 is stricter about `-Wshadow` and `-Wformat-truncation` than a current host
compiler.

## The repositories

Each is a folder of its own in one parent folder, none inside another, beside
what none of them owns: `gamefiles/`, `reference/`, `deps/` and `build/`
([the README's layout](../README.md#layout)). This one is the workspace: its
scripts find the others as its siblings (`DX_ROOT` in `scripts/lib/common.sh`
is the parent), and `dx.sh fetch` clones them there.

| Folder | Repository | Kept at |
|---|---|---|
| `port-ex-machina/` | this one, the workspace | -- |
| `deusex-launcher/main`, and `deusex-launcher/<port>` for each port | [deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher): `main` the original recreated, a branch per port | `LAUNCHER-PIN.txt`: each port's commit; `scripts/launcher.sh` fetches, checks and pins them |
| `VibeEngine/` | [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine), branch `deusex` | `ENGINE-PIN.txt`: one commit; `scripts/engine.sh` fetches, checks and pins it |
| `dx-reverse-info/` | [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info) | not pinned: docs and IDA scripts |

`deusex-launcher/main` is the clone; each port is a worktree of it, on the
branch of its name, with `deps/` and `gamefiles/` linked in. The worktrees are
linked to the clone by relative paths (git 2.48 or later), so the parent
folder can move; worktrees made with absolute links stop working when it does
(`git -C deusex-launcher/main worktree repair --relative-paths` mends them). Its launcher
builds inside it (`deusex-launcher/<port>/build/`); the engine and the staged
app go to `build/<port>/`. A change to the launcher or the engine is
committed in its repository, pushed, then pinned here (`launcher.sh pin
<port>`, `engine.sh pin`), and the pin committed with whatever depends on it.

## Cold start

Nothing depends on state from a previous session. In this repository, cloned
into a folder of its own ([the README's quick start](../README.md#quick-start)):

```sh
scripts/dx.sh fetch            # the engine, the launcher's branches and the RE, beside it
scripts/dx.sh deps <port>      # toolchains and sysroots (the Smart Pro's needs the device awake)
scripts/dx.sh build <port>     # then stage and run -- or deploy, which builds and stages first
scripts/dx.sh test [<port>]    # unit tests: linux-x86_64's, or a device branch's host build
scripts/dx.sh check            # the drift guards
```

For engine work, `VibeEngine/vibe/tools/host-tools.sh` unpacks `perf` and the
Vulkan validation layer.

### The recreation

The launcher's `main` -- the original `DeusEx.exe` recreated, not a port --
is built, installed and run by `scripts/recreation.sh`, with the engine
linux-x86_64 builds: `build` puts main in `build/main/launcher` and builds the
engine; `install [<GameDir>]` puts `DeusEx`, its `run-game.sh` and the
engine's three files in the game's `System/`, beside `DeusEx.exe`
(`gamefiles/` by default); `check` runs main's `tools/livecheck.py` on a copy
of that install, on a private X display, so nothing shows on the desktop and
the install is not written; `run` starts it; `uninstall` takes the five files
out again. While `DeusEx` is installed the original game does not start under
Wine or Proton -- it takes the file for the `DeusEx` package
([a package's file](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/core-dll.md#packages-and-linkers)) -- so uninstall
before playing the original from that folder. Main is not pinned: it is the clone, and the ports take it by
merging ([the launcher's README](https://github.com/JuggyMcNutty/deusex-launcher#branches)).

## This machine

Since 2026-10-04 Claude runs in a new Arch Linux distrobox,
`ai_dev_container` (rootless podman), on the same Fedora Atomic host, with a
home of its own, `/var/home/corpeder/containers/homes`: the parent folder is
`~/Documents/projects/projects/deusex` there, and Claude Code starts in
`port-ex-machina/`. IDA is IDA Pro 9.4 for Linux (`~/ida-pro-9.4`), and
Claude Code reaches it through Hex-Rays' own IDA MCP server, the plugin
`ida-mcp@HexRaysSA`, installed for the user on 2026-10-05 as the machine's pi
harness has it ([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).

Set up 2026-10-05, with pacman's multilib enabled (`/etc/pacman.conf`, the
original kept as `pacman.conf.pre-multilib`): to build, `base-devel`,
CMake, Ninja, `pkgconf`, `sdl2-compat`, `sdl2_ttf`, `sdl3`, OpenAL,
`libunwind`, `waylandpp`, `vulkan-headers` and `vulkan-tools`; to run,
Mesa with `vulkan-radeon` (the GPU is a Radeon 780M, `radv` and
`radeonsi`), `libpulse` and `libpipewire` (the desktop's PipeWire is
reachable) and `ttf-dejavu`; for the harness and `recreation.sh check`,
`xorg-server-xvfb`, `xdotool`, ImageMagick and gdb; for deploys, `sshpass`;
for the original under Proton, the 32-bit X11, Mesa, Vulkan, PulseAudio,
FreeType, fontconfig, GLib and GnuTLS libraries. Proton-CachyOS 11.0
(2026-10-05) is in `~/.local/share/Steam/compatibilitytools.d`, linked as
`Proton-CachyOS Latest`, the name `dxcap.sh` looks for; the user Python has
`unicorn`, the CPU emulator [`Fire.dll`'s check](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/fire-dll.md#how-it-was-checked)
ran the DLL's routines in. What came over with the folders and held the old
one's absolute paths was remade, not moved: the CMake build directories
([below](#gotchas-that-cost-time)), `build/dxcap`'s links
(`VibeEngine/vibe/tools/dxcap.sh setup`, then `compile`) and the Vulkan
validation layer's manifest (`VibeEngine/vibe/tools/host-tools.sh`). The
per-user engine settings, `~/.config/SurrealEngine/Settings.json`, which the
harness's fork runs take their renderer from, were made anew naming OpenGL, as
the previous container's did: a hidden run's Xvfb has no Vulkan present.
Checked: every port builds warning-free but for one third-party `#warning` a
clean Smart Pro build shows (agent.md's known defects), every branch's unit
tests pass, the hidden proving run on Liberty Island is clean, and
`recreation.sh check` passes its 35 checks.

For pushes (the remotes are `git@github.com:`), an SSH key is made,
`~/.ssh/id_ed25519` (2026-10-05, no passphrase), with GitHub's host key in
`~/.ssh/known_hosts`; its public half is on the owner's GitHub account
(2026-10-05), and pushes from here work. Not tried yet: a deploy, which wants the Smart Pro
awake on the network.

The previous container (to 2026-10-04) had the host's home -- `/home` there,
`/var/home` on the host -- with the parent folder at
`~/Documents/projects/deusex`, where Claude Code started and its `ida` MCP
server was registered. With `libpipewire`/`libpulse` the engine reached the
desktop's audio; the null OpenAL driver remains the way to run silently
([linux-x86_64's README](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/ports/linux-x86_64/README.md#audio)).

## Commits

- **Commit as JuggyMcNutty** (`11588877+JuggyMcNutty@users.noreply.github.com`),
  never the machine's global git identity, which is a personal address. Every
  checkout here sets it in its local git config, and the fetches set it in a
  fresh clone (`dx.sh fetch`, `engine.sh fetch`, `launcher.sh fetch`); a clone
  made any other way needs it before its first commit.
- **Never commit the game's files** -- not an ini, not a `.int`. The
  repositories are public; the launcher's `tests/fixtures` are written
  stand-ins, and `test_gamefiles` reads the real ones from `gamefiles/` in
  place.
- A change and the docs it affects go in the same commit, in the repository
  the change is in; the pin that follows it here, with the docs of this one.

## Docs

Each fact lives in one place, and everything else links to it; which doc holds
what is the [table in the README](../README.md#documentation). Between
repositories a link is absolute: to a branch's copy on GitHub
(`blob/<branch>/<path>`) -- `main` of this repository and the RE, `deusex` of
the engine, the port's branch of the launcher. Keep volatile details out --
commit counts and lists, which the pins' checks and git already know.
**Editing docs with string replacement fails silently** when the pattern does
not match (one README edit was reported done in a commit message and had not
happened): prefer full rewrites or scripted replacements that assert the
pattern was found.

### Drift guards

Docs and scripts that describe the tree are checked against it, so a move or a
rename fails loudly instead of leaving stale instructions:

- `scripts/check-docs.sh`: across the workspace's repositories, every path a
  doc names in backticks or links must exist, every link to a heading
  (`#anchor`) must find one, and every link into another of the repositories
  must resolve in that repository's checkout here. A checkout git cannot read
  fails rather than going unchecked.
- `scripts/engine.sh check`: the engine clone is at the commit
  `ENGINE-PIN.txt` names, on the fork's branch.
- `scripts/launcher.sh check`: each port's worktree is at the commit
  `LAUNCHER-PIN.txt` names, on its branch (and git can read it).
- `test_target_<port>` (the id's `-` as `_`), on the port's branch: each CPU
  mode a profile offers is named in its `port-hooks.sh` -- anywhere in the
  file, a comment included, so it does not prove a `case` handles it -- the
  hooks' fallback (`CPU_MODE_DEFAULT`) is the profile's default, and the
  profile's id is its port's.
- `scripts/dx.sh check` runs the first three, confirms every port has its
  required files, and checks the glibc ceiling of whatever is staged.

## Gotchas that cost time

Device-specific ones are in each port's README, the engine's in its
[`vibe/docs/DEVELOPMENT.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#gotchas).
These apply everywhere:

- **A binary built against a newer glibc than the device's will not load.**
  glibc 2.34 re-versioned the startup symbols; one `GLIBC_2.34` reference is
  enough. Every cross port sets a ceiling (`DXL_PORT_GLIBC_MAX`) that its
  branch's `scripts/check-abi.sh` enforces after each build.
- **Surreal Engine does not read what the original launcher wrote**, and after
  its first clean exit it reads `SE-DeusEx.ini`/`SE-User.ini`, not
  `DeusEx.ini`/`User.ini`; a stub `DeusEx.ini` kills it
  ([where the settings actually live](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md#where-the-settings-actually-live)).
- **Never `pkill -f` or `pgrep -f <pattern>`** in a command whose own text
  contains the pattern -- it matches the shell running it (this killed the
  session's shell twice, and a `pgrep -f` guard once found "a server still
  running" in itself and started none). Use `pidof`, `pgrep -x`, or bracket
  a letter (`pgrep -f "[S]urrealEngine --no-launcher"`).
- **Deus Ex's UnrealScript source is embedded in `System/DeusEx.u`**: search it
  before guessing what the game's script does
  ([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).
- **CMake build directories cannot move.** Their caches hold absolute paths;
  after moving a tree, delete its build directory and rebuild. So do
  `build/dxcap`'s links and the validation layer's manifest in `deps/`: after
  moving the parent folder, run `dxcap.sh setup` and `host-tools.sh` again.
- **`build` does not restage.** The staged app keeps whatever binary the last
  `stage` copied; a run after `build` alone tests the old engine, which shows
  up as long-fixed `Unimplemented` lines in its log (a three-day-old staged
  binary cost a round of confusion, 2026-09-25). Stage before running -- and
  never while the engine still runs: the copy fails with `Text file busy` and
  the old binary runs again.
