# Port: Linux aarch64

aarch64 devices running an ordinary distro: Raspberry Pi OS, Debian, Ubuntu,
and handheld images with a normal glibc userland, Mesa and a desktop-style
SDL2. The same as `linux-x86_64` in every other respect: the generic device
profile and `ports/common/packaging`.

Two ways to build:

- **Natively, on the device.** The preset's toolchain file steps aside on an
  aarch64 host, so `scripts/dx.sh build linux-aarch64` there builds the
  launcher *and* the engine against the device's own libraries, exactly as
  `linux-x86_64` does on a PC.
- **Cross, from a PC** -- the launcher only, for now. Bootlin's
  stable-2020.08-1 toolchain (GCC 9.3, glibc 2.31, shared with the Smart Pro)
  links against SDL2 from Debian bookworm's arm64 packages; the glibc 2.31
  ceiling means the binary loads on any distro from 2020 on. Cross-building
  the engine needs the C++20 toolchain and a sysroot with the engine's
  libraries (SDL2, OpenAL, zlib, ...): the Smart Pro's `port.sh` and
  `fetch-sysroot.sh` show the shape, with Debian packages in place of the
  device's own.

**Status.** The launcher cross-builds warning-free, with `GLIBC_2.17` symbols
only. Not yet run on an aarch64 device.

| File | What it is |
|---|---|
| `port.cmake` | a cross build links SDL2 from `deps/sysroots/linux-aarch64` with a 2.31 ceiling; a native build uses the system's |
| `toolchain-c.cmake` | Bootlin GCC 9.3 / glibc 2.31; does nothing on an aarch64 host |
| `engine.cmake` | the engine, native builds only |
| `port.sh` | `deps` fetches the toolchain and builds the sysroot from Debian packages (`libsdl2`, `libsdl2-ttf`, and the Vulkan and EGL headers the GPU probe compiles against); `run` works natively |

## Build

```sh
# cross, from a PC
scripts/dx.sh deps  linux-aarch64
scripts/dx.sh build linux-aarch64       # the launcher
scripts/dx.sh stage linux-aarch64       # build/linux-aarch64/app: copy it to the device

# natively, on the device
scripts/dx.sh deps  linux-aarch64       # checks for SDL2
scripts/engine.sh fetch
scripts/dx.sh build linux-aarch64       # launcher and engine
scripts/dx.sh stage linux-aarch64 && scripts/dx.sh run linux-aarch64
```

A device whose GPU lacks descriptor indexing or some texture formats is what
engine patch 0002 exists for (the Smart Pro's PowerVR); a Mali or Adreno
under Mesa may not need it. The probes in `tools/probes/` answer that before
the first engine run -- see [`docs/PORTING.md`](../../docs/PORTING.md#device-probes).
