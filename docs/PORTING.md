# Porting

How a device becomes a port. **A port is a branch of the launcher**,
[deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher), and the
engine, [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine), is built for
it. **`main` is the working base**: each port branch is its own variant of it.
linux-x86_64 is where the project is developed and tested
([`DEVELOPMENT.md`](DEVELOPMENT.md)). The launcher the Linux ports run is
[`LAUNCHER.md`](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md);
the ports that exist are listed in the [root README](../README.md#ports).

## The branches

| Branch | Holds |
|---|---|
| `main` | the original launcher, recreated almost 1:1: no additions, no ports |
| `linux-x86_64` | `main`, plus the launcher as it grew for the ports (the tabbed home screen, `Settings.json`, pad layouts, the GPU probe), the generic device profile, and the base app every port ships (`ports/common/packaging`) with the desktop's port files (`ports/linux-x86_64`). `main`'s own program, its tools and its program's tests come along unbuilt: a port branch builds `deusex-launcher`, `dxl-cli`, `dxl-shots` and the core's tests |
| `<id>`, a device | its own variant: the Linux devices (`trimui-smartpro`, `linux-aarch64`) began from `linux-x86_64` and add **only what differs**, `ports/<id>/` and its preset; a port for another platform may start from `main` alone (an Xbox 360 port would replace the start-up with its platform's) |

Every port branch takes `main`'s changes by merging `main` into it; port
branches do not merge into each other. A fix to the original's behaviour is
made on `main`; a fix to the ports' launcher, which each Linux port branch
carries, is made on each of those branches; a fix for one device, on its
branch. A branch adds files rather than editing `main`'s where it can, and
resolves a merge's conflicts itself. `README.md` never flows: each port branch
keeps its own, so a merge from `main` conflicts on it and always keeps the
port branch's, never `main`'s. `scripts/launcher.sh status` says which pins
lack commits of `main`.

## The layers

| Layer | Where | Changes per port? |
|---|---|---|
| The launcher's core (the entry decision, command-line parsing, crash sentinel, config seeding) | `src/core/`, from `main`; `linux-x86_64` adds `Settings.json` and pad layouts | No. C11, no SDL, no device facts |
| The device profile | `src/platform/target.h`; `ports/<id>/target.c` | Data only: a port may supply one |
| The OS: handing over to the game, the GPU probe | `src/platform/launch.h`, `src/platform/posix/` | Only for a non-POSIX platform, and for Android's hand-over (below) |
| The screens | `src/ui/` (SDL2), from `linux-x86_64` | No |
| The engine | VibeEngine, its clone beside the workspace in `VibeEngine/`, pinned by `ENGINE-PIN.txt` | Built per port from its `ports/<id>/engine.cmake` |
| The app around the binaries | `ports/common/packaging/` (the base app, what linux-x86_64 ships) + `ports/<id>/packaging/` | The port's files are laid over the base |

## What a port is

A port's directory, `ports/<id>/` on its branch, holds **only what differs**
from the branch it started from (linux-x86_64, for the Linux devices):

| File | Purpose | Required |
|---|---|---|
| `port.cmake` | Included by the branch's `CMakeLists.txt` when `DXL_PORT=<id>`. Sets `DXL_PORT_SDL` (`system`: pkg-config; `sysroot`: the device's own SDL2 from `DXL_PORT_SYSROOT`), `DXL_PORT_GLIBC_MAX` (turns on the post-build `scripts/check-abi.sh`) and `DXL_PORT_LINK_OPTIONS` | yes |
| `port.sh` | Sourced by this workspace's `scripts/dx.sh`, with `scripts/lib/common.sh` loaded. `PORT_DESC`; `PORT_ENGINE=1` if the engine builds for this port here; hooks `port_deps` (fetch toolchains/sysroot), `port_stage` (adjust the staged app), `port_deploy` (send it to the device; over SSH, `dx_ssh_sync` in `scripts/lib/common.sh` sends only what changed, keeps the device's previous copies and verifies), `port_run` (run it here), `port_profile` (a frame-time profile on the device). Unset hooks have safe defaults | yes |
| `toolchain-c.cmake`, `toolchain-cxx.cmake` | Cross toolchains, referenced by the port's preset (C) and `engine.cmake` (C++). A toolchain file that `return()`s early when the host already is the target arch makes the same preset build natively | cross ports |
| `engine.cmake` | A CMake initial cache (`cmake -C`) for building the engine: its toolchain and `ENABLE_*` switches, each set with `FORCE` so an edit reaches an existing build too | if it builds the engine |
| `target.c` | The device profile, a `dxl_target`: fonts to try first, the panel size, the CPU modes the port's hooks can apply (with their help text), a note about the built-in pad, the About line, and the GPU `dxl-shots` should pretend to have. Without one the build uses `src/platform/target_default.c`, a generic desktop | optional |
| `packaging/` | Laid over `ports/common/packaging/` when staging. `port-hooks.sh` is sourced by the shared `run-game.sh`: `PORT_LIB_PATH`, `port_env`, `port_before_game`, `port_after_game`. Also here: the port's own `launcher.ini`, `renderers.ini`, `engine-settings.json.default`, and whatever the device's frontend needs to list the app | optional |
| `README.md` | The device as measured, the port's status, what was verified on it | yes |

