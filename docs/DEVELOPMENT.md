# Working on the project

How to work here, whatever the task. The rules, where things stand and what is open:
[`../AGENTS.md`](../AGENTS.md). Working on the engine itself -- scripted runs of both engines,
temporary debug hooks, its gotchas -- is DXEngine's
[`docs/DEVELOPMENT.md`](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/DEVELOPMENT.md).

## The development platform: linux-x86_64

Development happens on
[linux-x86_64](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/ports/linux-x86_64/README.md).
A change is clean when it builds and tests there *and* builds warning-free for each cross port
that ships: the Smart Pro and linux-aarch64 build with the same GCC 13.2, but code linux-x86_64
does not (the engine's SDL2 backend and the software device's NEON lanes, each port's own
files); windows-x86_64 with Clang, code linux-x86_64 does not either (`src/platform/win32/`, the
engine's Win32 window and D3D11), its tests passing under Wine.

## The repositories

Each is a folder of its own in one parent folder ([the layout](../README.md#layout)). This one
is the workspace: its scripts find the others as its siblings (`DX_ROOT` in
`scripts/lib/common.sh` is the parent), and `dx.sh fetch` clones them there. The engine is
pinned at one commit (`ENGINE-PIN.txt`; `scripts/engine.sh` fetches, checks, builds and pins
it), each port of the launcher at its own (`LAUNCHER-PIN.txt`; `scripts/launcher.sh`), the RE
not at all.

`deusex-launcher/main` is the clone; each port is a worktree of it, on the branch of its name,
with `deps/` and `gamefiles/` linked in. The worktrees link to the clone by relative paths (git
2.48 or later), so the parent folder can move; a worktree with absolute links breaks when it
does (`git -C deusex-launcher/main worktree repair --relative-paths` mends it). A port's
launcher builds inside its worktree (`deusex-launcher/<port>/build/`); the engine and the staged
app go to `build/<port>/`. A change to the launcher or the engine is committed in its
repository, pushed, then pinned here (`launcher.sh pin <port>`, `engine.sh pin`), and the pin
committed with whatever depends on it.

Beside the repositories, in no repository ([the layout](../README.md#layout)): `gamefiles/`
holds the IDA databases beside the game's binaries (`System/*.i64`), never committed, a fresh
one given its types, strings and names by dx-reverse-info's scripts
([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries));
`reference/DXD3D9_2.0` is a third-party Direct3D 9 renderer for the original, a closed DLL built
on Chris Dohnal's UTD3D9 1.3, whose source carries Epic's copyright and no licence: its ideas
may be taken, never its code; `deps/` and `build/` the scripts remake, and for engine work
`DXEngine/tools/host-tools.sh` unpacks `perf` and the Vulkan validation layer into
`deps/`.

The launcher's `main` -- the original `DeusEx.exe` recreated, not a port -- is built, installed,
checked and run by `scripts/recreation.sh` (its header has the commands) into the game's
`System/`, with the engine linux-x86_64 builds; while it is there the original does not start
under Wine or Proton
([installing](https://github.com/JuggyMcNutty/deusex-launcher/blob/main/README.md#installing)).
`main` is not pinned: it is the clone, and no port branch takes it by merging
([the branches](PORTING.md#the-branches)).

## Dependencies

Development runs in an Arch Linux container (a distrobox) that holds all of these; the original
game and the harness run inside it, never on the host outside it. Its packages, with pacman's
multilib repository enabled:

| For | Packages |
|---|---|
| building | `base-devel`, CMake, Ninja, `pkgconf`, `sdl2-compat` (with `sdl3` under it), `sdl2_ttf`, `sdl3_ttf` and `libxmp` (the desktop launcher's window), OpenAL, `waylandpp`, `vulkan-headers`, `vulkan-tools` |
| running | Mesa with the GPU's Vulkan driver, `libpulse` and `libpipewire` (the desktop's audio), `ttf-dejavu` |
| the harness and `recreation.sh check` | `xdotool`, ImageMagick, gdb, Python 3; MangoHud (`mangohud`, `lib32-mangohud`) for the harness's `DXCAP_FPS`; `vulkan-swrast` (Mesa's lavapipe) for the software device against Mesa's CPU drivers ([the software device](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/DEVELOPMENT.md#the-software-device)); `xorg-server-xvfb` for the recreation's private display |
| deploying | `sshpass` |
| the original under Proton | the 32-bit X11, Mesa, Vulkan, PulseAudio, FreeType, fontconfig, GLib and GnuTLS libraries |

Besides:

- **Proton**, to run the original and a Windows port's programs and tests (`scripts/wine.sh`),
  and **`~/.config/DXEngine/Settings.json`**, which the harness's dxe runs take their renderer
  from where `DXCAP_SETTINGS` names no other
  ([scripted runs](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/DEVELOPMENT.md#scripted-runs-of-both-engines)),
  as the recreation's runs do, its `Type` before the Renderer page's choice
  ([which device draws](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/ENGINE.md#rendering)).
  The desktop launcher never writes it: it gives the engine a folder of its own for each
  profile (`--userdir`), and refuses an engine that has none.
- **IDA Pro for Linux** with Hex-Rays' MCP plugin `ida-mcp@HexRaysSA`, for the RE
  ([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).
- **Access to the owner's GitHub repositories.** dx-reverse-info is public; port-ex-machina,
  DXEngine and deusex-launcher are private until the owner releases the work, so the fetches'
  HTTPS clones of them need the owner's credentials (a credential helper). Pushing takes
  `git@github.com:` remotes and an SSH key on the owner's account.
- **The Smart Pro awake on the network**, for its `deps` (the sysroot comes from the device)
  and `deploy`.

## Commits

- **As JuggyMcNutty** ([the rule](../AGENTS.md#rules)), never the machine's global git identity,
  which is a personal address: every checkout here sets it in its local git config, and the
  fetches set it in a fresh clone (`dx.sh fetch`, `engine.sh fetch`, `launcher.sh fetch`); a
  clone made any other way needs it before its first commit.
- **No game files** ([the rule](../AGENTS.md#rules)): the launcher's `tests/fixtures` are
  written stand-ins, and `test_gamefiles` reads the real ones from `gamefiles/` in place.
- A change and the docs it affects go in the same commit, in the repository the change is in;
  the pin that follows it here, with the docs of this one.

## Docs

Each fact lives in one place, and everything else links to it; which doc holds what is the
[table in the README](../README.md#documentation). Every doc in the four repositories is
written this way:

1. **The current state, in the present tense.** No dates on facts; no "was", "once", "since" or
   "no longer"; no story of how something was found; no run IDs; no superseded numbers. Git and
   the commit messages are the record.
2. **A why only where it stops someone undoing a deliberate choice.**
3. **Short sentences, lists and tables.** Plain words over chains of possessives.
4. **Between repositories a link is absolute**, to a branch's copy on GitHub
   (`blob/<branch>/<path>`): `main` of this repository, the RE and the engine, the launcher's
   `main` or the port branch that holds the fact.
5. **No volatile details** that the pins' checks and git already know, such as commit counts and
   lists.
6. **A plan is deleted when its work is done**; what lasts of it moves to the doc that owns it,
   in the same commit.
7. **`AGENTS.md` is edited in place**, never appended to, and kept small: every session loads
   it.
8. **A heading other docs link to keeps its words**; renaming one means fixing every link to it.
9. **A measurement lives in a table that names the engine commit it was taken at**, never in
   prose.
10. **One wrap width per file, 100 at most**; a bare link may run over.
11. **A README's screenshots** are in its repository's `docs/screenshots/`, the one exception
    to the rule against committing the game's files ([the rule](../AGENTS.md#rules)): the
    game's scenes as JPEG, the launcher's screens as PNG, each a few hundred KB at most, from
    real runs -- DXEngine's `shot` on a harness timeline
    ([timelines](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/DEVELOPMENT.md#timelines)),
    a launcher's `shot` in a `DXL_UI_SCRIPT` -- and taken again when what they show changes,
    with every copy: this repository's README shows copies of DXEngine's and the launcher's,
    and a shot of its own.

Editing docs with string replacement fails silently when the pattern does not match: prefer full
rewrites, or scripted replacements that assert the pattern was found.

## Drift guards

Docs and scripts that describe the tree are checked against it, so a move or a rename fails
loudly instead of leaving stale instructions:

- `scripts/check-docs.sh`: across the workspace's repositories, every path a doc names in
  backticks or links must exist, every link to a heading (`#anchor`) must find one, and every
  link into another of the repositories must name a branch the workspace has (the launcher's
  `main` and the ports `LAUNCHER-PIN.txt` pins, `main` elsewhere) and
  resolve in that repository's checkout here. A checkout git cannot read fails rather than
  going unchecked, and so does a missing `AGENTS.md`.
- `scripts/engine.sh check`: the engine clone is at the commit `ENGINE-PIN.txt` names, on its
  branch.
- `scripts/launcher.sh check`: each port's worktree is at the commit `LAUNCHER-PIN.txt` names,
  on its branch (and git can read it).
- `test_cpumode`, on trimui-smartpro: the Smart Pro's game hook, `port-hooks.sh`, run against
  stand-in spruceOS helpers: each CPU mode the launcher offers (`src/core/cpumode.c`) runs its
  own helper there, and one the hook does not know takes the launcher's default.
- `scripts/dx.sh check` runs the first three, confirms every port has its required files,
  checks the glibc ceiling of whatever is staged (a Windows port's floor and the DLLs its
  programs import), and checks that a port's list of the game's builds (`src/core/gamedb.c`,
  DXLauncher's) is the engine's (`UE1GameDatabase.h`).

## CI

GitHub's CI builds the ports where the pins move: a push to this repository's `main` builds
every port at the commits its pins name, when code changed, and publishes them as a release.
DXEngine and the launcher's branches run the same jobs on a pull request, or by hand from their
Actions tab, never on a push: a round pushes them before the pins, and a run of theirs would
build against the old ones.

| Workflow | Runs on | Builds |
|---|---|---|
| this repository's `.github/workflows/build.yml` | a push to `main`, a pull request into it, by hand | every pinned port, then the release |
| DXEngine's `.github/workflows/build.yml` | a pull request into `main`, by hand | every pinned port, the engine at the pull request's commit |
| each launcher branch's `.github/workflows/build.yml`, the same file on every branch | a pull request into the branch, by hand | the branch's port; on `main`, the recreation |

Each calls this repository's `.github/workflows/ports.yml`, every step of which is a script
here, so a run can be repeated on this machine:

- `scripts/ci.sh plan` decides once what a run builds, and at which commits: the caller's own
  repository at the event's, the others at the pins.
- A port's job checks the repositories out as the [layout](../README.md#layout) has them,
  installs what the runner lacks (`scripts/ci-apt-install.sh`, apt bounded in time), runs
  `scripts/dx.sh deps`, `build`, `stage` and `test`, and packs the staged app
  (`scripts/ci.sh package`). Windows' tests run under the Proton build this machine's Steam
  has, pinned in `scripts/ci.sh`, on an Xvfb display; the Smart Pro's libraries come from
  TrimUI's SDK
  ([the toolchains](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#the-toolchains)).
- A port's job fails on a compiler or linker warning in our own code, third-party code exempt
  (`scripts/ci.sh warnings`), and on a skipped test but `test_gamefiles` and `test_ui_scripts`,
  which need the game (`scripts/ci.sh skips`). A failed test is named on the run's page, which
  anyone can read, where its log needs a GitHub account (`scripts/ci.sh failed`).
- The recreation's job is `scripts/recreation.sh test`: its build, with the runner's compiler
  and SDL2, and its tests.

**What builds nothing**: a change to docs alone -- `*.md` and `docs/`, but not the notices an
app carries, the launcher's `THIRD-PARTY.md` and the engine's `LICENSE.md` -- or to what no
build reads (the engine's harness, its `tools/perf/`, `tools/host-tools.sh` and
`tools/natives_audit.py`; the workspace's `LICENSE`, `tools/probes/` and `scripts/check-docs.sh`)
starts no run, and
a push of pins whose commits change only those builds nothing. The list is `scripts/ci.sh`'s
`is_code`; each workflow's `paths` filter says the same.

**Releases**, each with notes naming the commits built and linking what changed since the one
before it (`scripts/ci.sh publish`):

| Repository | Tagged | Holds, for each port | When |
|---|---|---|---|
| this one | `<date>-<sha8>`, its commit | `DX-<port>`: the app, launcher and engine | every run of the pins that builds |
| DXEngine | `<date>-<sha8>`, the engine's commit | `DXEngine-<port>`: the engine alone, its notices with it | when its code changed since its last |
| deusex-launcher, one per port | `<port>-<date>-<sha8>`, the branch's commit | `DXLauncher-<port>`: the launcher alone | when that branch's code changed since its last |

Each file is a tarball, or a zip for the Windows port and the Smart Pro (the Smart Pro's app
folder `DeusEx`, as spruceOS takes it); `scripts/ci.sh package` makes all three from the staged
app, and they are each run's artifact too, kept two days (a private repository's artifacts
share a small quota; the releases keep the files), a pull request's among them.

**The repositories being private**, a run reads its own with GitHub's token and the other two
with `RELEASES_TOKEN`, which also makes DXEngine's and deusex-launcher's releases: a
fine-grained token of the owner's with Contents read and write on all three, a secret of each
(this repository's for the pins' runs, the others' for their own), and port-ex-machina's
Actions open to the owner's other repositories, which call its `ports.yml`. Without the token a
run cannot read the other two and fails at their checkout; when it expires, a new one replaces
the secrets. The releases are the owner's alone to see until the repositories are made public.
Here, `scripts/ci.sh`'s `plan` and `publish` reach them over SSH:
`DX_OWNER_GIT=git@github.com:JuggyMcNutty`.

## Gotchas that cost time

Device-specific ones are in each port's README, the engine's in its
[`docs/DEVELOPMENT.md`](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/DEVELOPMENT.md#gotchas).
These apply everywhere:

- **A binary built against a newer glibc than the device's will not load.** glibc 2.34
  re-versioned the startup symbols; one `GLIBC_2.34` reference is enough. Every Linux port sets
  a ceiling (`DXL_PORT_GLIBC_MAX`) that its branch's `scripts/check-abi.sh` enforces after each
  build, built with a toolchain whose own glibc is at most the ceiling: every port's 2.28 by
  Godot's Buildroot SDKs (`dx_fetch_buildroot`); Bootlin's (`dx_fetch_bootlin`) for a device
  older than that. A device's own libraries may need more than the ceiling, which GNU ld
  rejects: the link then ignores what shared libraries leave undefined, as
  [the Smart Pro's](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#the-toolchains)
  does.
- **Started without `--ini`, as an engine run by hand is, DXEngine
  reads `DXE-DeusEx.ini`/`DXE-User.ini` in the game's `System` after its first clean exit**,
  not `DeusEx.ini`/`User.ini`, and of the keys the original launcher writes it reads few; a stub
  `DeusEx.ini` kills it. The launchers start it with `--ini` and `--userini`, the profile's
  ([running it](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/ENGINE.md#running-it)).
- **Never `pkill -f` or `pgrep -f <pattern>`** in a command whose own text contains the pattern:
  it matches the shell running it. Use `pidof`, `pgrep -x`, or bracket a letter
  (`pgrep -f "[D]XEngine --no-launcher"`).
- **Deus Ex's UnrealScript source is embedded in `System/DeusEx.u`**: search it before guessing
  what the game's script does
  ([working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).
- **CMake build directories cannot move.** Their caches hold absolute paths; after moving a
  tree, delete its build directory and rebuild. So do `build/dxcap`'s links and the validation
  layer's manifest in `deps/`: after moving the parent folder, run `dxcap.sh setup` (then
  `compile`) and `host-tools.sh` again.
- **`build` does not restage.** The staged app keeps whatever binary the last `stage` copied, so
  a run after `build` alone tests the old engine (its log shows `Unimplemented` lines the
  current engine does not log). Stage before running -- and never copy over an engine that still
  runs (`stage` into the app it runs from; `recreation.sh install` into `gamefiles/System`): the
  copy fails with `Text file busy` and the old binary stays.
