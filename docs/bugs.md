# bugs.md

The bugs found and not yet fixed, each with its proof: the third sweep's (every doc of the four
repositories read for what is open, each place DXEngine or the launchers are not the original
that the owner has not chosen, and each check no run has made), and those found since. The
refuted are listed at the end so they are not reported again.

IDs are this list's own, `<section>.<n>`, an item's kept when those before it are fixed, each
section's numbers going on from the second sweep's, so no ID names two items; the first
sweep's (`E1.1`, `E10b.15`, `X2.L9`) are its own list's, named only in the commits that cite
them. A commit that fixes an item cites it, `(bugs.md VM.5)`, and takes it off this list.
DXEngine's [`NATIVES.md`](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/NATIVES.md)
cites each open difference by its ID, and says "by choice" of each the owner has agreed to.

| Section | Area | Open |
|---|---|---|
| [D](#d-for-the-owner) | differences made on purpose, for the owner to rule on or ruled to change | 1 |
| [VM](#vm-the-script-vm-and-the-object-system) | the script VM and the object system | 1 |
| [PKG](#pkg-saving-loading-travel-and-the-loading-screen) | saving, loading, travel and the loading screen | 0 |
| [PHYS](#phys-physics-traces-and-movers) | physics, traces and movers | 0 |
| [AI](#ai-the-npcs) | the NPCs | 0 |
| [DX](#dx-deusexs-natives) | DeusEx's natives | 0 |
| [EXT](#ext-the-window-tree-extension) | the window tree (Extension) | 0 |
| [NET](#net-the-network) | the network | 1 |
| [REN](#ren-the-renderer) | the renderer | 0 |
| [AUD](#aud-the-audio) | the audio | 0 |
| [ENG](#eng-the-engine-its-input-and-its-config) | the engine, its input and its config | 0 |
| [WIN](#win-the-game-window) | the game window | 0 |
| [MAIN](#main-the-recreation-deusex-launchers-main) | the recreation: deusex-launcher's main | 0 |
| [LAPP](#lapp-the-desktop-launchers-app-processes-and-posix-platform) | the desktop launcher's app, processes and POSIX platform | 0 |

## D: for the owner

Differences made on purpose, each the owner's to rule on. A ruling to keep one marks it "by
choice" where it is said and takes it off this list; one to change it stays here, the ruling
said, until the change is made and proven as any other fix. Nothing here changes before the
owner rules.

| ID | The original | DXEngine | Said in NATIVES.md | Recommended |
|---|---|---|---|---|
| D.38 | a net client runs at most its connection's speed / 64 frames a second (`UGameEngine::GetMaxTickRate`, [network.md](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/network.md#packets)): 40 at the game's default 2,600, so on a live server the world moves at 40 frames a second whatever the display, which reads as jitter on a 120 Hz one (bugs.md's NET.9, a playthrough on MTL and ANNA) | the same: `tools/dxcap.sh live` on an MTL server (46.142.84.124:17577) with MangoHud's frame log, at 2,600 every second of the game 40 frames, each 25 ms, three 120 Hz refreshes (1,279 frames, none held two or four), and at `netspeed 20000` 120 a second, one refresh each (2,436 of 2,438); the server's corrections none at either | not yet (multiplayer) | DXEngine's default internet speed 20,000 where the player has set none, the cap kept the original's: a server limits a client's rate itself (`MaxClientRate`), and the original's players set that speed by hand; or the original's, by choice |

## VM: the script VM and the object system

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| VM.5 | Strings are of bytes, Latin-1, where the original's are UTF-16: a typed character past Latin-1 goes nowhere, a wide string loads with `?` for each such character (a note in a save, every conversation comment of `DeusExConText.u`), and `Chr` keeps the low 8 bits | the UI; implemented, not as the original (small ones) | `TypeConsole` typing U+0100 and past; a note of such characters saved and loaded across both engines; `OpsConsole`'s `Chr` and `Asc` |

## PKG: saving, loading, travel and the loading screen

None open.

## PHYS: physics, traces and movers

None open.

## AI: the NPCs

None open.

## DX: DeusEx's natives

None open.

## EXT: the window tree (Extension)

None open.

## NET: the network

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| NET.8 | A joining client's player while it waits for its pawn: once the server's map is loaded, the original's viewport has a player in the Entry level, and its game engine ticks the Entry level a second time each frame meanwhile, that player's level not being the game's ([the game engine's frame](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#a-levels-tick)); DXEngine's viewport has no player until the pawn arrives (`Engine::LoadClientMap`), draws no world, and ticks the Entry level once | multiplayer ("until its pawn arrives, a joining DXEngine client draws no world") | `TravelJoinConsole` already shows the player's place: the original's `in entry.dx` at its fourth second, DXEngine's logging nothing between `DX.dx` and `DXMP_Cathedral`; the Entry level's time each tick of the wait, and a shot of it, in each |

## REN: the renderer

None open.

## AUD: the audio

None open.

## ENG: the engine, its input and its config

None open.

## WIN: the game window

None open.

## MAIN: the recreation (deusex-launcher's main)

None open.

## LAPP: the desktop launcher's app, processes and POSIX platform

None open.

## Refuted

| Area | Finding | Why refuted |
|---|---|---|
| PHYS | A walking player rests 4.8 over the floor at Liberty Island's start, where the original's rests 4.75, its landing lower, the first frame after the load shorter (`DXEngine/docs/NATIVES.md`, small) | Where it rests follows its landing, so the first frame's length after the load, which is the load's real time on the machine: run again, both engines rest it at 4.25 at the start and after the first walk, 12.2 after the second, 1.70 from the wall (`StandConsole`, two runs of each, every line alike but the moment's places) |
| ENG | A `SpawnNotify` is not asked about an actor spawned while a notification runs, DXEngine's guard against one spawning without end, the original's taken for unread (`DXEngine/DXEngine/Packages/Engine/Actors/UActor.cpp:128`) | The original's has the same guard: `SpawnActor` (`0x10394f60`) holds a flag (`GSpawnNotifying`, `0x10588b48`) over the notifications' walk, and an actor spawned meanwhile hears none (engine-dll.md, spawning) |
| VM | RadiusActors(None, 300) on the menu map finds one actor more than the original's, 99 against 98 (`DXEngine/DXEngine/VM/Iterator.cpp`, `DXEngine/tools/dxcap/Classes/OpsConsole.uc`) | DX.dx's player rides the menu's flyby, so a count from it is the moment's: 2 s in, the original's player stood at 2769, 308, -98 and DXEngine's at 2752, 299, -92, InterpolationPoint14 (a collision radius of 10) 327 units from the one and 307 from the other, outside 300 plus its radius and inside it, as the original's test has it (engine-dll.md, traces). OpsConsole counts round the level's first PlayerStart (DXEngine cbc56edd): both engines, twice each, give 96, every `DXOPS:` line alike. |
| REN | The laser fence's middle beam a fraction of a pixel thinner, missing the top row `D3DDrv` draws (`DXEngine/docs/NATIVES.md`, particles and lasers) | The multisampling's: with DXEngine's default 4x the beams' edge rows are partly lit, and with it off (`DXEngine/tools/dxcap/vulkan-settings.json`) `LaserConsole`'s three beams have `D3DDrv`'s rows, each within 11 levels of 256 of its red. Multisampling by default is D.4 |
| DX | ConBindEvents makes every ConListItem with the name None, and a save writes them all as exports of one class, name and outer (`DXEngine/DXEngine/Packages/Engine/Actors/UActor_Conversation.cpp:66`, `DXEngine/DXEngine/Packages/Engine/Actors/UActor_Conversation.cpp:68`) | These items never get into a save. Actor's ConListItems is declared `var transient Object ConListItems;` (gamefiles/System/Engine.u script). PropertyDataBlock::Save (DXEngine/DXEngine/Packages/Core/UObject.cpp:859) skips Transient properties. PackageWriter::WriteObjects (Package/PackageWriter.cpp:116-135) exports only Standalone objects and what they reference. No non-transient script var holds a ConListItem; the only other one is ConListItem.next, which is reached only from the list itself. So no ConListItem export named None is ever written, and nothing gets folded on load. |
| EXT | A text log drops only its oldest expired lines, so a shorter timeout waits behind them (`DXEngine/DXEngine/Packages/Extension/Windows/Text/UTextLogWindow.cpp:73`) | The original's tick (0x10047c30) first subtracts deltaTime from every line's time. It then counts the leading lines whose time is <= 0, stopping at the first that is still positive, and removes that leading run (TArray Remove(0, n)). DXEngine's leading-run erase matches it, as extension-dll.md's text logs say. |
| LPORT | dxl_game_close frees a pending overlapped read's buffer and OVERLAPPED before the read is cancelled (`deusex-launcher/windows-x86_64/src/platform/win32/process.c:225`, `deusex-launcher/windows-x86_64/src/platform/win32/process.c:226`) | The order is wrong (free(g->pending) at process.c:226, then CancelIo at :231, with no wait for completion), but the bad case cannot occur. It needs a read still pending after the process is signalled, which needs another process to hold the line's client end. The launcher passes the client only through a PROC_THREAD_ATTRIBUTE_HANDLE_LIST and closes its own copy (process.c:208). DXEngine's LauncherLine::Open (LauncherLine.cpp:50-54, called from Hello at GameApp.cpp:72) clears HANDLE_FLAG_INHERIT. Its relaunch passes a handle list of the stdio alone (GameApp.cpp:228-249), and the crash uploader inherits nothing (CrashReporter.cpp:187). Windows closes a process's handles before it signals the process object, so the read has completed, with its buffer and status written, before ENDED. |
| PKG | The level's time after a save is loaded: at the first tick after the load the original's `Level.TimeSeconds` stood 2.85 s past the time saved, its sleepers' `LatentFloat` 0.40 s less, and DXEngine's 0.17 s past and 0.17 s less, as though the original ran the old level's clock on through the load (`SleepConsole`) | Both engines add the load's frame whole to the level's time and give the actors at most 0.4 s of it ([a level's tick](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#a-levels-tick)): the original's load took 2.87 s under Proton, DXEngine's 0.15 s. `SleepConsole` with the wall clock: the level's time after the load past the time saved by the clock's step over the load, within a millisecond, in both engines (2.865 s against 2.866; 0.149 against 0.148) |
| PHYS | A buoyant crate in the water by Liberty Island's pier comes to rest at the surface some 3 to 6 s in, PHYS_None, flat, a downward velocity kept and no base, where the original's bobs on (`FloatConsole`) | The original's script stops it: Engine.u's `Decoration.ZoneChange` stops a decoration falling slowly back into water from dry, one time in twenty, where no player sees it. The original's own `FloatConsole` run has its other crate, `CrateBreakableMedGeneral1`, at rest from 3.20 s so; DXEngine sends one `ZoneChange` a crossing, and rested one crate in some two dozen re-entries over two runs (DXEngine NATIVES.md, water) |
| PHYS | A spawn of debris refused, `Could not find usable location when trying to spawn`: a broken window's `GlassFragment` on Liberty Island (`DeusExMover.BlowItUp`), a destroyed container's `TrashPaper` on 02_NYC_BatteryPark (`Containers.Destroyed`), the fit testing the filed movers (a playthrough) | The original refuses them at the same spots, unlogged, where DXEngine logs each. `DebrisConsole` (DXEngine 3de10816): of 128 directions round Liberty Island's glass, 111 placed alike and the same 17 refused in both; of 125 spots round Battery Park's `Trashbag1`, which falls out of the world at the map's start in both, 50 placed alike and the same 75 refused. Neither engine's fit sees a filed mover: the original's `AddPolyFragment` gives each filed node no collision hull, and DXEngine fits against the level's own BSP |
| AI | The terrorists Liberty Island's script kills as the statue's mission ends (`Mission01.Timer`: `TakeDamage` at no health, then each one's inventory deleted) leave carcasses that hold their inventory, where the original's are empty (a playthrough) | Both engines leave them empty. `StatueConsole` (DXEngine 1dc75709), `StatueMissionComplete` set at the island's start: each of the 29 goes to `Dying` holding nothing in the timer's tick, and its carcass comes 1.3 s later, empty, still empty after UNATCO HQ and back; every line of the two logs alike but their ticks and times. A state's code runs in its actor's tick alone in both. The one carcass that holds an item is the map's own `TerroristCarcass0`, a mini crossbow, in both |
| PHYS | Swimming out from under the water at some 1,000 frames a second: the original's player gains one float's step a frame and is six times as far out by 0.75 s, where DXEngine's velocity, the move it made, keeps to a step a frame (`SwimConsole`'s d, both engines uncapped) | The swim is the original's; the original's ticks take its floor of 5 ms there (D.17), so its game runs some five times fast. `SwimConsole`'s d uncapped: the original's velocity out the move rounded to the floats of its place over exactly 5 ms each tick, 5 float steps more each tick; DXEngine with that floor, a test build, the same velocity and distance out each tick until its speed's cap and the same speed after, its first tick a little over 5 ms |
| DX | `Accessed None` in `AugmentationCannister`'s pickup (`Pickup.Pickup.Frob`, `Pickup(Copy).PickupFunction`), a playthrough | The original's script: `AugmentationCannister.SpawnCopy` ends with a bare `Return` (DeusEx.u), so `Copy` is None and the original's `execContext` (Core.dll `0x101300d0`) logs it and passes over the call as DXEngine does; the canister reaches the inventory, the skipped `PickupFunction` is empty |
| NET | `?Team=128` in the travel URL from a second new game on, logged by `GameInfo.GetIntOption`, a playthrough | The game's own: Main Menu > Multiplayer > Player Setup saves its team choice on opening (`MenuChoice_Team.LoadSetting` ends in `SaveSetting`: `UpdateURL("Team", "128", True)`, 128 the AUTO team), and the original's `UpdateURL` (Engine.dll `0x103b87c0`) and relative travel (`FURL`, `0x103f7b60`) carry it on; nothing reads it in single player |
| DX | `Accessed None` in `MissionIntro.PreTravel` (`Player.SetInstantSoundVolume`) when the intro is skipped within its first second, a playthrough | The original's: `MissionScript.Player` is set by its first `Timer`, a second in, and the skip's travel sends `PreTravel` before it (`SetClientTravel`, `0x1038fa10`); the sound volume was not yet lowered either |
| DX | `Accessed None` in an infolink's end (`ConPlayBase.TerminateConversation`, `con.ClearBindEvents`) as a map exit is taken, a playthrough | The original's script: the infolink had ended and set `con` None, and the travel's `PreTravel` (`DeusExPlayer.PreTravel`, `DataLinkPlay.AbortAndSaveHistory`) ends it a second time during its END TRANSMISSION blink; no `DataLinkPlay` is saved (a transient class), nothing is lost |
| PKG | The belt changes no weapon after a save is loaded, nor does a conversation put one away (a playthrough) | The save held it: a new game begun from inside the game with the pistol drawn had brought the old pistol back beside the new one, the original's bug as much as DXEngine's (bugs.md D.33; `NewGameConsole`) |
| EXT | A viewport window's frame takes its canvas frame's corner in the original (`Extension.dll` `0x1004acad`), which a render viewport would move, a conversation's letterbox, where DXEngine's canvas frame is the screen's (EXT.22, from EXT.14's reading) | The canvas's frame is the whole view's while the windows draw: `XRootWindow::PreRender` (`0x1003a760`) pushes the frame it has and gives the scene the render viewport's, and `PostRender` (`0x1003a960`) keeps a copy of that one for the scene's projections and puts the pushed one back (`0x1003aa11`) before `PaintWindows`; so the corner a viewport window adds is (0, 0), as DXEngine's canvas has it |

## Not yet read

What no sweep has read, by area: the next sweep's ground, beside the first sweep's numbers no
commit cites ([`bugs-recovered.md`](bugs-recovered.md)).

- **VM:** nothing checked against the binaries; Expression.h's visitors, PrintPrettyExpression,
  the property classes' Copy and Compare.
- **PKG:** IniFile and IniProperty, NameString (skimmed); the loads of UBspNodes and UPolys,
  UMesh.
- **PHYS:** Collision/BottomLevel's TraceRayModel, Math past rotator.h, UPawn's reach and path
  code.
- **AI:** the walk, fly and swim reach moves in detail; AICanSee, and the blend tween's fields,
  against the binary; UActor_Base, Zone, Render and Conversation.
- **DX:** the binaries for the ConListItems' names, the bleed loop and the laser's maths;
  DeusExText's `ParseToken` past `NOTE` and `COMMENT` (`GOAL`'s case, the string `FindEndTag`
  gives back).
- **EXT:** UComputerWindow; UScaleWindow past its first 620 lines (its geometry and drawing);
  the other natives' wrappers in detail; the binary past what the RE docs say.
- **NET:** UnrealURL's parsing; the package cache (FindPackageFile, UsePackageFile); what only
  the binary shows.
- **REN:** BspClipper's rasterizer, the LightEffect shapes but NoneEffect and FireEngine's spark
  kinds (skimmed); the binaries.
- **VK:** vulkanobjects.h's command wrappers, GLSLCompiler::Compile, VulkanSurface and the
  widget surface code.
- **GL:** gl_load.c past its loader entry, DXWidgets' GL context code, ES drivers but Mesa's.
- **SWD3D:** the widgets' PresentBitmap backends, D3D11's bloom and present HLSL (skimmed),
  Wine's and Windows' behaviour at run time; nothing run.
- **AUD:** OpenAL's and OpenMPT's output against Galaxy's; the MSVC and Windows path of
  `-nosound`; nothing run.
- **ENG:** CrashReporter (MSVC's alone), Array.h, JsonValue's parser, MD5, SHA1 and PEIcon
  (skimmed); the binaries.
- **WIN:** Wayland past its timer, the dbus and Win32 file dialogs, DXWidgets' widgets past the
  error window's, the canvas and font code.
- **LCORE:** keys, theme_colors, padcurve and settings_table (skimmed); sha1.c, keys.c's
  scancodes against DXWidgets', platform/ and app/ past their callers.
- **LAPP:** src/screens past window.c, src/core past the files the core's reading named, the
  tests.
- **LUI:** uiscript.c, test-only (skimmed); pad3d.c, padmesh.c, icon.c, glyphs.c past
  dxl_hints.
- **LPORT:** the tests, fetch-sysroot.sh and profile-map.sh (skimmed); the vdf unescaping.
- **MAIN:** the drawing of gui.c's text and dialog.c's layout, the pixels against the
  original's.
- **X:** natives_audit.py, dxcap's Python, tools/probes and check-abi.sh (skimmed).