A port also gets a configure preset in its branch's `CMakePresets.json` (its
name is the port id; it only adds the toolchain file for a cross port). The
planned ports, `android` and `x360`, have none yet.

## The pipeline

The commands are [the README's quick start](../README.md#quick-start). Each
step runs a port's hooks: `deps` runs `port_deps`; `build` the launcher's
preset in `deusex-launcher/<port>`, then `scripts/engine.sh build <port>`;
`deploy` builds and stages, then runs `port_deploy`; `run` and `profile` run
`port_run` and `port_profile`.

Each port's launcher builds in its own checkout, `deusex-launcher/<port>/build/`;
the engine goes to `build/<port>/engine`, and `build/<port>/app` is the staged
result, both beside the repositories. Everything fetched lands in `deps/`,
there too: `deps/toolchains/<name>` is shared between ports,
`deps/sysroots/<port>` is one device's; each launcher checkout links it in.
None of it is versioned and all of it can be deleted; `deps/` costs a download
(and for a vendor sysroot, the device) to rebuild.

Staging installs the binaries, lays `ports/common/packaging` and then the
port's `packaging/` beside them, copies in the engine's `SurrealEngine`,
`libSurrealVideo.so` and `SurrealEngine.pk3`, and runs `port_stage`. `launcher.ini`
belongs to whoever runs the app: it is created from `launcher.ini.default`
only when missing.

## Adding a port

1. **Measure the device before writing anything.** Every wrong guess about
   the device costs time. Find out: the glibc version (`/lib/ld-*.so`,
   `ldd --version`); whether SDL2 is the distro's or a vendor build with its own
   video driver; which GPU APIs exist (`dxl-cli --probe` from any aarch64 or
   x86_64 build, and the probes below); what the pad reports
   (`probe-sdl.c --pad`); the screen size; the fonts on the system; what the
   device's own frontend expects an app to look like; how its CPU governor is
   managed.
2. **Branch from `main`, or from the port closest to it**
   (`git -C deusex-launcher/main worktree add -b <id> ../<id> <base>`), and
   keep only what differs. A Linux device that runs the ports' launcher starts
   from linux-x86_64: one with an ordinary distro is often nothing more than a
   new name (`linux-aarch64` is linux-x86_64 built for another architecture);
   for a vendor-firmware handheld -- cross-built against the device's own
   libraries, its own frontend and CPU modes -- `trimui-smartpro` shows what
   the differences look like. A platform that shares only `main`'s launcher
   with the others, such as the Xbox 360, starts from `main`.
3. **Pick a toolchain whose glibc is at or below the device's** -- one newer
   symbol and the binary will not load (see the Smart Pro's README for how that
   was found). Bootlin publishes many; `dx_fetch_bootlin <name> <ceiling>` in
   `port.sh` fetches one and refuses it if its glibc is too new. C11 is enough
   for the launcher; the engine needs C++20 (GCC 10 or later).
4. **Decide where SDL2 comes from.** The system's (`DXL_PORT_SDL system`) on a
   normal distro or a native build; the device's own libraries in a sysroot when
   the vendor's build is the only one that can draw. The launcher needs SDL
   2.0.18 or later.
5. **Write `renderers.ini`** for what this engine build can run on the
   device's GPU APIs, and **`engine.cmake`** for the display backends the
   device has.
6. **Write `target.c`** if the device has anything to say: its fonts, CPU modes
   its hooks can apply (then `port-hooks.sh` must handle each one and fall back
   to the profile's default, `CPU_MODE_DEFAULT` -- `test_target_<port>`,
   with the port id's `-` as `_`, checks), a note about its pad.
7. **Write `packaging/`**: what the device's frontend needs to list and start
   the app, `port-hooks.sh` for its library path and CPU modes, a
   `launcher.ini` with the usual `GameDir`.
8. **Add the preset**, then `scripts/dx.sh build`, `stage`, `deploy`, and write
   the port's `README.md` (below).
9. **Commit and push the branch, then pin it** here (`scripts/launcher.sh pin
   <id>` adds it to `LAUNCHER-PIN.txt`); `scripts/dx.sh check` and
   `scripts/dx.sh test <id>` must pass
   ([the drift guards](DEVELOPMENT.md#drift-guards)).

## Device probes

`tools/probes/` holds small single-purpose programs for answering questions
about a device instead of assuming answers. They are not part of any build:
compile one with the port's toolchain against its sysroot, copy it over, run
it (the Smart Pro's README has the exact command line).

| Probe | Answers | Links |
| --- | --- | --- |
| `probe-sdl.c` | video driver, surface size, renderer backend, what the pad reports; `--pad [s]` records every pad event with a per-axis range summary (no window) | `-lSDL2` |
| `probe-vulkan.c` | is there a usable Vulkan device at all | `-lvulkan` |
| `probe-sdl-vulkan.c` | can SDL2 hand out a Vulkan surface here | `-lSDL2 -lvulkan` |
| `probe-vulkan-caps.c` | every requirement the engine's Vulkan device filter checks, with a verdict | `-lvulkan` |
| `probe-texture-formats.c` | which texture formats the GPU can sample and linearly filter (BCn, RGB8, RGBA32F) | `-lvulkan` |
| `probe-gles-sampler.c` | what an OpenGL ES driver accepts: each sampler parameter the engine sets, the extensions it looks for (S3TC, float-linear filtering, anisotropy, mirror-clamp, `glBufferStorage`), and the scene buffers' formats (RGBA16F, R32UI and D32F in one framebuffer); it draws to a pbuffer | `-ldl` (EGL and GLES are opened at run time) |

`dxl-cli --probe` (built with every port) reports the Vulkan device and API,
the OpenGL ES version and renderer, and desktop GL -- what the launcher's
renderer list is built from.

## Other operating systems

Every port built so far is POSIX, and so is Android (bionic has everything below).
The launcher's OS dependencies are few and named:

| What | Where | POSIX mechanism |
|---|---|---|
| Handing over to the game | `platform/posix/launch.c` behind `platform/launch.h` | `exec` of `GameCommand` |
| Crash-isolated GPU detection | `platform/posix/gpu_probe.c` | `fork` + `dlopen` of the Vulkan loader and EGL |
| Single instance, and handing a command line to it | `core/instance.c` | `flock` on a pidfile, an abstract unix socket |
| The launcher's own path; case-insensitive lookups | `core/paths.c` | `/proc/self/exe`, `opendir` |
| Atomic config writes | `core/ini.c`, `core/json.c` | write a temporary file, `rename` |

**Android** needs a different hand-over -- the engine has to run inside the
app's process -- and more besides: [its README](https://github.com/JuggyMcNutty/deusex-launcher/blob/android/ports/android/README.md).
**Xbox 360** (planned) is not POSIX, so it would need its own version of
each; nothing about it is worked out yet ([its README](https://github.com/JuggyMcNutty/deusex-launcher/blob/x360/ports/x360/README.md)).
**Windows** would need win32 versions of all five; the original binary's own
answers to the same questions are
[launch-flow.md's platform seams](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/launch-flow.md#platform-seams).

## A port's README

Every port's `README.md` has the same sections, in this order, leaving out any
that do not apply:

| Section | Holds |
|---|---|
| **Status** | one paragraph: what runs, what does not yet |
| **Build and run** | the commands, where things live on the device, how to reach it |
| **What differs from linux-x86_64** | the port's files, and why each difference was needed |
| **The device, as measured** | what was probed, not assumed |
| **Verified** | what was checked on real hardware |
| **Performance** | where it is measured: the numbers and how to take them |
| **Gotchas** | what bit on this device |
