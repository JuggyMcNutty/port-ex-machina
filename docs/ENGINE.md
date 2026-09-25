# The engine

The game runs on [Surreal Engine](https://github.com/dpjudas/SurrealEngine), an
open-source reimplementation of Unreal Engine 1 that recognises this build of
Deus Ex directly: `DeusEx.exe` SHA1 `2a933e26aa9cfb33b37f78afe21434caa031f14a`
is its `DEUS_EX_1112fm` database entry.

We use it as a vendored dependency: [our own fork](https://github.com/JuggyMcNutty/SurrealEngine)
of upstream, carrying what our ports need -- starting from our launcher,
embedded GPUs, pads, the speed a handheld needs, and what Deus Ex needs from
it to play as it should. `ENGINE-PIN.txt` names the one fork commit this
repository builds. The fork is a separate clone in `engine/SurrealEngine`
(branch `deusex`, ignored by this repository); builds go to
`build/<port>/engine`, never into the clone.

## How it is kept

**Pinned.** `ENGINE-PIN.txt` names the fork repository, its branch and the
commit this repository builds. `scripts/engine.sh fetch` clones the fork and
checks that commit out, on any machine; `check` proves the clone is at it.
After a fork commit is pushed, `pin` moves the file, and its move is committed
here with whatever depends on it. That file is the engine's version; no engine
source is kept here.

**Not upstream's.** We do not send changes upstream: the engine ships a
`NO-AI Code Rule.md` --

> If you are primarily using LLM tools such as Claude to make code changes to
> this codebase then please do not PR it to us. Keep it in a fork. Thank you.

-- and the fork's commits were written with Claude. A change worth upstreaming would
need rewriting by a person from the problem statement, not adapting from a
diff. Nor do we follow upstream: its new commits reach the fork only when
someone chooses to [upgrade](#upgrading-surreal-engine) (owner, 2026-09-23). Using and building the engine is
permitted by its own licence, which grants use "for any purpose".

## Commands

```sh
scripts/engine.sh fetch                     # clone the fork at the pin
scripts/engine.sh check                     # the clone is at the pin, on its branch
scripts/engine.sh status                    # the pin, the fork, how far upstream has moved
scripts/engine.sh pin                       # after a pushed fork commit: move ENGINE-PIN.txt to it
scripts/engine.sh upgrade [<ref>]           # merge upstream in (below); --continue, --abort
scripts/engine.sh build <port>              # build/<port>/engine, from ports/<port>/engine.cmake
scripts/engine.sh perf on|off|save          # the profiling hooks (below)
```

`scripts/dx.sh build <port>` calls `build` for ports that ship the engine;
`scripts/dx.sh check` runs `check`.

## Changing the engine

Commit the change in the fork, push it, then `scripts/engine.sh pin`, and
commit the moved `ENGINE-PIN.txt` here with the docs the change affects. Each
fork commit's message says what it changes, why, and how it was checked, and
opens with the fork-only note (never PR upstream, above);
[what the fork changes](#what-the-fork-changes) below adds what it did on the
Smart Pro. The fork's history is published and never rewritten. Its first 34
commits began as this repository's patch stack, engine-patches (retired
2026-09-24); "patch NNNN" here and in the game's docs is such a commit's
place in that series.

Temporary debugging hooks never go into a patch: they carry a
`TEMPORARY DEBUG TOOL` comment and are reverted before committing.

### The profiling hooks

The frame-time profiling hooks live in
`scripts/perf-instrumentation.patch` so they can be re-applied:
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

### Natives from the original

Where upstream has a Deus Ex native wrong or as a stub -- a stub logs
`Unimplemented: <class>.<name>` the first time it runs in a session, and only
then -- the original is in the game's DLLs, and [`re/`](re/README.md) is what
has been read of them: how to read them is
[working on the binaries](re/README.md#working-on-the-binaries), and what the
fork still lacks is [`re/natives.md`](re/natives.md) (`tools/natives_audit.py`
lists it). Patch 0034 was read this way.

## Upgrading Surreal Engine

Only when someone decides to. `scripts/engine.sh status` fetches upstream and
says how many commits it is past the fork. To take them in:

```sh
scripts/engine.sh perf off                  # if the profiling hooks are on
scripts/engine.sh upgrade                   # upstream's latest; or upgrade <sha|tag|branch>
```

`upgrade` needs the clone to be at the pin with a clean tree. It merges the
chosen upstream commit into the fork's branch, keeping both histories. If
files conflict it stops: resolve them in `engine/SurrealEngine`, `git add`
them, then `scripts/engine.sh upgrade --continue` -- or `--abort`, which
leaves the fork and the pin as they were.

Then, before pinning: build and run linux-x86_64, check the Vulkan validation
layer ([below](#profiling-and-validating-on-the-desktop)), `perf on` (and
`perf save` if the hooks moved), profile on the devices, push the branch, and
`scripts/engine.sh pin` -- committed with
[what the fork changes](#what-the-fork-changes) brought up to date.

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

By area; the number is the commit's place in the series (the retired patch
stack's numbering), its link the commit in the fork repository, whose message
says how it works. **Smart Pro** is what it did there, measured with the
hooks in Liberty Island's opening fight (the whole frame's numbers after each
patch are in [the Smart Pro's Performance](../ports/trimui-smartpro/README.md#performance));
**checked** is how it was shown not to change the game, or for a gameplay fix
to work, where the message does not already say.

### Running on our devices

- [**0001**](https://github.com/JuggyMcNutty/SurrealEngine/commit/22a5e87cc51aa83be550abe1c17e0b4203f18c79)
  `headless-and-embedded-support` -- the engine started by our launcher with no
  desktop: no launcher window, errors and the log on stderr, a non-zero exit
  after a caught exception (the launcher's crash sentinel reads it); and a
  cross build for an embedded aarch64 device: SDL2 only, no X11/Wayland/desktop
  GL, SDL from pkg-config, a host-built `zipdir`, fonts without GSettings or
  fontconfig (`SURREALWIDGETS_FONT`).
- [**0002**](https://github.com/JuggyMcNutty/SurrealEngine/commit/a40bec64d33574529da21d63c1b57b3b3ebfe85e)
  `nonbindless-fallback-and-format-support` -- Vulkan on GPUs without desktop
  texture support: a per-batch descriptor set path when
  `VK_EXT_descriptor_indexing` is missing (`SURREAL_VK_NO_BINDLESS=1` forces
  it), and CPU decoders for texture formats the GPU cannot sample or filter
  (BC1–5, RGB8, RGBA32F). **Smart Pro:** the GE8300 has neither; the intro went
  from speckle to clean. A desktop GPU keeps the bindless path. The format
  table came from `tools/probes/probe-texture-formats.c`.
- [**0003**](https://github.com/JuggyMcNutty/SurrealEngine/commit/af99616f537480cc63f9f781865e2e76634abe44)
  `gamepad-and-deusex-fixes` -- the pad as polled state, turned into UE1
  joystick keys and axes so `User.ini` bindings decide what it does, with
  menu-mode controls and a `Gamepad` block in `Settings.json` ([the launcher's
  controller support](LAUNCHER.md#controller-support)); and two Deus Ex fixes:
  `CycleActors` resumes where it stopped, as the game's script expects, and the
  first pause-menu press after skipping the intro is no longer swallowed.

### Settings the launcher exposes

- [**0008**](https://github.com/JuggyMcNutty/SurrealEngine/commit/ce78355fb3cc47b2ac27dd18b3751c2c564dd5ce)
  `ai-level-of-detail` -- with `Settings.json` `Performance.AiLevelOfDetail`
  (the Video tab's Distant AI), a pawn out of sight and not within 1500 units
  runs its script thinking every third frame. **Smart Pro:** game tick ~124 →
  ~104 ms; ~38 pawns a frame skip their thinking. **Checked:** the scene
  renders normally (framebuffer capture); whether distant NPCs still behave is
  not yet judged by hand.
- [**0009**](https://github.com/JuggyMcNutty/SurrealEngine/commit/a1a2926f93fbd6be6288f4dd87191ca36ae9f0f9) `render-scale` --
  `Performance.RenderScale` (the Video tab's Resolution): the scene drawn
  smaller than the window and scaled up; Vulkan only. **Checked:** Liberty
  Island at 960×540 and 853×480 fills the panel, the HUD larger (framebuffer
  captures); synchronization validation clean on the desktop at scale 0.667.
- [**0022**](https://github.com/JuggyMcNutty/SurrealEngine/commit/a41d14b1180e2e04957d1b19d7a5406b88800b6c) `ai-lod-far-tier` --
  with Distant AI, a pawn also beyond 4000 units thinks every sixth frame.
  **Smart Pro:** game tick ~65 → ~63 ms; ~48 pawns a frame fall in the tier, ~8
  of them thinking, ~9 fewer thinking each frame.

### Rendering

- [**0004**](https://github.com/JuggyMcNutty/SurrealEngine/commit/e56866259cfd555d44669701e65643e2d0c69b2a)
  `vulkan-frame-overlap` -- the game tick runs while the GPU draws the previous
  frame; swapchain rebuilds wait for the device. **Smart Pro:** GPU wait ~76 →
  ~0.2 ms, the tick ~20 ms longer (CPU and GPU share the SoC's memory).
  **Checked:** Liberty Island mid-fight renders correctly (framebuffer
  capture).
- [**0005**](https://github.com/JuggyMcNutty/SurrealEngine/commit/03afa604679b0e8e89ea5100bb58c1484d677f41)
  `lightmap-lit-spans` -- lightmaps lit only where a light reaches. **Smart
  Pro:** lightmaps ~98 → ~11 ms. **Checked:** the dock pixel-identical before
  and after (framebuffer captures).
- [**0010**](https://github.com/JuggyMcNutty/SurrealEngine/commit/d635be4bda5c027b5e0b34ee3c13aa64ebc28217)
  `clipper-sized-to-image` -- the visibility clipper's occlusion grid has one
  row per image row (it was a fixed 2048×1080). **Smart Pro:** frame ~222 →
  ~213 ms native, ~208 → ~191 ms at 853×480. **Checked:** the dock
  pixel-identical at both; ~830 surfaces pass visibility where ~740 did, all
  hidden by the depth test.
- [**0011**](https://github.com/JuggyMcNutty/SurrealEngine/commit/7599d2b600015df7f2eec1e683cd94b6f55c2e5b)
  `cull-one-sided-back-faces` -- one-sided surfaces seen from behind skipped
  before the visibility test. **Smart Pro:** surface tests ~4,800 → ~2,400 a
  frame and ~22 → ~12 ms; lightmaps ~10 → ~4 ms and their uploads ~11 → ~4 ms
  (most of the burning barrel's lightmaps were back faces). **Checked:**
  captures of Liberty Island and UNATCO HQ's interior differ only in the stats
  overlay's surface count.
- [**0018**](https://github.com/JuggyMcNutty/SurrealEngine/commit/efc2a80026cbc0768503c0c365e15db0d9df2c4b)
  `mesh-vertices-once` -- each mesh vertex animated, lit and fogged once a
  draw, not once per face using it. **Smart Pro:** actor meshes ~28 → ~14 ms,
  render CPU ~92 → ~80 ms.
- [**0019**](https://github.com/JuggyMcNutty/SurrealEngine/commit/abe6d2735c4e4a2a61479e7fc7964b36b61f8e82)
  `mesh-face-batches` -- a run of mesh faces with one texture drawn in one
  device call. **Smart Pro:** actor meshes ~14 → ~12 ms. **Checked:** a capture
  of the dock matches one from before patch 0012 except where time moves things
  (the sky, the NPCs, the stats); the statue and props identical to the pixel.
- [**0020**](https://github.com/JuggyMcNutty/SurrealEngine/commit/96f1b6b4b1d7b59cdfca4c878a93a243116cf98c)
  `clipper-arm-clip-test` -- the clipper's non-SSE (ARM) build skips clipping
  for triangles inside the view, as the SSE build did (an upstream bug).
  **Smart Pro:** visibility ~25 → ~20 ms (with the per-part timers), surface
  tests ~11.8 → ~7.4 ms. **Checked:** a capture of the dock differs only in the
  sky's clouds and the NPCs.
- [**0021**](https://github.com/JuggyMcNutty/SurrealEngine/commit/377cf462b1452f880723cce4305087172bfe7463)
  `surface-points-on-demand` -- a surface's points gathered only when a test
  needs them. **Smart Pro:** visibility ~20.5 → ~19.6 ms.
- [**0023**](https://github.com/JuggyMcNutty/SurrealEngine/commit/56e86e57548c00aa5ccb597a52aed093ceaa9172) `light-tree-kept` --
  the light tree, and each surface's lights from it, kept while no light
  changes. **Smart Pro:** the BSP surfaces' section ~11 → ~8 ms. **Checked:** a
  capture at 853×480 shows the dock's lightmaps as before.
- [**0024**](https://github.com/JuggyMcNutty/SurrealEngine/commit/03e4d0cb9696bbad5b26cdc0489dcc028152c29b)
  `lightmap-neon-conversion` -- the lightmaps' float-to-byte conversion for the
  GPU in NEON on ARM. **Smart Pro:** texture uploads ~3.9 → ~2.1 ms.

### Script VM

With 0013, these took the Smart Pro's script time from ~125 ms a frame to
~30 by 0017; with the collision patches (0025–0027) it was ~25 before 0028.

- [**0006**](https://github.com/JuggyMcNutty/SurrealEngine/commit/af2ed868bfe107485ad905a2c183405f01139391)
  `vm-call-path-without-casts` -- parameters from `Properties`, and a per-class
  virtual-function cache: no `dynamic_cast` on the call path. **Smart Pro:**
  script ~125 → ~84 ms.
- [**0007**](https://github.com/JuggyMcNutty/SurrealEngine/commit/9cc49e284b2e5f3d9f9a12fbd0449117afbc9d79)
  `vm-call-overheads` -- native frames without locals, event names looked up
  once, plain-data locals zero-filled. **Smart Pro:** script ~84 → ~77 ms.
- [**0012**](https://github.com/JuggyMcNutty/SurrealEngine/commit/f4ea318b0b71718e83c19b0e0efd208379bfc91c)
  `vm-evaluator-per-statement` -- one expression evaluator per statement,
  nested values returned directly. **Smart Pro:** script ~60 → ~55 ms.
- [**0014**](https://github.com/JuggyMcNutty/SurrealEngine/commit/829adcbd7e1d6e109ae2cd81f67f4e46a84f5c95)
  `vm-calls-without-allocation` -- script calls without heap allocations or
  walks over every local. **Smart Pro:** script ~39 → ~35 ms.
- [**0015**](https://github.com/JuggyMcNutty/SurrealEngine/commit/6ba1983af99b9fd70a1e6133a70332578a13431c)
  `vm-fast-operators` -- the 25 commonest operators evaluated in place. **Smart
  Pro:** script ~35 → ~31 ms.
- [**0016**](https://github.com/JuggyMcNutty/SurrealEngine/commit/e28aa410d11e3a07848d602d814171d0b99c470f)
  `vm-event-lookup-cache` -- events found through the virtual-call cache.
  **Smart Pro:** within the noise (tick ~71.2 → ~70.7 ms).
- [**0017**](https://github.com/JuggyMcNutty/SurrealEngine/commit/51d45aa37c96a9bc5656d4ce5d89992e94fa8d13)
  `vm-leaf-expressions` -- the commonest leaf expressions made without the
  visitor. **Smart Pro:** tick ~70.7 → ~69.3 ms.
- [**0028**](https://github.com/JuggyMcNutty/SurrealEngine/commit/40e219ac8bc05a349950766408daea74c77c57bb)
  `vm-typed-evaluation` -- conditions, `&&` and `||`, the fast operators and
  assignments to plain variables evaluated as plain values, each node
  classified once, instead of through an 88-byte `ExpressionValue`; a typed
  node carries the offset or operands it reads. **Smart Pro:** script ~25.4
  → ~22.7 ms, tick ~54 → ~51 ms; at 853×480 the frame ~108.5 → ~106 ms.
  **Checked:** also a hash of every actor's state, frame by frame, with the
  frame time and random seeds fixed (the message has both checks).
- [**0029**](https://github.com/JuggyMcNutty/SurrealEngine/commit/7e93fe7b86f0e449454db03d9d8eb02b55d6dcfd)
  `vm-statements-in-place` -- conditions, jumps, assignments to plain
  variables, calls, `return;` and a foreach's next pass run by `Frame::Run`
  in place, without an `ExpressionEvalResult` each; a jump keeps its target's
  statement index. **Smart Pro:** script ~22.7 → ~21.4 ms, tick ~51 → ~50
  ms; at 853×480 the frame ~106 → ~105 ms. **Checked:** the actor-state hash,
  on Liberty Island and UNATCO HQ.

### Game tick

- [**0013**](https://github.com/JuggyMcNutty/SurrealEngine/commit/9720823c814691ca1455cbef65d13c629fac2a60)
  `actor-iterators-by-class` -- the actor iterators find a class's actors from
  an index, not a scan of the level (`CycleActors` alone had been ~16 ms of the
  tick). **Smart Pro:** script ~56 → ~39 ms.
- [**0025**](https://github.com/JuggyMcNutty/SurrealEngine/commit/f984a7800675f85cc5e7eb134de36b03ffa8aef8)
  `ray-trace-segment-split` -- a ray trace hands each BSP child only its own
  part of the segment. **Smart Pro:** tick ~61.5 → ~58 ms.
- [**0026**](https://github.com/JuggyMcNutty/SurrealEngine/commit/0f9ce6cccd7dbc52bf0c71a57ae9490573e08d18) `sight-line-cells`
  -- sight lines test the actors of only the collision cells they cross; hull
  planes on the stack. **Smart Pro:** tick ~58 → ~56 ms.
- [**0027**](https://github.com/JuggyMcNutty/SurrealEngine/commit/469d9c8a26d8e910b14576bfca4fb650862681d3)
  `step-down-one-trace` -- a walking pawn's step to the ground made with the
  trace its dry run made. **Smart Pro:** tick ~56 → ~53 ms.
- [**0030**](https://github.com/JuggyMcNutty/SurrealEngine/commit/2d315e3602663f73f802a25a58c282ae545dafec)
  `ray-plane-tests-first` -- a ray tests a polygon's plane before reading its
  vertex count and surface, which lie on other cache lines. **Smart Pro:** the
  sight rays' polygon tests (`NodeRayIntersect`) ~3.3 → ~1.9 ms.
- [**0031**](https://github.com/JuggyMcNutty/SurrealEngine/commit/0098ca8c5f69c1d2c8ff397a75d1915afc20872b)
  `collision-cell-table` -- each collision cell's actors found in an
  open-addressed table and kept in an array, not a `std::unordered_map` of
  `std::list`s. **Smart Pro:** the traces' walk of the cells
  (`TraceTester::Trace`) ~1.4 → ~0.5 ms, and ~0.3 in the new `FindCell`.
- [**0032**](https://github.com/JuggyMcNutty/SurrealEngine/commit/e9a806d56f88f63efded8ec14f8487afbc863b0a)
  `collision-move-overheads` -- a move asks once per actor whether it is a
  player or a projectile, and traces sort and sift their hits without heap
  allocations. **Smart Pro:** `dynamic_cast` in the tick ~2.9 → ~1.5 ms;
  `TraceTexture`, which collects every hit along a laser beam, ~1.9 → ~0.8
  (0030's share included).

0030–0032 were measured on the Smart Pro together: tick ~50 → ~39 ms, the
collision traces ~12 → ~8 ms a frame, and the render CPU ~2.5 ms less. Each
was checked with the actor-state hash on Liberty Island and UNATCO HQ (their
messages say how).

### Gameplay

What Surreal Engine lacked for Deus Ex to play as it should.

- [**0033**](https://github.com/JuggyMcNutty/SurrealEngine/commit/bec6e261edcd00d9225cb95ef7e4a8e0b7298261)
  `vm-omitted-optional-arguments` -- a script call that leaves out an optional
  struct or array argument no longer crashes copying it (upstream's bug): the
  first NPC to attack hit it, in `ScriptedPawn.ComputeBestFiringPosition`.
- [**0034**](https://github.com/JuggyMcNutty/SurrealEngine/commit/b5d08853dbf4e24894d56942c07a5a743438e824) `deusex-ai-sight`
  -- NPCs see: `IsValidEnemy`, `AICanSee` and `AIVisibility` as the original
  DLLs have them; upstream had the first wrong and the others as stubs, so no
  NPC ever noticed the player, or anyone. **Smart Pro:** the sight checks take
  ~0.3 ms of the tick. **Checked:** on Liberty Island an NSF terrorist, with
  the player moved in front of it, made the player its enemy ~3.5 s later
  (the build-up the script gives a faint sighting at night), and it and two
  more shot at the player; NPCs of hostile alliances check each other.
- [**what stopped the game**](https://github.com/JuggyMcNutty/SurrealEngine/commit/db0df9a1205ef9f55c9abf83d61c7da26efdbac0) --
  the roadmap's M0 ([`ROADMAP.md`](ROADMAP.md)): `ReachablePathnodes` makes
  an (empty) iterator instead of stopping the VM in Battery Park's opening
  fight; the save's `DeusExSaveInfo` lives in package DeusEx, so a save no
  longer dies writing it; `GetConfig` answers from the system ini;
  `GetPawnAllianceType(None)` is Neutral; integer division by zero gives 0;
  string `>` is native 116, not the typo 1186. **Checked:** an 80 s Battery
  Park run and quick saves in two maps run out their clocks; the commit's
  message has the rest.
- [**saves, the original's way**](https://github.com/JuggyMcNutty/SurrealEngine/commit/4dfc7a6dd5b1571d7fbab03fac6fd4e5a899d78b) --
  the first slice of the roadmap's M1: slots numbered highest-plus-one, the
  quick save in QuickSave, Current copied into the slot with the level saved
  on top, the SaveInfo filled and named as the original's
  (`MyDeusExSaveInfo`), the save listing and kept infos, and DELETEGAME.
  **Checked:** against the original's reference saves; the commit's message
  has the runs.
- [**loading**](https://github.com/JuggyMcNutty/SurrealEngine/commit/0058015cccdadd262b56eb819bbd00f7213426a3) --
  `?loadgame=N` as the original's `Browse`: the slot's SaveInfo names the
  map, Current takes the slot's copy, the map loads from Current, the saved
  pawn is possessed; -1 the quick save. Behind it, the save info in a
  package of its own, and packages born empty carrying version 68 -- with
  either wrong, a written SaveInfo.dxs cannot be read back. **Checked:** a
  slot and the quick save round-trip in play; the original's saves stop at
  their saved event manager (ported with the AI event system, later).
- [**travel keeps the mission**](https://github.com/JuggyMcNutty/SurrealEngine/commit/1a654c7e481bcf5ab223d702df14d85c4f76a11f) --
  within a mission the departing level is pruned and saved into Current,
  and a map saved there is revisited as the player left it, its pawn found
  again by the game's own login; a new mission, a new game or ?restart
  empties Current. **Checked:** a travel out and back revisits from
  Current, and the slot save after it has the reference hub save's shape.
- [**history in the level**](https://github.com/JuggyMcNutty/SurrealEngine/commit/33c1e3f896010ca4138d912becc71be732063f82) --
  the player's history, log and notes are made in the level, as the
  original's, so a save keeps them.
- [**flags as the original's**](https://github.com/JuggyMcNutty/SurrealEngine/commit/cd973b15a6dbf9042173c2a1039dd55b6dbc3d98) --
  chains past 64 by a CRC of the name, stamped expirations, expiry to a
  criteria, -1 for a missing flag, and typed flags found again. Its cleanup
  of a test hook cut real main-loop code, restored by
  [the commit after](https://github.com/JuggyMcNutty/SurrealEngine/commit/4f6ed66a469ac232245e3fa1493d44c24b4dc0c0).
  **Checked:** an in-engine self-test for the flags; a 60 s run for the loop.
- [**list fields read back**](https://github.com/JuggyMcNutty/SurrealEngine/commit/fb7b29d64cbca6e48a81032b99197200e430b7e9) --
  GetField's column test was inverted, so every screen keeping what a row
  stands for in a hidden column read nothing. The save picture stays
  missing until the renderer gets a capture point its frame overlap
  allows.
- [**flags kept and carried**](https://github.com/JuggyMcNutty/SurrealEngine/commit/c8bf4374ac27a97ea80d04487ad9406ee930d52f) --
  the flag base and its flags live in the level package, so a save keeps
  them, and they cross a travel in the pawn's travel graph: the travel
  serialization walks every element of a fixed-array object property (the
  base's 64 buckets), travelled non-actors land in the new level's
  package, the travel info is captured before the pre-travel prune, and
  the prune deletes the departing level's flags as the original's does.
  GameDirectory objects are made per call again. **Checked:** 21 flags
  set across the buckets survive a travel and a save's load-back.
- [**conversations as the original's**](https://github.com/JuggyMcNutty/SurrealEngine/commit/39e9df99bee7938823ec3ee9ddf5a21468d1d0e6) --
  the roadmap's M2 conversations item: comment events kept, an actor's
  conversations bound by the original's bark rule from the list the level's
  `ConversationPackage` names, the bound-actor slots filled so a destroyed
  actor ends its conversation, cycle-once chatter holding its last line,
  and each line's sound loaded alone, by name
  ([conversations](re/natives.md#conversations)). **Checked:** the intro's
  scene plays its lines at their own lengths; a temporary hook printed
  Liberty Island's bound lists, the named troopers owning their own
  conversations plus the `_Bark`s; 90 s and 75 s runs clean.
- [**the text parser as the original's**](https://github.com/JuggyMcNutty/SurrealEngine/commit/752ff87d222426b2aa305eac563b74344bc88ca5) --
  the roadmap's M2 parser item: the original's tokens (CR and LF as spaces,
  nothing trimmed, the first `<P>` swallowed), its 30-name tag table matched
  by start, its fields split at commas, its hiding to an end tag, the
  player's first name, and the colours read
  ([what the player reads](re/natives.md#what-the-player-reads)). **Checked:**
  a temporary hook put five texts through it against their SDK sources --
  emails list with their fields, bulletins open, comments hide, header
  spaces and blank lines keep, `PLAYERFIRSTNAME` gives the first name; a
  70 s run after the hook's removal is clean.
- [**the list window as the original's**](https://github.com/JuggyMcNutty/SurrealEngine/commit/48c0987a562d64820081012a2602ff3e61425d85) --
  the roadmap's M2 lists item: rows activate on a double click or Enter
  (`ListRowActivated`, which key rebinding hangs on), `MoveRow` takes the
  keys and a pad's d-pad through a list, the original's sorting and column
  keys, auto-expanding columns and the original's new-column defaults,
  hidden columns unseen, float fields keeping their number and shown
  through the column's format ([lists](re/natives.md#lists)). **Checked:**
  a temporary in-engine self-test drove sorting (name, number, reverse),
  the number reader (hex, octal, hours and minutes), the format, the moves
  and a delete's focus; a 70 s run after its removal is clean.
- [**render time and stasis**](https://github.com/JuggyMcNutty/SurrealEngine/commit/c7b3e00624c314757db5655caf8f35f9549d40c7) --
  the roadmap's first M3 item: the renderer stamps when each actor, zone
  and decal was last drawn, `LastRendered()` answers the time since, and
  the tick of an actor in stasis -- the original's full test, not "stasis
  allowed" -- does nothing and destroys a transient one
  ([out of sight](re/natives.md#out-of-sight)). **[perf]** to re-measure on
  the Smart Pro with M3's AI work. **Checked:** a temporary snapshot hook
  counted Liberty Island each 8 s -- unseen trees and lamps entered stasis
  as they aged past 5 s while the drawn-recently count fell; a 70 s run
  after its removal is clean.
- [**the AI event system**](https://github.com/JuggyMcNutty/SurrealEngine/commit/248538769f65b8e3393d102bdf6136ebe369ed22) --
  the roadmap's M3 hearing item: the original's `UEventManager`, one per
  level, saved with it -- senders' 16-frame rings and current levels,
  receivers in one ring walked under the original's turn and 2 ms rules,
  scores, senses (`AICanSee`, the original's `AICanHear`), and Begin, End,
  Pulse and ChangeBest to the listeners' script
  ([hearing](re/natives.md#hearing-the-ai-event-system)). The manager's
  class is synthesized into the Engine package, so a save's import of it
  resolves; the original game's saved manager is recognized and skipped,
  which is what lets the original's saves load now. **Checked:** a
  temporary hook stood the player beside a terrorist and raised
  WeaponFire -- distress seen by sight, footsteps heard fading, HandleShot
  fired, and the terrorists' own gunfire became senders; a quick save
  carried 10 event types and 325 listeners through a load; the reference
  Liberty Island save of the original game loads and plays; a 75 s run
  after the hooks' removal is clean.
- [**ScriptedPawn's native tick**](https://github.com/JuggyMcNutty/SurrealEngine/commit/4bd245b6e24503319ae5da17eee8c84ddc1e67cd) --
  the roadmap's M3 tick item: disappearing, the pivot's easing, agitation
  and fear (the script's own unused `UpdateAgitation` and `UpdateFear`),
  the sixteen AI timers, cloaking, the advanced-tactics manoeuvre's end,
  burning out and bleeding, in the original's order before the actor tick
  ([the native tick](re/natives.md#the-native-tick-ascriptedpawntick)).
  **Checked:** a temporary hook set a patrolling terrorist's fields and
  read the timers counting, the distress rising, the pivot tweening under
  the script's own values and the bleeding correctly gated off beyond
  1,200 units; 90 s and 65 s runs show no script error.
- [**moving**](https://github.com/JuggyMcNutty/SurrealEngine/commit/9df38d9520c7daed9d78fc28bdf59eef6934d9c8) --
  the roadmap's M3 moving item plus the traces item's
  `RandomBiasedRotation`: `AIDirectionReachable` walks, swims or flies the
  pawn itself along a direction and puts it back; `AIPickRandomDestination`
  tries biased random directions through it; `ReachablePathnodes` iterates
  the original's `GetPathnodeList` and `ComputePathnodeDistances` floods
  the network from it
  ([moving](re/natives.md#moving-wandering-and-tactical-movement)).
  **Checked:** temporary probes on Liberty Island -- a spot found 280
  units along a pawn's facing, 13 pathnodes nearest first, the flood
  reaching 876 of 1,198 navpoints; a 70 s run after their removal is
  clean, no moving native left unimplemented in it.
- [**traces, moves, probes and conversions**](https://github.com/JuggyMcNutty/SurrealEngine/commit/6f9b5e80cde903b4341d5656ffefc188515b39c1) --
  the roadmap's last M3 item: one probe mask per object set at every
  `GotoState` and saved as the original's `FStateFrame` keeps it (a save
  from before this commit restores its pawns' probes wrongly -- dev saves
  only); the bool, vector, rotator and object conversions; `VRand` inside
  the unit sphere; the trace iterators over the original's
  `MultiLineCheck`; `ParabolicTrace` whole; `GetBoundingBox` at a test
  place; `SetPhysics` taking its floor; the strafes at Deus Ex's speed
  ([implemented, not as the original](re/natives.md#implemented-not-as-the-original)).
  **Checked:** a terrorist's probe mask -- a real sparse mask, not the old
  all-on -- round-trips a quick save exactly; the intro's scene plays 238
  lip-sync lines through the new mask; 60-70 s runs on both maps after the
  hook's removal are clean.
