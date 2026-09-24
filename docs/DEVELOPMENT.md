# Working on the project

How to work here, whatever the task. Where things stand and what is next is
[`../agent.md`](../agent.md).

## The base: linux-x86_64

Development happens on [linux-x86_64](../ports/linux-x86_64/README.md): the
unit tests, `dxl-shots`, the engine's validation and the side-by-side checks of
engine changes all build and run there. Every other port starts from it
([`PORTING.md`](PORTING.md)). A change is clean when it builds and tests there
*and* builds warning-free for each cross port that ships -- the Smart Pro's GCC
9.3 is stricter about `-Wshadow` and `-Wformat-truncation` than a current host
compiler.

## Two repositories

This one, and `engine/SurrealEngine`: the engine fork, a separate clone this
one ignores (so do `build/`, `deps/`, `gamefiles/` and `reference/`). The fork is
upstream at `engine-patches/UPSTREAM-BASE.txt` plus one commit per patch file;
`scripts/engine.sh check` proves that, and `scripts/engine.sh fetch` recreates
it from nothing ([`ENGINE.md`](ENGINE.md)).

## Cold start

Nothing depends on state from a previous session:

```sh
scripts/dx.sh deps <port>      # toolchains and sysroots (the Smart Pro's needs the device awake)
scripts/engine.sh fetch        # the engine fork
scripts/dx.sh build <port>     # then stage and run -- or deploy, which builds and stages first
scripts/host-tools.sh          # perf and the Vulkan validation layer, for engine work
scripts/dx.sh test             # unit tests
scripts/dx.sh check            # the drift guards
```

## This machine

Claude runs in an Arch Linux distrobox on a Fedora Atomic host: `/home` here is
`/var/home` there, and paths configured in one differ from the other (the old
CMake caches, the engine's embedded source paths). The container has no
`libpipewire`/`libpulse`, so the engine cannot open audio in it: run it with
the null OpenAL driver ([linux-x86_64's README](../ports/linux-x86_64/README.md#audio)).

## Commits

- **Commit as JuggyMcNutty** (`11588877+JuggyMcNutty@users.noreply.github.com`),
  never the machine's global git identity, which is a personal address. This
  clone and the engine clone set it in their local git config; a fresh clone
  needs it before its first commit. The engine patches' `From:` lines carry it,
  and `scripts/engine.sh fetch` commits with it.
- **Never commit the game's files** -- not an ini, not a `.int`. The repository
  is public; `tests/fixtures` are written stand-ins, and `test_gamefiles` reads
  the real ones from `gamefiles/` in place.
- **Temporary debug hooks** (screenshots from the renderer, extra logging)
  carry a `TEMPORARY DEBUG TOOL` comment and are reverted before committing.
- A change and the docs it affects go in the same commit.

## Docs

Each fact lives in one place, and everything else links to it; which doc holds
what is the [table in the README](../README.md#documentation). Keep volatile
details out -- commit counts and lists, which `scripts/engine.sh check` and git
already know. **Editing docs with string replacement fails silently** when the
pattern does not match (one README edit was reported done in a commit message
and had not happened): prefer full rewrites or scripted replacements that
assert the pattern was found.

### Drift guards

Docs and scripts that describe the tree are checked against it, so a move or a
rename fails loudly instead of leaving stale instructions:

- `scripts/check-docs.sh` (also the `docs_paths` test): every repository path
  a doc names in backticks or links must exist, and every link to a heading
  (`#anchor`) must find one.
- `scripts/engine.sh check`: the fork's commits over `UPSTREAM-BASE.txt` are
  exactly `engine-patches/*.patch`, in order.
- `test_target_<port>`: each CPU mode a profile offers is handled by its
  `port-hooks.sh`, the hooks' fallback (`CPU_MODE_DEFAULT`) is the profile's
  default, and the profile's id is its port's.
- `scripts/dx.sh check` runs the first two, confirms every port has its
  required files, and checks the glibc ceiling of whatever is staged.

## Gotchas that cost time

Device-specific ones are in each port's README. These apply everywhere:

- **A binary built against a newer glibc than the device's will not load.**
  glibc 2.34 re-versioned the startup symbols; one `GLIBC_2.34` reference is
  enough. Every cross port sets a ceiling (`DXL_PORT_GLIBC_MAX`) that
  `scripts/check-abi.sh` enforces after each build.
- **Surreal Engine does not read what the original launcher wrote**, and after
  its first clean exit it reads `SE-DeusEx.ini`/`SE-User.ini`, not
  `DeusEx.ini`/`User.ini`; a stub `DeusEx.ini` kills it
  ([where the settings actually live](LAUNCHER.md#where-the-settings-actually-live)).
- **The engine takes `--url=<map>` only** and **ignores SIGTERM**
  ([running it](ENGINE.md#running-it)). `-u <map>` silently loads the intro,
  which is how a whole round of "Liberty Island" profiling measured the intro.
- **Never `pkill -f <pattern>`** in a command whose own text contains the
  pattern -- it matches the shell running it (this killed the session's shell
  twice). Use `pidof` or `pgrep -x`.
- **Profile the handheld on the handheld.** Its Cortex-A53 pays far more for a
  cache miss than the desktop, so the costs come in a different order
  (`CycleActors` was ~6% of the desktop's game tick and ~18% of the device's).
  Its kernel has no perf events; the hooks' own sampler does it
  ([the Smart Pro's Performance](../ports/trimui-smartpro/README.md#performance)).
- **Deus Ex's UnrealScript source is embedded in `System/DeusEx.u`**: search it
  before guessing what the game's script does
  ([working on the binaries](re/README.md#working-on-the-binaries)).
- **CMake build directories cannot move.** Their caches hold absolute paths;
  after moving the tree, delete `build/` and rebuild.
- **The profiling hooks go off before changing the engine**: a commit made with
  them on carries them ([the profiling hooks](ENGINE.md#the-profiling-hooks)).
- **An unattended run tests no AI.** Liberty Island starts the player 7,000 to
  20,000 units from every NSF, and no NPC reacts to a player it cannot see: to
  check AI on the desktop, move the player in front of one with a temporary
  hook.
