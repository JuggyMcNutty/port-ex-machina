# GLES renderer for VibeEngine — port or from scratch?

Status: **done** — M1 to M5, 2026-09-30 to 2026-10-01: the fork's GL device runs on desktop GL
and OpenGL ES 3.2 from one code path, selectable on the Smart Pro as `Type=GLES`. Every commit
named below was pushed and the engine pinned with it (2026-10-01, the owner's go-ahead), so the
milestones' "not yet pushed" notes are as of their day; what the renderer is now is VibeEngine's
[`ENGINE.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#rendering).
Before the work: scope questions answered 2026-09-29 (M1 kept, RGBA8 gated on Hdr, IDA skipped);
validated against the tree 2026-09-30: three errors fixed (RGBA16F is not renderable in
ES 3.0 core; sampler `layout(binding)` is not GLSL ES 3.00; the desktop context request is
3.2 core where the shaders need 4.20), two files added to the list, three nits corrected.

## Context

- [Decided 3](../agent.md#decided): on aarch64 the goal is Vulkan, **OpenGL ES** and
  software rendering all selectable. Vulkan is the only GPU renderer the engine has run on devices.
  The GLES renderer was defined as "porting Surreal's desktop OpenGL 3.2 renderer".
- [Decided 9](../agent.md#decided): at the Smart Pro panel's 1280×720 the Vulkan GPU's
  **~63 ms** holds the frame; the GLES renderer is what should bring it down (at 853×480 the frame is
  CPU-bound, so GLES changes nothing there).
- The device (from the Smart Pro port README): **PowerVR Rogue GE8300**, **OpenGL ES 3.2** through EGL
  on the vendor SDL2 `mali` driver, **no desktop OpenGL**, no BC1–5/RGBA32F texture filtering
  (engine patch 0002), no `VK_EXT_descriptor_indexing`. Vulkan 1.3.225 runs the game at 8.2 FPS
  (853×480) / ~2 FPS (native) today.

## Findings

### The upstream GL renderer (`SurrealEngine/RenderDevice/OpenGL/`)

A **complete, modern** `RenderDevice` implementation, ~2,900 lines of device code plus a GLEW-style
function loader (`SurrealEngine/RenderDevice/OpenGL/gl_load/`, generated for desktop GL 4.5-compat) and embedded GLSL 4.20 shaders
(`GLFileResource.cpp`):

- Full engine contract: `DrawComplexSurface`, `DrawGouraudPolygon`/`DrawGouraudTriangles`, `DrawTile`,
  3D/2D lines, points, `Lock`/`Unlock`, `ReadPixels`, `PrecacheTexture`, `UpdateTextureRect` — the same
  interface the Vulkan device implements and the fork's whole M0–M7 engine drives.
- Scene pass: 33 polyflag pipeline states (blend/depth/alpha-test), 16k-vertex/32k-index **dynamic
  mapped buffers** with state-change batching, samplers per filter/mipmap mode.
- Present pass: 16 compiled variants of the **same present shader as the Vulkan device** — D3D9 and
  **XOpenGL** (the original `OpenGLDrv`'s gamma curve, already RE'd) gamma modes, 3 color-correct
  formulas, dither, HDR; 4-level bloom; MSAA resolve.
- Texture path: cache keyed on `CacheID`, masked/unmasked, P8 (palette), RGB8, BGRA8 (+LM), R5G6B5,
  BC1 (S3TC), RGBA32F uploaders; lightmap rect re-uploads.
- Scene buffers: **RGBA16F** color, **R32UI** hit, **D32F** depth (+MSAA variants) — **identical to the
  Vulkan renderer's `SceneTextures`**, i.e. exactly the formats already proven on the GE8300.
- Fork state: arrived with the upstream import (`42b0d64`); never run by the fork; the only fork touch
  is `fdb0e8e` (gamma 2.5 × Brightness). Hit-test readback is `#if 0` (D3D11 leftovers) — editor-only
  feature, the game always passes `nullptr` for `HitData` (`RenderSubsystem`), so it doesn't block play.
- Upstream history (GitHub API): written in a 3-day burst 2026-08-14→16
  ("Create scene buffer textures" … "Bug fixes that gets some visuals out of the renderdev",
  "Fix compressed texture upload"), last commit 2026-09-04 (clang warnings), **dormant ~7 weeks**;
  upstream removed the GL option for macOS (no context there, PR #352) and a Haiku PR defaults to it
  as a Vulkan-less fallback. The author's active work is Vulkan (SurrealGPU).

### GLES 3.0 compatibility — checked item by item

- **API**: every entry point the device uses is ES 3.0 core — VAOs, `glMapBufferRange`,
  `glTexImage2D`/`glTexImage2DMultisample`, `glTexStorage2DMultisample`, `glBlitFramebuffer`, uniform blocks +
  `glBindBufferBase`, per-draw-buffer blend/color-mask/enable, `glVertexAttribIPointer`,
  `glDrawBuffer`/`glReadBuffer`, `gl_VertexID` in the fullscreen pass, uint fragment outputs.
  Only `glObjectLabel` (KHR_debug, guard it) and `GL_COMPRESSED_RGBA_S3TC_DXT1_EXT` (device extension)
  are non-core; the GE8300 lacks S3TC, so BC1 must CPU-decode to RGBA8 — the Vulkan side already has a
  BC1 decoder (patch 0002) to reuse.
- **Formats**: ES 3.0 *requires* R32UI color-renderable and D32F depth, but **not RGBA16F** — half-float
  color targets became core color-renderable in **ES 3.2** (ES 3.0 needs `EXT_color_buffer_half_float`;
  RGBA16F is only filterable there). The GE8300 is GLES 3.2, so the scene buffer design ports unchanged
  — but the honest baseline is **GLES 3.2**, not 3.0. (Optional M4 perf lever: RGBA8 scene buffer when
  `Hdr` is off — the original XOpenGLDrv also used no half-float buffer.)
- **Shaders**: the `#version 420` is prepended in one place, `GLRenderDevice::CompileGlsl`
  (`GLRenderDevice.cpp:2224`) — not spread through the shader sources; `flat in uint` varyings
  (`layout(location=N)` on in/out is core ES 3.0) are *mandatory* in ES 3.0 and supported;
  `centroid`, uniform blocks, `textureSize`, `textureOffset`, integer fragment outputs all
  ES 3.0. **Two semantic breaks**, not one: (1) the detail-texture distance fade uses
  `gl_FragCoord.w`, which is always 1.0 in ES (the Vulkan side uses the same expression, where
  glslang gives it true clip-w) → move the fade to a vertex-computed varying; (2) the shaders
  bind their samplers with `layout(binding = N)` (Scene.frag, Present.frag, the bloom pass) —
  a desktop-GLSL 4.20 feature the device relies on entirely (it never calls `glUniform1i`),
  and it is not in GLSL ES 3.00 (nor 3.10; it arrives in GLSL ES 3.20). Fix: after program link,
  set each sampler uniform with `glUniform1i` to its binding index (the device already binds
  textures to unit == binding via `glActiveTexture`+`glBindTexture`), or compile the ES
  variants as `#version 320 es`. UBOs are safe: every program uses exactly one block at
  binding 0 — ES 3.0 resets block bindings to 0 at link, desktop defaults to the block index,
  both are 0 here.
- **Loader**: `gl_load.c` is glLoadGen output (`-style=pointer_c -spec=gl -version=4.5`), resolved
  through the window layer's `GetGLProcAddress` (EGL or `SDL_GL_GetProcAddress`). A GLES 3.0
  generation of the same style (or a hand-written ~70-entry table) slots in unchanged — the device
  code calls the same function names.
- **Context**: verified in vendor SDL **2.30** (`SDL_egl.c`): `SDL_GL_CONTEXT_PROFILE_ES` +
  major 3 / minor 0 → `EGL_OPENGL_ES3_BIT_KHR` + `eglBindAPI(EGL_OPENGL_ES_API)`. The device's display
  path is SDL2 (patch 0001: "SDL2 only"), and `SDL2DisplayWindow` already receives the `RenderAPI` in
  its constructor and branches on it. X11/Wayland backends have their own EGL code (currently desktop
  3.2 core) if a desktop-GLES path is ever wanted. **Desktop note (found 2026-09-30, tested):**
  both
  `SDL2DisplayWindow::CreateGLContext` and the X11 EGL path request a **3.2 core** context, but the
  shaders compile as GLSL 4.20 (the `#version` is prepended in `GLRenderDevice::CompileGlsl`, one
  place, not in the shader sources). Measured on this machine (Mesa 26.2.3, `plans/glctx-test.c`
  on Xvfb): a 3.2 core request still returns a 4.6-capable context and `#version 420` compiles —
  M1 runs as-is on Mesa. A strict driver could reject GLSL 4.20 on a 3.2 context, so raising the
  request to 4.2+ core is a cheap hardening worth folding into M1, not a requirement for it. The
  device is desktop-GL-4.2-class in practice (`layout(binding)` samplers, `#version 420`;
  `glBufferStorage` is loaded by the loader but unused by the device).
- **Launcher** (deusex-launcher) is **already wired for GLES**: `renderers.ini` supports
  `Requires=gles`, the GPU probe dlopens EGL in a forked child and reports the ES renderer/version
  (Smart Pro probe: "OpenGL ES 3.2, no desktop GL"). The `[GLES]` section already exists too — what's
  missing is only filling its `EngineType=` value once the engine accepts it.

### What the fork's GL device lacks (Vulkan-only today)

- `SupportsRenderScale()` (commit `a1a2926`) — needed for the device's 853×480 setting; the Vulkan
  implementation is small (scene buffers at render size, present scales up; the common plumbing
  `RenderDevice::GetRenderWidth/Height` is already in the base class).
- The GE8300 workarounds from patch 0002 (format support checks, BC1 CPU decode).

## Recommendation: **port the upstream GL renderer — with a decision gate**

Not a from-scratch renderer:

1. It is a parallel implementation of *the same design* as the Vulkan device (same buffers, same
   present math, same shader sources) — porting it is adapting a proven-shaped implementation, not
   someone's foreign architecture.
2. The port delta is small and now fully enumerated (loader, `#version`, one varying, BC1 fallback,
   context creation, RenderScale). Every ES 3.0 requirement has been checked against the actual code.
3. Writing from scratch means re-implementing the whole `RenderDevice` contract (~2,900 lines) plus
   re-establishing visual parity with the D3DDrv RE — more new code, more review, more divergence
   risk, and the same unknowns (GE8300 driver behavior) at the end.
4. As a side benefit it makes the launcher's existing desktop **OpenGL** row real (upstream considers
   it a usable fallback on Vulkan-less platforms; the fork has never verified it).

**Decision gate at M1**: the device has never been run. First make it run on desktop (linux-x86_64
base, desktop GL 3.3 via EGL) and compare frames against the Vulkan renderer. If it turns out
fundamentally broken (bigger than "fix bugs in an untested device"), pivot to a from-scratch GLES
device modeled directly on the Vulkan renderer — M1–M2 work (ES context, loader, shader port) carries
over into that path unchanged.

**IDA / original `OpenGLDrv.dll`: not needed.** The original driver is a 2000-vintage fixed-function
desktop GL driver — nothing of its implementation transfers to a modern ES renderer. Its one visual
contract (the gamma ramp) is already RE'd (d3ddrv-dll.md / NATIVES.md "screen flash": gamma 1.5 at
Brightness 0.6) and already implemented in the present shader as `GAMMA_MODE_XOPENGL`. The fork's
visual contract is `D3DDrv` (third RE pass, done), which the GL device's shaders already match.
Loading `OpenGLDrv.dll` in IDA stays an option only if a specific visual question comes up in M1/M4.

## Approach

One dual-context `GLRenderDevice` (works on desktop GL 4.2+ **and** GLES 3.2+, detected at init from
`glGetString(GL_VERSION)`), plus a new `RenderAPI::GLES` / `RenderDeviceType::GLES` that selects an ES
context. Desktop and embedded builds link the loader their context needs.

1. **Window layer** (`SurrealWidgets`): add `RenderAPI::GLES`; `SDL2DisplayWindow::CreateGLContext`
   creates an ES 3.0 context for it (`PROFILE_ES`, 3/0 — vendor 2.30 verified); X11/Wayland get the
   same via their EGL code if a desktop GLES path is wanted (optional).
2. **Loader**: `gl_load_gles` — GLES 3.0 entry points in the existing `pointer_c` style (glLoadGen
   spec `gles`, or hand-written from the ~70 used), resolved through the existing `GetGLProcAddress`.
3. **Shaders**: ES variants of `GLFileResource` — `#version 320 es` (GLSL ES 3.00 has no sampler
   `layout(binding)`; 3.20 does, and the device is GLES 3.2) or `#version 300 es` plus post-link
   `glUniform1i` for every sampler + the detail-fade distance varying (the varying fix is also correct
   for desktop; optionally applied to the Vulkan shader sources to keep the pair in sync).
4. **Device**: BC1 → RGBA8 CPU decode when S3TC is absent (reuse the Vulkan BC1 decoder from 0002);
   `glObjectLabel` is *already* guarded (`if (UseDebugLayer && glObjectLabel)`, no change needed);
   add `SupportsRenderScale()` (port of `a1a2926`); keep the fork's gamma
   2.5 × Brightness; **RGBA8 scene/PP/bloom buffers when `Hdr` is off, RGBA16F when on** (owner
   2026-09-29) — the first M4 perf lever, matching the original XOpenGLDrv's 8-bit buffers.
5. **Engine**: `LauncherSettings` parses `Type=GLES`; `RenderDevice::Create` maps it to the GL device
   with an ES-forced context.
6. **Launcher**: `[GLES]` section in the Smart Pro's `renderers.ini` (`EngineType=GLES`,
   `Requires=gles`); the probe and Video tab already handle it.
