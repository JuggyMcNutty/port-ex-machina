# DeusEx.exe launcher — RE working log

Living document. Updated at the end of every phase.

**Goal:** document `System/DeusEx.exe` well enough to reimplement the Deus Ex launcher
on non-Windows platforms, and to modernize it.

---

## 1. What this binary actually is

`System/DeusEx.exe` is **not the game**. It is the Unreal Engine 1 **`Launch` module** —
a thin bootstrap shell. The engine lives in `Core.dll`, `Engine.dll`, `DeusEx.dll`.

| Property | Value |
|---|---|
| Size | 253,952 bytes |
| Imagebase | `0x10900000` |
| `GPackage` | `"Launch"` (string at `0x1092C534`) |
| Functions | 762 (367 named — mostly import thunks — / 371 unnamed) |
| Strings | 697 |
| MD5 | `795137104d97da1bf4282fd6979bb38d` |
| Build | Nov 2021 GOG repack of the 1112f-era binary |

Everything we mean by "the launcher" is in here: first-run detection, the video/audio
config wizard, safe mode, crash recovery, splash screen, single-instance handling,
CD check, and engine bootstrap.

**Scope decision:** document `DeusEx.exe` only. `Window.dll` (widgets + dialog
templates) and `Core.dll` (ini/file I/O) are treated as a *documented interface* read
from the SDK headers, not reversed. A port replaces `Window.dll` wholesale.

## 2. The two cheat codes

Reverse engineering here is much cheaper than normal, for two reasons.

**(a) The SDK ships this build's own source headers.**
`ReleaseSDK1112f/Headers/DxHeaders.zip` — 327 headers, including:

- `Launch/Src/LaunchPrivate.h`, `Launch/Src/Res/LaunchRes.h`
- `Window/Inc/Window.h` (145 KB — every widget class)
- `Window/Src/Res/WindowRes.h` (every dialog + control ID)
- all of `Core/Inc/` (`Core.h`, `UnTemplate.h`, `FConfigCacheIni.h`, …)

The binary's own assert strings name these exact paths (`..\..\Core\Inc\UnTemplate.h`,
`..\..\Window\Inc\Window.h`), so they are the real headers for this build. **Struct
layouts can be read, not guessed.**

Extracted for this session to: `$SCRATCH/hdr/` (re-extract with `unzip DxHeaders.zip`).

**(b) `System/Startup.int` is the launcher's own string table.** It names every wizard
page and every control on it. Cross-check all UI findings against it.

## 3. Confirmed anchors

Established by decompilation. These are facts, not guesses.

| Address | Meaning |
|---|---|
| `0x10908A30` | `WinMain` (real body; `0x10901366` is the thunk from CRT `start`) |
| `0x1090A050` | `InitEngine` — the launcher/wizard brain, 974 decompiled lines |
| `0x10914630` | `MainLoop(Engine)` |
| `0x10901190` / `0x109011B3` | `InitSplash` / `ExitSplash` thunks |
| `0x10901320` | `FConfigCacheIni` factory passed to `appInit` |
| `0x1092E7B0` | `GExec` local exec handler object (vtables `off_109270A0`, `off_1092708C`) |
| `off_10926D70` | `WConfigPageRenderer` vtable — page size 464, dialog ID 2017 |
| `off_10926E7C` | `WConfigPageSafeMode` vtable — page size 564, dialog ID 2020 |
| `off_10926F88` | Launch's `WWizardDialog` subclass vtable |
| `off_10926C88` | Launch's `WLog` subclass vtable (`GameLog`) |
| `sub_109010C3` / `sub_1090128F` / `sub_1090121C` / `sub_109013ED` | SafeMode page button delegates (Run / Video / SafeMode / Web) |
| `sub_10901375` | Renderer page list/radio delegate |

## 4. Verified struct sizes

Transcribed from `Window/Inc/Window.h` into `docs/types/launch.h`, then **cross-checked
against real `GMalloc` allocation sizes and field offsets in the binary**:

| Type | Size | Confirmed by |
|---|---|---|
| `FName` | 4 | `NAME_INDEX Index` |
| `FArray` / `FString` | 12 | stack locals at `ebp-0x98/-0x94/-0x90` |
| `FDelegate` | 12 | virtual → vtable + target + member-ptr; `WButton` = 48+6×12 |
| `WWindow` | 44 | anchors everything below |
| `WControl` | 48 | `WWindow` + `WNDPROC` |
| `WLabel` | 48 | renderer page slot 400..448 |
| `WListBox` | 108 | renderer page slot 52..160 |
| `WButton` | 120 | renderer page slots 160, 280 |
| `WCoolButton` | 128 | safemode page slots 52..564 |
| `WDialog` | 44 | `WWizardDialog` total 620 |
| `WWizardPage` | 48 | `WDialog` + `Owner` |
| `WWizardDialog` | 620 | 44 + 128×4 + 48 + 12 + 4; matches `ebp` span `0x26C` |

