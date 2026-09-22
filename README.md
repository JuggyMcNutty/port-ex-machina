# deusex-launcher — native port

A native, controller-first launcher for Deus Ex on non-Windows handhelds, plus
the work to get the game itself running behind it. First target: **TrimUI Smart
Pro** (aarch64, spruceOS).

You supply your own Deus Ex game files. None are included.

## Two halves

**The launcher** (this repo) takes the place of `System/DeusEx.exe` — the Unreal
Engine 1 `Launch` module, the ~250 KB bootstrap shell that decides whether to ask
anything, writes the config the engine reads, manages the crash-detection
sentinel and starts the engine. It keeps that binary's contract, documented in
[`docs/re/`](docs/re/) and reverse-engineered before any of this was written,
but its screens are a tabbed home screen driven by the pad (Play, Video,
Controls, System) with settings the engine actually reads.
[`docs/DESIGN.md`](docs/DESIGN.md) records every divergence and why.

**The engine** is [Surreal Engine](https://github.com/dpjudas/SurrealEngine), an
open-source UE1 reimplementation that recognises this exact build. It is not
vendored here; [`engine-patches/`](engine-patches/) holds the fork patches and
the reason they must stay in a fork.

Status: the launcher and the engine both run on the handheld. The intro plays at
~30 FPS; Liberty Island runs at 2–3 FPS, CPU-bound on NPC AI and lightmap
rebuilds — the measurements and the candidate fixes are in
[`docs/DESIGN.md`](docs/DESIGN.md#performance).

## Layout

```
src/core/         the launcher contract and settings -- C11, no SDL
                    policy, cmdline, sentinel, instance, install: the original's contract
                    config: DeusEx.ini / SE-DeusEx.ini / User.ini, seeding from Default.ini
                    engine_settings, json: Surreal Engine's Settings.json
                    renderers: which renderer can run here; bindings: pad layouts
src/platform/     gpu_probe: Vulkan/EGL detection in a forked child
src/ui/           SDL2 frontend: ui.c widgets, screens.c session + rows, tab_*.c, remap.c
src/app.c         the launcher's sequence, shared by both front ends
src/cli_main.c    dxl-cli: the same contract with no display
cmake/            cross toolchain files (C11 for the launcher, C++20 for the engine)
packaging/        the spruceOS app: config.json, launch.sh, run-game.sh, defaults
engine-patches/   fork patches for Surreal Engine, and why they stay in a fork
docs/re/          the reverse-engineering spec (synced copy; see below)
docs/DESIGN.md    what this port does differently, and why; measurements
tools/            device probes, the screenshot tool, profiling
tests/            host unit tests -- no display needed
```

## Build

Two toolchains, because the launcher is C11 and the engine needs C++20. Both
must stay at or below the device's glibc 2.33; `scripts/check-abi.sh` enforces
that on every cross build and is wired into both as a post-build step.

```sh
scripts/fetch-toolchain.sh      # GCC 9.3  / glibc 2.31 -- the launcher
scripts/fetch-toolchain-cxx.sh  # GCC 10.3 / glibc 2.33 -- the engine
scripts/fetch-sysroot.sh        # device libraries + matching headers
```

`fetch-sysroot.sh` pulls SDL2, SDL2_ttf, freetype, EGL, GLES, OpenAL, ALSA,
zlib and the Vulkan loader off the device itself, so what we link against is
exactly what will be there at run time.

### The launcher

```sh
# host -- tests and UI iteration
cmake -B build-host && cmake --build build-host && ctest --test-dir build-host
build-host/dxl-shots /tmp/shots          # every tab and overlay as .bmp, 1280x720

# device
cmake -B build-trimui -DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-trimui.cmake
cmake --build build-trimui
scripts/deploy.sh                         # --run to start it over SSH too
```

The device's gcc 9.3 warns about things the host compiler does not
(`-Wshadow`, `-Wformat-truncation`); build both before calling a change clean.

### The engine

Cloned separately (`../engine/SurrealEngine`, branch `deusex-handheld`) with
`engine-patches/` applied. `zipdir` is a build-time tool, so a cross build needs
a host-built one:

```sh
cd ../engine/SurrealEngine
cmake -B build-trimui \
  -DCMAKE_TOOLCHAIN_FILE=../../port/cmake/aarch64-trimui-cxx.cmake \
  -DENABLE_SDL3=OFF -DENABLE_SDL2=ON -DENABLE_X11=OFF -DENABLE_WAYLAND=OFF \
  -DZIPDIR_EXECUTABLE=/path/to/host/zipdir
cmake --build build-trimui --target SurrealEngine
cd -; scripts/deploy-engine.sh
```

## Running it

On the device the app installs to `/mnt/SDCARD/App/DeusEx`. `launcher.ini`
belongs to the device's owner (`deploy.sh` installs it only when missing):

```ini
[Launcher]
GameDir=/mnt/SDCARD/Roms/PORTS/DeusEx
GameCommand=./run-game.sh
CpuMode=Performance
```

The launcher opens on its home screen every time; START plays. `GameCommand` is
what it execs afterwards. `run-game.sh` switches the CPU to `CpuMode` (Smart,
Performance or Overclock — spruceOS's modes; the menu otherwise leaves the
handheld in power-save), starts Surreal Engine, puts the CPU back when it exits,
and clears the crash sentinel only on a clean exit — so an engine crash still
produces the crash notice on the next launch.

`DXL_NO_HOME=1` skips the home screen for unattended runs over SSH: the
launcher then execs straight into the game unless the entry decision itself has
a question (first run, `-safe`, a surviving crash sentinel).

## Diagnosing

`dxl-cli` is the launcher without a display — the tool for a device with no
terminal:

```sh
./dxl-cli --dry-run              # decision, game config, Settings.json, renderers; writes nothing
./dxl-cli --dry-run --probe      # ...with the renderer list checked against this device's GPU
./dxl-cli --probe                # just the GPU: Vulkan device, OpenGL ES version
./dxl-cli --dry-run -safe        # ...for a given command line
```

`--dry-run` touches nothing at all, not even the log. The launcher logs to
`<GameDir>/System/DeusExLauncher.log`; `run-game.sh` logs to
`run-game.log` beside itself (engine output, exit status, CPU mode); the engine
writes `SE-Log-LastRun.txt` under `home/.config/SurrealEngine`. The System tab
shows the engine and script logs on screen.

To see the device's screen over SSH, dump the framebuffer (`cat /dev/fb0`, gzip
before copying) and decode the first 1280×720 as BGRA. Pause the spruceOS menu
(`kill -STOP $(pidof MainUI)`, then `-CONT`) while running anything that draws.

`tools/probe-vulkan-caps.c` reports every requirement the engine's Vulkan device
filter checks, with a verdict. `tools/probe-texture-formats.c` reports which
texture formats the GPU can sample and linearly filter.
`tools/profile-map.sh` with `tools/perf-instrumentation.patch` breaks a map's
frame time down (see [`docs/DESIGN.md`](docs/DESIGN.md#performance)).

## Engine settings on the handheld

Surreal Engine takes its renderer and render options from
`home/.config/SurrealEngine/Settings.json` (`run-game.sh` pins `HOME` to
`<AppDir>/home`), not from `DeusEx.ini`. The launcher writes it before every
launch from its Video and Controls tabs, starting from
`packaging/trimui-smartpro/engine-settings.json.default` for anything missing.
The non-negotiable entry is `Antialias: Off`: the engine defaults to 4x MSAA,
and the PowerVR Rogue GE8300's resolve turns partially covered pixels into
speckle. The launcher locks it off on PowerVR. The engine only reads this file
on the handheld — it saves it solely from its own desktop launcher window.

## Keeping docs honest

`docs/re/` is a copy of the canonical spec that lives with the game install
(`../docs/` and `../agent.md`), so this repo travels on its own.
`scripts/sync-re-docs.sh --check` reports drift; without `--check` it refreshes.
