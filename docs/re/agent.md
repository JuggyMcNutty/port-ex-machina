# Deus Ex on aarch64 — working log and session handoff

**Goal.** Run Deus Ex on an aarch64 handheld. That splits into a launcher we
write and an engine we don't: `System/DeusEx.exe` is only the Unreal Engine 1
bootstrap shell, so reimplementing it natively is tractable, while the game
itself needs a UE1 engine.

**State.** The launcher is finished and verified on hardware. The engine (a
fork of Surreal Engine) runs the game on a desktop and cross-builds for the
device, where it stops at one GPU capability gap. One decision is open.

---

## Start here

```
deusex-launcher/                 the game install (your own files; unversioned)
├── agent.md                     this file
├── docs/                        the reverse-engineering spec -- CANONICAL
├── System/ Maps/ Textures/ ...  game data
├── port/                        git repo: the launcher we wrote  (branch: aarch64)
│   ├── docs/re/                 synced copy of ../docs -- see sync-re-docs.sh
│   ├── docs/DESIGN.md           what the port does differently, and why
│   ├── engine-patches/          fork patches for the engine + the no-upstream rule
│   └── README.md                build, run, diagnose
└── engine/SurrealEngine/        git clone, branch deusex-handheld, patches applied
```

Two separate git repos: `port/` (ours) and `engine/SurrealEngine/` (a fork of
someone else's). The game install itself is deliberately not versioned.

**Cold start:** `port/README.md` has the toolchain fetches and both builds.
Nothing in `port/` depends on state from a previous session; the toolchains and
sysroot are fetched by script.

**Device:** TrimUI Smart Pro at `spruce@192.168.1.211`, password `happygaming`
(the stock firmware password; it is a default in the scripts on purpose). The
app lives at `/mnt/SDCARD/App/DeusEx`, the game data at
`/mnt/SDCARD/Roms/PORTS/DeusEx` (740 MB, all 38 `.u` packages, already copied).

## Where each part stands

| Part | State |
|---|---|
| Reverse-engineering spec (`docs/`) | Complete. Five phases, cross-verified, one live run under Proton |
| Launcher (`port/`) | Complete. 7 host test suites green; full matrix verified on hardware |
| Engine on the host | Runs the game. Main menu renders and takes input; New Game reaches `00_Intro` |
| Engine on the device | Cross-builds and runs up to Vulkan device selection, then stops |

### The one open decision

`VulkanRenderDevice.cpp:34` requires `VK_EXT_descriptor_indexing` for bindless
textures. The PowerVR Rogue GE8300 supports descriptor indexing by **none** of
the three routes — the EXT extension, the Vulkan 1.2 core feature, or the older
per-extension struct — although it advertises API 1.3.225. Everything else the
device filter wants is present. `port/tools/probe-vulkan-caps.c` prints the
whole verdict in one run.

1. **Non-bindless texture path in the fork.** Per-batch descriptor sets instead
   of indexed arrays: `DescriptorSetManager`, `TextureManager`,
   `GetTextureIndexes`, and the shaders. The only route that ends with the game
   running well on this GPU.
2. **Software rendering.** The engine's OpenGL backend has no
   descriptor-indexing concept, so Mesa llvmpipe sidesteps the gap — but that
   backend wants desktop GL 3.2 while the device's SDL2 offers only GLES, so
   llvmpipe would need its own window system. Slower, plus a winsys problem.

Not pursued: box64/box86/wine. None is on the device, and box64's own notes
record Deus Ex under Wine crashing before the menu on much stronger hardware.

## Gotchas that cost time

- **Toolchain glibc is load-bearing.** glibc 2.34 re-versioned the startup
  symbols, so anything built against ≥ 2.34 emits `__libc_start_main@GLIBC_2.34`
  and will not load on this device's 2.33. Arch's cross gcc (2.44) and ARM GNU
  13.3 (2.38) both fail. We use Bootlin **2020.08-1** (GCC 9.3 / glibc 2.31) for
  the C11 launcher and **bleeding-edge 2021.05-1** (GCC 10.3 / glibc 2.33) for
  the C++20 engine. `port/scripts/check-abi.sh` enforces the ceiling on every
  cross build.
- **The vendor SDL2 is the only display path**, and its `mali` driver wires
  Vulkan surface creation to the PowerVR implementation — so
  `SDL_Vulkan_CreateSurface` does work, via `VK_KHR_display`. There is no X11,
  no Wayland, no desktop GL.
- **Never `pkill -f <pattern>`** in a command whose own text contains the
  pattern. It matches the shell running it. This killed the session's own shell
  twice, locally and over SSH. Use `pkill -x <name>` / `killall -x`.
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
  message and had not happened. Prefer full rewrites, and verify.

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
only — never observed live. **Do not replicate;** `port/tests/test_safemode.c`
is the regression that keeps it fixed.

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

See `port/README.md` to build and `port/docs/DESIGN.md` for the twelve
deliberate divergences from the original and the hardware verification table.
In short: C11 core with no SDL and no globals, an SDL2 frontend that only
appears when there is something to ask, and `dxl-cli` for driving the whole
contract with no display.

Verified on hardware: install validation naming each missing file; `FirstRun=0`
→ first-time flow; a settled install exec'ing straight through with no display
created; the `Running.ini` lifecycle; a simulated crash producing
`main/recovery`; the same sentinel with a live instance producing `forward`
instead; and a `FirstRun 500→1100` clamp rewriting the ini with all 25 sections
intact and not one line losing its CR.

# Part 3 — the engine

A fork of [Surreal Engine](https://github.com/dpjudas/SurrealEngine) at
`engine/SurrealEngine`, branch `deusex-handheld`.

**It ships a `NO-AI Code Rule.md` asking that LLM-written changes stay in a fork
and never be PR'd.** Our patches were written with Claude, so they stay in the
fork permanently. `port/engine-patches/README.md` records this; honour it.
Building and using the engine is separately permitted by its own license.

Nine patches, all in `port/engine-patches/`: skipping the desktop launcher
window, reporting exceptions to stderr, streaming the engine log, gating the
X11/Wayland/EGL desktop backends, two missing includes in the SDL2 backend, SDL
linking nothing when found via pkg-config, `zipdir` as a host tool, system fonts
without a desktop, and a non-zero exit status on a caught exception.

Regenerate the patch file after any engine change:

```sh
cd engine/SurrealEngine && git diff > ../../port/engine-patches/0001-headless-and-embedded-support.patch
```
