# Deus Ex on aarch64 — working log and session handoff

**Goal.** Run Deus Ex on an aarch64 handheld (TrimUI Smart Pro, spruceOS),
driven entirely by the pad. That splits into a launcher we write and an engine
we don't: `System/DeusEx.exe` is only the Unreal Engine 1 bootstrap shell, so
reimplementing it natively is tractable, while the game itself needs a UE1
engine — a fork of Surreal Engine.

**State (2026-09-22).** The game runs on the handheld, launched from a
controller-first launcher. The intro plays at ~30 FPS; **Liberty Island runs at
2–3 FPS**, CPU-bound on NPC AI and lightmap rebuilds — measured, with candidate
fixes waiting on the owner's choice (see "Open decisions"). Everything is
committed in both repos; "Repository state" says where.

---

## Start here

```
deusex-launcher/                 the game install (your own files; unversioned)
├── agent.md                     this file
├── docs/                        the reverse-engineering spec -- CANONICAL
├── System/ Maps/ Textures/ ...  game data
├── port/                        git repo: the launcher we wrote  (branch: aarch64)
│   ├── docs/re/                 synced copy of ../docs and this file -- sync-re-docs.sh
│   ├── docs/DESIGN.md           what the port does differently, why, and measurements
│   ├── engine-patches/          fork patches for the engine + the no-upstream rule
│   ├── tools/                   device probes, profile-map.sh + perf-instrumentation.patch
│   └── README.md                build, run, diagnose
└── engine/SurrealEngine/        git clone, branch deusex-handheld
```

Two separate git repos: `port/` (ours) and `engine/SurrealEngine/` (a fork of
someone else's). The game install itself is deliberately not versioned.

**Read, in order:** this file → `port/docs/DESIGN.md` (design, the settings
files, controller support, performance numbers) → `port/engine-patches/README.md`
(every engine change and why) → `port/README.md` (build/run/diagnose).

**Cold start:** `port/README.md` has the toolchain fetches and both builds.
Nothing in `port/` depends on state from a previous session; the toolchains and
sysroot are fetched by script.

**Device:** TrimUI Smart Pro at `spruce@192.168.1.211`, password `happygaming`
(the stock firmware password; it is a default in the scripts on purpose). SSH
is root. The app lives at `/mnt/SDCARD/App/DeusEx`, the game data at
`/mnt/SDCARD/Roms/PORTS/DeusEx` (all 38 `.u` packages). The device drops off
the network when it sleeps (no ping, SSH times out): ask the owner to wake it.

## Where each part stands

| Part | State |
|---|---|
| Reverse-engineering spec (`docs/`) | Complete. Five phases, cross-verified, one live run under Proton |
| Launcher (`port/`) | Redesigned this session: tabbed pad-driven home (Play/Video/Controls/System), GPU detection, Settings.json, pad layouts + per-button remap, CPU mode. 11 host suites green; on the device the home screen, GPU probe, config repair and CPU mode are verified |
| Engine on the host | Runs the game (main menu, New Game into `00_Intro`) |
| Engine on the device | Runs. Non-bindless texture path, CPU texture decode, MSAA off (patch 0002). Pad support in game (patch 0003). Intro ~30 FPS, Liberty Island 2–3 FPS |

## Repository state

**`port/`** (branch `aarch64`): the launcher redesign is the commit
"Controller-first launcher: tabs, real renderer choice, pad layouts, CPU mode",
followed by a docs commit (DESIGN, README, engine-patches README, synced
`docs/re/`, and patches 0001/0003). Both builds are warning-free and `ctest`
passes (11/11).

**`engine/SurrealEngine/`** (branch `deusex-handheld`): `2328c37` (0001),
`e5c9935` (0002), `73c8c51` (0003: gamepad, `CycleActors`, the intro
`bIgnoreNextShowMenu` swallow). `git am` of the three patch files onto
`UPSTREAM-BASE.txt` reproduces this tree exactly. No `TEMPORARY DEBUG TOOL`
code is committed; the profiling hooks are `port/tools/perf-instrumentation.patch`
(`git apply` / `git apply -R`). Untracked `build-host/`, `.clangd` and
`compile_commands.json` are local build clutter.

**The device** runs builds of exactly these commits (checksums matched at
handoff for the launcher, `dxl-cli`, the engine, `run-game.sh`, `renderers.ini`
and both defaults). `Running.ini` is present in the game's `System/` — left by
profiling runs killed with SIGKILL; the launcher shows a crash banner until the
next clean exit or "Clear crash marker".

## Open decisions

1. **Performance** (owner to choose; nothing started). Liberty Island, from
   `port/docs/DESIGN.md#performance`: game tick ~40% (NPC AI through a slow
   script VM), lightmap rebuilds ~20% (muzzle flashes re-light surfaces on the
   CPU), other render CPU ~20% (not yet broken down), GPU ~15% and serialised
   with the CPU. Candidates, in order of payoff per effort:
   - AI level of detail: tick far/unseen pawns every 2–4 frames. Trade-off:
     distant AI reacts slightly later.
   - Lightmaps: don't re-light for short-lived flashes; spread rebuilds over
     the four cores. Trade-off: flashes light characters, not walls.
   - Lower internal resolution + let CPU and GPU overlap
     (`CommandBufferManager::SubmitCommands` waits on the fence right after
     submit). Trade-off: a softer image.
   - Speed up the VM call path (`Frame::Call` copies its argument array and
     re-walks the parameter list per call). No trade-off; largest effort.
   Next measurement: break down the ~100 ms of "other render CPU".
2. **OpenGL ES backend** (planned as "Phase 4"): recommended to drop or park —
   the CPU is the bottleneck and Vulkan is the better API on this GPU. The
   launcher already lists GLES (device has ES 3.2) as "not in this engine build";
   setting `EngineType=GLES` in `renderers.ini` is all it would need later.
3. **Verify on the device by hand** (owner): START opens the pause menu on the
   first press after skipping the intro; SELECT opens it too; B/Y/SELECT/START
   close menus; the Customize buttons screen; the retired-layout upgrade being
   written on Play/Quit; CPU mode chosen from the Video tab; stick speeds —
   look (`Speed=3.75`/`2.25`) and pointer speed are calibrated by reasoning, not
   by feel.
4. **Release polish** (owner's request, deferred): the home screen is
   deliberately verbose for development; a final build needs a declutter pass,
   and Surreal Engine's always-on Deus Ex stats overlay (FPS/actors/surfaces,
   `RenderCanvas.cpp` `DrawTimedemoStats`) hidden behind an option.
5. **Cleanups**: `core/strings.{c,h}` (Startup.int reading) is no longer used by
   the app, only by `test_strings`; `dxl_config_set_render_device` and the
   `DescFlags` accessors are only used by tests. Remove or keep deliberately.

## Gotchas that cost time

- **Toolchain glibc is load-bearing.** glibc 2.34 re-versioned the startup
  symbols, so anything built against ≥ 2.34 emits `__libc_start_main@GLIBC_2.34`
  and will not load on this device's 2.33. We use Bootlin **2020.08-1** (GCC 9.3
  / glibc 2.31) for the C11 launcher and **bleeding-edge 2021.05-1** (GCC 10.3 /
  glibc 2.33) for the C++20 engine. `port/scripts/check-abi.sh` enforces the
  ceiling. GCC 9.3 also warns (`-Wshadow`, `-Wformat-truncation`) where the host
  compiler is quiet: build both.