7. **Docs**: `vibe/docs/ENGINE.md` entry (how it works, device numbers), `NATIVES.md` where behavior
   diverges, port-ex-machina `agent.md` state update, pins moved.

## Files to modify

VibeEngine (branch `deusex`):
- `SurrealWidgets/include/surrealwidgets/window/window.h` — `RenderAPI::GLES`
- `SurrealWidgets/src/window/sdl2/sdl2_display_window.{h,cpp}` — ES context (the device path)
- `SurrealWidgets/src/window/x11/x11_display_window.{h,cpp}` (+wayland) — optional desktop ES path
- `SurrealEngine/RenderDevice/RenderDevice.{h,cpp}` — `GLES` in `Create`
- `SurrealEngine/LauncherSettings.{h,cpp}` — `RenderDeviceType::GLES`
- `SurrealEngine/GameWindow.cpp` (~line 163) and `SurrealEngine/EditorApp.cpp` (~line 36) — the
  `RenderDeviceType` → `RenderAPI` switches there need a `GLES` case (missed 2026-09-29)
- `SurrealEngine/RenderDevice/OpenGL/GLRenderDevice.{h,cpp}`, `GLFileResource.cpp`,
  `GLTextureUploader.{h,cpp}`, `GLUploadManager.cpp` — ES mode, render scale, BC1 fallback
