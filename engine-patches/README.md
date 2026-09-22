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

Applied on top of the upstream commit in `UPSTREAM-BASE.txt`. Regenerate after
any change:

```sh
cd engine/SurrealEngine
git diff > ../../port/engine-patches/0001-headless-and-embedded-support.patch
```

## What the patch changes

One patch file, nine changes, in two groups.

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

4. **Gate the desktop display backends.** `SurrealWidgets` hard-required
   `find_package(OpenGL REQUIRED COMPONENTS EGL)` and always compiled and linked
   the X11 backend. A handheld has no X11, no Wayland compositor, no dbus and no
   desktop GL. New `ENABLE_X11`/`ENABLE_WAYLAND` options (both ON by default)
   let an SDL2-only build configure; `window.cpp` already had null-returning
   stubs for every backend whose `USE_*` define is absent, so nothing else
   needed touching.

5. **Missing includes in the SDL2 backend.** `sdl2_display_backend.cpp` and
   `sdl2_display_window.cpp` used `strcmp`, `memcpy` and `std::round` without
   `<cstring>`/`<cmath>`. That backend is rarely built on Linux, where X11 and
   Wayland are preferred, so it had not been noticed.

6. **SDL discovered by pkg-config links nothing.** The link step used
   `${SDL2_LIBRARY}`, which `find_package` sets but `pkg_search_module` does not
   — it sets `SDL2_LIBRARIES`. A cross build takes the pkg-config path, so every
   SDL symbol came out undefined. Now tries the imported target, then
   `SDL2_LIBRARIES`, then `SDL2_LIBRARY`.

7. **`zipdir` must run on the build machine.** It packs the resource zip during
   the build, so a cross build produces an aarch64 binary that cannot execute.
   `ZIPDIR_EXECUTABLE` now points at a host-built one, and the in-tree target is
   skipped when it is set.

8. **System font lookup without a desktop.** `resourcedata_unix.cpp` asked
   GSettings for the GNOME UI font and resolved it with fontconfig. Neither
   exists on a handheld. Guarded by `SURREALWIDGETS_DESKTOP_FONTS` (defined only
   when both glib and fontconfig are found); otherwise it reads the fonts the
   device actually ships, overridable via `SURREALWIDGETS_FONT`.

9. **Exit status reflects failure.** `GameApp::main` returned 0 even after
   catching an exception. Our launcher keys its crash sentinel off the exit
   status, so a failed start was being recorded as a clean run.

## Known blocker on the TrimUI Smart Pro

The cross build runs and gets as far as creating a Vulkan surface, then stops at:

    Could not create vulkan renderer: No Vulkan device found supports
    the minimum requirements of this application

`VulkanRenderDevice.cpp:34` requires `VK_EXT_descriptor_indexing`. The PowerVR
Rogue GE8300 supports descriptor indexing **neither** as that extension **nor**
as Vulkan 1.2 core, despite advertising API 1.3.225:

    VK_EXT_descriptor_indexing                   ABSENT  (of 87 extensions)
    descriptorIndexing (1.2 core)                NO
    runtimeDescriptorArray                       NO
    shaderSampledImageArrayNonUniformIndexing    NO
    bufferDeviceAddress                          yes

Everything else the device filter demands is present: `VK_KHR_swapchain`,
`samplerAnisotropy`, `fragmentStoresAndAtomics`, `multiDrawIndirect`,
`independentBlend`, and a graphics queue that can present to the surface.

The renderer uses descriptor indexing for bindless textures
(`GetTextureIndexes` returns indices into a large descriptor array), so this is
a design dependency rather than a flag to flip. `port/tools/probe-vulkan-caps.c` reproduces every line of that in one run, and
`probe-sdl-vulkan.c` shows the surface being created.

## Running it headlessly

```sh
ALSOFT_DRIVERS=null \
SurrealEngine --no-launcher /path/to/deusex
```

`ALSOFT_DRIVERS=null` is only needed where there is no audio device (a container);
the handheld has ALSA.
