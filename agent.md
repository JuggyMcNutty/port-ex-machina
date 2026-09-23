# Port Ex Machina — working log and session handoff

**Goal.** Run Deus Ex natively, driven by a pad, on devices it was never made
for. It began as one handheld (TrimUI Smart Pro, aarch64, spruceOS); it is now
a porting framework, so desktop Linux, other aarch64 devices, custom
vendor-firmware handhelds and eventually Android are each a small port. Two
halves: a launcher we write, which replaces `System/DeusEx.exe` (only the Unreal
Engine 1 bootstrap shell, so a native reimplementation is tractable), and the
engine, a fork of Surreal Engine.

**State (2026-09-22).**

- **The repository is the project root** (it used to be the port/ directory,
  briefly ports/aarch64-TSP), laid out as shared launcher + `ports/<id>/`. The
  work is on branch `ports-framework`, on top of `aarch64`, not merged.
- **trimui-smartpro**: the game runs. Intro ~30 FPS; **Liberty Island 2–3
  FPS**, CPU-bound on NPC AI and lightmap rebuilds -- measured, candidate fixes
  waiting on the owner's choice (Open decisions). The framework's build was
  deployed and started the game on the device.
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
`scripts/dx.sh build`, `stage`, `deploy`/`run`. `scripts/dx.sh check` runs the
drift guards; `scripts/dx.sh test` the unit tests.

**The device.** Address, login and the ways it bites are in
[`ports/trimui-smartpro/README.md`](ports/trimui-smartpro/README.md). It drops
off the network when it sleeps: ask the owner to wake it. The previous build's
files are in `/mnt/SDCARD/App/DeusEx/.prev-20260922/` on the device, should a
rollback be wanted. `Running.ini` is present in the game's `System/` there
(profiling runs killed with SIGKILL), so the launcher shows a crash banner
until the next clean exit or "Clear crash marker".

**This machine.** Claude runs in an Arch Linux distrobox on a Fedora Atomic
host: `/home` here is `/var/home` there, and paths configured in one differ from
the other (the old CMake caches, the engine's embedded source paths). The
container has no `libpipewire`/`libpulse`, so the engine cannot open audio in
it: run it with the null OpenAL driver ([`ports/linux-x86_64/README.md`](ports/linux-x86_64/README.md#audio)).

## Open decisions

1. **Performance** on the Smart Pro (owner to choose; nothing started).
   Liberty Island, from [its README](ports/trimui-smartpro/README.md#performance):
   game tick ~40% (NPC AI through a slow script VM), lightmap rebuilds ~20%
   (muzzle flashes re-light surfaces on the CPU), other render CPU ~20% (not yet
   broken down), GPU ~15% and serialised with the CPU. Candidates, in order of
   payoff per effort:
   - AI level of detail: tick far/unseen pawns every 2–4 frames. Trade-off:
     distant AI reacts slightly later.
   - Lightmaps: don't re-light for short-lived flashes; spread rebuilds over
     the four cores. Trade-off: flashes light characters, not walls.
   - Lower internal resolution + let CPU and GPU overlap
     (`CommandBufferManager::SubmitCommands` waits on the fence right after
     submit). Trade-off: a softer image.
   - Speed up the VM call path (`Frame::Call` copies its argument array and
     re-walks the parameter list per call). No trade-off; largest effort.
   Next measurement: break down the ~100 ms of "other render CPU".
2. **OpenGL ES backend** (was "Phase 4"): recommended to drop or park -- the
   CPU is the bottleneck and Vulkan is the better API on the GE8300. The Smart
   Pro's `renderers.ini` already lists GLES as "not in this engine build";
   setting `EngineType=GLES` there is all it would need later.
3. **Verify by hand** (owner):
   - On the Smart Pro: START opens the pause menu on the first press after
     skipping the intro; SELECT opens it too; B/Y/SELECT/START close menus; the
     Customize buttons screen; the retired-layout upgrade being written on
     Play/Quit; CPU mode chosen from the Video tab; stick speeds -- look
     (`Speed=3.75`/`2.25`) and pointer speed are calibrated by reasoning, not by
     feel.
   - On a desktop: `scripts/dx.sh run linux-x86_64`, the home screen driven
     into a game, a pad in game. The desktop defaults (4x MSAA, VSync on) are
     chosen by reasoning.
   - linux-aarch64 on any real device.
4. **Publishing on GitHub** as `port-ex-machina` (the name is explained in the
   README; the licence is zlib). The history holds none of the game's files
   (rewritten 2026-09-22 to take out the retail ini/int test fixtures). Left
   before it goes public: merging `ports-framework` into `aarch64`/`main` --
   the owner's call; the branch name `aarch64` no longer describes the
   repository. The checkout directory is still named
   `deusex-launcher`; renaming it means deleting `build/` (see the CMake
   gotcha below).
5. **Release polish** (owner's request, deferred): the home screen is
   deliberately verbose for development; a final build needs a declutter pass,
   and Surreal Engine's always-on Deus Ex stats overlay (FPS/actors/surfaces,
   `RenderCanvas.cpp` `DrawTimedemoStats`) hidden behind an option.
6. **Next ports**: a cross-built engine for linux-aarch64 (a sysroot with the
   engine's libraries, as the Smart Pro has); Android (its README lists the
   work, starting with an in-process hand-over).
7. **Cleanups**: `core/strings.{c,h}` (Startup.int reading) is no longer used
   by the app, only by `test_strings`; `dxl_config_set_render_device` and the
   `DescFlags` accessors are only used by tests. Remove or keep deliberately.

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
  frame-time profiling hooks are `scripts/engine.sh perf on|off`.
- **Editing docs with string replacement fails silently** when the pattern does
  not match. One README edit in this project was reported as done in a commit
  message and had not happened. Prefer full rewrites or scripted replacements
  that assert the pattern was found, and run `scripts/check-docs.sh`, which
  fails on any repository path a doc names that does not exist.
