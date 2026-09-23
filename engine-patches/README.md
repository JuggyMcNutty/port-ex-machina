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
  the aarch64 cross build (fork commit `2328c37`).
- `0002-nonbindless-fallback-and-format-support.patch` — the texture path
  (`e5c9935`).
- `0003-gamepad-and-deusex-fixes.patch` — controller support and two Deus Ex
  fixes (`73c8c51`).

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

Fork commit `73c8c51`, built and running on the device.

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
