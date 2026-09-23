# Surreal Engine patches

[Surreal Engine](https://github.com/dpjudas/SurrealEngine) is an open-source
reimplementation of Unreal Engine 1. It recognises this exact Deus Ex build —
`DeusEx.exe` SHA1 `2a933e26aa9cfb33b37f78afe21434caa031f14a` is its
`DEUS_EX_1112fm` database entry — and it is the runtime this launcher is aimed
at.

## Fork only. Never upstream.

The project ships a `NO-AI Code Rule.md`:

> If you are primarily using LLM tools such as Claude to make code changes to
> this codebase then please do not PR it to us. Keep it in a fork. Thank you.

These patches were written with Claude. They stay in a fork and are **not** to
be submitted upstream, in any form. Using and building the engine is separately
permitted by its own license, which grants use "for any purpose".

If a change here turns out to be worth upstreaming, it needs rewriting by a
person from the problem statement, not adapting from this diff.

## Base

Applied on top of the upstream commit in `UPSTREAM-BASE.txt`, one fork commit
per patch file:

- `0001-headless-and-embedded-support.patch` — headless/desktop-less support and
  the aarch64 cross build (fork commit `f764c27`).
- `0002-nonbindless-fallback-and-format-support.patch` — the texture path
  (`868e8d2`).
- `0003-gamepad-and-deusex-fixes.patch` — controller support and two Deus Ex
  fixes (`e84d3e8`).
- `0004-vulkan-frame-overlap.patch` — the game tick runs while the GPU draws
  the previous frame (`0bfde8a`).
- `0005-lightmap-lit-spans.patch` — lightmaps are lit only where a light
  reaches (`c42fae4`).
- `0006-vm-call-path-without-casts.patch` — script calls find parameters and
  virtual functions without `dynamic_cast` (`1fb6deb`).
- `0007-vm-call-overheads.patch` — native frames without locals, event names
  looked up once, plain-data locals zero-filled (`c4b46f1`).
- `0008-ai-level-of-detail.patch` — pawns out of sight think every third frame,
  when `Settings.json` asks for it (`4222051`).
- `0009-render-scale.patch` — the scene drawn smaller than the window and
  scaled up, when `Settings.json` asks for it (`2f0143b`).
- `0010-clipper-sized-to-image.patch` — the visibility clipper's occlusion grid
  has one row per row of the image (`e955949`).
- `0011-cull-one-sided-back-faces.patch` — one-sided surfaces seen from behind
  are skipped before the visibility test (`a8b5a2f`).
- `0012-vm-evaluator-per-statement.patch` — one expression evaluator per
  statement, nested values returned directly (`fdf89ce`).
- `0013-actor-iterators-by-class.patch` — the actor iterators find a class's
  actors from an index instead of a scan of the level (`e5ae2ca`).
- `0014-vm-calls-without-allocation.patch` — script calls without heap
  allocations or walks over every local (`4d5b8e7`).
- `0015-vm-fast-operators.patch` — the commonest operators evaluated in place
  (`1950f5e`).
- `0016-vm-event-lookup-cache.patch` — events found through the virtual-call
  cache (`568646c`).
- `0017-vm-leaf-expressions.patch` — the commonest leaf expressions made
  without the visitor (`8843675`).
- `0018-mesh-vertices-once.patch` — each vertex of a mesh draw animated, lit
  and fogged once, not once per face using it (`734c6b3`).
- `0019-mesh-face-batches.patch` — a run of mesh faces with one texture drawn
  in one device call (`587ce9c`).
- `0020-clipper-arm-clip-test.patch` — the visibility clipper's non-SSE
  (ARM) build skips clipping for triangles inside the view, as the SSE build
  did (`f7188e7`).
- `0021-surface-points-on-demand.patch` — a surface's points gathered only
  when a visibility test needs them (`2d08d1d`).
- `0022-ai-lod-far-tier.patch` — with AI level of detail, pawns out of sight
  beyond 4000 units think every sixth frame (`da0fa3f`).
- `0023-light-tree-kept.patch` — the light tree, and each surface's lights
  from it, kept while no light changes (`f3ee690`).
- `0024-lightmap-neon-conversion.patch` — the lightmaps' float-to-byte
  conversion for the GPU in NEON on ARM (`edb0ecb`).

Each file is its commit's `git format-patch` output (`0001` was regenerated
with its header on 2026-09-22; it had been a bare diff), so `git am` applies
them in order onto `UPSTREAM-BASE.txt` and reproduces the fork's tree exactly.
`scripts/engine.sh` keeps it that way:

```sh
scripts/engine.sh fetch                     # clone upstream at the base, git am the patches
scripts/engine.sh check                     # the fork's commits == these files, in order
scripts/engine.sh export <commit> 000N-name # after committing a change in the fork
scripts/engine.sh build <port>              # build/<port>/engine, from ports/<port>/engine.cmake
```

