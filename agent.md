# Port Ex Machina — working log and session handoff

**Goal.** Run Deus Ex natively, driven by a pad, on devices it was never made
for. It began as one handheld (TrimUI Smart Pro, aarch64, spruceOS); it is now
a porting framework, so desktop Linux, other aarch64 devices, custom
vendor-firmware handhelds and eventually Android are each a small port. Two
halves: a launcher we write, which replaces `System/DeusEx.exe` (only the Unreal
Engine 1 bootstrap shell, so a native reimplementation is tractable), and the
engine, a fork of Surreal Engine.

**State (2026-09-23).**

- **The repository is the project root** (it used to be the port/ directory,
  briefly ports/aarch64-TSP), laid out as shared launcher + `ports/<id>/`. The
  work is on `main`, public at https://github.com/JuggyMcNutty/port-ex-machina.
  Its history was rewritten before publishing (2026-09-22) to drop the game's
  files and a personal email address.
- **trimui-smartpro**: the game runs. Intro ~30 FPS; Liberty Island's opening
  firefight **5.3 FPS** at native resolution, 6.0 at 853×480 (2.2 before engine
  patches 0004–0011), still CPU-bound on NPC AI script and render CPU; UNATCO
  HQ indoors ~20. The target is ~20 FPS in the fight (Decided). The device
  has the current build: patches 0001–0011, Overclock, Distant AI on, native
  resolution.
- **linux-x86_64**: launcher and engine build natively; the staged app's
  `run-game.sh` ran the engine into the intro level on the development PC.
- **linux-aarch64**: the launcher cross-builds; never run on a device.
- **android**: planned; `ports/android/README.md` is the plan.

---

## Start here

Read, in order: this file → [`README.md`](README.md) (layout, ports, quick
start) → [`docs/DESIGN.md`](docs/DESIGN.md) → [`docs/PORTING.md`](docs/PORTING.md)
→ the port you are working on (`ports/<id>/README.md`) →
[`engine-patches/README.md`](engine-patches/README.md). The reverse-engineering
spec is [`docs/re/`](docs/re/), with its findings summarised in
[`docs/re/README.md`](docs/re/README.md).

**Two git repositories.** This one, and `engine/SurrealEngine` -- a clone of
someone else's project on branch `deusex-handheld`, which this one ignores
(so do `build/`, `deps/`, `gamefiles/` and `reference/`). The fork is upstream
at `engine-patches/UPSTREAM-BASE.txt` plus one commit per patch file;
`scripts/engine.sh check` proves that, and `scripts/engine.sh fetch` recreates
it from nothing.

**Cold start.** Nothing depends on state from a previous session: toolchains
and sysroots are fetched by `scripts/dx.sh deps <port>` (the Smart Pro's
sysroot needs the device awake), the engine by `scripts/engine.sh fetch`, then
`scripts/dx.sh build`, `stage` and `run` -- or `deploy`, which builds and
stages first. `scripts/host-tools.sh` fetches `perf` and the Vulkan
validation layer for engine work. `scripts/dx.sh check` runs the
drift guards; `scripts/dx.sh test` the unit tests.

**The device.** Address, login and the ways it bites are in
[`ports/trimui-smartpro/README.md`](ports/trimui-smartpro/README.md). It drops
off the network when it sleeps: ask the owner to wake it. Every deploy keeps
the device's previous copy of each file it replaces in a `.prev-<date-time>`
directory of `/mnt/SDCARD/App/DeusEx/` (the device's clock, newest last; older
hand-made ones are `.prev-<date>`): copying one back is a rollback. Profiling runs use the CPU mode
`launcher.ini` names -- Overclock, the port's default since 2026-09-22.
`Running.ini` is present in the game's `System/` there
(profiling runs killed with SIGKILL), so the launcher shows a crash banner
until the next clean exit or "Clear crash marker".