Three independent totals close exactly:

- `WConfigPageRenderer` = `48+4+108+120+120+48+4+12` = **464** == observed `GMalloc(464)`
- `WConfigPageSafeMode` = `48+4+128×4` = **564** == observed `GMalloc(564)`
- `WWizardDialog` = **620** == observed stack span

> **Correction (Phase 1).** An earlier reading put `WWizardPage` at 52. It is **48**.
> The slot at +48 is not part of `WWizardPage` — it is each config page's *own* typed
> `Owner` pointer, declared by the derived class in addition to the inherited
> `WWizardPage::Owner`. Same pattern as UT's `Launch.cpp`. This matters: at 52 the
> derived member offsets are all off by one slot and the totals do not close.

Any future struct must pass this same size gate before being applied.

## 5. Gotchas

**MSVC reverses overload groups in the vtable.** In `FConfigCache`
(`Core/Inc/Core.h:195`), `GetString(FString&)` sits at **+12**, *before*
`GetString(TCHAR*, INT)` at **+16** — the opposite of declaration order. Resolve every
`GConfig` call site by **argument arity and types**, never by index arithmetic alone.
Verified against 4 call sites. This is the one place where blind index math silently
produces wrong documentation.

**IDA runs under Proton.** Windows IDA 9.4 via umu, in-IDA HTTP server on
`127.0.0.1:13337` + Linux-side proxy. `idalib` headless mode is impossible here.
If MCP tools go dark mid-session, that bridge broke — not the analysis.

**The IDB is unpacked while open.** IDA keeps `.id0/.id1/.id2/.nam/.til` as working
files; the `.i64` is only written on save. Backup of all components at
`.idb-backup/` (taken before annotation began).

## 6. Progress

- [x] **Phase 0** — workspace, IDB backup (`.idb-backup/`)
- [x] **Phase 1** — types written to `docs/types/launch.h`, declared in IDA, **all size-gated**
- [x] **Phase 2** — 70+ names applied; all six wizard pages found and the `GetNext` graph closed
- [x] **Phase 3** — contract extracted (flags, ini keys, files, registry, IPC, console commands)
- [x] **Phase 4** — spec written to `docs/`
- [x] **Phase 5** — cross-source verification passed **and live run completed** (see §9)

### Output

| File | Contents |
|---|---|
| `docs/launch-flow.md` | startup→shutdown sequence, all exit paths |
| `docs/wizard.md` | page graph, `FirstRun` gates, control inventory, the shipped bug |
| `docs/cli-flags.md` | every flag, with which of the three parsers reads it |
| `docs/ini-keys.md` | every read/write, files, the one registry key |
| `docs/porting-notes.md` | load-bearing vs. incidental; the seven platform seams |
| `docs/types/launch.h` | transcribed struct definitions + size ledger |
| `docs/live-verification.md` | the live Proton run: what it confirmed, corrected, added |

## 7. Key findings

**The wizard graph** (was the main unknown; now closed):

```
SafeMode(2020) ─Run→ launch │ ─Video→ Renderer │ ─SafeMode→ SafeOptions │ ─Web→ URL
Renderer(2017) → (D3D ? Driver(2022) → Detail(2018) : Detail(2018))
Detail(2018) → FirstTime(2019) → EndDialog(1) = launch
SafeOptions(2021) → ShellExecute(self, flags); EndDialog(0)
```

**Safe mode re-executes the binary.** `WConfigPageSafeOptions__GetNext` (`0x10911C00`)
does not apply settings in-process — it builds a flag string, optionally deletes
`<Package>.ini`, `ShellExecute`s `GModuleFilename` with those flags, and ends the
current process. A port must reproduce this or consciously replace it.

**Crash detection is one file.** `Running.ini` is created after the wizard and deleted
on clean exit. If it survives, the next launch shows RecoveryMode. That is the whole
mechanism.

**⚠ Shipped bug — three safe-mode checkboxes are dead.** Verified in raw disassembly:
of eight `BM_GETCHECK` sites, five read the *same* control (`+0xB0`, `IDC_No3DSound`).
Checkboxes #3 `No3DVideo` (`+0x128`), #4 `Window` (`+0x1A0`) and #5 `Res` (`+0x218`) are
constructed but never read. So ticking "Disable 3D sound hardware" silently also applies
`-nohard -noddraw -defaultres`, and three checkboxes do nothing. **Do not replicate.**

