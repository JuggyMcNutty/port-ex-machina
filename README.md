# deusex-launcher — native port

A reimplementation of the Deus Ex launcher (`System/DeusEx.exe`, the Unreal Engine 1
`Launch` module) for non-Windows platforms. First target: **TrimUI Smart Pro**
(aarch64, spruceOS), SDL2.

The launcher is not the game. It is the ~250 KB bootstrap shell that decides whether to
show a configuration wizard, writes the ini keys the engine reads, manages the
crash-detection sentinel, and then starts the engine. This port reproduces that contract
natively; it does not reimplement the engine.

You supply your own Deus Ex game files. None are included.

## Layout

```
src/core/      the launcher contract -- C11, no SDL, no platform assumptions
src/ui/        SDL2 frontend, gamepad-driven
src/platform/  POSIX process and filesystem glue
docs/re/       the reverse-engineering spec this is built from
docs/DESIGN.md what this port does differently, and why
tools/         device probes
```

## Build

```sh
scripts/fetch-toolchain.sh     # Bootlin aarch64 gcc 9.3 / glibc 2.31
scripts/fetch-sysroot.sh       # device SDL2 + freetype, SDL 2.30.8 headers

# device
cmake -B build-trimui -DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-trimui.cmake
cmake --build build-trimui

# host, for tests and UI iteration
cmake -B build-host && cmake --build build-host && ctest --test-dir build-host
```

`scripts/deploy.sh --run` pushes to the device and streams stdout.

The toolchain's glibc must not exceed the device's 2.33 — see
[`docs/DESIGN.md`](docs/DESIGN.md). `scripts/check-abi.sh` enforces it on every build.
