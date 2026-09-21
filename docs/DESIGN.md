# Design notes

Companion to the reverse-engineering spec in [`re/`](re/). That folder says what
`DeusEx.exe` *does*; this file records what this port does differently, and why.

## Target, as measured

Probed over SSH on 2026-09-21, not assumed. `tools/probe-sdl.c` reproduces it.

| | |
|---|---|
| Device | TrimUI Smart Pro (`hwserial TG5040`), spruceOS `PLATFORM=SmartPro` |
| SoC | Allwinner A133 `sun50iw10p1`, 4× Cortex-A53 |
| OS | TinaLinux "Neptune", kernel 4.9.191, **glibc 2.33**, busybox 1.36.1 |
| SDL | vendor build 2.30.8 in `/usr/trimui/lib` |
| Video driver | **`mali`** (`SDL_malivideo.c`, an EGL/fbdev driver not in upstream SDL) |
| Surface | 1280×720 @60Hz, `SDL_PIXELFORMAT_RGBX8888`, fullscreen |
| Renderer | **`opengles2`**, accelerated + vsync, max texture 8192² |
| Pad | enumerates as `"Xbox 360 Controller"`, GUID `0300a3845e0400008e02000014010000`, **recognised by SDL_GameController with a built-in mapping** — no custom mapping needed |
| Fonts | `/usr/trimui/res/regular.ttf`, `full.ttf`; `/mnt/SDCARD/spruce/Font Files/Noto.ttf` |
| Storage | SD is **exFAT** — case-insensitive, no meaningful permission bits |

## Why the toolchain choice is load-bearing

The device runs glibc 2.33. glibc 2.34 folded `libpthread`/`libdl` into `libc` and
re-versioned the startup symbols, so a toolchain built against ≥ 2.34 emits
`__libc_start_main@GLIBC_2.34` from `crt1.o` — and the binary fails to load, whatever
our own code calls.

Measured:

| Toolchain | glibc | Result |
|---|---|---|
| Arch `aarch64-linux-gnu-gcc` | 2.44 | rejected |
| ARM GNU 13.3.rel1 | 2.38 | rejected — emitted `GLIBC_2.34` in a one-line test |
| **Bootlin `aarch64--glibc--stable-2020.08-1`** (gcc 9.3) | **2.31** | **used** — emits only `GLIBC_2.17`, verified running on the device |

`scripts/check-abi.sh` runs on every cross build and fails on any reference above 2.33.

## Why we link the device's SDL2 rather than building our own

The vendor SDL2 carries a `mali` video driver that upstream SDL2 does not have, and the
device's PowerVR stack ships only `libpvrNULL_WSEGL.so` — so an upstream KMSDRM build
would have no window system to attach to. `scripts/fetch-sysroot.sh` pulls the device's
`libSDL2`, `libSDL2_ttf`, `libSDL2_image`, `libSDL2_mixer` and `libfreetype` into
`sysroot/trimui/lib`; headers come from the matching SDL 2.30.8 release tarball.

## Deliberate divergences from `DeusEx.exe`

Each is argued for in the RE docs; this is the ledger.

| # | Original | Here | Why |
|---|---|---|---|
| 1 | Three safe-mode checkboxes are dead — five of eight `BM_GETCHECK` sites read the same control `+0xB0` | All eight wired independently | Shipped bug. Ticking "Disable 3D sound hardware" silently also applies `-nohard -noddraw -defaultres`, and three boxes do nothing. [`re/wizard.md`](re/wizard.md) §"Shipped bug" |
| 2 | Missing splash bitmap → assert → process dies before the wizard | Non-fatal; logged and skipped | The fallback to `..\Help\Logo.bmp` is applied without an existence check (`0x109090A4`). [`re/live-verification.md`](re/live-verification.md) |
| 3 | `MPLAYER` / `HEAT` console commands, one `HKLM\software\mpath` read | Dropped | Services dead since ~2001; `GotoHEAT.exe` is not shipped. [`re/porting-notes.md`](re/porting-notes.md) |
| 4 | `.ICD`→`.EXE` rewrite in `InitPathnames` | Dropped | SafeDisc artifact; the GOG build is not wrapped |
| 5 | `-make` rejected with a fatal error | Dropped | Points at `ucc`, shipped separately |
| 6 | Renderer page runs Win32 3D device detection, re-execing itself per candidate | Candidate list read from `renderers.ini` | There is no Win32 and no runtime yet to detect against. `GameRenderDevice` is still written exactly as before |
| 7 | `CreateMutex` + `FindWindowEx`/`WM_COPYDATA` handoff | `flock` pidfile + abstract unix socket | Same protocol (one string, the command-line tail), different transport. The four `appStrfind` bypass tokens still skip it |
| 8 | CD check loops on `<CdPath>Textures\Palettes.utx` with a modal box | Install-validation screen | Generalises to "did the user supply the game files?", which is the actual first-run failure here |

Everything else is reproduced as specified — in particular the `FirstRun` gates
(220/400/1100), the `Running.ini` sentinel lifecycle, the three non-equivalent
command-line parsers, and safe mode's **re-exec rather than in-process apply**.

## What this port does not do yet

Nothing on the device can execute the x86 Windows `Core.dll`/`Engine.dll`/`DeusEx.dll`
— there is no box64, box86, wine or qemu, and no native UE1 engine. So the launcher's
final step execs a **configured** command:

```ini
[Launcher]
GameDir=/mnt/SDCARD/Roms/PORTS/DeusEx
GameCommand=./run-game.sh
```

`run-game.sh` ships as a stub that logs its argv. That is deliberate: it makes the
safe-mode re-exec and the crash-sentinel lifecycle observable end to end today, and
swapping in a real runtime later is a config change rather than a rewrite.