**Dead legacy paths:** `MPLAYER` / `HEAT` console commands (services dead since ~2001,
`GotoHEAT.exe` not shipped), the single registry read
(`HKLM\software\mpath\mplayer\main`, only reachable from `MPLAYER`), and `.ICD`→`.EXE`
rewriting in `InitPathnames` (`0x10901C40`) — SafeDisc copy-protection handling the GOG
build does not need.

**`appStrfind` flags match anywhere.** `readini`, `Server`, `NewWindow`, `changevideo`,
`TestRenDev` are raw substring matches with no leading `-` required — they will trigger
from inside a map name or URL. A naive `argv` parser in a port behaves differently.

**`DescFlags` / `Description` are runtime values**, written by device detection and read
back by the wizard. They appear in no shipped ini; that is expected, not a gap.

## 8. Live verification (2026-09-21)

Ran the real binary under Proton. Full report in `docs/live-verification.md`.
Install restored afterwards — `DeusEx.ini` is byte-identical to `.ini-backup/`.

**Confirmed:** `FirstRun` clamp value 1100 *is* the engine version (`Init: Version: 1100`);
binds exactly Core/Engine/Window.dll; `[Engine.Engine] CdPath` read at startup;
`FirstRun=0 < 400` opens the first-time wizard; the Renderer page runs device detection;
`DescFlags`/`Description` really are runtime-written (detection added
`Description=ATI Radeon HD 5600 Series` + `DescFlags=1` to an ini that shipped with
neither); and `Running.ini` is created **after** the wizard — it stayed absent through
2m47s of open wizard.

The splash condition got confirmed from both sides by accident: the `-testrendev` run
never touched the splash, the `-firstrun` run did.

**Corrections made to the spec:**

1. **A missing splash bitmap is fatal.** The fallback to `..\Help\Logo.bmp` is applied
   without checking it exists (`0x109090A4`); if neither bitmap is there, `InitSplash`
   asserts and the process dies before the wizard. A port should not copy this.
2. `Detected.ini` is **also** written during first-run renderer detection, not only by an
   explicit `-testrendev=` — the Renderer page `ShellExecute`s itself per candidate
   driver (`0x1090DB18`) so a bad driver kills only a child.
3. `[WindowPositions] GameLog=(...)` is written by the log window — was undocumented.

**Environment gotcha (not a game defect):** under plain Proton, `WLog::OpenWindow` fails
creating its child `EDIT` (`CreateWindowEx` → NULL, `GetLastError()==0`). Running inside
`explorer /desktop=…` avoids it. Matters because the log window is created
unconditionally on every launch, so when it fails the game cannot start at all.

**Note on this install:** both splash bitmaps had been deleted. `Help/Logo.bmp` was
restored from `ReleaseSDK1112f/Help/Logo.bmp`. Without it the game does not start.

## 9. Remaining / open

- **Not verified live:** the SafeMode/RecoveryMode entry paths (`-safe`, stale
  `Running.ini`), the safe-mode re-exec, and the `GetNext` chain past the Renderer page.
- **The three-dead-checkboxes bug is still static-analysis only.** Well evidenced (eight
  `BM_GETCHECK` sites, five reading `+0xB0`; struct size confirmed by `GMalloc(1012)`),
  but not yet observed live. Repro is in `docs/porting-notes.md`.
- `MainLoop` (`0x10914630`) named but not analysed in depth — Win32 message pump plus
  `Engine->Tick`; a port replaces it wholesale.
- `Window.dll` dialog *templates* (geometry, styles, tab order) deliberately not
  extracted — outside agreed scope; needed only for a pixel-faithful recreation.

## 10. Test harness

Reusable for further live checks (`/tmp/dxtest2.sh`):

```sh
cd <install>/System
export WINEPREFIX=~/Games/umu/umu-default
export PROTONPATH="~/.local/share/Steam/compatibilitytools.d/Proton-CachyOS Latest"
export GAMEID=umu-default PROTON_VERB=run
umu-run explorer /desktop=dxtest,1024x768 \
    'X:\Documents\projects\deusex-launcher\System\DeusEx.exe' "$@"
```

Run it from the host (this session is inside a distrobox container — use
`distrobox-host-exec`). The virtual desktop is required; see the environment gotcha above.
Evidence to watch: `System/DeusEx.log`, `Running.ini`/`Detected.ini` presence, and
`diff .ini-backup/DeusEx.ini System/DeusEx.ini`. Screenshots via `spectacle -b -n -f -o`
work only while no modal Wine window holds an input grab.
