# Porting

How a device becomes a port: **a branch of the launcher**,
[deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher), with the
engine, [DXEngine](https://github.com/JuggyMcNutty/DXEngine), built for it.
Each port branch's launcher is its own `docs/LAUNCHER.md`: DXLauncher's,
the same on linux-x86_64 and linux-aarch64
([linux-x86_64's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md)),
and the tabbed launcher's
([trimui-smartpro's](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/docs/LAUNCHER.md));
the ports that exist are [the root README's](../README.md#engine-ports).

## The branches

| Branch | Holds |
|---|---|
| `main` | the original launcher, recreated almost 1:1: no additions, no ports; the reference the port branches start from |
| `linux-x86_64` | the default of desktop Linux: DXLauncher, the desktop's own launcher, begun from `main`'s, whose core it keeps, with the desktop's port around it (`ports/linux-x86_64`, its preset, its block in `CMakeLists.txt`) |
| `linux-aarch64` | the default of aarch64 Linux with an ordinary distro: DXLauncher, as linux-x86_64 has it but for the engine's SDL window it asks for (`src/app/run.c`), each launcher commit cherry-picked from one to the other, with the port around it (`ports/linux-aarch64`, its presets), cross-built or native |
| `trimui-smartpro` | the TrimUI Smart Pro's own launcher, the tabbed home screen: its device profile, CPU modes, cross toolchains and packaging (`ports/trimui-smartpro`) |

A port branch lives while its platform or device is targeted. It holds only
what it builds, with its own `README.md` and `docs/LAUNCHER.md`; every port
branch but linux-x86_64 keeps the desktop's port as its host build
(`ports/linux-x86_64/port.cmake` and its preset), all `scripts/dx.sh test`
needs. Nothing merges between the branches, `main` included: a change
crosses only by a deliberate cherry-pick, in a commit of its own on each
branch it serves, with that branch's docs. `scripts/launcher.sh status` says
which pins lack their branch's newer commits.

## The layers

The tabbed launcher's, trimui-smartpro's, and DXLauncher's, linux-x86_64's
and linux-aarch64's.

| Layer | Where | Changes per port? |
|---|---|---|
| The launcher's core (the entry decision, command-line parsing, crash sentinel, config seeding) | `src/core/`: `main`'s, with its own config, `Settings.json`, renderer list and pad layouts; DXLauncher's with its folders, profiles, installs, settings and the engine's command line too | A device branch's own facts (trimui-smartpro's buttons, PowerVR anti-aliasing lock and defaults). C11, no SDL |
| The device profile | `src/platform/target.h`, `target_default.c`; trimui-smartpro's profile code and its `ports/trimui-smartpro/target.c` | Data only, on a branch with that code |
| The OS: handing over to the game, the GPU probe | `src/platform/`: the tabbed launcher's `launch.h`, its own replaced by the game's; DXLauncher's `process.h`, the game a child it waits for; `src/platform/posix/` | Only for a non-POSIX platform, and for Android's hand-over (below) |
| One start and the game | DXLauncher's `src/app/`, no SDL | no |
| The screens | the tabbed launcher's `src/ui/` (SDL2); DXLauncher's `src/screens/`, on its UI kit, `src/ui/` (SDL3 and SDL3_ttf) | A device branch's own choices (trimui-smartpro's) |
| The engine | DXEngine, its clone beside the workspace in `DXEngine/`, pinned by `ENGINE-PIN.txt` | Built per port from its `ports/<id>/engine.cmake` |
| The app around the binaries | `ports/common/packaging/` (the base app, trimui-smartpro's) + `ports/<id>/packaging/` | The port's files are laid over the base |

## What a port is

A port's directory, `ports/<id>/` on its branch, holds **only what differs**
from the branch it started from:

| File | Purpose | Required |
|---|---|---|
| `port.cmake` | Included by the branch's `CMakeLists.txt` when `DXL_PORT=<id>`. Sets `DXL_PORT_GLIBC_MAX` (turns on the post-build `scripts/check-abi.sh`), `DXL_PORT_SYSROOT` and `DXL_PORT_LINK_OPTIONS`; on DXLauncher's branches also where SDL3, SDL3_ttf and libxmp come from (`DXL_PORT_DEPS_DEFAULT`: `auto`, `system`, or `vendored`, built from `cmake/deps.lock`'s sources, with `DXL_PORT_DEPS_ENV` and `DXL_PORT_DEPS_C_FLAGS` for a cross build's) and the sysroot's header folders (`DXL_PORT_INCLUDE_DIRS`); on the tabbed launcher's, `DXL_PORT_SDL` (`system`: pkg-config; `sysroot`: SDL2 from `DXL_PORT_SYSROOT`, the device's own) | yes |
| `port.sh` | Sourced by this workspace's `scripts/dx.sh`, with `scripts/lib/common.sh` loaded. `PORT_DESC`; `PORT_ENGINE=1` if the engine builds for this port here; `PORT_LAUNCHER`, the launcher's program that `stage` and `run` look for (`deusex-launcher` unless set); hooks `port_deps` (fetch toolchains/sysroot), `port_stage` (adjust the staged app), `port_deploy` (send it to the device; over SSH, `dx_ssh_sync` in `scripts/lib/common.sh` sends only what changed, keeps the device's previous copies and verifies), `port_run` (run it here), `port_profile` (a frame-time profile on the device). Unset hooks have safe defaults | yes |
| `toolchain-c.cmake`, `toolchain-cxx.cmake` | Cross toolchains, referenced by the port's preset (C) and `engine.cmake` (C++); linux-x86_64 has one for both, `toolchain.cmake`, its SDK's, a native build against a sysroot. A toolchain file that `return()`s early when the host already is the target arch makes the same preset build natively | cross ports, and a port built by an SDK (`toolchain-cxx.cmake` where the engine cross-builds) |
| `engine.cmake` | A CMake initial cache (`cmake -C`) for building the engine: its toolchain and `ENABLE_*` switches, each set with `FORCE` so an edit reaches an existing build too | if it builds the engine |
| `target.c` | A device's profile, a `dxl_target`, on a branch with trimui-smartpro's profile code: fonts to try first, the panel size, the CPU modes its hooks can apply (with their help text), a note about the built-in pad, the About line, and the GPU `dxl-shots` should pretend to have. Without one the build uses `src/platform/target_default.c`, a generic desktop | optional |
| `packaging/` | Laid over `ports/common/packaging/` when staging. On a branch whose `run-game.sh` sources one (trimui-smartpro's), `port-hooks.sh`: `PORT_LIB_PATH`, `port_env`, `port_before_game`, `port_after_game`. Also here: the port's own `launcher.ini`, `renderers.ini`, `engine-settings.json.default`, and whatever the device's frontend needs to list the app | optional |
| `README.md` | The device as measured, the port's status, what is verified on it | yes |

A port also gets a configure preset in its branch's `CMakePresets.json` (its
name is the port id; it only adds the port's toolchain file, if it has one).

On DXLauncher's branches, linux-x86_64 and linux-aarch64, `CMakeLists.txt`
takes `port.cmake`'s sysroot and its header folders, the glibc ceiling and
link options for both programs, and where SDL3, SDL3_ttf and libxmp come
from. Neither has `ports/common/packaging` for a `packaging/` to be laid
over, nor a profile for a `target.c`.

## The pipeline

The commands are [the README's quick start](../README.md#quick-start). Each
step runs a port's hooks: `deps` runs `port_deps`; `build` the launcher's
preset in `deusex-launcher/<port>`, then `scripts/engine.sh build <port>`;
`deploy` builds and stages, then runs `port_deploy`; `run` and `profile` run
`port_run` and `port_profile`.

Each port's launcher builds in its own checkout,
`deusex-launcher/<port>/build/`; the engine goes to `build/<port>/engine`, and
`build/<port>/app` is the staged result, both beside the repositories.
Everything fetched lands in `deps/`, there too: `deps/toolchains/<name>` is
shared between ports, `deps/sysroots/<port>` is one device's; each launcher
checkout links it in. None of it is versioned and all of it can be deleted;
`deps/` costs a download (and for a vendor sysroot, the device) to rebuild.

Staging installs the binaries, lays `ports/common/packaging` and then the port's
`packaging/` beside them on a branch that has them, copies in the engine's
`DXEngine` and `DXEngine.pk3`, and runs `port_stage`. On such a
branch `launcher.ini` belongs to whoever runs the app: it is created from
`launcher.ini.default` only when missing.

## Adding a port

1. **Measure the device before writing anything.** Every wrong guess about
   the device costs time. Find out: the glibc version (`/lib/ld-*.so`,
   `ldd --version`); whether SDL2 is the distro's or a vendor build with its own
   video driver; which GPU APIs exist (`dxl-cli --probe` from trimui-smartpro's
   build, for any aarch64 Linux or the host, and the probes below); what the
   pad reports (`probe-sdl.c --pad`); the screen size; the fonts on the
   system; what the device's own frontend expects an app to look like; how its
   CPU governor is managed.
2. **Branch from the default of the device's platform**, that platform's
   launcher and port
   (`git -C deusex-launcher/main worktree add -b <id> ../<id> <base>`), and
   keep only what it builds. A device with an ordinary distro may need nothing
   more than that default (`linux-x86_64`, `linux-aarch64`); for a
   vendor-firmware handheld -- cross-built against the device's own libraries,
   its own frontend and CPU modes -- `trimui-smartpro` shows what a device
   branch holds.
3. **Pick a toolchain whose glibc is at or below the device's**: one newer
   symbol and the binary will not load
   ([the glibc ceiling](DEVELOPMENT.md#gotchas-that-cost-time)). A branch of
   linux-x86_64 or linux-aarch64 starts with theirs, Godot's Buildroot SDK,
   GCC 13.2 on glibc 2.28 (`dx_fetch_buildroot` in `port.sh`); for an older
   glibc, Bootlin publishes many (`dx_fetch_bootlin <name> <ceiling>`). Both
   refuse a toolchain whose glibc is too new. C11 is enough for the launcher;
   the engine needs C++20 (GCC 10 or later).
4. **Decide where SDL comes from.** DXLauncher's SDL3 and SDL3_ttf, 3.2 or
   later, and libxmp 4.4 or later: built in from `cmake/deps.lock`'s sources
   on a build with the SDK, SDL3's backends against a sysroot of a distro's
   headers (linux-x86_64's and linux-aarch64's), or the system's, when it has
   them, on a build with the machine's own compiler. The tabbed launcher's
   SDL2, 2.0.18 or later: the system's (`DXL_PORT_SDL system`), or a
   sysroot's, the device's own libraries when the vendor's build is the only
   one that can draw. `main`'s launcher needs SDL 2.0.16 and SDL_ttf 2.0.18
   or later.
5. **Write `renderers.ini`**, on a branch with the tabbed launcher, for what
   this engine build can run on the device's GPU APIs, and **`engine.cmake`**
   for the display backends the device has.
6. **Write `target.c`**, on a branch with trimui-smartpro's profile code, if
   the device has anything to say: its fonts, a note about its pad, the CPU
   modes its hooks can apply (then `port-hooks.sh` must handle each one and
   fall back to the profile's default, `CPU_MODE_DEFAULT`: `test_target_<port>`
   checks, [drift guards](DEVELOPMENT.md#drift-guards)).
7. **Write `packaging/`**, on a branch with `ports/common/packaging`: what the
   device's frontend needs to list and start the app, `port-hooks.sh` for its
   library path and CPU modes where the branch has them, a `launcher.ini` with
   the usual `GameDir`.
8. **Add the preset, commit and push the branch, then pin it** here
   (`scripts/launcher.sh pin <id>` adds it to `LAUNCHER-PIN.txt`;
   `scripts/dx.sh` takes only pinned ports), and run
   `scripts/launcher.sh fetch`, which links `deps/` and `gamefiles/` into its
   worktree.
9. **`scripts/dx.sh build`, `stage` and `deploy`**, write the port's
   `README.md` (below), and pin it again; `scripts/dx.sh check` and
   `scripts/dx.sh test <id>` must pass
   ([the drift guards](DEVELOPMENT.md#drift-guards)).

## Device probes

`tools/probes/` holds small single-purpose programs for answering questions
about a device instead of assuming answers. They are not part of any build:
compile one with the port's toolchain against its sysroot, copy it over, run
it (the Smart Pro's README has the exact command line).

| Probe | Answers | Links |
|---|---|---|
| `probe-sdl.c` | video driver, surface size, renderer backend, what the pad reports; `--pad [s]` records every pad event with a per-axis range summary (no window) | `-lSDL2` |
| `probe-sdl-vulkan.c` | can SDL2 hand out a Vulkan surface here | `-lSDL2 -lvulkan` |
| `probe-vulkan-caps.c` | every requirement the engine's Vulkan device filter checks, with a verdict | `-lvulkan` |
| `probe-texture-formats.c` | which texture formats the GPU can sample and linearly filter (BCn, RGB8, RGBA32F) | `-lvulkan` |
| `probe-gles-sampler.c` | what an OpenGL ES driver accepts: each sampler parameter the engine sets, the extensions it looks for (S3TC, float-linear filtering, anisotropy, mirror-clamp, `glBufferStorage`), and the scene buffers' formats with `Hdr` (RGBA16F, R32UI and D32F in one framebuffer); it draws to a pbuffer | `-ldl` (EGL and GLES are opened at run time) |
| `probe-gles-scene.c` | what the GL device's scene asks of an OpenGL ES driver beyond the samplers: a clip distance (`GL_EXT_clip_cull_distance`; without one the device clips a mirror by a discard) that clips a draw only while `GL_CLIP_DISTANCE0` is on, and RGB10_A2 scene buffers made with `glTexStorage2D` beside R32UI and D32F, single and multisampled, resolved by a blit, sampled by a pass and read back as `UNSIGNED_BYTE` and `2_10_10_10_REV`, their ten bits kept; it draws to framebuffers in a pbuffer's context | `-ldl` |
| `probe-fbdev-egl.c` | what a video driver of a port's own needs where fbdev and the vendor's EGL are the only display path: the framebuffer's mode and layout, whether it takes 2x and 3x `yres_virtual`, pans and waits for vsync; which EGL and GLES sonames load; which native window `eglCreateWindowSurface` takes (`{ushort w,h}`, `{int w,h}`, NULL), each read back from fb0; swap timings; five create-draw-terminate cycles; the consoles' modes; a 1280x720 copy into fb0; each input device's name, ids and capabilities, and whether `libudev.so.1` loads. `--draw <s>` draws until killed, `--beside <cmd>` runs a command beside a live surface | `-ldl` |

`dxl-cli --probe` (trimui-smartpro's build, for any aarch64 Linux and the host)
reports the Vulkan device and API, the OpenGL ES version and renderer, and
desktop GL -- what the launcher's renderer list is built from.

## Other operating systems

Every Linux port is POSIX, and so is Android: bionic has everything below,
though its app model changes the hand-over (below) and the GPU probe.
The tabbed launcher's OS dependencies, the Smart Pro's, are few and named:

| What | Where | POSIX mechanism |
|---|---|---|
| Handing over to the game | `platform/posix/launch.c` behind `platform/launch.h` | `exec` of `GameCommand` |
| Crash-isolated GPU detection | `platform/posix/gpu_probe.c` | `fork` + `dlopen` of the Vulkan loader and EGL |
| Single instance, and handing a command line to it | `core/instance.c` | `flock` on a pidfile, an abstract unix socket |
| The launcher's own path; case-insensitive lookups | `core/paths.c` | `/proc/self/exe`, `opendir` |
| Atomic config writes | `core/ini.c`, `core/json.c` | write a temporary file, `rename` |

DXLauncher, linux-x86_64's and linux-aarch64's, has the same single instance,
paths, atomic writes (`mkstemp`, `fsync`, `rename`) and GPU probe, and in
place of the hand-over the game as a child it waits for, over a socket line
(`platform/posix/process.c`: `fork`, `execv`, `socketpair`; a folder or page
opened by `xdg-open`); and its folders (`core/xdg.c`: the XDG base
directories), its entry in the desktop's app menu, which takes the
`deusex://` links too (`platform/posix/desktop.c`: a freedesktop.org desktop
entry), the trash (`platform/posix/trash.c`: freedesktop.org's Trash spec, a
`rename`) and the server browser's network (`platform/posix/net.c`:
`getaddrinfo`, TCP and UDP sockets, `poll`, and for the local network's servers
`SO_BROADCAST` to each interface's broadcast address from `getifaddrs`).
`main`'s launcher shares `core/instance.c`, `core/paths.c` and `core/ini.c`, and
has three more: the game as a child, as DXLauncher's; its own start again for
safe mode (`platform/posix/launch.c`: `execv`); the Detail page's machine
(`platform/posix/machine.c`: `/proc/cpuinfo`, cpufreq's maximum, `sysinfo`).

**Android** needs a different hand-over: the engine has to run inside the
app's process. **Xbox 360** is not POSIX, so it would need its own version of
each. **Windows** would need win32 versions of each; the original binary's
own answers to the same questions are
[launch-flow.md's platform seams](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/launch-flow.md#platform-seams).

## A port's README

Every port's `README.md` has the same sections, in this order, leaving out any
that do not apply:

| Section | Holds |
|---|---|
| **Status** | one paragraph: what runs, what does not yet |
| **Build and run** | the commands, where things live on the device, how to reach it |
| **What differs from the branch it started from** | the port's files, and why each difference is needed; linux-x86_64, the desktop's default, has **What it is made of** in its place |
| **The device, as measured** | what is probed, not assumed |
| **Verified** | what is checked on real hardware |
| **Performance** | where it is measured: the numbers and how to take them |
| **Gotchas** | what bit on this device |
