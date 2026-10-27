# Porting

How a device becomes a port: **a branch of the launcher**,
[deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher), with the
engine, [DXEngine](https://github.com/JuggyMcNutty/DXEngine), built for it.
Each port branch's launcher is DXLauncher, its `docs/LAUNCHER.md` its own:
the same on linux-x86_64 and linux-aarch64
([linux-x86_64's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md)),
on windows-x86_64 a copy on Windows' own calls
([windows-x86_64's](https://github.com/JuggyMcNutty/deusex-launcher/blob/windows-x86_64/docs/LAUNCHER.md)),
and on a device branch a copy with the device's changes
([trimui-smartpro's](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/docs/LAUNCHER.md));
the ports that exist are [the root README's](../README.md#engine-ports).

## The branches

| Branch | Holds |
|---|---|
| `main` | the original launcher, recreated almost 1:1: no additions, no ports; the reference the port branches start from |
| `linux-x86_64` | the default of desktop Linux: DXLauncher, the desktop's own launcher, begun from `main`'s, whose entry decision, command line, crash sentinel and config it keeps, with the desktop's port around it (`ports/linux-x86_64`, its preset, its block in `CMakeLists.txt`) |
| `linux-aarch64` | the default of aarch64 Linux with an ordinary distro: DXLauncher, as linux-x86_64 has it but for the engine's SDL window it asks for (`src/app/run.c`), each launcher commit cherry-picked from one to the other, with the port around it (`ports/linux-aarch64`, its presets), cross-built or native |
| `windows-x86_64` | the default of Windows on x86_64: DXLauncher, as linux-x86_64 has it, on Windows' own calls (`src/platform/win32/`, the finder's Windows places in `src/core/finder_places_win32.c`), each launcher commit cherry-picked from linux-x86_64, with the port around it (`ports/windows-x86_64`, its presets), cross-built by llvm-mingw and tested under Wine |
| `trimui-smartpro` | the TrimUI Smart Pro's: linux-aarch64's DXLauncher, each change for the device a commit of its own on it -- an SDL3 video driver, the spruceOS app, CPU modes, the big screen alone, no desktop -- with the port around it (`ports/trimui-smartpro`). Tag `trimui-smartpro-tabbed` holds the device's tabbed launcher |

A port branch lives while its platform or device is targeted. It holds only
what it builds, with its own `README.md` and `docs/LAUNCHER.md`; every port
branch but linux-x86_64 keeps the desktop's port as its host build
(`ports/linux-x86_64/port.cmake` and its preset), all `scripts/dx.sh test`
needs. Nothing merges between the branches, `main` included: a change
crosses only by a deliberate cherry-pick, in a commit of its own on each
branch it serves, with that branch's docs. `scripts/launcher.sh status` says
which pins lack their branch's newer commits.

A device branch's tailoring is direct edits to the launcher it started from,
few and as small as they can be, each listed with the shared files it
touches in the port's README: that list is where a cherry-pick from its
base may meet them.

## The layers

DXLauncher's, every port branch's.

| Layer | Where | Changes per port? |
|---|---|---|
| The launcher's core (the entry decision, command-line parsing, crash sentinel, config seeding) | `src/core/`: `main`'s entry decision, command line, crash sentinel and config, with DXLauncher's folders, profiles, installs, settings, `Settings.json`, renderer list, pad layouts and the engine's command line | A branch's own facts (windows-x86_64's finder places, no-sound advice and renderer list; trimui-smartpro's CPU modes, pad, finder and renderer list). C11, no SDL |
| The OS: the game as a child, the GPU probe, the single instance and its waits, the file system and folders, the network ([its seams](#other-operating-systems)) | `src/platform/`, a header per seam: POSIX's in `src/platform/posix/`, Windows' in `src/platform/win32/` | For a non-POSIX platform (windows-x86_64's), a device without a desktop (trimui-smartpro's, which deletes for good), and Android's hand-over (below) |
| One start and the game | `src/app/`, no SDL | A device branch's hook around the game (trimui-smartpro's `gamehook.c`) |
| The screens | `src/screens/`, on its UI kit, `src/ui/` (SDL3 and SDL3_ttf); SDL3 with a video driver of the port's own where only the vendor's draws | A device branch's own choices (trimui-smartpro's) |
| The engine | DXEngine, its clone beside the workspace in `DXEngine/`, pinned by `ENGINE-PIN.txt` | Built per port from its `ports/<id>/engine.cmake` |
| The app around the binaries | `cmake --install`'s (the programs, `data/`, `THIRD-PARTY.md`) + `ports/<id>/packaging/` | A device's app files laid over it |

## What a port is

A port's directory, `ports/<id>/` on its branch, holds **only what differs**
from the branch it started from:

| File | Purpose | Required |
|---|---|---|
| `port.cmake` | Included by the branch's `CMakeLists.txt` when `DXL_PORT=<id>`. Sets `DXL_PORT_GLIBC_MAX`, a Linux port's glibc ceiling, or `DXL_PORT_WINDOWS_MIN`, a Windows port's floor (either turns on the post-build `scripts/check-abi.sh`), `DXL_PORT_SYSROOT` and `DXL_PORT_LINK_OPTIONS`; where SDL3, SDL3_ttf and libxmp come from (`DXL_PORT_DEPS_DEFAULT`: `auto`, `system`, or `vendored`, built from `cmake/deps.lock`'s sources, with `DXL_PORT_DEPS_ENV` and `DXL_PORT_DEPS_C_FLAGS` for a cross build's); the sysroot's header folders (`DXL_PORT_INCLUDE_DIRS`); and on a branch whose `cmake/deps.cmake` takes them, SDL3's own options (`DXL_PORT_SDL3_ARGS`) and a video driver's files (`DXL_PORT_SDL3_DRIVER`, trimui-smartpro's); and the programs' further sources (`DXL_PORT_LAUNCHER_SOURCES`, `DXL_PORT_CLI_SOURCES`, `DXL_PORT_TEST_SOURCES`: windows-x86_64's resources in `win32/`) | yes |
| `port.sh` | Sourced by this workspace's `scripts/dx.sh`, `scripts/engine.sh` and `scripts/ci.sh`, with `scripts/lib/common.sh` loaded. `PORT_DESC`; `PORT_ENGINE=1` if the engine builds for this port here; `PORT_LAUNCHER`, the launcher's program that `stage` and `run` look for (`deusex-launcher` unless set); `PORT_ENGINE_FILES`, the engine build's files that `stage` copies, its program first (`DXEngine DXEngine.pk3` unless set); hooks `port_deps` (fetch toolchains/sysroot), `port_stage` (adjust the staged app), `port_deploy` (send it to the device; over SSH, `dx_ssh_sync` in `scripts/lib/common.sh` sends only what changed, keeps the device's previous copies and verifies), `port_run` (run it here), `port_profile` (a frame-time profile on the device), `port_test` (tests of the port's own, which `scripts/dx.sh test <port>` runs after the host build's; a Windows port's under Wine, `scripts/wine.sh`). Unset hooks have safe defaults | yes |
| `toolchain-c.cmake`, `toolchain-cxx.cmake` | Cross toolchains, referenced by the port's preset (C) and `engine.cmake` (C++); linux-x86_64 and windows-x86_64 have one for both, `toolchain.cmake`: linux-x86_64's its SDK's, a native build against a sysroot; windows-x86_64's llvm-mingw's, the tests' emulator `scripts/wine.sh`. A toolchain file that `return()`s early when the host already is the target arch makes the same preset build natively | cross ports, and a port built by an SDK (`toolchain-cxx.cmake` where the engine cross-builds) |
| `engine.cmake` | A CMake initial cache (`cmake -C`) for building the engine: its toolchain and `ENABLE_*` switches, each set with `FORCE` so an edit reaches an existing build too | if it builds the engine |
| `packaging/` | Laid over the staged app: whatever the device's frontend needs to list and start the app (trimui-smartpro's `launch.sh`, `config.json`, icon), and on a branch with the game hook, `port-hooks.sh` (`before <CPU mode>`, `after`) | optional |
| `sdl3/` | A video driver of the port's own for SDL3, where only the vendor's library draws (trimui-smartpro's, fb0 and the PowerVR EGL), on a branch whose `cmake/deps.cmake` takes `DXL_PORT_SDL3_DRIVER` | optional |
| `win32/` | The programs' manifest (Windows 10's, the UTF-8 code page) and resource scripts with their versions, which `port.cmake` names (windows-x86_64's) | a Windows port |
| `fetch-sysroot.sh`, `tools/` | Scripts of the device's own that `port.sh` runs: its sysroot from the device (`port_deps`), a map's profile on it (`port_profile`); trimui-smartpro's | optional |
| `README.md` | The device as measured, the port's status, what is verified on it | yes |

A port also gets a configure preset in its branch's `CMakePresets.json` (its
name is the port id; it only adds the port's toolchain file, if it has one).

`CMakeLists.txt` takes `port.cmake`'s sysroot and its header folders, the
glibc ceiling or Windows' floor, the programs' link options and further
sources, and where SDL3, SDL3_ttf and libxmp come from.

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

Staging makes `build/<port>/app` from empty: it installs the binaries and
what the launcher's install rules add, lays the port's `packaging/` over
them, copies in the engine's files (`PORT_ENGINE_FILES`) and its notices
(`DXEngine-LICENSE.md`, the engine's `LICENSE.md`), and runs `port_stage`.
The launcher's own files are in its home, never in the app.

## Adding a port

1. **Measure the device before writing anything.** Every wrong guess about
   the device costs time. Find out: the glibc version (`/lib/ld-*.so`,
   `ldd --version`); whether SDL is the distro's or a vendor build with its own
   video driver, and what the display is (`probe-fbdev-egl.c` where there is
   no DRM or window system); which GPU APIs exist (`dxl-cli --probe` from
   any DXLauncher build, and the probes below); what the
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
   headers (linux-x86_64's and linux-aarch64's) or the device's
   (trimui-smartpro's), or the system's, when it has them, on a build with
   the machine's own compiler. Where only a vendor's library draws on the
   device, SDL3 gets a video driver of the port's own, its private driver
   (`sdl3/`, trimui-smartpro's), and nothing else to draw with. `main`'s
   launcher needs SDL 2.0.16 and SDL_ttf 2.0.18 or later.
5. **Write the device's data and `engine.cmake`**: `data/renderers.ini` for
   what this engine build can run on the device's GPU APIs, and Software,
   which draws on the CPU and needs none,
   `data/engine-settings.json.default` for its defaults, and `engine.cmake`
   for the display backends the device has.
6. **Tailor the launcher to the device**, where it needs it -- its screen,
   its pad, what it lacks of a desktop, a hook around the game for its CPU
   modes -- each change a commit of its own with its tests and its
   `docs/LAUNCHER.md` paragraphs, each listed in the port's README with the
   shared files it touches.
7. **Write `packaging/`**: what the device's frontend needs to list and
   start the app, and `port-hooks.sh` where the branch has the game hook.
8. **Add the preset, commit and push the branch, then pin it** here
   (`scripts/launcher.sh pin <id>` adds it to `LAUNCHER-PIN.txt`), and run
   `scripts/launcher.sh fetch`, which links `deps/` and `gamefiles/` into its
   worktree. Before the pin, `scripts/dx.sh` takes the port by its worktree
   alone, with a warning, its links made by hand, and `scripts/dx.sh check`
   passes it over. The branch has the launcher's CI file from its base,
   `.github/workflows/build.yml`, the same on every branch; once pinned, the
   pins' push builds the port on GitHub's CI
   ([CI](DEVELOPMENT.md#ci)), its `deps` with no device to reach: a vendor
   sysroot needs another source there, as the Smart Pro's takes TrimUI's SDK.
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
| `probe-gles-sampler.c` | what an OpenGL ES driver accepts: each sampler parameter the engine sets, the extensions it looks for (S3TC, float-linear filtering, anisotropy, mirror-clamp), and two things no device uses, `glBufferStorage` and an RGBA16F, R32UI and D32F framebuffer; it draws to a pbuffer | `-ldl` (EGL and GLES are opened at run time) |
| `probe-gles-scene.c` | what the GL device's scene asks of an OpenGL ES driver beyond the samplers: a clip distance (`GL_EXT_clip_cull_distance`; without one the device clips a mirror by a discard) that clips a draw only while `GL_CLIP_DISTANCE0` is on, and RGB10_A2 scene buffers made with `glTexStorage2D` beside D32F (and an R32UI attachment, which the engine's scene lacks), single and multisampled, resolved by a blit, sampled by a pass and read back as `UNSIGNED_BYTE` and `2_10_10_10_REV`, their ten bits kept; it draws to framebuffers in a pbuffer's context | `-ldl` |
| `probe-fbdev-egl.c` | what a video driver of a port's own needs where fbdev and the vendor's EGL are the only display path: the framebuffer's mode and layout, whether it takes 2x and 3x `yres_virtual`, pans and waits for vsync; which EGL and GLES sonames load; which native window `eglCreateWindowSurface` takes (`{ushort w,h}`, `{int w,h}`, NULL), each read back from fb0; swap timings; five create-draw-terminate cycles; the consoles' modes; a 1280x720 copy into fb0; each input device's name, ids and capabilities, and whether `libudev.so.1` loads. `--draw <s>` draws until killed, `--beside <cmd>` runs a command beside a live surface | `-ldl` |

`dxl-cli --probe` (any DXLauncher build: the host's, linux-aarch64's, the
Smart Pro's, windows-x86_64's) reports the Vulkan device and API, the OpenGL
ES version and renderer where there is OpenGL ES (Windows has none) and
desktop GL; on linux-x86_64 and linux-aarch64, Mesa's CPU drivers too,
lavapipe and llvmpipe, apart from the GPU's -- what the launcher's renderer
list is built from.

## Other operating systems

Every Linux port is POSIX, and so is Android: bionic has everything below,
though its app model changes the hand-over (below) and the GPU probe.
DXLauncher's OS dependencies are few and named: each behind a header in
`src/platform/`, POSIX's in `src/platform/posix/` and Windows' in
`src/platform/win32/` (windows-x86_64's), what both C libraries do alike
(stdio, `stat`, `opendir`) called as it is.

| What | Seam | POSIX's | Windows' |
|---|---|---|---|
| The game as a child it waits for, over a line both ways | `platform/process.h` | `process.c`: `fork`, `execve`, `socketpair` | `process.c`: `CreateProcessW` with the list of handles it inherits, a named pipe, `TerminateProcess` |
| Crash-isolated GPU detection | `platform/gpu_probe.h` | `gpu_probe.c`: `fork` + `dlopen` of the Vulkan loader and EGL | `gpu_probe.c`: `vulkan-1.dll` and WGL, in `dxl-cli`, on a thread a hung driver is left behind on |
| Single instance, and handing a command line to it | `platform/instance.h` | `instance.c`: `flock` on a pidfile, a unix socket file, both in the user's runtime folder | `instance.c`: a named mutex, a named pipe of messages |
| Waiting on that, and on a signal | `platform/wait.h` | `wait.c`: a pipe each, `poll` | `wait.c`: an event each, `WaitForMultipleObjects` |
| The file system where it differs: the launcher's own path, a path's root, links, atomic writes | `platform/fs.h` | `fs.c`: `/proc/self/exe`, `realpath`, `symlink`, `mkstemp`, `fsync`, `rename` | `fs.c`: the wide calls, `GetFinalPathNameByHandleW`, `CreateSymbolicLinkW`, `CREATE_NEW`, `FlushFileBuffers`, `MoveFileExW` |
| Its folders | `platform/folders.h` | `folders.c`: the XDG base directories | `folders.c`: the Known Folders |
| How a program it started ended; its environment | `platform/os.h` | `os.c`: the signals' names, `setenv` | `os.c`: the exception codes' names, `_wputenv_s` |
| What ends a start from outside | `platform/startup.h` | `startup.c`: SIGINT, SIGTERM, SIGHUP | `startup.c`: the console's Ctrl+C, Ctrl+Break and closing |
| The server browser's network | `platform/net.h` | `net.c`: `getaddrinfo`, TCP and UDP sockets, `poll`, and for the local network's servers `SO_BROADCAST` to each interface's broadcast address from `getifaddrs` | `net.c`: Winsock 2, `WSAPoll`, each adapter's broadcast address from `GetAdaptersAddresses` |

Where the game is on the platform is the finder's places,
`core/finder_places.c` (desktop Linux's: Steam's, Heroic's, Lutris',
Bottles', the Wine prefixes, the drives) and `core/finder_places_win32.c`
(Windows': the Steam, GOG and installer keys in the registry, Heroic's, the
drives), made of `core/finder.c`'s looking. On the desktop's branches,
linux-x86_64 and linux-aarch64, also: a folder or page opened by `xdg-open`
(`platform/posix/process.c`); its entry in the desktop's app menu, which
takes the `deusex://` links too (`platform/posix/desktop.c`: a
freedesktop.org desktop entry); the trash (`platform/posix/trash.c`:
freedesktop.org's Trash spec, a `rename`). On windows-x86_64 the same
three: `ShellExecuteW` (`platform/win32/process.c`); the Start menu's
shortcuts and `deusex://` in the user's registry
(`platform/win32/desktop.c`); the Recycle Bin (`platform/win32/trash.c`). The
Smart Pro has no desktop for them: it deletes for good
(`platform/posix/delete.c`: `nftw`). `main`'s launcher has its own
`core/instance.c` and `core/paths.c`, and three more: the game as a child,
as DXLauncher's; its own start again for safe mode
(`platform/posix/launch.c`: `execv`); the Detail page's machine
(`platform/posix/machine.c`: cpufreq's maximum, else `/proc/cpuinfo`;
`sysinfo`).

**Android** needs a different hand-over: the engine has to run inside the
app's process. **Xbox 360** is not POSIX, so it would need its own version of
each, as Windows has; the original binary's own answers to the same
questions are
[launch-flow.md's platform seams](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/launch-flow.md#platform-seams).

## A port's README

Every port's `README.md` has the same sections, in this order, leaving out any
that do not apply:

| Section | Holds |
|---|---|
| **Status** | one paragraph: what runs, what does not |
| **Build and run** | the commands, where things live on the device, how to reach it |
| **What differs from the branch it started from** | the port's files, and why each difference is needed; linux-x86_64, the desktop's default, has **What it is made of** in its place |
| **The device, as measured** | what is probed, not assumed |
| **Verified** | what is checked on real hardware |
| **Performance** | where it is measured: the numbers and how to take them |
| **Gotchas** | what bit on this device |
