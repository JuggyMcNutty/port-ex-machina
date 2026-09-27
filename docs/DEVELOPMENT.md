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

This one is the workspace; the other three are checked out inside it, each
ignored by it (as are `build/`, `deps/`, `gamefiles/` and `reference/`):

| Checkout | Repository | Kept at |
|---|---|---|
| `launcher/main`, and `launcher/<port>` for each port | [deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher): `main` the original recreated, a branch per port | `LAUNCHER-PIN.txt`: each port's commit; `scripts/launcher.sh` fetches, checks and pins them |
| `engine/SurrealEngine` | [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine), branch `deusex` | `ENGINE-PIN.txt`: one commit; `scripts/engine.sh` fetches, checks and pins it |
| `re/` | [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info) | not pinned: docs and IDA scripts |

`launcher/main` is the clone; each port is a worktree of it, on the branch of
its name, with the workspace's `deps/` and `gamefiles/` linked in. Its launcher
builds inside it (`launcher/<port>/build/`); the engine and the staged app go
to the workspace's `build/<port>/`. A change to the launcher or the engine is
committed in its repository, pushed, then pinned here (`launcher.sh pin
<port>`, `engine.sh pin`), and the pin committed with whatever depends on it.

## Cold start

Nothing depends on state from a previous session:

```sh
scripts/dx.sh fetch            # the engine, the launcher's branches and the RE
scripts/dx.sh deps <port>      # toolchains and sysroots (the Smart Pro's needs the device awake)
scripts/dx.sh build <port>     # then stage and run -- or deploy, which builds and stages first
scripts/dx.sh test [<port>]    # unit tests: linux-x86_64's, or a device branch's host build
scripts/dx.sh check            # the drift guards
```

For engine work, `engine/SurrealEngine/vibe/tools/host-tools.sh` unpacks `perf`
and the Vulkan validation layer.

## This machine

Claude runs in an Arch Linux distrobox on a Fedora Atomic host: `/home` here is
`/var/home` there, and paths configured in one differ from the other (the old
CMake caches, the engine's embedded source paths). The container has
`libpipewire`/`libpulse` (installed 2026-09-25, for M5's audio work), so the
engine reaches the desktop's audio -- a test run is audible on the owner's
speakers as well as visible on their screen. The null OpenAL driver remains
the way to run silently
([linux-x86_64's README](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/ports/linux-x86_64/README.md#audio)). Its pacman
has multilib (enabled 2026-09-26) and the 32-bit libraries the original needs
under Proton's wine: X11, Mesa, PulseAudio, FreeType, GLib and theirs.

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
  must resolve in that repository's checkout here.
- `scripts/engine.sh check`: the engine clone is at the commit
  `ENGINE-PIN.txt` names, on the fork's branch.
- `scripts/launcher.sh check`: each port's worktree is at the commit
  `LAUNCHER-PIN.txt` names, on its branch.
- `test_target_<port>`, on the port's branch: each CPU mode a profile offers is
  handled by its `port-hooks.sh`, the hooks' fallback (`CPU_MODE_DEFAULT`) is
  the profile's default, and the profile's id is its port's.
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
  after moving a tree, delete its build directory and rebuild.
- **`build` does not restage.** The staged app keeps whatever binary the last
  `stage` copied; a run after `build` alone tests the old engine, which shows
  up as long-fixed `Unimplemented` lines in its log (a three-day-old staged
  binary cost a round of confusion, 2026-09-25). Stage before running -- and
  never while the engine still runs: the copy fails with `Text file busy` and
  the old binary runs again.