- **The vendor SDL2 is the only display path**, and its `mali` driver wires
  Vulkan surface creation to the PowerVR implementation — so
  `SDL_Vulkan_CreateSurface` does work, via `VK_KHR_display`. No X11, no
  Wayland, no desktop GL.
- **Surreal Engine does not read what the original wizard wrote.** The renderer
  is `Settings.json` `RenderDevice.Type` (`GameRenderDevice` in `DeusEx.ini` is
  overridden inside the engine); texture/skin detail, sound quality and the
  safe-mode flags are ignored. On the device the engine only *reads*
  `Settings.json` — it saves it solely from its desktop launcher window.
- **After the engine's first clean exit it reads `SE-DeusEx.ini` and
  `SE-User.ini`, not `DeusEx.ini`/`User.ini`**, with client settings under
  `[Engine.SurrealClient]`. Anything written for the engine must go to whichever
  file it will read (`port/src/core/config.c` does this).
- **A stub `DeusEx.ini` kills the engine** (`Could not find package Core`).
  Surreal falls back to `Default.ini` only when the file is absent. An earlier
  launcher build wrote a 212-byte one on the device; the launcher now rebuilds
  such a file from `Default.ini`.
- **The engine takes `--url=<map>` only.** `-u <map>` silently loads the intro,
  which is how a whole round of "Liberty Island" profiling actually measured the
  intro.
- **The spruceOS menu leaves the CPU in power-save** (2 cores, conservative,
  ≤ 1.49 GHz). Games get their mode from spruce's helpers; `run-game.sh` applies
  the launcher's `CpuMode` and restores the previous state. **Source
  `helperFunctions.sh` only in a subshell**: it exports its own
  `LD_LIBRARY_PATH`, which hid `libSurrealVideo.so` from the engine.
