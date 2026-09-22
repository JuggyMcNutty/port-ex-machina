# deusex-launcher — native port

A native launcher for Deus Ex on non-Windows platforms, plus the work to get the
game itself running behind it. First target: **TrimUI Smart Pro** (aarch64,
spruceOS).

You supply your own Deus Ex game files. None are included.

## Two halves

**The launcher** (this repo) reimplements `System/DeusEx.exe` — the Unreal
Engine 1 `Launch` module. That binary is not the game; it is the ~250 KB
bootstrap shell that decides whether to show a configuration wizard, writes the
ini keys the engine reads, manages the crash-detection sentinel, and then starts
the engine. The contract it implements is documented in [`docs/re/`](docs/re/),
reverse-engineered before any of this was written.

**The engine** is [Surreal Engine](https://github.com/dpjudas/SurrealEngine), an
open-source UE1 reimplementation that recognises this exact build. It is not
vendored here; [`engine-patches/`](engine-patches/) holds the fork patches and
the reason they must stay in a fork.

Status: the launcher is complete and verified on hardware. The engine runs the
game on the desktop and on the handheld — the GE8300's missing descriptor
indexing and texture formats are worked around in the fork (see
[`engine-patches/README.md`](engine-patches/README.md)); the intro renders
clean at ~28 FPS. Weapons, menus and a map change are still unverified on
device.

## Layout

```
src/core/         the launcher contract -- C11, no SDL, no platform assumptions
src/ui/           SDL2 frontend, gamepad-driven
src/app.c         the launcher's sequence, shared by both front ends
src/cli_main.c    dxl-cli: the same contract with no display
cmake/            cross toolchain files (C11 for the launcher, C++20 for the engine)
packaging/        the spruceOS app: config.json, launch.sh, run-game.sh, engine settings default
engine-patches/   fork patches for Surreal Engine, and why they stay in a fork
docs/re/          the reverse-engineering spec this is built from
docs/DESIGN.md    what this port does differently, and why
tools/            device probes (display, input, Vulkan capabilities)
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

# device
cmake -B build-trimui -DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-trimui.cmake
cmake --build build-trimui
scripts/deploy.sh --run
```

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
cmake --build build-trimui
cd -; scripts/deploy-engine.sh
```

## Running it

On the device the app installs to `/mnt/SDCARD/App/DeusEx`. Point
`launcher.ini` at your game files:

```ini
[Launcher]
GameDir=/mnt/SDCARD/Roms/PORTS/DeusEx
GameCommand=./run-game.sh
```

`GameCommand` is what the launcher execs once configuration is settled.
`run-game.sh` starts Surreal Engine and, deliberately, clears the crash
sentinel only on a clean exit — so an engine crash still produces the recovery
screen on the next launch.

## Diagnosing

`dxl-cli` is the launcher without a display. It is the tool for working on a
device that has no terminal:

```sh
./dxl-cli --dry-run              # the decision and the config; writes nothing
./dxl-cli --dry-run -safe        # ...for a given command line
./dxl-cli --safe-flags window    # the flags one safe-mode box produces
```

`--dry-run` touches nothing at all, not even the log. The engine writes
`SE-Log-LastRun.txt` under `$HOME/.config/SurrealEngine`, which `run-game.sh`
pins to the SD card.

`tools/probe-vulkan-caps.c` reports every requirement the engine's Vulkan device
filter checks, with a verdict — one run instead of a series of guesses.
`tools/probe-texture-formats.c` reports which texture formats that GPU can
sample, and whether it can linearly filter them — the engine hardcodes a
`VkFormat` per texture format and must not sample what the device does not
support.

## Engine settings on the handheld

`run-game.sh` pins `HOME` to the SD card and seeds
`$HOME/.config/SurrealEngine/Settings.json` from
`packaging/trimui-smartpro/engine-settings.json.default` if none exists. The
non-negotiable entry is `Antialias: Off`: the engine defaults to 4x MSAA, and
the PowerVR Rogue GE8300's resolve turns partially covered pixels into speckle.
The file survives reinstalls (`deploy.sh` installs it if missing), and the
engine rewrites it with the same value on exit.

## Keeping docs honest

`docs/re/` is a copy of the canonical spec that lives with the game install, so
this repo travels on its own. `scripts/sync-re-docs.sh --check` reports drift;
without `--check` it refreshes.
