# Working on the project

How to work here, whatever the task. The rules, where things stand and what is open:
[`../AGENTS.md`](../AGENTS.md). Working on the engine itself -- scripted runs of both engines,
temporary debug hooks, its gotchas -- is VibeEngine's
[`vibe/docs/DEVELOPMENT.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md).

## The base: linux-x86_64

Development happens on
[linux-x86_64](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/ports/linux-x86_64/README.md):
the unit tests, `dxl-shots`, the engine's validation and the side-by-side checks of engine
changes all build and run there. Every other port starts from it ([`PORTING.md`](PORTING.md)).
A change is clean when it builds and tests there *and* builds warning-free for each cross port
that ships: the Smart Pro's GCC 9.3 is stricter about `-Wshadow` and `-Wformat-truncation` than
a current host compiler. The one standing warning is third-party (AGENTS.md's open items).

## The repositories

Each is a folder of its own in one parent folder, none inside another
([the README's layout](../README.md#layout)). This one is the workspace: its scripts find the
others as its siblings (`DX_ROOT` in `scripts/lib/common.sh` is the parent), and `dx.sh fetch`
clones them there. The commands: [the README's quick start](../README.md#quick-start).

| Folder | Repository | Kept at |
|---|---|---|
| `port-ex-machina/` | this one, the workspace | -- |
| `deusex-launcher/main`, and `deusex-launcher/<port>` for each port | [deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher): `main` the original recreated, a branch per port | `LAUNCHER-PIN.txt`: each port's commit; `scripts/launcher.sh` fetches, checks and pins them |
| `VibeEngine/` | [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine), branch `deusex` | `ENGINE-PIN.txt`: one commit; `scripts/engine.sh` fetches, checks and pins it |
| `dx-reverse-info/` | [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info) | not pinned: docs and IDA scripts |

`deusex-launcher/main` is the clone; each port is a worktree of it, on the branch of its name,
with `deps/` and `gamefiles/` linked in. The worktrees link to the clone by relative paths (git
2.48 or later), so the parent folder can move; a worktree with absolute links breaks when it
does (`git -C deusex-launcher/main worktree repair --relative-paths` mends it). A port's
launcher builds inside its worktree (`deusex-launcher/<port>/build/`); the engine and the staged
app go to `build/<port>/`. A change to the launcher or the engine is committed in its
repository, pushed, then pinned here (`launcher.sh pin <port>`, `engine.sh pin`), and the pin
committed with whatever depends on it.

Beside the repositories, in no repository:

- `gamefiles/`: your Deus Ex install (1112fm), and beside its binaries the IDA databases
  (`System/*.i64`). They hold the game's code, so they are never committed; a fresh database gets
  its types, strings and names from dx-reverse-info's scripts
  ([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).
- `reference/`: the 1112f SDK, the DeusExe launcher source, the IDA and ini backups, the
  original's saves and its wizard's captures.
- `deps/` and `build/`, which the scripts remake. For engine work,
  `VibeEngine/vibe/tools/host-tools.sh` unpacks `perf` and the Vulkan validation layer into
  `deps/`.

### The recreation

The launcher's `main` -- the original `DeusEx.exe` recreated, not a port -- is built, installed
and run by `scripts/recreation.sh`, with the engine linux-x86_64 builds:

- `build`: main into `build/main/launcher`, and the engine;
- `install [<GameDir>]`: `DeusEx`, its `run-game.sh` and the engine's three files into the
  game's `System/`, beside `DeusEx.exe` (`gamefiles/` by default);
- `check`: main's `tools/livecheck.py` on a copy of that install, on a private X display, so
  nothing shows on the desktop and the install is not written;
- `run`: starts it; `uninstall`: takes the five files out again.

While `DeusEx` is installed, the original game does not start from that folder under Wine or
Proton: it takes the file for the `DeusEx` package
([a package's file](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/core-dll.md#packages-and-linkers)).
Uninstall before playing the original there. `main` is not pinned: it is the clone, and the
ports take it by merging ([the launcher's README](https://github.com/JuggyMcNutty/deusex-launcher#branches)).

## Dependencies

Development runs in an Arch Linux container (a distrobox) that holds all of these; the original
game and the harness run inside it, never on the host outside it. Its packages, with pacman's
multilib repository enabled:

| For | Packages |
|---|---|
| building | `base-devel`, CMake, Ninja, `pkgconf`, `sdl2-compat`, `sdl2_ttf`, `sdl3`, OpenAL, `libunwind`, `waylandpp`, `vulkan-headers`, `vulkan-tools` |
| running | Mesa with the GPU's Vulkan driver, `libpulse` and `libpipewire` (the desktop's audio), `ttf-dejavu` |
| the harness and `recreation.sh check` | `xorg-server-xvfb`, `xdotool`, ImageMagick, gdb, Python 3 |
| deploying | `sshpass` |
| the original under Proton | the 32-bit X11, Mesa, Vulkan, PulseAudio, FreeType, fontconfig, GLib and GnuTLS libraries |

Besides:

- **Proton**, to run the original: `dxcap.sh` looks for a build linked as
  `Proton-CachyOS Latest` in `~/.local/share/Steam/compatibilitytools.d`.
- **`~/.config/SurrealEngine/Settings.json` naming OpenGL**: the harness's fork runs take their
  renderer from it, and a hidden run's Xvfb has no Vulkan present.
- **Python's `unicorn`**, the CPU emulator
  [`Fire.dll`'s check](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/fire-dll.md#how-it-was-checked)
  runs the DLL's routines in.
- **IDA Pro for Linux** with Hex-Rays' MCP plugin `ida-mcp@HexRaysSA`, for the RE
  ([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).
- **Write access to the owner's GitHub repositories**, to push. The fetches clone over HTTPS,
  read-only; pushing takes `git@github.com:` remotes and an SSH key on the owner's account.
- **The Smart Pro awake on the network**, for its `deps` (the sysroot comes from the device)
  and `deploy`.

## Commits

- **Commit as JuggyMcNutty** (`11588877+JuggyMcNutty@users.noreply.github.com`), never the
  machine's global git identity, which is a personal address. Every checkout here sets it in its
  local git config, and the fetches set it in a fresh clone (`dx.sh fetch`, `engine.sh fetch`,
  `launcher.sh fetch`); a clone made any other way needs it before its first commit.
- **Never commit the game's files** -- not an ini, not a `.int`. The repositories are public; the
  launcher's `tests/fixtures` are written stand-ins, and `test_gamefiles` reads the real ones
  from `gamefiles/` in place.
- A change and the docs it affects go in the same commit, in the repository the change is in;
  the pin that follows it here, with the docs of this one.

## Docs

Each fact lives in one place, and everything else links to it; which doc holds what is the
[table in the README](../README.md#documentation). Every doc in the four repositories is
written this way:

1. **The current state, in the present tense.** No dates on facts; no "was", "once", "since" or
   "no longer"; no story of how something was found; no run IDs; no superseded numbers. Git and
   the commit messages are the record. A table of measurements may name the engine commit it
   was measured at.
2. **A why only where it stops someone undoing a deliberate choice.**
3. **Short sentences, lists and tables.** Plain words over chains of possessives.
4. **Between repositories a link is absolute**, to a branch's copy on GitHub
   (`blob/<branch>/<path>`): `main` of this repository and the RE, `deusex` of the engine, the
   port's branch of the launcher.
5. **No volatile details** that the pins' checks and git already know, such as commit counts and
   lists.
6. **A plan is deleted when its work is done**; what lasts of it moves to the doc that owns it,
   in the same commit.
7. **`AGENTS.md` is edited in place**, never appended to, and stays within 150 lines.
8. **A heading other docs link to keeps its words**; renaming one means fixing every link to it.

Editing docs with string replacement fails silently when the pattern does not match: prefer full
rewrites, or scripted replacements that assert the pattern was found.

### Drift guards

Docs and scripts that describe the tree are checked against it, so a move or a rename fails
loudly instead of leaving stale instructions:

- `scripts/check-docs.sh`: across the workspace's repositories, every path a doc names in
  backticks or links must exist, every link to a heading (`#anchor`) must find one, and every
  link into another of the repositories must resolve in that repository's checkout here. A
  checkout git cannot read fails rather than going unchecked, and so does an `AGENTS.md` over
  its budget.
- `scripts/engine.sh check`: the engine clone is at the commit `ENGINE-PIN.txt` names, on the
  fork's branch.
- `scripts/launcher.sh check`: each port's worktree is at the commit `LAUNCHER-PIN.txt` names,
  on its branch (and git can read it).
- `test_target_<port>` (the id's `-` as `_`), on the port's branch: each CPU mode a profile
  offers is named in its `port-hooks.sh` -- anywhere in the file, a comment included, so it does
  not prove a `case` handles it -- the hooks' fallback (`CPU_MODE_DEFAULT`) is the profile's
  default, and the profile's id is its port's.
- `scripts/dx.sh check` runs the first three, confirms every port has its required files, and
  checks the glibc ceiling of whatever is staged.

## Gotchas that cost time

Device-specific ones are in each port's README, the engine's in its
[`vibe/docs/DEVELOPMENT.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#gotchas).
These apply everywhere:

- **A binary built against a newer glibc than the device's will not load.** glibc 2.34
  re-versioned the startup symbols; one `GLIBC_2.34` reference is enough. Every cross port sets a
  ceiling (`DXL_PORT_GLIBC_MAX`) that its branch's `scripts/check-abi.sh` enforces after each
  build.
- **Surreal Engine does not read what the original launcher wrote**, and after its first clean
  exit it reads `SE-DeusEx.ini`/`SE-User.ini`, not `DeusEx.ini`/`User.ini`; a stub `DeusEx.ini`
  kills it
  ([where the settings actually live](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md#where-the-settings-actually-live)).
- **Never `pkill -f` or `pgrep -f <pattern>`** in a command whose own text contains the pattern:
  it matches the shell running it. Use `pidof`, `pgrep -x`, or bracket a letter
  (`pgrep -f "[S]urrealEngine --no-launcher"`).
- **Deus Ex's UnrealScript source is embedded in `System/DeusEx.u`**: search it before guessing
  what the game's script does
  ([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).
- **CMake build directories cannot move.** Their caches hold absolute paths; after moving a
  tree, delete its build directory and rebuild. So do `build/dxcap`'s links and the validation
  layer's manifest in `deps/`: after moving the parent folder, run `dxcap.sh setup` (then
  `compile`) and `host-tools.sh` again.
- **`build` does not restage.** The staged app keeps whatever binary the last `stage` copied, so
  a run after `build` alone tests the old engine (its log shows long-fixed `Unimplemented`
  lines). Stage before running -- and never while the engine still runs: the copy fails with
  `Text file busy` and the old binary runs again.