**This machine.** Claude runs in an Arch Linux distrobox on a Fedora Atomic
host: `/home` here is `/var/home` there, and paths configured in one differ from
the other (the old CMake caches, the engine's embedded source paths). The
container has no `libpipewire`/`libpulse`, so the engine cannot open audio in
it: run it with the null OpenAL driver ([`ports/linux-x86_64/README.md`](ports/linux-x86_64/README.md#audio)).

## Decided

1. **Smart Pro performance** (owner, 2026-09-22): the target is **~20 FPS on
   Liberty Island** (~50 ms a frame), and every trade-off below is accepted.
   Now 5.3 FPS at native resolution (~190 ms) and 6.0 at 853×480, facing the
   fight in overclock. Where it goes is in
   [its README](ports/trimui-smartpro/README.md#performance): game tick ~97 ms
   (NPC AI, ~58 ms of it script), render CPU ~83 ms (actor meshes 27,
   visibility ~21, BSP surfaces 11), lightmaps and their uploads ~8 ms; the
   GPU overlaps the tick. 20 FPS needs the script VM several times faster, so the
   deep VM work is in scope.

   Done, one engine patch each, measured: CPU/GPU overlap (0004); lightmaps
   lit only where a light reaches (0005 -- the cost was a burning barrel's
   animated light over whole surfaces, not flashes, so that trade-off was not
   needed); the script call path without casts (0006) or per-call set-up
   (0007); AI level of detail (0008, the Video tab's Distant AI, on by default
   here); render scale (0009, the Video tab's Resolution, owner's choice,
   native by default); an occlusion grid the size of the image (0010);
   one-sided back faces skipped before the visibility test (0011).
   [engine-patches/README.md](engine-patches/README.md) has what each found.

   Next, in order, re-measuring after each:
   - **The script interpreter** (`Frame::Run`, `ExpressionEvaluator::Eval`,
     `ExpressionValue` copies), the large part of the tick; one function,
     `ScriptedPawn.CheckEnemyPresence`, is about a third of all script time.
     Profile the desktop build with `perf`
     ([engine-patches/README.md](engine-patches/README.md#profiling-and-validating-on-the-desktop)).
   - **Actor meshes** (~27 ms, now the largest render item): vertex
     animation on the CPU (`VisibleMesh::DrawLodMeshFaceDX`, the desktop
     profile's top render function).
   - **Visibility** (~21 ms): ~2,400 surface tests a frame against the
     occlusion grid are most of it (the profile splits it by part).
   - A second tier of AI level of detail (every sixth frame beyond, say,
     4000 units).
   - **Lightmap uploads** (~4 ms since patch 0011): a rebuilt lightmap is
     re-uploaded whole, though only the rows its lights reach changed.
2. **Renderers on aarch64** (owner, 2026-09-22): the goal is Vulkan, OpenGL ES
   and software rendering all selectable. Not now: Vulkan is the only one the
   engine has. GLES means porting Surreal's desktop OpenGL 3.2 renderer (the
   Smart Pro's `renderers.ini` then needs only `EngineType=GLES`); Surreal has
   no software renderer at all.

## Open decisions

1. **Verify by hand** (owner):
   - On the Smart Pro: that NPCs out of sight still behave (Distant AI on:
     they think every third frame); the Video tab's Resolution at 960×540 and
     853×480 -- the look, and the menu pointer's speed; START opens the pause menu on the first
     press after skipping the intro; SELECT opens it too; B/Y/SELECT/START close menus; the
     Customize buttons screen; the retired-layout upgrade being written on
     Play/Quit; CPU mode chosen from the Video tab; stick speeds -- look
     (`Speed=3.75`/`2.25`) and pointer speed are calibrated by reasoning, not by
     feel.
   - On a desktop: `scripts/dx.sh run linux-x86_64`, the home screen driven
     into a game, a pad in game. The desktop defaults (4x MSAA, VSync on) are
     chosen by reasoning.
   - linux-aarch64 on any real device.
2. **Release polish** (owner's request, deferred): the home screen is
   deliberately verbose for development; a final build needs a declutter pass,
   and Surreal Engine's always-on Deus Ex stats overlay (FPS/actors/surfaces,
   `RenderCanvas.cpp` `DrawTimedemoStats`) hidden behind an option.
3. **Next ports**: a cross-built engine for linux-aarch64 (a sysroot with the
   engine's libraries, as the Smart Pro has); Android (its README lists the
   work, starting with an in-process hand-over).

## Gotchas that cost time

Device-specific ones (the Smart Pro's SSH, framebuffer, busybox, CPU helpers)
are in its README. These apply everywhere:

- **A binary built against a newer glibc than the device's will not load.**
  glibc 2.34 re-versioned the startup symbols; one `GLIBC_2.34` reference is
  enough. Every cross port sets a ceiling (`DXL_PORT_GLIBC_MAX`) that
  `scripts/check-abi.sh` enforces after each build.
- **Surreal Engine does not read what the original wizard wrote.** The
  renderer is `Settings.json` `RenderDevice.Type` (`GameRenderDevice` in
  `DeusEx.ini` is overridden inside the engine); texture/skin detail, sound
  quality and the safe-mode flags are ignored. With `--no-launcher` the engine
  only *reads* `Settings.json` -- it saves it solely from its desktop launcher
  window.
- **After the engine's first clean exit it reads `SE-DeusEx.ini` and
  `SE-User.ini`, not `DeusEx.ini`/`User.ini`**, with client settings under
  `[Engine.SurrealClient]`. Anything written for the engine must go to whichever
  file it will read (`src/core/config.c` does this).
- **A stub `DeusEx.ini` kills the engine** (`Could not find package Core`).
  Surreal falls back to `Default.ini` only when the file is absent; the launcher
  rebuilds such a file from `Default.ini`.
- **The engine takes `--url=<map>` only.** `-u <map>` silently loads the intro,
  which is how a whole round of "Liberty Island" profiling actually measured the
  intro.
- **The engine ignores SIGTERM**; stop it with SIGKILL. That leaves
  `Running.ini` behind like any crash.
- **Commit as JuggyMcNutty** (`11588877+JuggyMcNutty@users.noreply.github.com`),
  never the machine's global git identity, which is a personal address. This
  clone and the engine clone set it in their local git config; a fresh clone
  needs it before its first commit. The engine patches' `From:` lines carry
  it, and `scripts/engine.sh fetch` commits with it.
- **Never commit the game's files** -- not an ini, not a `.int`. The
  repository is public; `tests/fixtures` are written stand-ins, and
  `test_gamefiles` reads the real ones from `gamefiles/` in place.
- **Never `pkill -f <pattern>`** in a command whose own text contains the
  pattern -- it matches the shell running it (this killed the session's shell
  twice). Use `pidof` or `pgrep -x`.
- **Deus Ex's UnrealScript source is embedded in `System/DeusEx.u`**: search
  it before guessing what the game's script does ([`docs/re/README.md`](docs/re/README.md#working-on-the-binary)).
- **CMake build directories cannot move.** Their caches hold absolute paths;
  after moving the tree, delete `build/` and rebuild.
- **Temporary debug hooks** (screenshots from the renderer, extra logging) carry
  a `TEMPORARY DEBUG TOOL` comment and are reverted before committing; the
  frame-time profiling hooks are `scripts/engine.sh perf on|off|save`
  ([engine-patches/README.md](engine-patches/README.md#base)). Take them off
  before changing the engine: a commit made with them on carries them.
- **Editing docs with string replacement fails silently** when the pattern does
  not match. One README edit in this project was reported as done in a commit
  message and had not happened. Prefer full rewrites or scripted replacements
  that assert the pattern was found, and run `scripts/check-docs.sh`, which
  fails on any repository path a doc names that does not exist.
