# The engine

The game runs on [Surreal Engine](https://github.com/dpjudas/SurrealEngine), an
open-source reimplementation of Unreal Engine 1 that recognises this build of
Deus Ex directly: `DeusEx.exe` SHA1 `2a933e26aa9cfb33b37f78afe21434caa031f14a`
is its `DEUS_EX_1112fm` database entry.

We use it as a vendored dependency: a fork pinned to one upstream commit, plus
patches for what our ports need -- starting from our launcher, embedded GPUs,
pads, and the speed a handheld needs. The fork is a separate clone in
`engine/SurrealEngine` (branch `deusex-handheld`, ignored by this repository);
builds go to `build/<port>/engine`, never into the clone.

## How it is kept

**Pinned.** `engine-patches/UPSTREAM-BASE.txt` names the upstream commit, and
each `engine-patches/NNNN-*.patch` is one fork commit's `git format-patch`
output. `scripts/engine.sh fetch` clones upstream at that commit and applies
the patches, reproducing the fork's commits exactly, ids included, on any
machine. That pair is the engine's version; no other engine source is kept here.

**Not upstream's.** We do not send changes upstream: the engine ships a
`NO-AI Code Rule.md` --

> If you are primarily using LLM tools such as Claude to make code changes to
> this codebase then please do not PR it to us. Keep it in a fork. Thank you.

-- and these patches were written with Claude. A change worth upstreaming would
need rewriting by a person from the problem statement, not adapting from a
diff. Nor do we follow upstream: its new commits reach the fork only when
someone chooses to upgrade (owner, 2026-09-23). Using and building the engine is
permitted by its own licence, which grants use "for any purpose".

## Commands

```sh
scripts/engine.sh fetch                     # clone upstream at the pin, apply the patches
scripts/engine.sh check                     # the fork's commits == engine-patches/*.patch, in order
scripts/engine.sh export <commit> NNNN-name # write a fork commit to engine-patches/
scripts/engine.sh build <port>              # build/<port>/engine, from ports/<port>/engine.cmake
scripts/engine.sh perf on|off|save          # the profiling hooks (below)
```

`scripts/dx.sh build <port>` calls `build` for ports that ship the engine;
`scripts/dx.sh check` runs `check`.

## Changing the engine

Commit the change in the fork, `export` it as the next patch, and `check`.
Each patch's commit message says what it changes, why, and how it was checked;
[what the fork changes](#what-the-fork-changes) below adds what it did on the
Smart Pro. Patch 0001's message points at a README under port/, this
repository's old directory name -- rewording it would change the fork's commit
ids.

Temporary debugging hooks never go into a patch: they carry a
`TEMPORARY DEBUG TOOL` comment and are reverted before committing.

### The profiling hooks

The frame-time profiling hooks live in
`engine-patches/optional/perf-instrumentation.patch` so they can be re-applied:
`scripts/engine.sh perf on`, and `perf off` afterwards. Take them off before
changing the engine -- a commit made with them on carries them -- and commit
before putting them back: `perf on` cannot merge over uncommitted changes to a
file the hooks touch, and says so. The patch is against the fork's head, so a
fork commit that touches the same lines moves them: `perf on` then falls back to
a three-way merge (and stops if that leaves conflicts), and `perf save` rewrites
the patch from the tree so the next `on` and `off` apply cleanly.

The hooks build the engine with frame pointers (~1% slower on the handheld) and
carry a sampling profiler for devices without `perf`:
`SURREAL_PERF_SAMPLE=<file>` samples the main thread's CPU time, recording each
sample's program counter and the return addresses a frame-pointer walk finds,
one block per 60-frame report, with `/proc/self/maps` beside it.
[`scripts/sample-report.py`](../scripts/sample-report.py) (Python 3, and the port
toolchain's `nm` for a cross build) turns that into self and inclusive time per
function, optionally under one caller (`--root ULevel::Tick`: the game tick) or
with the callers of one (`--callers`). A leaf function keeps no frame record,
so its samples show its caller's caller as the next frame. Profiling the
handheld: [its README](../ports/trimui-smartpro/README.md#performance).

## Running it

`run-game.sh` starts it for every real launch. By hand, from the game's
directory:

```sh
SurrealEngine --no-launcher /path/to/deusex --url=01_NYC_UNATCOIsland.dx
```

- `--no-launcher` (or a game folder on the command line) skips upstream's
  desktop launcher window (patch 0001).
- **Maps are `--url=<map>` only.** `-u <map>` sets an empty `-u` and the map name
  becomes a stray argument, so the intro loads with no warning.
- **The engine ignores SIGTERM**: stop it with SIGKILL (`timeout -s KILL`). That
  leaves `Running.ini` behind like any crash.
- Where there is no audio device (a container), give OpenAL Soft the null
  driver ([linux-x86_64's README](../ports/linux-x86_64/README.md#audio)).

### Profiling and validating on the desktop

The handheld has no `perf` and no Vulkan validation layer, so validation runs
against the base port's build (`scripts/dx.sh build linux-x86_64 engine`), and
so can a quick CPU profile -- but the desktop's proportions are not the
handheld's (its Cortex-A53 pays far more for a cache miss), so what to work on
next is decided by the handheld's own samples. `scripts/host-tools.sh` unpacks
pinned copies of Linux `perf` and the Khronos validation layer into `deps/`
without installing anything:

```sh
scripts/host-tools.sh
cd gamefiles    # the engine is started from the game's directory
# CPU profile of Liberty Island, recording from 25 s in (after the load):
LD_LIBRARY_PATH=../deps/perf/usr/lib ../deps/perf/usr/bin/perf record -F 2000 --delay=25000 -o /tmp/se.data -- \
    timeout -s KILL 55 ../build/linux-x86_64/engine/SurrealEngine --no-launcher "$PWD" --url=01_NYC_UNATCOIsland.dx
LD_LIBRARY_PATH=../deps/perf/usr/lib ../deps/perf/usr/bin/perf report -i /tmp/se.data --no-children --sort symbol
# Synchronization validation (add SURREAL_VK_NO_BINDLESS=1 for the handheld's texture path):
VK_LAYER_PATH=$PWD/../deps/vulkan-layers/layers VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation \
VK_KHRONOS_VALIDATION_VALIDATE_SYNC=true \
    timeout -s KILL 60 ../build/linux-x86_64/engine/SurrealEngine --no-launcher "$PWD" --url=01_NYC_UNATCOIsland.dx
```

`--call-graph dwarf` on `perf record` gives callers (the desktop build has no
frame pointers unless the hooks are on).

## What the fork changes

By area; the number is the patch's place in the series. How each works is in
its patch file's message. **Smart Pro** is what it did there, measured with the
hooks in Liberty Island's opening fight (the whole frame's numbers after each
patch are in [the Smart Pro's Performance](../ports/trimui-smartpro/README.md#performance));
**checked** is how it was shown not to change the game, where the message does
not already say.

### Running on our devices

- [**0001**](../engine-patches/0001-headless-and-embedded-support.patch)
  `headless-and-embedded-support` -- the engine started by our launcher with no
  desktop: no launcher window, errors and the log on stderr, a non-zero exit
  after a caught exception (the launcher's crash sentinel reads it); and a
  cross build for an embedded aarch64 device: SDL2 only, no X11/Wayland/desktop
  GL, SDL from pkg-config, a host-built `zipdir`, fonts without GSettings or
  fontconfig (`SURREALWIDGETS_FONT`).
- [**0002**](../engine-patches/0002-nonbindless-fallback-and-format-support.patch)
  `nonbindless-fallback-and-format-support` -- Vulkan on GPUs without desktop
  texture support: a per-batch descriptor set path when
  `VK_EXT_descriptor_indexing` is missing (`SURREAL_VK_NO_BINDLESS=1` forces
  it), and CPU decoders for texture formats the GPU cannot sample or filter
  (BC1–5, RGB8, RGBA32F). **Smart Pro:** the GE8300 has neither; the intro went
  from speckle to clean. A desktop GPU keeps the bindless path. The format
  table came from `tools/probes/probe-texture-formats.c`.
- [**0003**](../engine-patches/0003-gamepad-and-deusex-fixes.patch)
  `gamepad-and-deusex-fixes` -- the pad as polled state, turned into UE1
  joystick keys and axes so `User.ini` bindings decide what it does, with
  menu-mode controls and a `Gamepad` block in `Settings.json` ([the launcher's
  controller support](LAUNCHER.md#controller-support)); and two Deus Ex fixes:
  `CycleActors` resumes where it stopped, as the game's script expects, and the
  first pause-menu press after skipping the intro is no longer swallowed.

### Settings the launcher exposes

- [**0008**](../engine-patches/0008-ai-level-of-detail.patch)
  `ai-level-of-detail` -- with `Settings.json` `Performance.AiLevelOfDetail`
  (the Video tab's Distant AI), a pawn out of sight and not within 1500 units
  runs its script thinking every third frame. **Smart Pro:** game tick ~124 →
  ~104 ms; ~38 pawns a frame skip their thinking. **Checked:** the scene
  renders normally (framebuffer capture); whether distant NPCs still behave is
  not yet judged by hand.
- [**0009**](../engine-patches/0009-render-scale.patch) `render-scale` --
  `Performance.RenderScale` (the Video tab's Resolution): the scene drawn
  smaller than the window and scaled up; Vulkan only. **Checked:** Liberty
  Island at 960×540 and 853×480 fills the panel, the HUD larger (framebuffer
  captures); synchronization validation clean on the desktop at scale 0.667.
- [**0022**](../engine-patches/0022-ai-lod-far-tier.patch) `ai-lod-far-tier` --
  with Distant AI, a pawn also beyond 4000 units thinks every sixth frame.
  **Smart Pro:** game tick ~65 → ~63 ms; ~48 pawns a frame fall in the tier, ~8
  of them thinking, ~9 fewer thinking each frame.

### Rendering

- [**0004**](../engine-patches/0004-vulkan-frame-overlap.patch)
  `vulkan-frame-overlap` -- the game tick runs while the GPU draws the previous
  frame; swapchain rebuilds wait for the device. **Smart Pro:** GPU wait ~76 →
  ~0.2 ms, the tick ~20 ms longer (CPU and GPU share the SoC's memory).
  **Checked:** Liberty Island mid-fight renders correctly (framebuffer
  capture).
- [**0005**](../engine-patches/0005-lightmap-lit-spans.patch)
  `lightmap-lit-spans` -- lightmaps lit only where a light reaches. **Smart
  Pro:** lightmaps ~98 → ~11 ms. **Checked:** the dock pixel-identical before
  and after (framebuffer captures).
- [**0010**](../engine-patches/0010-clipper-sized-to-image.patch)
  `clipper-sized-to-image` -- the visibility clipper's occlusion grid has one
  row per image row (it was a fixed 2048×1080). **Smart Pro:** frame ~222 →
  ~213 ms native, ~208 → ~191 ms at 853×480. **Checked:** the dock
  pixel-identical at both; ~830 surfaces pass visibility where ~740 did, all
  hidden by the depth test.
- [**0011**](../engine-patches/0011-cull-one-sided-back-faces.patch)
  `cull-one-sided-back-faces` -- one-sided surfaces seen from behind skipped
  before the visibility test. **Smart Pro:** surface tests ~4,800 → ~2,400 a
  frame and ~22 → ~12 ms; lightmaps ~10 → ~4 ms and their uploads ~11 → ~4 ms
  (most of the burning barrel's lightmaps were back faces). **Checked:**
  captures of Liberty Island and UNATCO HQ's interior differ only in the stats
  overlay's surface count.
- [**0018**](../engine-patches/0018-mesh-vertices-once.patch)
  `mesh-vertices-once` -- each mesh vertex animated, lit and fogged once a
  draw, not once per face using it. **Smart Pro:** actor meshes ~28 → ~14 ms,
  render CPU ~92 → ~80 ms.
- [**0019**](../engine-patches/0019-mesh-face-batches.patch)
  `mesh-face-batches` -- a run of mesh faces with one texture drawn in one
  device call. **Smart Pro:** actor meshes ~14 → ~12 ms. **Checked:** a capture
  of the dock matches one from before patch 0012 except where time moves things
  (the sky, the NPCs, the stats); the statue and props identical to the pixel.
- [**0020**](../engine-patches/0020-clipper-arm-clip-test.patch)
  `clipper-arm-clip-test` -- the clipper's non-SSE (ARM) build skips clipping
  for triangles inside the view, as the SSE build did (an upstream bug).
  **Smart Pro:** visibility ~25 → ~20 ms (with the per-part timers), surface
  tests ~11.8 → ~7.4 ms. **Checked:** a capture of the dock differs only in the
  sky's clouds and the NPCs.
- [**0021**](../engine-patches/0021-surface-points-on-demand.patch)
  `surface-points-on-demand` -- a surface's points gathered only when a test
  needs them. **Smart Pro:** visibility ~20.5 → ~19.6 ms.
- [**0023**](../engine-patches/0023-light-tree-kept.patch) `light-tree-kept` --
  the light tree, and each surface's lights from it, kept while no light
  changes. **Smart Pro:** the BSP surfaces' section ~11 → ~8 ms. **Checked:** a
  capture at 853×480 shows the dock's lightmaps as before.
- [**0024**](../engine-patches/0024-lightmap-neon-conversion.patch)
  `lightmap-neon-conversion` -- the lightmaps' float-to-byte conversion for the
  GPU in NEON on ARM. **Smart Pro:** texture uploads ~3.9 → ~2.1 ms.

### Script VM

With 0013, these took the Smart Pro's script time from ~125 to ~30 ms a frame.

- [**0006**](../engine-patches/0006-vm-call-path-without-casts.patch)
  `vm-call-path-without-casts` -- parameters from `Properties`, and a per-class
  virtual-function cache: no `dynamic_cast` on the call path. **Smart Pro:**
  script ~125 → ~84 ms.
- [**0007**](../engine-patches/0007-vm-call-overheads.patch)
  `vm-call-overheads` -- native frames without locals, event names looked up
  once, plain-data locals zero-filled. **Smart Pro:** script ~84 → ~77 ms.
- [**0012**](../engine-patches/0012-vm-evaluator-per-statement.patch)
  `vm-evaluator-per-statement` -- one expression evaluator per statement,
  nested values returned directly. **Smart Pro:** script ~60 → ~55 ms.
- [**0014**](../engine-patches/0014-vm-calls-without-allocation.patch)
  `vm-calls-without-allocation` -- script calls without heap allocations or
  walks over every local. **Smart Pro:** script ~39 → ~35 ms.
- [**0015**](../engine-patches/0015-vm-fast-operators.patch)
  `vm-fast-operators` -- the 25 commonest operators evaluated in place. **Smart
  Pro:** script ~35 → ~31 ms.
- [**0016**](../engine-patches/0016-vm-event-lookup-cache.patch)
  `vm-event-lookup-cache` -- events found through the virtual-call cache.
  **Smart Pro:** within the noise (tick ~71.2 → ~70.7 ms).
- [**0017**](../engine-patches/0017-vm-leaf-expressions.patch)
  `vm-leaf-expressions` -- the commonest leaf expressions made without the
  visitor. **Smart Pro:** tick ~70.7 → ~69.3 ms.

### Game tick

- [**0013**](../engine-patches/0013-actor-iterators-by-class.patch)
  `actor-iterators-by-class` -- the actor iterators find a class's actors from
  an index, not a scan of the level (`CycleActors` alone had been ~16 ms of the
  tick). **Smart Pro:** script ~56 → ~39 ms.
- [**0025**](../engine-patches/0025-ray-trace-segment-split.patch)
  `ray-trace-segment-split` -- a ray trace hands each BSP child only its own
  part of the segment. **Smart Pro:** tick ~61.5 → ~58 ms.
- [**0026**](../engine-patches/0026-sight-line-cells.patch) `sight-line-cells`
  -- sight lines test the actors of only the collision cells they cross; hull
  planes on the stack. **Smart Pro:** tick ~58 → ~56 ms.
- [**0027**](../engine-patches/0027-step-down-one-trace.patch)
  `step-down-one-trace` -- a walking pawn's step to the ground made with the
  trace its dry run made. **Smart Pro:** tick ~56 → ~53 ms.