- **Pause the spruceOS menu when running anything that draws over SSH**:
  `kill -STOP $(pidof MainUI)` and `kill -CONT` afterwards (use a `trap`). Two
  programs on one framebuffer fight, and pad presses would also drive the menu.
- **The engine ignores SIGTERM**; stop it with SIGKILL. That leaves
  `Running.ini` behind like any crash.
- **Never `pkill -f <pattern>`** in a command whose own text contains the
  pattern — it matches the shell running it (this killed the session's shell
  twice). Likewise `ps | grep deusex` over SSH matches the SSH command itself;
  use `pidof`. Over SSH the reliable kill is
  `kill -9 $(ps | grep <name> | grep -v grep | awk '{print $1}')`; busybox
  `killall` rejects `-x`. Always check afterwards: SSH-launched engines survive
  sloppy kills, and two engines fight over the display.
- **The device screen can only be seen over SSH by dumping the framebuffer**:
  `cat /dev/fb0 > /tmp/fb.raw` (64 MB), gzip it before scp, decode the first
  1280×720 as BGRA (`magick -size 1280x720 -depth 8 bgra:frame -alpha off`).
  The engine has no screenshot facility; env-gated debug hooks (e.g. a
  `SURREAL_DEBUG_SHOTS` in `VulkanRenderDevice::Unlock` writing `ReadPixels`
  BMPs) are the standard technique, but every one carries a
  `TEMPORARY DEBUG TOOL` comment and is reverted before commit.
- **Deus Ex's UnrealScript source is embedded in `System/DeusEx.u`** (and the
  other `.u` files). Search it — a regex over the file — before guessing what
  the game's script does: that is how the `CycleActors` semantics, the
  `bIgnoreNextShowMenu` swallow and the key-menu command list were found.
- **`zipdir` is a build-time tool.** A cross build produces an aarch64 binary
  that cannot run on the build host; pass `-DZIPDIR_EXECUTABLE=` a host-built
  one.
- **The device's busybox has no `timeout` and no `nohup`.** Use `setsid` with
  all three fds redirected, or SSH will hang waiting on the pipes.
- **A missing `Save/` directory is fatal to the engine** (`directory iterator
  cannot open directory`). It is empty in a fresh install, so it does not
  survive a `tar` that lists only populated directories.
- **Editing docs with string replacement fails silently** when the pattern does
  not match. One README edit in this project was reported as done in a commit
  message and had not happened. Prefer full rewrites, or scripted replacements
  that assert the pattern was found, and verify.

---

# Part 1 — the reverse-engineering spec

`System/DeusEx.exe` is **not the game**. It is the UE1 `Launch` module — a thin
bootstrap shell. The engine lives in `Core.dll`, `Engine.dll`, `DeusEx.dll`.

| Property | Value |
|---|---|
| Size | 253,952 bytes |
| Imagebase | `0x10900000` |
| `GPackage` | `"Launch"` (string at `0x1092C534`) |
| Functions | 762 (367 named — mostly import thunks — / 371 unnamed) |
| MD5 | `795137104d97da1bf4282fd6979bb38d` |
| **SHA1** | **`2a933e26aa9cfb33b37f78afe21434caa031f14a`** |
| Build | Nov 2021 GOG repack of the 1112f-era binary |

That SHA1 matters twice: it identifies the build, and it is Surreal Engine's
`DEUS_EX_1112fm` database entry, so the engine recognises this install directly.

**Two things made this cheap.** `ReleaseSDK1112f/Headers/DxHeaders.zip` ships
this build's own source headers (327 of them, including `LaunchPrivate.h` and
`Window/Inc/Window.h`), so struct layouts were *read*, not guessed — the
binary's assert strings name those exact paths. And `System/Startup.int` is the
launcher's own string table, naming every wizard page and control.

### Confirmed anchors

| Address | Meaning |
|---|---|
| `0x10908A30` | `WinMain` (real body; `0x10901366` is the CRT thunk) |
| `0x1090A050` | `InitEngine` — the launcher/wizard brain, 974 decompiled lines |
| `0x10914630` | `MainLoop(Engine)` |
| `0x10901320` | `FConfigCacheIni` factory passed to `appInit` |
| `0x1092E7B0` | `GExec` local exec handler (vtables `off_109270A0`, `off_1092708C`) |
| `off_10926D70` | `WConfigPageRenderer` vtable — page size 464, dialog ID 2017 |
| `off_10926E7C` | `WConfigPageSafeMode` vtable — page size 564, dialog ID 2020 |
| `off_10926F88` | Launch's `WWizardDialog` subclass vtable |