- new: `SurrealEngine/RenderDevice/OpenGL/gl_load_gles.{c,h}`
- `CMakeLists.txt` — new files; loader selection per build
- `vibe/docs/ENGINE.md`, `vibe/docs/NATIVES.md`

deusex-launcher (`trimui-smartpro`, `ports/common`):
- `ports/trimui-smartpro/packaging/renderers.ini` — `[GLES]` section
- `ports/trimui-smartpro/README.md` — renderer status, measurements

port-ex-machina: `agent.md` (state/next), pins at the end.

## Steps

- [x] **M1 — Desktop GL bring-up (decision gate).** Run the untested upstream GL device on the
      linux-x86_64 base (desktop GL via the existing X11 EGL path, `Type=OpenGL`); fix startup bugs
      until the intro + Liberty Island level start render; compare frames against the Vulkan renderer
      and the D3DDrv reference captures (dxcap / scripted runs). Tested 2026-09-30: on this
      machine's Mesa a 3.2 core context request already accepts the GLSL-4.20 shaders (Mesa returns
      a 4.6-capable context), so no context fix is needed to start — raising the request to 4.2+
      core is optional hardening for strict drivers.
      *If the device proves unfixable in reasonable time: pivot to a from-scratch GLES device modeled
      on the Vulkan renderer.*

      **Done 2026-09-30 — the device is NOT broken; no pivot.** Two bugs found and fixed, two commits
      (`e77ac55`, `494e22e` on `deusex`, not yet pushed):
      1. **`ReadPixels` was a stub** — the whole readback was the old D3D11 code inside `#if 0`, so
         every screenshot (a proving run's shots, the game's own `SHOT`) came out **black** while
         the pipeline itself rendered fine. Implemented against the Vulkan device's contract (BGRA,
         top row first, from `PPImage[0]`/`[1]`); GL's bottom-up rows flipped. The 2026-09-29
         aborted attempt chased this black screen for a whole session with censuses and test quads
         and never found it.
      2. **Shutdown crashed** — the engine closes the window (and its GL context) before the device
         is destroyed, and the intro level ends the game mid-frame: `Exit()` unmapped gone-context
         buffers (abort out of the destructor) and a still-flying draw tripped `DrawBatches`'
         error check. `Exit()` now tolerates a failed unmap, runs once, and `DrawBatches` no-ops
         after exit. The intro now renders its shot and shuts down cleanly, exit 0.

      Verification: `01_NYC_UNATCOIsland` proving runs clean on llvmpipe (hidden Xvfb) and radeonsi
      (visible) — two shots, mean 0.040 — **matching a fresh Vulkan run on the same tree, same
      settings to 0.04 %** (the pixel diff, ~5.5 %, sits in the animated harbor water whose phase
      differs by frame pacing; static regions 2–5 %; the old 2026-09-28 VK reference's ~6 %
      brightness gap was settings, not the device). The D3DDrv-capture comparison moves to M5's
      parity pass as planned there. Unit tests 12/12; the staged app carries the fix.
