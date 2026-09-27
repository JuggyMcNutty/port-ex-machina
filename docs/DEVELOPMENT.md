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
one ignores (so do `build/`, `deps/`, `gamefiles/` and `reference/`). The fork
is our own repository, forked from upstream; `ENGINE-PIN.txt` names the commit
this one builds, `scripts/engine.sh check` proves the clone is at it, and
`scripts/engine.sh fetch` recreates the clone from nothing
([`ENGINE.md`](ENGINE.md)).

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
CMake caches, the engine's embedded source paths). The container has
`libpipewire`/`libpulse` (installed 2026-09-25, for M5's audio work), so the
engine reaches the desktop's audio -- a test run is audible on the owner's
speakers as well as visible on their screen. The null OpenAL driver remains
the way to run silently
([linux-x86_64's README](../ports/linux-x86_64/README.md#audio)). Its pacman
has multilib (enabled 2026-09-26) and the 32-bit libraries the original needs
under Proton's wine: X11, Mesa, PulseAudio, FreeType, GLib and theirs.

## Commits

- **Commit as JuggyMcNutty** (`11588877+JuggyMcNutty@users.noreply.github.com`),
  never the machine's global git identity, which is a personal address. This
  clone and the engine clone set it in their local git config; a fresh clone
  needs it before its first commit; `scripts/engine.sh fetch` sets it in a
  fresh engine clone.
- **Never commit the game's files** -- not an ini, not a `.int`. The repository
  is public; `tests/fixtures` are written stand-ins, and `test_gamefiles` reads
  the real ones from `gamefiles/` in place.
- **Temporary debug hooks** (screenshots from the renderer, extra logging)
  carry a `TEMPORARY DEBUG TOOL` comment and are reverted before committing --
  by replacing their exact text, never by a looser scripted cut: one such cut
  took live main-loop code with it, the build still compiled, and the engine
  died half a minute into a run. A slice's proving run goes 60 s or more --
  25 s once hid exactly that -- and looks at what was drawn:
  `scripts/dxcap.sh prove <map>` ([scripted runs](#scripted-runs-of-both-engines)).
  A clean log once hid a world that was not drawn at all.
- A change and the docs it affects go in the same commit.

## Scripted runs of both engines

`scripts/dxcap.sh` runs the original game (under Proton's wine, in this
container) and the engine fork alike, each driven by a console class of the DXCapture package --
UnrealScript in `tools/dxcap`, compiled by the SDK's `UCC.exe`
(`reference/ReleaseSDK1112f`) into `build/dxcap`. Each run gets a private ini
made from the game's own, naming the console class, with a 1280x720 window;
both engines take the game's settings from it, and the game's inis are never
written. A run's shots, log and recording land in `build/dxcap/runs/`.

```sh
scripts/dxcap.sh setup && scripts/dxcap.sh compile   # once, and after changing tools/dxcap
scripts/dxcap.sh prove 01_NYC_UNATCOIsland.dx        # the fork: shots at 20 s and 60 s, checked, exit at 65 s
scripts/dxcap.sh fork <console> <map>                # the fork with any console class
scripts/dxcap.sh original <console>                  # the original, from its menu map
DXCAP_RECORD=1 scripts/dxcap.sh ...                  # either, its audio recorded into the run's audio.wav
```

The console classes:

- **`ProveConsole`**: the proving run.
- **`CaptureConsole`**: M0's pictures -- Liberty Island's lasers and coronas,
  a tripwire walked into, and in Brooklyn a conversation whose jump lands on
  a comment's label, played through.
- **`NetConsole`**: the scripts' sockets -- conversions and GameSpy answers
  logged, 333networks' master server asked for Deus Ex's servers and five
  of them pinged, then the game's own Join Internet screen opened (the
  run's ini names that master server: the game's names GameSpy's, closed).
- **`ServeConsole`**, either engine: a listen server for the other to join -- a
  deathmatch on DXMP_Cathedral, never on the master servers' lists (the
  run's ini has no uplink). Once another player is in, the host's own player
  stands in its sight -- in front of it where there is room -- and walks to
  and fro across its view; where each
  player stands is logged every 2 s. It exits after 290 s: give the
  original's run 300 s (`scripts/dxcap.sh original ServeConsole 300`).
- **`JoinConsole`**, either engine: the joining side -- from the menu map it
  opens `127.0.0.1:7790`, stands its player 5 s, walks it forward 5 s and
  stands again, logging each second where it and every other pawn stand, and
  shots at the stops (`scripts/dxcap.sh fork JoinConsole DX.dx`, or
  `scripts/dxcap.sh original JoinConsole` against the fork's
  `scripts/dxcap.sh fork ServeConsole DX.dx`). Start it once the server
  answers, some 13 s after the original starts; the original's log comes at
  its exit.
- **`SoundConsole`**: M0's sounds -- a steady sound heard in the open and from
  behind a wall, shots in a reverb zone and out of it, and beeps from the
  right, the left and ahead. It silences the level first (ambient sounds,
  pawns, whatever watches for the player, datalinks) and starts each part
  with three beeps; `tools/dxcap/sound.py <run>` lays the recording against
  the log by them and measures.

The classes stand the player where the original's searches did, written into
them: the two engines' `SetLocation`s fit the player in differently
([engine-dll.md](re/engine-dll.md#teleporting-an-actor)), so a search would
stand them apart.

**Shots.** The fork's `shot` writes the next free `ShotNNNN.bmp`. The
original's own `shot` gives noise (D3D) or black (the others) under Proton, so
its run draws through `OpenGLDrv` on a hidden X display -- Xvfb on `:99`, in
the container -- which `tools/dxcap/grab.py` reads five times a second,
keeping each frame whose corner carries the console's mark: a magenta block,
then the shot's number in eight black or white blocks. The original's
brightness is a gamma ramp, which the hidden display lacks: its shots are
darker than the fork's, and brightness is not compared.

**Recording.** `DXCAP_RECORD=1` sends the engine's sound to a private null
sink on the desktop's sound server (`PULSE_SINK`) and records the sink with
`parecord`; the run's ini turns the music off.

What it takes to run the original there, each found the hard way:

- **It boots its menu map** whatever map its command line or ini names; a
  console class travels with `open <map>` itself.
- **UCC needs a short base directory** (a long one crashes it while it reads
  its ini) and both `UCC.ini` and `DeusEx.ini`; hence `build/dxcap`.
- **A stale `Running.ini` opens the recovery wizard**, which waits for a
  click: the script removes it first. Every run of the original overwrites
  `System/DeusEx.log`, as any launch of it does.
- **It runs in this container, never on the host**: the Proton build's own
  `wine` with the prefix, as IDA's headless server runs
  ([`tools/ida/idalib-mcp.sh`](../tools/ida/idalib-mcp.sh)); the two share
  a wineserver, which nothing stops. The 32-bit game needs the container's
  32-bit libraries ([this machine](#this-machine)).
- **No Wine desktop**: `explorer /desktop` fails to set its display up on
  Xvfb and exits without starting the game, so the game runs straight on the
  hidden display, where the grabber finds its frames by their mark.
- **Only its own process is stopped** at the end: the prefix may hold IDA
  too.
- **Runs of both engines at once** (the net tests) lose the fork's shots:
  the original's run deletes every shot that appears in the game's folder
  while it runs, its own being black.

The fork's window opens on this machine's desktop, as any run's does; the
original's, on the hidden display.

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
- `scripts/engine.sh check`: the engine clone is at the commit
  `ENGINE-PIN.txt` names, on the fork's branch.
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
- **`build` does not restage.** The staged app keeps whatever binary the last
  `stage` copied; a run after `build` alone tests the old engine, which shows
  up as long-fixed `Unimplemented` lines in its log (a three-day-old staged
  binary cost a round of confusion, 2026-09-25). Stage before running -- and
  never while the engine still runs: the copy fails with `Text file busy` and
  the old binary runs again.
