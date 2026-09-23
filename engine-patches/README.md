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
afterwards.

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

The handheld has no profiler and no Vulkan validation layer, so both run
against the desktop build (`scripts/dx.sh build linux-x86_64 engine`), whose
CPU hot spots on the script and render paths are the handheld's too; timing
on the device is `optional/perf-instrumentation.patch`'s job.
`scripts/host-tools.sh` unpacks pinned copies of Linux `perf` and the Khronos
validation layer into `deps/` without installing anything:

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