- [x] **M2 — GLES context + loader + shaders.** `RenderAPI::GLES`, SDL2 ES context, `gl_load_gles`,
      ES shader variants (320 es, or 300 es + `glUniform1i` sampler setup; distance varying),
      `Type=GLES` in `LauncherSettings`/`Create` (and the `GameWindow`/`EditorApp` switches).
      Build for the base port; sanity-check on desktop (EGL-ES) or straight to M3.

      **Done 2026-09-30 — commit `db0961e` on `deusex` (not yet pushed), simpler than planned:**
      - **No `gl_load_gles` needed**: the one desktop loader table resolves the same entry-point
        names on an ES context through the same `GetGLProcAddress` — the plan's second loader
        drops out entirely.
      - **No shader variants needed beyond the version line**: the sources compile unchanged as
        GLSL ES 3.20 once `CompileGlsl` prepends `#version 320 es` (+ highp precision for fragment
        stages). Sampler `layout(binding)` is 3.20's; the sources only needed `u`-suffixed flag
        masks and float literals (ES converts no int literals); **`gl_FragCoord.w` is 1/w in ES as
        well** — the plan's distance-varying fix is unnecessary (corrected above). The
        push-constant blocks now declare `std140`, which they already assumed.
      - **The window layer**: the base port's runs land on the Wayland backend by default —
        `USE_SDL2` is not defined in the desktop build (SDL3 is), so `SURREALWIDGETS_DISPLAY_BACKEND=SDL2`
        silently fell back to Wayland (the host compositor's socket is reachable). All four
        backends (SDL2, SDL3, X11, Wayland) therefore got the ES branch; the Smart Pro's device path
        (SDL2) and the desktop paths all work.
      - **Desktop-only GL calls** replaced or guarded: `glClearDepthf`/`glDepthRangef` (both desktop
        4.2+ core — the shaders already require 4.20), `glDrawBuffers` in place of every
        `glDrawBuffer` (ES has only the plural), the null texture uploads as `UNSIGNED_BYTE`
        (`UNSIGNED_INT_8_8_8_8` is a desktop type), `GL_DEPTH_CLAMP`/`GL_MULTISAMPLE` and the
        indexed-blend calls skipped on ES (ES clips and rasterizes what its target carries; the
        integer hit attachment ignores blending by the spec).
      - **Verified**: the level-start proving run with `Type=GLES` on an ES 3.2 context (llvmpipe,
        Xvfb, forced X11 backend) is clean — exit 0, zero GL errors, two shots — and matches the
        desktop GL device to **0.04 % overall brightness, 0.036 % pixel difference**; Vulkan
        unchanged; the intro exits cleanly; desktop GL and the unit tests (12/12) unchanged.
        Note for harness runs: Vulkan needs a real display for present (Xvfb surfaces carry no GPU
        present support) — the default harness env (Wayland fallback) works; `SURREALWIDGETS_DISPLAY_BACKEND=X11`
        + Xvfb is for GL/GLES only.
      - **Environment**: pi now runs inside the Arch container (`opencoder`) per the owner; all
        builds and runs happen there directly. (This session started outside it — the host — which
        the builds worked around through `distrobox enter`; corrected.)

      Still open for M3/M4 (as planned): BC1→RGBA8 CPU decode when S3TC is absent (the GE8300 has
      none — Vulkan's decoder to reuse), `SupportsRenderScale()` (the 853×480 lever), the RGBA8
      scene buffer when `Hdr` is off.
- [x] **M3 — Smart Pro bring-up.** Cross-build (patch 0001's SDL2-only embedded build), deploy, run
      the game with `Type=GLES` at 853×480 and native; basic visual pass (lighting, lightmaps,
      particles, UI canvas, dither/gamma).

      **Done 2026-10-01 — commits `aa94a7a` (render scale + CPU decoders) and `b9bc870` (the
      GE8300's sampler set and fullscreen) on `deusex`, launcher `c3d6008`, none pushed yet.**
      The cross-build is warning-free under the port's GCC 9.3 and its ABI check passes; the game
      runs on the device under `Type=GLES` fullscreen at the owner's 853×480 render scale and at
      native 1280×720, and the level start matches the Vulkan device's own capture to **1.4 %
      pixel difference** at the same moment (the difference is the clouds' and water's animation
      phase), with Vulkan on the device unchanged through the same window changes.

      Three findings from the bring-up, the first two via a standalone on-device probe
      (`plans/gles-sampler-probe.c`, dlopen-based like the launcher's GPU probe — also
      confirming the scene buffers' whole format set renders there: RGBA16F + R32UI + D32F in
      one FBO, two draw buffers, complete):
      1. **The vendor driver rejects three sampler parameters** a desktop GL takes in stride:
         `GL_TEXTURE_LOD_BIAS` (no ES has it on a sampler — skipped on ES), `GL_TEXTURE_MAX_ANISOTROPY_EXT`
         (now gated on the extension, closing the upstream to-do — the GE8300 has none), and
         `GL_MIRROR_CLAMP_TO_EDGE` (3.2 ES core on paper, rejected there anyway — probed once at
         init, samplers clamp to edge where absent).
      2. **`SDL_SetWindowFullscreen(FULLSCREEN_DESKTOP)` deadlocks on a GL window** on the device's
         vendor SDL2 (mali backend) — the EGL surface recreation never completes; a Vulkan window
         switches fine. The SDL2 backend's fullscreen is now borderless cover at the display's
         size, done by hand entirely on the still-hidden window (hidden resizes are safe there —
         proven by the windowed run) — which is what FULLSCREEN_DESKTOP means anyway; the desktop
         build never compiles this backend (wayland/x11/sdl3 serve it).
      3. The device's driver exposes **no S3TC, no float-linear, no anisotropy** — exactly the
         extensions M2's CPU decoders substitute (BC1→RGBA8, RGBA32F→RGBA8 lightmaps), so the
         decoders carried their weight on the first device run.

      A windowed run isolating the fullscreen deadlock first confirmed the renderer itself was
      already drawing on the device (the engine hung only at the fullscreen switch). The owner's
      device settings are restored (Vulkan, 853×480); the launcher's `[GLES]` row is selectable
      (`c3d6008`). Perf numbers and the RGBA8 scene buffer are M4's, as planned.
- [x] **M4 — Perf at native 1280×720.** Profile with the existing hooks (`dx.sh profile
      trimui-smartpro`): the Vulkan GPU's ~63 ms is the bar. Measure; the first lever is the RGBA8
      scene buffer (Hdr is off on the device; matches the original XOpenGLDrv), then upload
      batching/row-only lightmap re-upload, then batch sizes. Record the numbers in the port README.

      **Done 2026-10-01 — commits `765f180` (the RGBA8 buffers) and `0c8e99b` (CPU staging
      streams) on `deusex`, launcher `732cda4` (the README numbers), none pushed yet.** Same-
      session numbers at the level start, Overclock, the engine with the profiling hooks:

      | renderer @ setting | fps | frame | tick | render |
      |---|---|---|---|---|
      | Vulkan @ native 1280×720 | 11.4 | 87.4 | 30.0 | 53.5 (gpu-wait 0.8) |
      | GLES @ native 1280×720 | 8.5–8.8 | 113–117 | 29–31 | 82–96 |
      | GLES @ 853×480 | 9.9–10.0 | 100.4 | 23.3 | 75.6 |

      Two levers, both found by the CPU samples (`SAMPLE=1`, `sample-report.py`):
      1. **The RGBA8 scene buffers when Hdr is off** (as decided; the original XOpenGLDrv's own
         8-bit buffer). The readback also now reads what the buffer is — an `UNSIGNED_BYTE` read
         of the 8-bit buffer (a `GL_FLOAT` read of it is an error on ES; the unchecked failure
         was black shots and a pending error that felled the next Lock) and fails loudly.
      2. **CPU staging streams**: the first device profile put **2/3 of the frame (270 of 381 ms
         of main-thread CPU) in the stream buffers' per-flush map/unmap** — `DrawBatches` runs
         hundreds of times a frame (`SetSceneNode` flushes per BSP node, ~600 a frame) and each
         cycle mapped/unmapped the buffers' whole unused tails (GL cannot unmap a part),
         submitting the vendor driver's command buffer every time. The draw code now writes CPU
         staging arrays and every flush `glBufferSubData`s the range written since the last one;
         `glBufferStorage` (persistent mapping) is absent on the GE8300 (probed). 2.4 → 8.8 fps.

      What remains between GLES and the Vulkan device at the same settings (~13–30 render-CPU
      ms) is the GL driver's per-draw-call cost across the frame's ~700 calls — state, program
      and texture binds included; the engine sections around them profile the same shape as the
      Vulkan device's. The numbers are recorded in the port README's Where-a-frame-goes and its
      measurement table (launcher `732cda4`). The profiling hooks are off again and the device
      runs the clean build under `Type=GLES` at the owner's 853×480.
- [x] **M5 — Parity + ship.** Frame comparison vs D3DDrv at the level start (the existing harness),
      launcher `[GLES]` row + probe status line, docs (`ENGINE.md`, port README), engine pin,
      `agent.md` state.

      **Done 2026-10-01, with the push.** The level-start captures sit at D3DDrv's by the look
      work's documented deltas (5.9 %) and match the Vulkan device's verified look to 0.15 %
      (VibeEngine's `ENGINE.md`, the GLES renderer's entry); the Smart Pro's Video tab lists the
      `[GLES]` row as OpenGL ES (`c3d6008`), its numbers in the port README (`732cda4`); the
      engine pinned at what was pushed and `agent.md`'s State updated.

## Verification

- `scripts/dx.sh test` (base port) after every engine change.
- M1/M3/M5: scripted runs + dxcap frames (Vulkan vs GLES vs the original's `D3DDrv`/`OpenGLDrv`
  captures) at Liberty Island level start — the same comparison used for the M0–M6 look work.
- M3/M4: on-device profile (`SAMPLE=1`, `dx.sh profile trimui-smartpro [seconds] [label]`): frame,
  tick, render CPU, GPU wait at 853×480 and native; the ~20 FPS target at native needs GPU < ~50 ms.
- Manual (owner, per open decision 1's style): Video tab shows Vulkan/GLES/Software with the probe's
  one-liners; selecting GLES starts the game; resolution/AA/brightness settings behave.

## Decided with the owner (2026-09-29)

1. **M1 stays**: the desktop GL bring-up is the first milestone (and makes the launcher's desktop
   OpenGL row real).
2. **RGBA8 scene buffer gated on `Hdr`**: RGBA8 scene/PP/bloom when `Hdr` is off, RGBA16F when on;
   the first M4 perf lever.
3. **No IDA pass on `OpenGLDrv.dll`**: kept only as a documented fallback for if a specific visual
   question comes up in M1/M4.

## Risks

- ~~Untested GL device: unknown bug count — bounded by the M1 gate.~~ (M1 done 2026-09-30: the device
  ran; its only missing piece was `ReadPixels`.)
- GE8300 driver (panfrost): GLES may not beat Vulkan's GPU time out of the box — M4 measures before
  any optimization; the 853×480 CPU-bound target is unaffected either way.
- Upstream drift: the GL device is dormant upstream; fork-only changes are marked per the NO-AI Code
  Rule, and upstream merges (the fork's `vibe/tools/upgrade.sh`) may touch the GL directory.
- The vendor SDL2 is a black box; the ES-context path is verified against stock 2.30's EGL logic, and
  the probe already exercises the device's EGL.