The fork itself lives in `engine/SurrealEngine` (branch `deusex-handheld`), a
separate clone the main repository ignores. Builds go to `build/<port>/engine`,
never into the clone. (Patch 0001's commit message points at the README under
port/ -- what this repository's directory was called then. Rewording it would
change the fork's commit ids.)

Temporary debugging hooks never go into a patch. They carry a
`TEMPORARY DEBUG TOOL` comment and are reverted before committing; the
frame-time profiling hooks live in `optional/perf-instrumentation.patch` so
they can be re-applied: `scripts/engine.sh perf on`, and `perf off`
afterwards. The patch is against the fork's head, so a fork commit that
touches the same lines moves them: `perf on` then falls back to a three-way
merge (and stops if that leaves conflicts to resolve), and `perf save`
rewrites the patch from the tree so the next `on` and `off` apply cleanly.
Take the hooks off before changing the engine itself -- a commit made with
them on carries them -- and commit before putting them back: `perf on` cannot
merge over uncommitted changes to a file the hooks touch, and says so.

The hooks also build the engine with frame pointers (~1% slower on the
handheld) and carry a sampling profiler for devices without `perf`:
`SURREAL_PERF_SAMPLE=<file>` samples the main thread's CPU time with a
thread CPU-time timer, recording each sample's program counter and the
return addresses a frame-pointer walk finds, one block per 60-frame report,
with `/proc/self/maps` beside it. [`scripts/sample-report.py`](../scripts/sample-report.py)
(Python 3, and the port toolchain's `nm` for a cross build) turns that into
self and inclusive time per function, optionally under one caller
(`--root ULevel::Tick`: the game tick) or with the callers of one
(`--callers`). A leaf function keeps no frame record, so its samples show
its caller's caller as the next frame. Running it on the handheld is in
[its README](../ports/trimui-smartpro/README.md#performance).

## What the patches change

Patch 0001 has nine changes, in two groups.

### Driving the engine without a desktop

Three changes, all in `SurrealEngine/GameApp.cpp`. None touch engine behaviour;
they exist because the upstream front end assumes a desktop with a mouse.

1. **Skip the launcher window** when a game folder is given on the command line,
   or on `--no-launcher`. Upstream always calls `LauncherWindow::ExecModal()`,
   so the engine cannot be started without clicking a desktop GUI. Needed to
   drive it from a test rig, and needed on the handheld — there is no mouse, and
   game selection already happened in our own launcher before this process
   started.

2. **Report exceptions to stderr** as well as the modal error window. The error
   text was otherwise only visible inside a GUI; with no one to close it, the
   modal spins at 100% CPU in its event loop. This is how the real first failure
   ("Failed to initialize OpenAL device") was found at all.

3. **Stream the engine log to stderr** in headless/verbose mode. `LogUnimplemented()`
   is how the engine reports which natives a game still needs, and that list is
   the actual work item for Deus Ex support — it is worth nothing trapped in a
   window.

### Cross-compiling for an embedded aarch64 target

All in the build system and the SDL2 backend. None of it changes behaviour on a
desktop build.

1. **Gate the desktop display backends.** `SurrealWidgets` hard-required
   `find_package(OpenGL REQUIRED COMPONENTS EGL)` and always compiled and linked
   the X11 backend. A handheld has no X11, no Wayland compositor, no dbus and no
   desktop GL. New `ENABLE_X11`/`ENABLE_WAYLAND` options (both ON by default)
   let an SDL2-only build configure; `window.cpp` already had null-returning
   stubs for every backend whose `USE_*` define is absent, so nothing else
   needed touching.

2. **Missing includes in the SDL2 backend.** `sdl2_display_backend.cpp` and
   `sdl2_display_window.cpp` used `strcmp`, `memcpy` and `std::round` without
   `<cstring>`/`<cmath>`. That backend is rarely built on Linux, where X11 and
   Wayland are preferred, so it had not been noticed.

3. **SDL discovered by pkg-config links nothing.** The link step used
   `${SDL2_LIBRARY}`, which `find_package` sets but `pkg_search_module` does not
   — it sets `SDL2_LIBRARIES`. A cross build takes the pkg-config path, so every
   SDL symbol came out undefined. Now tries the imported target, then
   `SDL2_LIBRARIES`, then `SDL2_LIBRARY`.

4. **`zipdir` must run on the build machine.** It packs the resource zip during
   the build, so a cross build produces an aarch64 binary that cannot execute.
   `ZIPDIR_EXECUTABLE` now points at a host-built one, and the in-tree target is
   skipped when it is set. `scripts/engine.sh build` does this for any port
   whose `engine.cmake` names a cross toolchain, building the host `zipdir`
   once into `build/host-tools/`.

5. **System font lookup without a desktop.** `resourcedata_unix.cpp` asked
   GSettings for the GNOME UI font and resolved it with fontconfig. Neither
   exists on a handheld. Guarded by `SURREALWIDGETS_DESKTOP_FONTS` (defined only
   when both glib and fontconfig are found); otherwise it reads the fonts the
   device actually ships, overridable via `SURREALWIDGETS_FONT`.

6. **Exit status reflects failure.** `GameApp::main` returned 0 even after
   catching an exception. Our launcher keys its crash sentinel off the exit
   status, so a failed start was being recorded as a clean run.

## Patch 0002 — the texture path

Two changes, both needed for GPUs without desktop texture support. Both were
written for the PowerVR Rogue GE8300 (TrimUI Smart Pro, Vulkan 1.3.225) and
tested there; the host AMD card still takes the bindless path.

### 10. Non-bindless texture fallback

The device filter no longer requires `VK_EXT_descriptor_indexing` outright.
`VulkanRenderDevice` computes `SupportsBindless` from the enabled features and
falls back to **one four-binding descriptor set per texture combination**
(surface/macro/detail/lightmap) instead of indexing an unbounded array:

- `DescriptorSetManager` gains `SceneLayout`/`SceneSets`/`SceneSetCache` with
  pooled allocation and a `TexDescriptorKey` cache; the cache is drained with
  a `FlushDrawBatchAndWait()` first, because dropping sets that pending
  commands still reference is a validation error.
- `VulkanRenderDevice` splits batches on descriptor set changes
  (`Batch.DescriptorSet` + `SetTextureSet`), and both `GetTextureIndexes`
  overloads return `ivec4(0)` in fallback mode.
- `Scene.frag` compiles both ways under `BINDLESS_TEXTURES`; without it the
  four samplers are bound directly. `EndFlash` binds a dummy set, since
  nothing is sampled there but a pipeline still needs one.
- The scene pipeline layout takes whichever set layout the device can do.
- `SURREAL_VK_NO_BINDLESS=1` forces the fallback on capable GPUs, which is how
  it was exercised on the host before the device run.

### 11. Texture format support checks

`TextureUploader::GetUploader` now takes the physical device and requires both
`VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT` and
`VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT` — either missing is
undefined behavior with the engine's linear samplers. Unsupported formats fall
back to CPU decoders writing a format the device does support:

| Format | Decodes to | Why |
| --- | --- | --- |
| BC1 (and BC1_PA) | RGBA8 | DXT1; GE8300: unsupported |
| BC2, BC3 | RGBA8 | DXT3/DXT5; GE8300: unsupported |
| BC4 | R8 | GE8300: unsupported |
| BC5 | RG8 | GE8300: unsupported |
| RGB8 | RGBA8 | `R8G8B8_UNORM` unsupported on the GE8300 |
| RGBA32F | RGBA8 | lightmaps and fog maps; samples but cannot be linearly filtered on the GE8300 |

Formats with no decoder (BC6H, BC7, ETC, ASTC) keep the existing white
fallback. The probe that produced the format table is
`tools/probes/probe-texture-formats.c`.

On-device result: the intro previously rendered as dense speckle garbage
(BCn sampled in unsupported formats, then lightmap/fog speckle from the
missing filter bit). With the decoders it renders clean.

### MSAA off on the GE8300 (port side, not a patch)

With the texture path fixed, 4x MSAA — the engine default when there is no
`Settings.json` — still produced edge speckle from the PowerVR resolve. The
launcher writes `Settings.json` before every launch with `Antialias: Off` and
turns it off again if it finds anything else on a PowerVR GPU; the Smart Pro
port's `engine-settings.json.default` (seeded by `run-game.sh` and by
`scripts/dx.sh deploy` when the file is missing) says `Off` too. Host AMD was
never affected because its settings file already said `Off`.

## Patch 0003 — controller support, and two Deus Ex fixes

Fork commit `e84d3e8`, built and running on the device.

### 12. Gamepad as a polled device

`SurrealWidgets/include/surrealwidgets/window/window.h` gains `GamepadState`
(six axes, fifteen buttons in SDL's order) and two `DisplayBackend` virtuals
with no-op defaults: `GetGamepadState()` and `SetGamepadKeyEmulation()`. The
SDL2 backend implements both — it keeps the first attached controller, reopens
after a hot-unplug, and when emulation is off it drops the controller button
events it used to turn into Enter/Escape/arrow keys (every button would
otherwise arrive twice). Polling instead of a new event keeps the change out of
`Widget`'s focus dispatch entirely; other backends report no pad.

### 13. `GamepadInput`

`SurrealEngine/GamepadInput.{h,cpp}`, called from `Engine::UpdateInput` after
`TickWindow`:

- In play, buttons become the UE1 joystick keys (A=`Joy1` … START=`Joy8`,
  L2/R2=`Joy11`/`Joy12` past half travel with hysteresis, d-pad=`JoyPov*`) and
  go through `Engine::OnWindowKeyDown/Up` like keys, so `User.ini` bindings
  decide what they do. Sticks and triggers become `JoyX/Y/U/V/Z/R` axis events
  every tick while deflected (±100 at full, up positive, radial dead zone, a
  response curve on the look stick), with one `IST_Release` when a stick
  recentres so `activeInputAxes` clears.
- While `dxRootWindow->IsModalOpen()` (menus, inventory, conversations,
  keypads): either stick moves the pointer through `OnWindowRawMouseMove`,
  A/X click, B/Y/SELECT/START are Escape, the d-pad is the arrow keys with
  key-style repeat, L1/R1 are the mouse wheel.
- A button is released to whichever side saw its press, so a button held while
  a menu opens or closes cannot stay down.
- `PollSkip()` lets A/B/SELECT/START skip a movie: `PlayVideo`'s loop reads no
  other input.

### 14. `Gamepad` in `LauncherSettings`

`Enabled`, `DeadZone`, `LookSensitivityX/Y`, `InvertY`, `CursorSpeed`,
`Layout`, read from `Settings.json` (absent members keep their defaults — a
missing bool would otherwise read as false and turn the pad off) and written by
`Save()` so the desktop launcher window does not drop them. `Enabled=false`
restores the old key emulation.

### 15. `CycleActors` resumes where it stopped

Deus Ex's native 1002 (`Actor.CycleActors(class BaseClass, out Actor, out int
Index)`) is a resumable scan: `ScriptedPawn.CheckEnemyPresence` keeps
`CycleIndex` between ticks, checks ~20 candidates per tick, and treats "the
index went down" as "cycled through everyone". Surreal's iterator ignored the
incoming index — it rebuilt a list of every matching actor (a name-based `IsA`
on all ~2,500 actors of Liberty Island) on every call and always started from
the first. So it was the costliest native in the game (~0.66 ms a call, ~46
calls a frame), and each NPC only ever considered the first ~21 pawns. It now
continues after `Index`, wraps, stops after one lap, and tests the class by
pointer along `BaseStruct`.

### 16. The first pause-menu press after the intro

`DeusExPlayer.ShowMainMenu` sets the travel variable `bIgnoreNextShowMenu` when
Escape (or START) skips the intro, to swallow a second menu event the original
engine delivered for that press. Surreal delivers one, so the flag carried into
the first level and ate the player's first attempt to open the pause menu.
Nothing else sets it; `LoginPlayer` and `PossessSavedPlayer` clear it once a
level is running (Deus Ex only).

## Patch 0004 — the CPU and the GPU in parallel

Fork commit `0bfde8a`, measured on the device: the fight on Liberty Island went
from ~450 to ~400 ms a frame
([the Smart Pro's Performance](../ports/trimui-smartpro/README.md#performance)).

### 17. The end-of-frame wait moves to the next frame

`Unlock` submitted the frame and waited on its fence straight away, so a frame
cost CPU time plus GPU time. The end-of-frame submit (`VulkanRenderDevice::Submit`
with `wait = false`) now leaves the frame in flight: `CommandBufferManager`
keeps its command buffers and the frame's delete list alive, and
`WaitForFrame()` collects it before anything could touch what it uses -- at
`Lock`, when a new draw or transfer command buffer starts, before any write to
the shared upload buffer, before the texture and descriptor caches are cleared,
and before a swapchain rebuild. Everything the frame reads is still single:
the next frame's drawing starts only after the wait, so the gain is the game
tick (and the rest of the main loop) overlapping the GPU, with no
double-buffering. Mid-frame flushes wait as before, and so does `Unlock` when a
hit buffer must be read back.

### 18. Swapchain rebuilds wait for the device

A fence covers a submit, not the present queued after it. Rebuilding the
swapchain (a resize, a VSync or HDR change) destroyed the old one and its
semaphores while the presentation engine could still hold them -- the
validation layer's `VUID-vkDestroySwapchainKHR-swapchain-01282` and
`VUID-vkDestroySemaphore-semaphore-05149`, present before patch 0004 as well.
It now calls `vkDeviceWaitIdle` first; the path runs only on a rebuild.
Synchronization validation is clean on Liberty Island with the patch, on both
the bindless and the per-batch descriptor set path (`SURREAL_VK_NO_BINDLESS=1`).

## Patch 0005 — lightmaps lit only where a light reaches

Fork commit `c42fae4`. On Liberty Island's fight, lightmaps went from ~98 to
~11 ms a frame on the Smart Pro
([Performance](../ports/trimui-smartpro/README.md#performance)).

### 19. Lit spans

Every light's effect (`LightEffect::Run`) and contribution
(`LightmapBuilder::AddLightContribution`) ran over every texel of a lightmap,
though every effect but the cylinder gives exactly zero beyond the light's
radius. The cost showed on Liberty Island, where a burning barrel -- a dynamic
light with the fire waver effect, which re-lights what it touches every frame --
had twelve large surfaces rebuilt per frame: ~300,000 texels walked for ~7
lights each, ~550 texel-light pairs actually in reach.

`LightmapBuilder::FindLitSpans` now finds, row by row, the texels within the
light's radius, and both passes work on those spans only. A row of texel
positions is a line -- `CalcWorldLocations` interpolates between the row's ends
-- so a row's span is where that line is inside the light's sphere, plus a
texel of margin. The lightmap as a whole is *not* an affine grid (its
positions are extrapolated from a small triangle, and rows drift up to ~100
units from a plane), which is why this is per row: a first, per-rectangle
version missed 124 of 3.3 million texels in reach. The per-row version missed
none of 3.1 million, across 232,000 light passes; the check was a temporary
walk over every texel, not part of the patch.

Static lights skip their shadow map when nothing is in reach. Cylinder lights
and one-texel-wide lightmaps still cover the whole lightmap. The values are
the same, save that a texel can now land in a vector loop's scalar tail or
the other way round (rounding at 1e-7); screenshots of the dock before and
after match pixel for pixel.

## Patch 0006 — script calls without casting

Fork commit `1fb6deb`. Profiling the desktop build on Liberty Island (Linux
`perf`) put `dynamic_cast` at ~15% of all samples, nearly all on the script
call path. On the Smart Pro, script time went from ~125 to ~84 ms a frame and
the fight from 3.3 to 4.0 FPS
([Performance](../ports/trimui-smartpro/README.md#performance)).

### 20. Parameters from `Properties`

`Frame::Call`, `CallScript` and `CallNative` found a function's parameters by
walking its `Children` and casting each one to `UProperty` -- three or four
walks per call. A function's `Properties` array is exactly those children in
order (`UStruct::Load` collects them, and functions inherit none), so the walks
use it. `CallScript` also stopped building a variable reference for every
local, only for the parameters it copies. A temporary check compared the two
for every function called on Liberty Island: all matched.

### 21. A virtual function cache per class

`ExpressionEvaluator::Expr(VirtualFunctionExpression*)` searched on every
virtual call: the class hierarchy's state maps, then every field of every
class, cast to `UFunction`, until the name matched. The answer depends only on
the class, the state name and the function name, none of which change once
loaded, so `UClass::VirtualFunctionCache` keeps it, keyed by the two names'
compare indexes. A search that finds nothing is not cached (it throws, as
before).

## Patch 0007 — less work per script call

Fork commit `c4b46f1`, from a profile of the game tick alone (`perf` with
DWARF call graphs, samples under `ULevel::Tick`): allocation and set-up were
about a quarter of it. On the Smart Pro, script time went from ~84 to ~77 ms
and the fight from 4.0 to 4.2 FPS.

### 22. Native frames without locals

`CallNative` put a full `Frame` on the call stack, which allocated and
constructed the native function's locals. Natives get their arguments
directly and never `Run`; only `Frame::Run` reads a frame's `Variables`. The
frame is now built with `Frame::NoLocals` -- for operators alone that is
thousands of allocations a frame.

### 23. Event names looked up once

`Frame::Call` mapped the function's name to an `EventName` on every call.
`UFunction::EventIndex` keeps the answer, and `UObject::IsNonEventEnabled` is
the rest of `IsEventEnabled(name)` for names that are not events.

### 24. Plain-data locals

`LocalVariables` constructed and destructed each local through a virtual
call. When every property of a function is plain data -- numbers, bools,
names, object references, and structs of those, all of which construct to
zero bytes and have nothing to destruct -- the locals are zero-filled at once
and not destructed. `UStruct::PlainDataLocals` remembers which functions
qualify; strings, dynamic arrays and maps keep the old path.

## Patch 0008 — AI level of detail

Fork commit `4222051`. A speed-for-fidelity choice, off unless
`Settings.json` `Performance.AiLevelOfDetail` is true -- the launcher's Video
tab row Distant AI, on by default on the Smart Pro. There the fight went from
~124 to ~104 ms of game tick, 4.2 to 4.5 FPS.

### 25. Thinking every third frame

`UActor::ThinkThisFrame` decides, for a pawn the player does not control,
whether its script `Tick` event and state code run this frame. A pawn that
was not in view last frame and is not within ~1500 units thinks every third
frame, and its `Tick` then gets all the time that passed; pawns are spread
over the three frames. Movement and latent moves, physics, animation,
timers and the engine's sight checks still run every frame, so nothing
jumps. The cost is reaction time: such a pawn notices things up to two
frames late.

"In view" is the renderer's own per-actor test: `VisibleActor::Process` sets
`UActor::LastVisibleFrame` when an actor's box passes the clipper, and
`RenderSubsystem::SceneFrameStart` says where the last scene's frame numbers
began. The BSP node test behind `LastDrawFrame` was tried first; outdoors on
Liberty Island it called ~50 of 73 pawns visible.

`LauncherSettings` reads and writes the `Performance` block like `Gamepad`:
absent members keep their defaults.

## Patch 0009 — render scale

Fork commit `2f0143b`. `Settings.json` `Performance.RenderScale` (default 1)
is the scene's size as a fraction of the window's -- the launcher's Video tab
row Resolution, which offers the panel's own and the usual resolutions below
it down to 480 lines. On the Smart Pro, 853×480 took the fight from 4.5 to
4.8 FPS ([Performance](../ports/trimui-smartpro/README.md#performance)).

### 26. The render size

The Vulkan renderer already draws into offscreen images and scales the result
to the swapchain (`DrawPresentTexture`, through a linear sampler), so drawing
smaller is a matter of what size the game is told. `RenderDevice` gains
`GetRenderScale`, `GetRenderWidth` and `GetRenderHeight`; only a device that
`SupportsRenderScale` (Vulkan) applies the setting, so OpenGL and D3D11 keep
drawing at the window's size. The render size replaces the window's wherever
the game used it: `GameWindow`'s pixel size (and so the viewport rect and
`getcurrentres`), the scene textures, `ReadPixels`, the widget canvas' frame.
The swapchain stays at the window's size.

Window positions and pointer deltas arrive in window pixels, so the UI scales
them into render pixels (`URootWindow::OnWindowRawMouseMove`, the viewport's
`WindowsMouseX`/`Y`); mouse look is untouched. Deus Ex's UI scale is
`round(height / 600)`, at least 1, so at 540 and 480 lines the menus keep their
pixels and cover more of the screen.

The engine's own resolution (`FullscreenViewportX`/`Y`, `setres`) is the
window's, snapped to the display's modes -- on the handheld only 1280×720 --
which is why this is a separate setting. `BspClipper`'s occlusion grid
followed neither until patch 0010.

## Patch 0010 — an occlusion grid the size of the image

Fork commit `e955949`. On the Smart Pro the fight went from ~222 to ~213 ms at
native resolution and from ~208 to ~191 ms at 853×480 (5.2 FPS).

### 27. One grid row per image row

`BspClipper` decides what is hidden by rasterising occluders into per-row
span lists, and it had a fixed 2048×1080 grid whatever the image. Its cost is
per row -- every occluder and every box test walks the rows it covers -- so
on a 720-line screen it did half as much work again as the image needs, and a
lower render resolution saved it nothing. `VisibleFrame::Process` now sizes
the grid to the frame it draws (`BspClipper::SetViewportSize`), capped at the
old 2048×1080: finer than the image only costs, and one row per image row keeps
what shows through a one-pixel gap. A portal's frame is the same size, so the
spans it inherits stay in the same grid.

The coarser grid is the more permissive one: at 720 rows the stats counted
~830 visible surfaces where 1080 rows had ~740, all of them hidden by the
depth test, and captures of the dock before and after match pixel for pixel.

## Patch 0011 — one-sided back faces skipped

Fork commit `a8b5a2f`. On the Smart Pro the fight went from ~213 to ~190 ms at
native resolution (5.3 FPS) and from ~191 to ~168 ms at 853×480 (6.0 FPS).

### 28. Back faces before the clipper

`ProcessNodeSurface` ran the clipper's triangle test on every surface of every
visible node. On Liberty Island's fight that was ~4,800 tests a frame (~22 ms
on the Smart Pro), and ~2,700 of them were one-sided surfaces facing away
from the camera -- ~5% of which came out visible, against ~22% of those facing
it: back faces lie behind the front faces the front-to-back walk has already
drawn. UE1 never drew a one-sided surface from behind, and in a closed level
one hides nothing a front face does not, so they are now skipped before the
test: not drawn, not occluding. Two-sided surfaces are tested as before, and
so is everything in a mirror's frame, whose view comes from the reflected
position; portals, skies and mirrors are handled before this point.

Surface tests fell from ~4,800 to ~2,400 a frame and from ~22 to ~12 ms, and
lightmaps from ~10 to ~4 ms with their uploads from ~11 to ~4 ms: most of the
burning barrel's rebuilt lightmaps had belonged to back faces. Captures of
Liberty Island and of UNATCO HQ's interior before and after differ only in the
stats overlay's surface count.

## Patch 0012 — one evaluator per statement

Fork commit `fdf89ce`. On the Smart Pro, script time went from ~60 to
~55 ms a frame and the fight from 5.2 to 5.4 FPS
([Performance](../ports/trimui-smartpro/README.md#performance)).

### 29. Nested expressions without results of their own

Every expression node -- each constant, variable read and operator argument
-- was evaluated by a new `ExpressionEvaluator` holding a whole
`ExpressionEvalResult` (a value, a label, an iterator, the control-flow
fields), checked against the breakpoint list and copied out twice. A
statement now gets one evaluator and one result, and nested expressions
are evaluated by that evaluator into a value their caller owns
(`ExpressionEvaluator::Value`). Only the statement decides what the frame
does next: a nested expression's control flow (a None context's
AccessedNone) is dropped as before, and Skip, Context and ClassContext
still pass their inner expression's through. Breakpoints are only ever set
on statements, so only statements are checked. `ExpressionValue` gains a
move assignment, since every value is now handed up by assignment.

A jump found its target in a `std::map` of every expression in the
function; statements are read in offset order, so a binary search over
their offsets gives the same index. A temporary check compared the two for
6.2 million offsets across every function loaded on Liberty Island: all
equal.

## Patch 0013 — the actors of a class from an index

Fork commit `e5ae2ca`. On the Smart Pro, script time went from ~56 to
~39 ms a frame and the fight from 5.3 to 5.8 FPS (both measured with the
frame pointers the hooks now build with).

### 30. Actor iterators without a scan of the level

The actor iterators found the actors of a class by testing every actor in
the level: ~2,500 on Liberty Island, a cache miss each on the handheld's
Cortex-A53. Every NPC's `ScriptedPawn.CheckEnemyPresence` steps through the
pawns with `CycleActors` each tick, and a CPU sample of the device's game
tick put `CycleActorsIterator::Next` at 17.6% of it, ~16 ms a frame (on
the desktop it was ~6%). The name tests of `AllActors`, `RadiusActors` and
`VisibleActors` (`UObject::IsA`) came next.

The level now keeps, for each class asked for, the slots of `Actors` that
hold one (`ULevelBase::ActorsByClass` by class pointer, as `CycleActors`
tests it since patch 0003; `ActorsByClassName` by name, as the others
test it), rebuilt when `ActorsVersion` has moved -- load, the GameInfo,
`Spawn`, `Destroy` and the compaction below bump it. `CycleActors`
binary-searches the next slot after its position, going round, and counts
the slots the scan would have passed, so its lap ends where the scan's did;
`AllActors` steps through the slots from its position; `RadiusActors` and
`VisibleActors` build their lists from them. Destroyed actors stay in an
index and are skipped where they were skipped. Temporary checks ran the
scans beside the indexes from the same state: 2.9 million `CycleActors`
steps, 36,000 `AllActors` steps and 1,800 lists of each of the other two,
all the same. On Liberty Island the index was rebuilt 68 times in 8,571
frames.

### 31. Compaction only after a Destroy

`ULevel::Tick` rebuilt the actor list every frame to drop nulled slots,
rewriting every actor's `Index`. It now does so only when a `Destroy` has
nulled a slot since (and once after loading, as before); without holes the
result was the same list.

## Patch 0014 — calls without allocations

Fork commit `4d5b8e7`. On the Smart Pro, script time went from ~39 to
~35 ms a frame and the fight from 5.8 to 5.9 FPS.

### 32. Parameters, arguments and locals

- `Frame::Call`, `CallScript` and `CallNative` walked the function's
  `Properties` -- every local, a cache miss per property object on the
  handheld -- three or four times a call to find its parameters.
  `UFunction::CallParms` keeps the parameters in order and `ReturnParm` the
  return value, gathered at the first call. A temporary sweep of all 7,673
  functions loaded on Liberty Island found every return value also a
  parameter and none with two, so the lists say what the walks did.
- The arguments were an `Array` on the heap, grown again when a native's
  return value was appended. `CallArguments` keeps up to eight on the
  stack, with room made up front for what `Frame::Call` appends; the
  `Array` overload of `Frame::Call`, for native callers, moves into one.
- A script call allocated a `LocalVariables` and then its data. The frame
  now holds its `LocalVariables`, whose data lives in a 256-byte buffer
  inside it when the function's locals fit.
- Every call looked up the object's state name and disabled-event set,
  though almost no object disables anything; the lookups are skipped when
  `DisabledEvents` is empty, and `EnableEvent` removes a state's entry when
  its set empties so that it stays so.

## Patch 0015 — the commonest operators in place

Fork commit `1950f5e`. On the Smart Pro, script time went from ~35 to
~31 ms a frame and the game tick from ~75 to ~71 ms (5.9 to 6.0 FPS).

### 33. Operators without the call path

Natives are 84% of the VM calls on Liberty Island (45 million against 8.3
million script calls in a desktop run), and 25 operators -- object and name
(in)equality, int and float comparisons, `!`, float arithmetic, int
subtraction, `++`, `+=` and `-=` -- are 83% of those. Each went through the
argument list, `Frame::Call`'s checks, `CallNative`'s frame, a
`std::function` and a return slot to compute one comparison.

`ExpressionEvaluator::CallFastOperator` computes them in place, each as its
native in `NObject.cpp` does, with the same conversions, read only after
every argument has been evaluated (in the caller's context), as the general
path reads them. A function is recognised once, by its native index and
its name (`UFunction::FastOperator`), so a game whose indexes differ keeps
the general path. If a conversion throws, nothing has been changed yet: the
same argument values go through the general path, which makes the same
conversions in the native's own frame and reports and recovers as before.
What these calls skip is `Frame::Call`'s event-enabled check on the
operator's name, which a script would have to `Disable` to matter. A
temporary check had the registered native compute every fifth call from
the same values (out parameters restored after): 8.8 million calls across
all 25 operators, results and out parameters identical to the bit.

## Patch 0016 — events through the virtual-call cache

Fork commit `568646c`. Within the device's noise on its own (the tick
~71.2 → ~70.7 ms).

### 34. One cached lookup for virtual calls and events

`CallEvent` looked up its function with `FindEventFunction` on every call:
`std::map` lookups of the state and the function name in every class up
the hierarchy. That is the search a virtual call makes, whose answers
patch 0006 already kept per class. `FindScriptFunction` is that search with
the cache, serving both; misses are kept too, since an event a class does
not have is asked for often (a virtual call that finds nothing still throws
each time). A temporary check compared 3 million event lookups with the old
`FindEventFunction` and 3,379 uncached searches with the old virtual-call
search: all the same.

## Patch 0017 — leaf expressions without the visitor

Fork commit `8843675`. On the Smart Pro the game tick went from ~70.7 to
~69.3 ms.

### 35. Leaves made directly

About half the expressions the evaluator meets are leaves: local, instance
and bool variables, `Self`, `None`, and object, name, int, byte, float and
bool constants. Each went through the visitor's two indirect calls, set
`Frame::StepExpression` and handed its value up by assignment.
`Expression::Leaf` tags those node types and `ExpressionEvaluator::Value`
makes their values directly, exactly as their `Expr` functions do; none
can throw, so none needs to be the debugger's `StepExpression`.

## Patch 0018 — each mesh vertex once a draw

Fork commit `734c6b3`. On the Smart Pro the actor meshes went from ~28 to
~14 ms a frame, render CPU from ~92 to ~80 ms, and the fight from 6.0 to 6.6
FPS.

### 36. A vertex cache per draw

`VisibleMesh::DrawLodMeshFaceDX`, the actor meshes' vertex animation on the
CPU, interpolated, blended, transformed, lit and fogged the vertex of every
corner of every face, though faces share vertices. Within one draw a
vertex's position, normal, light and fog depend only on the vertex (the
light also on the face's unlit and two-sided flags), so `VisibleMesh` keeps
them per vertex for the draw (`CachedMeshVertex`, by generation) and
computes each at its first use, with the same arithmetic; a vertex out of
the mesh's bounds still ends the draw at the face that first uses it. The
texture's info is reused for consecutive faces with the same texture, its
modified flag cleared as a second `UpdateTextureInfo` would have. A
temporary check recomputed every face's corners the old way beside the
new: 83.5 million faces on Liberty Island, identical to the bit.

## Patch 0019 — mesh faces in runs

Fork commit `587ce9c`. On the Smart Pro the actor meshes went from ~14 to
~12 ms a frame and the fight to 6.7 FPS.

### 37. One device call per run of faces

Each face then went to the device on its own, which chose a pipeline,
looked the texture up (a hash map, and a re-upload check), found its
descriptor set and reserved vertex space -- the same for every face of a
material. `RenderDevice::DrawGouraudTriangles` draws a run of triangles
with one texture and one set of flags; its default is one
`DrawGouraudPolygon` each, so the OpenGL and D3D11 devices are as they were,
and the Vulkan device sets up once and writes the same vertices and
indices in the same order (`WriteGouraudVertices` is shared). The texture's
modified flag goes with the first triangle, as it did. `DrawLodMeshFaceDX`
draws its run when the texture or flags change, at the end and before its
out-of-bounds returns, so every face is drawn in the order and at the point
it was. Vulkan validation, with synchronization validation, is clean on
Liberty Island on both texture paths, and a capture of the dock matches the
one from before patch 0012 except where time moves things (the sky, the
NPCs, the stats): the statue and props are identical to the pixel.

## Patch 0020 — the ARM build's clip test

Fork commit `f7188e7`. On the Smart Pro the visibility pass went from ~25
to ~20 ms a frame (with the profile's per-part timers), its surface tests
from ~11.8 to ~7.4 ms, and the fight from 6.7 to 6.9 FPS.

### 38. Only triangles that need clipping are clipped

`BspClipper::ClipEdge` returns a triangle as it is when none of its clip
distances is negative. The SSE path tests that; the plain C++ path, which
every non-x86 build takes (`Precomp.h` defines `NOSSE` there), tested
`clipd[i]` -- a vertex index, and the value's truth rather than its sign --
so on the handheld every visibility test went through the full six-plane
clipping loop, and `ClipEdge` was the visibility pass's largest function
(~5 ms a frame). The test is now the SSE path's, with a NaN distance
counted as needing clipping. Whenever it lets a triangle skip, the loop
would have returned the triangle's own three vertices with weights of
exactly 1 and 0: a standalone test ran both through it on a million random
triangles (inside, crossing the planes, degenerate, some with NaNs), 48%
skipped and none different. The bug is upstream's; captures of the dock
before and after differ only in the sky's moving clouds and the NPCs.

## Patch 0021 — surface points on demand

Fork commit `2d08d1d`. On the Smart Pro the visibility pass went from ~20.5
to ~19.6 ms a frame and render CPU from ~74 to ~73 ms (7.0 FPS).

### 39. Points only for the surfaces tested

`VisibleFrame::ProcessNodeSurface` copied each surface's points out of the
model -- two dependent lookups a vertex -- before deciding anything, and
since patch 0011 over half the surfaces in view, one-sided back faces, are
then skipped untested. The points are now gathered before a sky, warp zone
or mirror's portal check and before the surface test, the only places that
read them.

## Patch 0022 — a far tier of AI level of detail

Fork commit `da0fa3f`. On the Smart Pro the game tick went from ~65 to ~63
ms at native resolution -- where the frame barely moved, the render waiting
on the GPU instead -- and at 853×480 the fight went from 7.9 to 8.4 FPS.

### 40. Every sixth frame when far

With `Performance.AiLevelOfDetail` (patch 0008), a pawn out of the player's
sight and not within 1500 units thinks every third frame. One also beyond
4000 units (~76 m) now thinks every sixth, with all the time it skipped:
the second tier the owner chose for the handheld, at the cost of such a
pawn noticing things up to five frames late. On Liberty Island ~48 pawns a
frame fall in that tier, ~8 of them thinking; ~9 fewer pawns think each
frame than before. The launcher's Distant AI row says so.

## Patch 0023 — the light tree kept while no light changes

Fork commit `f3ee690`. On the Smart Pro the BSP surfaces' section went from
~11 to ~8 ms a frame and the frame from ~142 to ~139 ms (7.2 FPS).

### 41. Surface lights from a kept answer

Every frame `LightSystem::BeginFrame` rebuilt the light tree, and every
visible surface's lightmap asked it again for the lights touching the
surface (~6 ms a frame of `LightActorTree::CollectLights` and
`TestSphereAABB` on the handheld), though the answers change only when a
light does. The tree is built from the lights, their order, locations and
radii alone: `BeginFrame` keeps what it last built it from (`TreeLights`)
and rebuilds it, bumping `LightTreeVersion`, only when any of that differs.
`GetLightmap` takes a surface's lights from `CollectSurfaceLights`, which
keeps each lightmap's answer until the version moves or the surface's
sphere differs; from the same tree the answer is the same, in the same
order. A temporary check built a fresh tree whenever the rebuild was
skipped and asked the tree afresh for every kept answer: 4,500 skipped
builds identical node for node, 4.8 million answers identical. In the
fight the light set changes in ~30% of frames (lights switching on and
off), and each change drops every kept answer.

## Patch 0024 — lightmap conversion in NEON

Fork commit `edb0ecb`. On the Smart Pro the texture uploads went from ~3.9
to ~2.1 ms a frame (7.3 FPS).

### 42. Four channels at once

The GE8300 cannot filter RGBA32F, so lightmaps are converted to RGBA8 on
the CPU as they are uploaded (patch 0002), whole, several a frame while the
burning barrel's light animates. On ARM the same clamp, scale and
truncation now run on four channels at once. A test on the device, built
with the engine's flags, compared it with the scalar loop for every float
from 0 to 1 (1,073,741,824 values) and for negatives, overflows, infinities
and NaNs: all the same; a 256×128 lightmap takes ~0.77 ms instead of ~1.72.

## Running it headlessly

```sh
printf '[general]\ndrivers = null\n' > /tmp/alsoft.conf
ALSOFT_CONF=/tmp/alsoft.conf \
SurrealEngine --no-launcher /path/to/deusex
```

The `ALSOFT_CONF` redirect is only needed where there is no audio device (a
container); the handheld has ALSA. `ALSOFT_DRIVERS`, an older note here, does
not exist in OpenAL Soft; the config file is the mechanism.

Start a map directly with `--url=<map>`, e.g. `--url=01_NYC_UNATCOIsland.dx`.
The parser only takes the `=` form: `-u <map>` sets an empty `-u` and the map
name becomes a stray argument, so the default (intro) map loads with no
warning.

## Profiling and validating on the desktop

The handheld has no `perf` and no Vulkan validation layer, so validation runs
against the desktop build (`scripts/dx.sh build linux-x86_64 engine`), and so
can a quick CPU profile -- but the desktop's proportions are not the
handheld's. Its Cortex-A53 pays far more for a cache miss: `CycleActors` was
~6% of the desktop's game tick and ~18% of the handheld's. What to work on
next is decided by the handheld's own samples (`SURREAL_PERF_SAMPLE`, in
[Base](#base)); timing on the device is `optional/perf-instrumentation.patch`'s
job. `scripts/host-tools.sh` unpacks pinned copies of Linux `perf` and the
Khronos validation layer into `deps/` without installing anything:

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

`--call-graph dwarf` on `perf record` gives callers (the build has no frame
pointers). The engine ignores SIGTERM, hence `timeout -s KILL`; in a container
without a sound server, add the null OpenAL driver from above.
