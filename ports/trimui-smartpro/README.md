# Port: TrimUI Smart Pro (spruceOS)

The first target, and the one everything was measured on: an Allwinner A133
handheld (4× Cortex-A53, PowerVR GE8300) running spruceOS. Cross-built from a
PC; the launcher is the app the spruceOS menu starts.

**Status.** The launcher and the engine both run on the device. The intro plays
at ~30 FPS; Liberty Island runs at 2–3 FPS, CPU-bound on NPC AI and lightmap
rebuilds -- see [Performance](#performance).

| File | What it is |
|---|---|
| `port.cmake` | links the device's vendor SDL2 from the sysroot; glibc ceiling 2.33 |
| `toolchain-c.cmake`, `toolchain-cxx.cmake` | Bootlin GCC 9.3 (launcher, C11) and GCC 10.3 (engine, C++20) |
| `engine.cmake` | the engine for this device: SDL2 only, no X11/Wayland |
| `port.sh` | `deps` (both toolchains + the sysroot), `deploy` over SSH, the device's address |
| `fetch-sysroot.sh` | the link sysroot: libraries off the device, pinned headers |
| `target.c` | the device profile: fonts, spruceOS CPU modes, the pad note |
| `packaging/` | the spruceOS app (`config.json`, `launch.sh`, icon), `port-hooks.sh` (vendor library path, CPU mode), `launcher.ini`, `renderers.ini`, `engine-settings.json.default` |
| `tools/profile-map.sh` | frame-time profile of one map, run on the device |

## Build, deploy, run

```sh
scripts/dx.sh deps   trimui-smartpro       # toolchains + sysroot (the device must be awake)
scripts/engine.sh fetch                    # once: the engine fork
scripts/dx.sh build  trimui-smartpro       # launcher, then engine
scripts/dx.sh stage  trimui-smartpro       # build/trimui-smartpro/app
scripts/dx.sh deploy trimui-smartpro       # --no-engine to skip the 16 MB engine, --run to start it over SSH
```

The build is warning-free with GCC 9.3, which is stricter than a current host
compiler about `-Wshadow` and `-Wformat-truncation`: build this port as well as
`linux-x86_64` before calling a change clean.

On the device the app lives in `/mnt/SDCARD/App/DeusEx` and the game data in
`/mnt/SDCARD/Roms/PORTS/DeusEx` (all 38 `.u` packages). `launcher.ini`
belongs to the device's owner: `deploy` installs it only when missing (from
`launcher.ini.default`), and never touches `home/` or `run-game.log`.

```ini
[Launcher]
GameDir=/mnt/SDCARD/Roms/PORTS/DeusEx
GameCommand=./run-game.sh
CpuMode=Performance
```

**Access.** `spruce@192.168.1.211`, password `happygaming` -- the stock
firmware password, a default in `port.sh` on purpose; SSH is root. Override
with `DEVICE`, `DEVICE_USER`, `DEVICE_PASS`, `DEVICE_APPDIR`. The device drops
off the network when it sleeps (no ping, SSH times out): wake it.

## The device, as measured

Probed over SSH on 2026-09-21/22, not assumed. `tools/probes/probe-sdl.c` and
`dxl-cli --probe` reproduce it.

