# Porting

How a device becomes a port. The launcher and the engine fork are shared; a
port is the small set of facts and scripts that differ for one device, in
`ports/<id>/`. The design reasons behind the launcher are in
[`DESIGN.md`](DESIGN.md); the ports that exist are listed in the
[root README](../README.md#ports).

## The layers

| Layer | Where | Changes per port? |
|---|---|---|
| The launcher contract (the original's entry decision, command-line parsing, crash sentinel, config seeding, `Settings.json`, pad layouts) | `src/core/` | No. C11, no SDL, no device facts |
| The device profile | `src/platform/target.h`; `ports/<id>/target.c` | Data only: a port may supply one |
| The OS: handing over to the game, the GPU probe | `src/platform/launch.h`, `src/platform/posix/` | Only for a non-POSIX platform (below) |
| The screens | `src/ui/` (SDL2) | No |
| The engine | the fork in `engine/SurrealEngine`, patches in `engine-patches/` | Built per port from `ports/<id>/engine.cmake` |
| The app around the binaries | `ports/common/packaging/` + `ports/<id>/packaging/` | The port's files are laid over the common ones |

## What a port is

A port directory holds **only what differs** from the common ground:

| File | Purpose | Required |
|---|---|---|
| `port.cmake` | Included by the root `CMakeLists.txt` when `DXL_PORT=<id>`. Sets `DXL_PORT_SDL` (`system`: pkg-config; `sysroot`: the device's own SDL2 from `DXL_PORT_SYSROOT`), `DXL_PORT_GLIBC_MAX` (turns on the post-build `scripts/check-abi.sh`) and `DXL_PORT_LINK_OPTIONS` | yes |
| `port.sh` | Sourced by `scripts/dx.sh`. `PORT_DESC`; `PORT_ENGINE=1` if the engine builds for this port here; hooks `port_deps` (fetch toolchains/sysroot), `port_stage` (adjust the staged app), `port_deploy` (send it to the device; over SSH, `dx_ssh_sync` in `scripts/lib/common.sh` sends only what changed, keeps the device's previous copies and verifies), `port_run` (run it here). Unset hooks have safe defaults | yes |
| `toolchain-c.cmake`, `toolchain-cxx.cmake` | Cross toolchains, referenced by the port's preset (C) and `engine.cmake` (C++). A toolchain file that `return()`s early when the host already is the target arch makes the same preset build natively | cross ports |
| `engine.cmake` | A CMake initial cache (`cmake -C`) for building the engine: its toolchain and `ENABLE_*` switches, each set with `FORCE` so an edit reaches an existing build too | if it builds the engine |
| `target.c` | The device profile, a `dxl_target`: fonts to try first, the panel size, the CPU modes the port's hooks can apply (with their help text), a note about the built-in pad, the About line, and the GPU `dxl-shots` should pretend to have. Without one the build uses `src/platform/target_default.c`, a generic desktop | optional |
| `packaging/` | Laid over `ports/common/packaging/` when staging. `port-hooks.sh` is sourced by the shared `run-game.sh`: `PORT_LIB_PATH`, `port_env`, `port_before_game`, `port_after_game`. Also here: the port's own `launcher.ini`, `renderers.ini`, `engine-settings.json.default`, and whatever the device's frontend needs to list the app | optional |
| `README.md` | The device as measured, the port's status, what was verified on it | yes |

A port also gets a configure preset in `CMakePresets.json` (its name is the
port id; it only adds the toolchain file for a cross port).

## The pipeline

```sh
scripts/dx.sh deps   <port>        # port_deps: toolchains and sysroot into deps/
scripts/engine.sh fetch            # once: clone the engine fork (upstream + engine-patches/)
scripts/dx.sh build  <port>        # the launcher preset, then scripts/engine.sh build <port>
scripts/dx.sh stage  <port>        # build/<port>/app, exactly what ships
scripts/dx.sh deploy <port>        # build, stage, then port_deploy
scripts/dx.sh run    <port>        # port_run
scripts/dx.sh check                # the drift guards (below)
```

Everything built lands in `build/<port>/`: `build/<port>/launcher` and
`build/<port>/engine` are the two CMake trees, `build/<port>/app` is the staged
result. Everything fetched lands in `deps/`: `deps/toolchains/<name>` is
shared between ports, `deps/sysroots/<port>` is one device's. Both are ignored
by git and can be deleted; `deps/` costs a download (and for a vendor sysroot,
the device) to rebuild.

Staging installs the binaries, copies the engine's `SurrealEngine`,
`libSurrealVideo.so` and `SurrealEngine.pk3`, lays `ports/common/packaging` and
then the port's `packaging/` over them, and runs `port_stage`. `launcher.ini`
belongs to whoever runs the app: it is created from `launcher.ini.default`
only when missing.

## Adding a port

1. **Measure the device before writing anything.** Every wrong guess about
   the Smart Pro cost time. Find out: the glibc version (`/lib/ld-*.so`,
   `ldd --version`); whether SDL2 is the distro's or a vendor build with its own
   video driver; which GPU APIs exist (`dxl-cli --probe` from any aarch64 or
   x86_64 build, and the probes below); what the pad reports
   (`probe-sdl.c --pad`); the screen size; the fonts on the system; what the
   device's own frontend expects an app to look like; how its CPU governor is
   managed.
2. **Start from the nearest port.** A device running an ordinary distro is
   `linux-aarch64` or `linux-x86_64` (often nothing more than a new name). A
   vendor-firmware handheld is `trimui-smartpro`.
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
   its hooks can apply (then `port-hooks.sh` must handle each one --
   `test_target_<port>` checks), a note about its pad.
7. **Write `packaging/`**: what the device's frontend needs to list and start
   the app, `port-hooks.sh` for its library path and CPU modes, a
   `launcher.ini` with the usual `GameDir`.
8. **Add the preset**, then `scripts/dx.sh build`, `stage`, `deploy`, and record
   what was verified in the port's `README.md`.
9. **`scripts/dx.sh check`** and **`scripts/dx.sh test`** must pass.

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

`dxl-cli --probe` (built with every port) reports the Vulkan device and API,
the OpenGL ES version and renderer, and desktop GL -- what the launcher's
renderer list is built from.

## Other operating systems

Every port so far is POSIX, and so is Android (bionic has everything below).
The launcher's OS dependencies are few and named:

| What | Where | POSIX mechanism |
|---|---|---|
| Handing over to the game | `platform/posix/launch.c` behind `platform/launch.h` | `exec` of `GameCommand` |
| Crash-isolated GPU detection | `platform/posix/gpu_probe.c` | `fork` + `dlopen` of the Vulkan loader and EGL |
| Single instance, and handing a command line to it | `core/instance.c` | `flock` on a pidfile, an abstract unix socket |
| The launcher's own path; case-insensitive lookups | `core/paths.c` | `/proc/self/exe`, `opendir` |
| Atomic config writes | `core/ini.c`, `core/json.c` | write a temporary file, `rename` |

**Android** needs a different hand-over -- the engine has to run inside the
app's process -- and more besides: [`../ports/android/README.md`](../ports/android/README.md).
**Windows** would need win32 versions of all five; the original binary's own
answers to the same questions are in [`re/porting-notes.md`](re/porting-notes.md).

## Drift guards

Docs and scripts that describe the tree are checked against it, so a move or a
rename fails loudly instead of leaving stale instructions:

- `scripts/check-docs.sh` (also the `docs_paths` test): every repository path
  a doc names in backticks or links must exist.
- `scripts/engine.sh check`: the fork's commits over `UPSTREAM-BASE.txt` are
  exactly `engine-patches/*.patch`, in order.
- `test_target_<port>`: each CPU mode a profile offers is handled by its
  `port-hooks.sh`, and the profile's id is its port's.
- `scripts/dx.sh check` runs the first two, confirms every port has its
  required files, and checks the glibc ceiling of whatever is staged.