### Verified struct sizes

Transcribed from `Window/Inc/Window.h` into `docs/types/launch.h`, then
cross-checked against real `GMalloc` sizes and field offsets:

`FName` 4 · `FString`/`FArray` 12 · `FDelegate` 12 · `WWindow` 44 ·
`WControl` 48 · `WLabel` 48 · `WListBox` 108 · `WButton` 120 ·
`WCoolButton` 128 · `WDialog` 44 · `WWizardPage` 48 · `WWizardDialog` 620

Three independent totals close exactly: `WConfigPageRenderer` = 464 == observed
`GMalloc(464)`; `WConfigPageSafeMode` = 564 == `GMalloc(564)`;
`WConfigPageSafeOptions` = 1012 == `GMalloc(1012)`.

> **Correction kept for the record.** An earlier reading put `WWizardPage` at
> 52. It is **48** — the slot at +48 is each config page's own typed `Owner`,
> declared by the derived class in addition to the inherited one. At 52 every
> derived offset is out by a slot and the totals do not close.

### RE gotchas

- **MSVC reverses overload groups in the vtable.** In `FConfigCache`,
  `GetString(FString&)` sits at **+12**, *before* `GetString(TCHAR*, INT)` at
  **+16**. Resolve every `GConfig` call by argument arity and types, never by
  index arithmetic. This is the one place blind index math silently produces
  wrong documentation.
- **IDA runs under Proton** (Windows IDA 9.4 via umu, in-IDA HTTP server on
  `127.0.0.1:13337` plus a Linux-side proxy). `idalib` headless mode is
  impossible here. If the MCP tools go dark mid-session that bridge broke, not
  the analysis.
- **The IDB is unpacked while open** — `.id0/.id1/.id2/.nam/.til` are working
  files and the `.i64` is only written on save. Backup at `.idb-backup/`.

### Output

| File | Contents |
|---|---|
| `docs/launch-flow.md` | startup→shutdown sequence, all exit paths |
| `docs/wizard.md` | page graph, `FirstRun` gates, control inventory, the shipped bug |
| `docs/cli-flags.md` | every flag, and which of the three parsers reads it |
| `docs/ini-keys.md` | every read/write, files, the one registry key |
| `docs/porting-notes.md` | load-bearing vs. incidental; the seven platform seams |
| `docs/types/launch.h` | struct definitions + size ledger |
| `docs/live-verification.md` | the live Proton run: confirmed, corrected, added |

### Key findings

**The wizard graph:**

```
SafeMode(2020) ─Run→ launch │ ─Video→ Renderer │ ─SafeMode→ SafeOptions │ ─Web→ URL
Renderer(2017) → (D3D ? Driver(2022) → Detail(2018) : Detail(2018))
Detail(2018) → FirstTime(2019) → EndDialog(1) = launch
SafeOptions(2021) → ShellExecute(self, flags); EndDialog(0)
```

**Safe mode re-executes the binary.** `WConfigPageSafeOptions__GetNext`
(`0x10911C00`) does not apply settings in-process — it builds a flag string,
optionally deletes `<Package>.ini`, `ShellExecute`s `GModuleFilename`, and ends
the current process. The relaunch *is* the mechanism.

**Crash detection is one file.** `Running.ini` is created after the wizard and
deleted on clean exit. If it survives, the next launch shows RecoveryMode.