| | |
| --- | --- |
| Device | TrimUI Smart Pro (`hwserial TG5040`), spruceOS `PLATFORM=SmartPro` |
| SoC | Allwinner A133 `sun50iw10p1`, 4× Cortex-A53; 3 GB RAM (`free`) |
| CPU modes | The spruceOS menu runs in **power-save: cores 0 and 3 only, `conservative`, 408 MHz–1.49 GHz**. spruceOS `set_performance` = all four cores, `performance`, 1.8 GHz; `set_overclock` = 2.0 GHz (`spruce/scripts/platform/SmartPro.cfg`) |
| OS | TinaLinux "Neptune", kernel 4.9.191, **glibc 2.33**, busybox 1.36.1 |
| SDL | vendor build 2.30.8 in `/usr/trimui/lib` |
| Video driver | **`mali`** (`SDL_malivideo.c`, an EGL/fbdev driver not in upstream SDL) |
| Surface | 1280×720 @60Hz, `SDL_PIXELFORMAT_RGBX8888`, fullscreen |
| SDL renderer | **`opengles2`**, accelerated + vsync, max texture 8192² |
| GPU APIs | Vulkan **1.3.225** on the PowerVR Rogue GE8300; **OpenGL ES 3.2** (`build 1.19@6345021`) through EGL; no desktop OpenGL. Detection takes ~0.5 s |
| Pad | enumerates as `"Xbox 360 Controller"`, GUID `0300a3845e0400008e02000014010000`, **recognised by SDL_GameController with a built-in mapping** — no custom mapping needed |
| Pad controls | A B X Y, L1 R1, SELECT START MENU, d-pad (a hat), two sticks. **L2/R2 are digital**, though the mapping puts them on axes `a2`/`a5`. **No L3/R3**: the mapping lists `leftstick:b9`/`rightstick:b10`, but the sticks do not click. (Controls per the device's owner, 2026-09-22; mapping from `probe-sdl.c --pad`.) MENU belongs to spruceOS |
| Fonts | `/usr/trimui/res/regular.ttf`, `full.ttf`; `/mnt/SDCARD/spruce/Font Files/Noto.ttf` |
| Storage | SD is **exFAT** — case-insensitive, no meaningful permission bits |

## Why the toolchain choice is load-bearing

The device runs glibc 2.33. glibc 2.34 folded `libpthread`/`libdl` into `libc` and
re-versioned the startup symbols, so a toolchain built against ≥ 2.34 emits
`__libc_start_main@GLIBC_2.34` from `crt1.o` — and the binary fails to load, whatever
our own code calls.

Measured:

| Toolchain | gcc | glibc | Result |
| --- | --- | --- | --- |
| Arch `aarch64-linux-gnu-gcc` | 16 | 2.44 | rejected |
| ARM GNU 13.3.rel1 | 13.3 | 2.38 | rejected — emitted `GLIBC_2.34` in a one-line test |
| **Bootlin `stable-2020.08-1`** | 9.3 | **2.31** | **the launcher** — emits only `GLIBC_2.17`, verified on the device |
| **Bootlin `bleeding-edge-2021.05-1`** | 10.3 | **2.33** | **the engine** — C++20 capable, exact glibc match |

Two toolchains, because the launcher is C11 and Surreal Engine needs C++20, which
gcc 9.3 cannot build. The C++20 one links `libstdc++` and `libgcc` statically;
the device ships `libstdc++.so.6.0.28` (`GLIBCXX_3.4.28`) which gcc 10.3 would
actually be compatible with — static linking just removes the question, at ~1 MB.

Trying to keep one toolchain by pointing a modern compiler at the old sysroot
failed: the sysroot's `libc.so` linker script hardcodes absolute `/lib/...`
paths, which resolve to the host's libraries.

`scripts/check-abi.sh --max 2.33` runs on every build of both, and fails on any
reference above 2.33: the launcher's build runs it because `port.cmake` sets
`DXL_PORT_GLIBC_MAX`, and `scripts/engine.sh build` runs it on the engine.

## Why we link the device's SDL2 rather than building our own

The vendor SDL2 carries a `mali` video driver that upstream SDL2 does not have, and the
device's PowerVR stack ships only `libpvrNULL_WSEGL.so` — so an upstream KMSDRM build
would have no window system to attach to. [`fetch-sysroot.sh`](fetch-sysroot.sh)
(run by `scripts/dx.sh deps trimui-smartpro`) pulls the device's `libSDL2`,
`libSDL2_ttf`, `libSDL2_image`, `libSDL2_mixer` and `libfreetype` into
`deps/sysroots/trimui-smartpro/lib`; headers come from the matching SDL 2.30.8
and SDL_ttf 2.0.15 releases.

The engine needed more of the same treatment, and the device turned out to have
most of it already: the same script pulls `libEGL`, `libGLESv2`, `libopenal`,
`libasound`, `libz`, `libstdc++` and the Vulkan loader, so no audio stack had to
be cross-built. Their headers (Vulkan, EGL/GLES/KHR, OpenAL, ALSA) are pinned
to exact package versions in the Arch Linux archive -- the headers the engine
was first built against, which had been copied off a build machine by hand
until the script was made to reproduce the sysroot byte for byte. Vulkan is not
linked at all: SurrealGPU loads the loader through volk at run time. The
launcher's GPU probe does the same: it `dlopen`s the Vulkan loader and EGL
rather than linking them, so a device without either still starts the
launcher.

## CPU mode

The spruceOS menu leaves the handheld in power-save -- cores 0 and 3,
`conservative`, at most 1.49 GHz -- which costs the game about a third of its
frame rate. The Video tab offers spruceOS's own three modes (`target.c`), kept
in `launcher.ini` `CpuMode`: Smart, Performance (the default: all four cores at
1.8 GHz) and Overclock (2.0 GHz). `packaging/port-hooks.sh` applies the mode
with spruceOS's helpers -- the ones its Ports launcher uses -- just before the
engine starts, and restores the exact previous state (online cores, governor,
min/max) when it exits.

The helpers are sourced in subshells only: `helperFunctions.sh` exports its own
`LD_LIBRARY_PATH`, which hid `libSurrealVideo.so` from the engine the one time
it was sourced directly.

## Engine settings on this device

The launcher writes `home/.config/SurrealEngine/Settings.json` in the app
directory before every launch (`run-game.sh` pins `HOME` there). The
non-negotiable entry is `Antialias: Off`: the engine defaults to 4x MSAA, and
the GE8300's resolve turns partially covered pixels into speckle. The launcher
locks it off on any PowerVR, and this port's `engine-settings.json.default`
says `Off` too (VSync is off as well: the game runs below the panel's 60 Hz,
and vsync would hold it to 30 or 20).

## Diagnosing

```sh
./dxl-cli --dry-run              # decision, game config, Settings.json, renderers; writes nothing
./dxl-cli --dry-run --probe      # ...with the renderer list checked against this GPU
./dxl-cli --probe                # just the GPU: Vulkan device, OpenGL ES version
```

(on the device, with `LD_LIBRARY_PATH=/usr/trimui/lib:/usr/lib:/lib`). The
launcher logs to `<GameDir>/System/DeusExLauncher.log`, `run-game.sh` to
`run-game.log` in the app directory (engine output, exit status, CPU mode), and
the engine writes `home/.config/SurrealEngine/SE-Log-LastRun.txt` there too.
The System tab shows the engine and script logs on screen.

## Verified on the device

| Check | Result |
| --- | --- |
| glibc ABI of both launcher binaries | `GLIBC_2.17` only; the engine stays at ≤ 2.33 |
| GPU probe (`dxl-cli --probe`) | Vulkan GE8300 1.3.225, OpenGL ES 3.2, no desktop GL; 0.5 s |
| `dxl-cli --dry-run` on the broken install | found the 212-byte stub, reported the rebuild; wrote nothing (checksums unchanged) |
| Launch after the repair | the game started (it had been failing with `Could not find package Core`); owner-verified |
| CPU mode | power-save → performance (0-3, 1.8 GHz) while running → power-save restored after |
| Home screen on the panel | renders with the device font; detected `X360 Controller`; recognised the retired layout |
| Pad in game | moving, looking and firing work (owner, first build); START was swallowed after skipping the intro — fixed since, **not yet re-verified** |
| The ports framework (2026-09-22) | `dx.sh deploy` put exactly the staged files on the device (checksums); `dxl-cli --dry-run --probe` ran there; the shared `run-game.sh` with this port's hooks started the out-of-tree engine build, the intro rendered at 31 FPS, and the CPU mode was applied and restored |

Verified with the earlier wizard build, on code paths unchanged since: install
validation naming each missing file; `Running.ini` created at commit and
removed by the game on clean exit; a simulated crash (`DXL_SIMULATE_CRASH=1`)
surviving to the next launch; the same sentinel with a live instance forwarding
instead; the `FirstRun=500` clamp rewriting the ini with all 25 sections intact
and no line losing its CR.

Not yet exercised on hardware: the tabs by hand beyond the owner's first
session; the Customize buttons screen; the retired-layout upgrade being written
out; `DXL_NO_HOME=1`; the messenger half of the single-instance handoff.

## Performance

Measured on the device with the frame-time instrumentation in
`engine-patches/optional/perf-instrumentation.patch` and
[`tools/profile-map.sh`](tools/profile-map.sh) (start a map directly with
`--url=<map>`; the hooks are never committed -- `scripts/engine.sh perf on|off`):

| Scene, CPU mode | FPS | Frame | Game tick | Render CPU | GPU wait |
| --- | --- | --- | --- | --- | --- |
| Intro, power-save | ~22 | 44 ms | 6.5 | 16 | 20 |
| Intro, performance | ~30 | 33 ms | 3 | 7 | 22 |
| Liberty Island start (a firefight), power-save, turning | ~2 | ~500 ms | ~330 | ~250 | ~45 |
| Liberty Island start, performance, turning | 3.0–3.3 | ~300–340 ms | 170–200 | 80–170 | ~45 |
| Liberty Island start, performance, facing the fight | ~2 | 485–535 ms | 190–245 | 210–295 | ~75 |

(Times in ms per frame, averaged over 60 frames. The last row's runs also had
the per-class or per-function hooks on, which add their own cost.)

On Liberty Island:

- **Game tick ≈ 40%**, almost all NPC AI: 29 Terrorists ~3 ms each, 10 UNATCO
  troops, thugs, bots. Surreal's UnrealScript VM costs microseconds per
  trivial operation on this CPU (14–20k VM calls per frame; every call copies
  its argument array and re-walks the function's parameters). The single worst
  native was `CycleActors` (see engine-patches, patch 0003); with it fixed,
  `ScriptedPawn.CheckEnemyPresence`/`Tick` and general VM overhead dominate.
- **Lightmap rebuilds ~100 ms/frame** during the firefight: muzzle flashes are
  dynamic lights, and Surreal re-lights every surface they touch on the CPU,
  ~13–19 rebuilds per frame.
- **Other render CPU ~100 ms** — not yet broken down (likely vertex animation of
  ~50 character meshes and visibility).
- **GPU ~75 ms**, and it does not overlap the CPU: `CommandBufferManager::SubmitCommands`
  waits on the frame's fence right after submitting, so a frame costs CPU + GPU,
  not the larger of the two.

The fixes chosen, their order and the target are in
[`agent.md`](../../agent.md#decided-not-started). The OpenGL ES backend would
not help here: the CPU is the bottleneck, and Vulkan is the better API on this
GPU.

## Device probes

The repository's `tools/probes/` holds small single-purpose programs for
answering questions about a device instead of assuming answers
([`docs/PORTING.md`](../../docs/PORTING.md#device-probes)). They are not part of
any build -- each is one compile against this port's sysroot, run over SSH:

```sh
TC=deps/toolchains/aarch64--glibc--stable-2020.08-1/bin/aarch64-linux-gcc
SYS=deps/sysroots/trimui-smartpro
$TC -O2 -mcpu=cortex-a53 -std=c11 -I $SYS/include -I $SYS/include/SDL2 \
    tools/probes/probe-vulkan-caps.c -o /tmp/probe \
    -L $SYS/lib -lvulkan -Wl,-rpath-link,$SYS/lib
scripts/check-abi.sh --max 2.33 /tmp/probe
# scp to the device, then: LD_LIBRARY_PATH=/usr/trimui/lib:/usr/lib:/lib ./probe
```

What each probe answers is in PORTING.md; pause the spruceOS menu before
`probe-sdl.c --pad`. Each exists because a guess about this hardware turned out
to be wrong at least once: `probe-vulkan-caps.c` found the missing descriptor
indexing, `probe-texture-formats.c` the BCn, RGB8 and RGBA32F gaps (engine
patch 0002), and `probe-sdl.c --pad` how SDL maps the built-in controls. Two
more tools serve this device:

- `dxl-shots` on a host build configured with `-DDXL_PROFILE=trimui-smartpro`
  renders every launcher screen as this device shows it -- its CPU mode row,
  its PowerVR anti-aliasing lock -- in a desktop font, since the device's are
  not on a PC.
- [`tools/profile-map.sh`](tools/profile-map.sh), with the engine built with
  `scripts/engine.sh perf on`, splits a map's frame time into input, tick,
  render CPU, GPU wait, lightmaps and texture uploads; tick by actor class;
  script functions by self time (Performance above).

## Gotchas on this device

- **Pause the spruceOS menu while running anything that draws over SSH**:
  `kill -STOP $(pidof MainUI)` and `kill -CONT` afterwards (use a `trap`). Two
  programs on one framebuffer fight, and pad presses would also drive the menu.
- **The engine ignores SIGTERM**; stop it with SIGKILL. That leaves
  `Running.ini` behind like any crash, and the launcher shows a crash banner
  until the next clean exit or "Clear crash marker".
- **Killing over SSH**: `ps | grep deusex` matches the SSH command itself; use
  `pidof`. busybox `killall` rejects `-x`. Always check afterwards:
  SSH-launched engines survive sloppy kills, and two engines fight over the
  display.
- **The screen can only be seen over SSH by dumping the framebuffer**:
  `cat /dev/fb0 > /tmp/fb.raw` (64 MB), gzip it before copying, decode the
  first 1280×720 as BGRA (`magick -size 1280x720 -depth 8 bgra:frame -alpha off`).
- **busybox has no `timeout` and no `nohup`.** Use `setsid` with all three fds
  redirected, or SSH will hang waiting on the pipes.
- **A missing `Save/` directory is fatal to the engine** (`directory iterator
  cannot open directory`). It is empty in a fresh install, so it does not
  survive a `tar` that lists only populated directories.
- **The vendor SDL2 is the only display path.** Its `mali` driver wires Vulkan
  surface creation to the PowerVR implementation, so `SDL_Vulkan_CreateSurface`
  does work, via `VK_KHR_display`. No X11, no Wayland, no desktop GL.

## What is genuinely absent

There is still no way to run the original x86 Windows `Core.dll`/`Engine.dll`/
`DeusEx.dll` on this device — no box64, box86, wine or qemu. box64's own notes
record Deus Ex under Wine as crashing before the menu on far stronger hardware,
so that route was not pursued.
