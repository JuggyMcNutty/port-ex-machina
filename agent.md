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
  work is on `main`, public at https://github.com/JuggyMcNutty/port-ex-machina.
  Its history was rewritten before publishing (2026-09-22) to drop the game's
  files and a personal email address.
- **trimui-smartpro**: the game runs. Intro ~30 FPS; **Liberty Island 2–3
  FPS**, CPU-bound on NPC AI and lightmap rebuilds. The fixes and a ~20 FPS
  target are decided (Decided); CPU/GPU overlap, lit-span lightmaps and a
  cheaper script call path (engine patches 0004–0007) took the fight from
  2.2 to 4.2 FPS. The framework's build was
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
`scripts/dx.sh build`, `stage` and `run` -- or `deploy`, which builds and
stages first. `scripts/host-tools.sh` fetches `perf` and the Vulkan
validation layer for engine work. `scripts/dx.sh check` runs the
drift guards; `scripts/dx.sh test` the unit tests.

**The device.** Address, login and the ways it bites are in
[`ports/trimui-smartpro/README.md`](ports/trimui-smartpro/README.md). It drops
off the network when it sleeps: ask the owner to wake it. Every deploy keeps
the device's previous copy of each file it replaces in a `.prev-<date>`
directory of `/mnt/SDCARD/App/DeusEx/` (named by the device's clock, newest
last): copying one back is a rollback. Profiling runs use the CPU mode
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
   Liberty Island**, and every trade-off below is accepted. 20 FPS needs the
   script VM several times faster, so the deep VM work is in scope. Where the
   time goes is in [its README](ports/trimui-smartpro/README.md#performance)
   (overclock, facing the fight, now 4.2 FPS, ~240 ms): game tick ~124 ms (NPC
   AI through a slow script VM, ~77 ms of it under script calls), other
   render CPU ~90 ms (visibility 34, actor meshes 23, BSP surfaces 14),
   lightmaps and their uploads ~22 ms; the GPU now overlaps the tick. Order of
   work, by payoff for effort, re-measuring after each:
   - ~~Let CPU and GPU overlap~~ -- done, engine patch 0004: 2.2 → 2.5 FPS.
     The GPU wait is gone, but the tick grew ~20 ms (shared memory).
   - ~~Lightmaps~~ -- done, engine patch 0005, with no trade-off: the cost was
     a burning barrel's animated light computed over every texel of twelve
     large surfaces, not flashes. 2.5 → 3.3 FPS. Left: ~22 ms of whole-surface
     copying, conversion and upload per frame; a rebuild could upload only its
     changed rows.
   - ~~VM call path~~ -- first step done, engine patch 0006: parameters from
     `Properties` and a per-class virtual function cache removed the
     `dynamic_cast`s (~15% of desktop samples); script ~125 → ~84 ms, 3.3 →
     4.0 FPS; patch 0007 took per-call set-up out (native frames without
     locals, event names looked up once, plain-data locals zero-filled):
     script ~77 ms, 4.2 FPS. What remains is the interpreter itself
     (`Frame::Run`, `ExpressionEvaluator::Eval`, `ExpressionValue` copies),
     the large part; one script function, `ScriptedPawn.CheckEnemyPresence`,
     is ~a third of all script time. Profile the desktop
     build with `perf` ([engine-patches/README.md](engine-patches/README.md#profiling-and-validating-on-the-desktop)).
   - AI level of detail: tick far/unseen pawns every 2–4 frames. Distant AI
     reacts slightly later.
   - Visibility (34 ms): `BspClipper` rasterises occluders on every scanline
     of the viewport. The render resolution below shrinks it; the clipper
     could also run coarser than the image.
   - Render resolution, **chosen by the user on the launcher's Video tab**
     (owner, 2026-09-22) and scaled up to the panel; the default stays native.
     The engine already draws each frame into an offscreen image and scales it
     when presenting (`VulkanRenderDevice::DrawPresentTexture`); a fork patch
     lets that image be smaller than the window. The game's own resolution menu
     refuses anything under 640×480 (`MenuChoice_Resolution` in `DeusEx.u`),
     so 480 lines is the floor on a 16:9 panel: 1280×720, 960×540, 854×480 on
     the Smart Pro. Still to settle when it is built: a new `Settings.json`
     field, or the engine's own `FullscreenViewportX` and `Y`, which the in-game
     menu writes too. A softer image, HUD included.
2. **Renderers on aarch64** (owner, 2026-09-22): the goal is Vulkan, OpenGL ES
   and software rendering all selectable. Not now: Vulkan is the only one the
   engine has. GLES means porting Surreal's desktop OpenGL 3.2 renderer (the
   Smart Pro's `renderers.ini` then needs only `EngineType=GLES`); Surreal has
   no software renderer at all.

## Open decisions

1. **Verify by hand** (owner):
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
  frame-time profiling hooks are `scripts/engine.sh perf on|off`.
- **Editing docs with string replacement fails silently** when the pattern does
  not match. One README edit in this project was reported as done in a commit
  message and had not happened. Prefer full rewrites or scripted replacements
  that assert the pattern was found, and run `scripts/check-docs.sh`, which
  fails on any repository path a doc names that does not exist.