**⚠ Shipped bug — three safe-mode checkboxes are dead.** Of eight `BM_GETCHECK`
sites, five read the *same* control (`+0xB0`, `IDC_No3DSound`). So ticking
"Disable 3D sound hardware" silently also applies `-nohard -noddraw
-defaultres`, and `No3DVideo`, `Window` and `Res` do nothing. Static analysis
only — never observed live. Moot for the port now: safe mode was dropped
because Surreal Engine honours none of its flags (`port/docs/DESIGN.md` #1).

**`appStrfind` flags match anywhere.** `readini`, `Server`, `NewWindow`,
`changevideo`, `TestRenDev` are raw substring matches needing no leading `-` —
they fire from inside a map name or URL. A naive `argv` parser behaves
differently, which is why the launcher keeps the command line as one string.

**Dead legacy paths:** `MPLAYER`/`HEAT` console commands, the single registry
read (`HKLM\software\mpath\mplayer\main`), and `.ICD`→`.EXE` rewriting in
`InitPathnames` (`0x10901C40`, a SafeDisc artifact).

**A missing splash bitmap is fatal** in the original: the fallback to
`..\Help\Logo.bmp` is applied without checking it exists (`0x109090A4`), so
`InitSplash` asserts and the process dies before the wizard. Observed live.

### Not verified against the original

- The SafeMode/RecoveryMode entry paths and the safe-mode re-exec, live.
- The three-dead-checkboxes bug, live. Well evidenced statically.
- `MainLoop` (`0x10914630`) in depth — a port replaces it wholesale.
- `Window.dll` dialog *templates* (geometry, styles, tab order) — out of scope;
  needed only for a pixel-faithful recreation.

---

# Part 2 — the launcher

Full design in `port/docs/DESIGN.md`; build, run and diagnose in
`port/README.md`. In short:

- **Core** (`src/core/`, C11, no SDL, no globals): the original's contract —
  `policy` (the entry decision tree, unchanged), `cmdline` (the three parsers),
  `sentinel` (`Running.ini`), `instance` (single-instance handoff), `install` —
  plus what the engine actually reads: `config` (DeusEx.ini / SE- files /
  User.ini, seeded and repaired from `Default.ini`/`DefUser.ini`),
  `engine_settings` + `json` (`Settings.json`), `renderers` (engine backend ×
  device API), `bindings` (pad presets, the action catalogue, retired presets).
- **Platform**: `gpu_probe` — Vulkan and EGL via `dlopen` in a forked child with
  a timeout, run before the display comes up.
- **UI** (`src/ui/`, SDL2): tabs Play/Video/Controls/System, driven by a row
  table (`screens_internal.h`); overlays for the renderer picker, confirmations,
  text (logs) and Customize buttons (`remap.c`). Every row has help text.
- **Flow** (`src/main.c`): home screen every launch; START plays;
  `DXL_NO_HOME=1` for unattended runs. Quit saves settings, creates no sentinel.
- **`dxl-cli`**: `--dry-run [--probe]`, `--probe`. Writes nothing.
- **`run-game.sh`**: CPU mode → engine → restore CPU → sentinel cleared only on
  a clean exit.

Verified on hardware this session: GPU probe results; the stub-ini repair
(the game started afterwards); CPU mode applied and restored; the home screen
on the panel (`port/docs/img/device-home.png`) with the retired pad layout
recognised. From the previous (wizard) build, on unchanged code paths: install
validation, the `Running.ini` lifecycle, a simulated crash, the `forward`
decision with a live instance, the `FirstRun` 500→1100 clamp keeping every CR.

# Part 3 — the engine

A fork of [Surreal Engine](https://github.com/dpjudas/SurrealEngine) at
`engine/SurrealEngine`, branch `deusex-handheld`.

**It ships a `NO-AI Code Rule.md` asking that LLM-written changes stay in a fork
and never be PR'd.** Our patches were written with Claude, so they stay in the
fork permanently. `port/engine-patches/README.md` records this; honour it.
Building and using the engine is separately permitted by its own license.

Patches, all in `port/engine-patches/` (details and rationale in its README):

- `0001-headless-and-embedded-support.patch` (`2328c37`) — nine changes:
  skipping the desktop launcher window, exceptions to stderr, streaming the
  engine log, gating the X11/Wayland/EGL backends, two missing includes in the
  SDL2 backend, SDL linking via pkg-config, `zipdir` as a host tool, system
  fonts without a desktop, a non-zero exit status on a caught exception.
- `0002-nonbindless-fallback-and-format-support.patch` (`e5c9935`) — the
  non-bindless fallback for GPUs without descriptor indexing, and texture format
  checks with CPU decoders (BC1/2/3, BC4, BC5, RGB8, RGBA32F).
- `0003-gamepad-and-deusex-fixes.patch` (`73c8c51`): gamepad as a polled device
  in SurrealWidgets, `GamepadInput` (UE1 Joy keys and axes in play, pointer and
  clicks in Deus Ex's menus), `LauncherSettings.Gamepad`, `CycleActors` made
  resumable (it was the costliest native and made NPCs consider only the first
  ~21 pawns), and the intro `bIgnoreNextShowMenu` swallow cleared on level load.

Regenerate a patch after committing:

```sh
cd engine/SurrealEngine
git format-patch -1 <commit> --stdout > \
    ../../port/engine-patches/000N-<name>.patch
```

Profiling (hooks never committed):

```sh
cd engine/SurrealEngine && git apply ../../port/tools/perf-instrumentation.patch
cmake --build build-trimui --target SurrealEngine && cd ../../port && scripts/deploy-engine.sh
# copy tools/profile-map.sh to the device; run e.g.
#   profile-map.sh 60 li perf 0        (Liberty Island, performance mode, still)
cd ../engine/SurrealEngine && git apply -R ../../port/tools/perf-instrumentation.patch
```
