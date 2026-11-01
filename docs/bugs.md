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
| [D](#d-for-the-owner) | differences made on purpose, for the owner to rule on or ruled to change | 6 |
| [VM](#vm-the-script-vm-and-the-object-system) | the script VM and the object system | 2 |
| [PKG](#pkg-saving-loading-travel-and-the-loading-screen) | saving, loading, travel and the loading screen | 2 |
| [PHYS](#phys-physics-traces-and-movers) | physics, traces and movers | 2 |
| [AI](#ai-the-npcs) | the NPCs | 0 |
| [DX](#dx-deusexs-natives) | DeusEx's natives | 0 |
| [EXT](#ext-the-window-tree-extension) | the window tree (Extension) | 5 |
| [NET](#net-the-network) | the network | 2 |
| [REN](#ren-the-renderer) | the renderer | 3 |
| [AUD](#aud-the-audio) | the audio | 0 |
| [ENG](#eng-the-engine-its-input-and-its-config) | the engine, its input and its config | 1 |
| [WIN](#win-the-game-window) | the game window | 1 |
| [MAIN](#main-the-recreation-deusex-launchers-main) | the recreation: deusex-launcher's main | 0 |
| [LAPP](#lapp-the-desktop-launchers-app-processes-and-posix-platform) | the desktop launcher's app, processes and POSIX platform | 1 |

## D: for the owner

Differences made on purpose, each the owner's to rule on. A ruling to keep one marks it "by
choice" where it is said and takes it off this list; one to change it stays here, the ruling
said, until the change is made and proven as any other fix. Nothing here changes before the
owner rules.

| ID | The original | DXEngine | Said in NATIVES.md | Ruled |
|---|---|---|---|---|
| D.2 | a mesh's level of detail from its own angle | from the Hor+ angle, so on a wide screen a mesh keeps less detail at the same distance | the view's width; mesh detail | from the 4:3 angle, the view staying Hor+ |
| D.15 | blink and strobe lights at the frame rate: a blink out when its turn count, in 65536ths, is odd, in effect a fresh state each frame; a strobe out every other frame; an ambient sound on such a light follows them | DXEngine's own: an even blink, 40 ÷ `LightPeriod` changes a second, and a 5 Hz strobe | lighting; sound | the original's pace at 60 frames, at any frame rate: a strobe out every other 60th of a second, a blink by its turn count's lowest bit taken 60 times a second |
| D.16 | a flickering light flickers each frame | at most 25 times a second (Surreal Engine's) | lighting | the original's pace at 60 frames: at most 60 times a second |
| D.21 | a 1/256 volume floor | not carried | sound | carry it |
| D.22 | a comment with no end tag reads past the text's end, so `09_EmailMenu_ShipOps`'s listing depends on the memory after it | stops at the end | what the player reads | the original run first: a listing that holds from run to run shown, one that varies DXEngine's stop at the end, by choice |
| D.33 | a new game begun from inside a game with a weapon drawn (the main menu's New Game, through the intro) brings that weapon back on the island beside the new game's: the player's `LastinHand`, a travel property the intro's `Paralyzed` state never updates, still names the old game's weapon after `StartNewGame`'s `ResetPlayer` has destroyed it, and the travel's export (`0x10391740`) writes a destroyed actor as any; the belt's slot holds the new pistol, the 1 key draws the old (`SwitchWeapon` finds it first), and `UpdateInHand` never puts down an `inHand` in `Idle2`, so no weapon can be drawn or put away, a conversation's too, until the old one is dropped; it travels and is saved so | the same (`NewGameConsole`: 9 items, two pistols, two 10mm ammo, two key rings, the 1 key then the 2 leaving `bInHandTransition` set, in both engines); a fix drafted: a destroyed actor not written by the travel's export (`DXEngine/DXEngine/ObjectTravelInfo.cpp:81`, `IsPendingKill`) | saving, loading and travel | depart, the original's bug fixed: a destroyed actor not written by the travel's export |

## VM: the script VM and the object system

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| VM.5 | Strings are of bytes, Latin-1, where the original's are UTF-16: a typed character past Latin-1 goes nowhere, a wide string loads with `?` for each such character (a note in a save, every conversation comment of `DeusExConText.u`), and `Chr` keeps the low 8 bits | the UI; implemented, not as the original (small ones) | `TypeConsole` typing U+0100 and past; a note of such characters saved and loaded across both engines; `OpsConsole`'s `Chr` and `Asc` |
| VM.12 | A variable assigned from a function that ends without a `return` is made None or zero, where the original's is left as it was: `CallScript` fills an empty result with the return type's default (`DXEngine/DXEngine/VM/Frame.cpp:324`) and the assignment stores it, where the original's `execLet` (Core.dll `0x1012fe40`) hands the call the variable itself as its result and a bare `Return` (`execNothing`, `0x10102095`) writes nothing (read; `AugmentationCannister.SpawnCopy` returns nothing, its caller's `Copy` None in both) | not yet | a console assigning a set variable from such a function, logged in both engines |

## PKG: saving, loading, travel and the loading screen

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| PKG.17 | A travelled item whose belt slot is taken arrives off the belt, where the original's goes to the belt's first free slot: the old game's pistol a new game brings back (bugs.md D.33), slot 1 the new pistol's, is in the original's slot 4 and off DXEngine's belt (`bInObjectBelt` False, `beltPos` -1); the order of the HUD belt's placing against the travel's events not yet read | saving, loading and travel | `NewGameConsole`: the two logs' `game 2 carries WeaponPistol` lines |
| PKG.18 | The travel's export writes every actor a travel property names, where the original's writes one only with `bTravel` set (`0x10391740`, read): an actor of a class without it, named by a travel property, would travel in DXEngine alone (`DXEngine/DXEngine/ObjectTravelInfo.cpp`, `ActorTravelInfo::Create`) | not yet | a console giving the player a travel property that names such an actor, then a map exit, logged in both engines |

## PHYS: physics, traces and movers

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| PHYS.18 | A box trace that meets both the level and a mover's brush at its start, or a mover beyond the level's hit, meets the mover where the original's meets the level: the original's multi-hit check (`ULevel::MultiLineCheck`, `0x1039b220`) asks the movers along the line cut 5 units past the level's hit, a brush's hit backed off a tenth of the cut line, and a cut to nothing meets none; DXEngine's asks each mover's brush along the whole line. `GrateConsole`, round the grates under Castle Clinton on 02_NYC_BatteryPark: 58 of the 1,842 places a terrorist walking through does not reach, the original's the level at the start, DXEngine's the grate or `BreakableGlass0` at the same place; 13 where a grate is met 0.06 units nearer (-2519.60 against the original's -2519.54) | the traces (a box) | `GrateConsole`'s sweeps, cell by cell, by the actor hit; then MoveConsole, DoorConsole and the other box consoles |
| PHYS.19 | A line's hit lies a float step from the original's where its start is off the float grid: DXEngine's walk takes the exact crossing of the plane, rounded, where the original's line walk (`0x103f3560`) works the crossing out in floats, the start plus the line times the start's distance over the two ends' difference, each step rounded, and the time and place follow from it. `VisibleConsole`: DXMP_Silo's `BreakableGlass8` and `9`, hit at x 178.5 from 148.000015, the original's 178.500015; UNATCO HQ's `DeusExMover21` and `22` reversed, at z -61.499996, the original's -61.5; and a cylinder's normal in its last digit, a line's on Liberty Island (`DeusExMover18`'s line at `DataLinkTrigger21`, `UPrimitive::LineCheck`) and a box's on 02_NYC_BatteryPark (`GrateConsole`'s four hits on `CageLight12`) | the traces (a line; a box) | `VisibleConsole` on the three maps, the `mover` lines diffed; `GrateConsole`'s `CageLight12` hits; the line walk's point read (`0x103f3560`) |

## AI: the NPCs

None open.

## DX: DeusEx's natives

None open.

## EXT: the window tree (Extension)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| EXT.8 | Windows are laid out once a frame, where the original lays each out as it asks: a window made this frame is 10 by 10 until the next; one that shows sits at the margins `SetPos` gives it at once, where a parent placing it again does so the next frame; one a parent placed keeps that place, where the original's goes back to its alignment when the parent's next layout leaves it out | the UI | `MenuConsole`, `ScreenConsole`, `SizeConsole` |
| EXT.9 | The first keyboard focus is seeded in DXEngine's layout pass, after the level's tick, where the original seeds it at its windows' next tick: a conversation's first choice has the focus a frame sooner there | the UI | `ChoiceConsole` |
| EXT.14 | Viewport windows, none of it reached by the game: no `CalcView` event; an origin or watch actor gone leaves the view at the window's fields, not at its last place and turn; a window turned off draws nothing, where the original tiles its default texture; `SetRotation` keeps the watch actor; `SetDefaultTexture` takes no defaults; `bShowWeapons` draws no overlays; `SetViewportActor` adds no network view (`AdditionalViews`); `ConvertVectorToCoordinates` in one, or in a window inside one, projects through the main view; the frame takes the window's whole rectangle, not its clip rectangle | viewport windows | `CameraConsole` with each |
| EXT.20 | What else reads a window's clip rectangle in the original: its draw (`XWindow::DrawTree`, `0x10051490`) draws nothing of a window none of which shows -- no background, no `DrawWindow` or `PostDrawWindow` event, none of its children -- where DXEngine's `RenderSubsystem::DrawWindow` sends the events for every shown window; and a list's page size and rows in view (`XListWindow::GetPageSize` `0x100311f0`, `PlayListSound` `0x10031370`, `Draw` `0x10033350`), an edit field's (`0x1001d250`, `0x10020d00`) and a large text window's (`XLargeTextWindow::Draw` `0x1002e500`), which DXEngine works out from the parent's height or its own clip | the UI | a console counting a window's `DrawWindow` events while its clip window scrolls it out of view, in both engines; a list's page size in a clip window shorter than its parent |
| EXT.21 | At a resolution whose 4:3 root is a fraction of a unit wide of a whole, the HUD sits a unit off the original's: the root is centred to a pixel (`DXEngine/DXEngine/Packages/Extension/UWindow.cpp:109`), its children taken back a whole unit (`round(106.67)`, `UWindow.cpp:159`), so at 2560x1440 (853.33 units at 3x) every whole-unit tile is drawn one unit left, the crosshair 3 px left of the original's (its T's stems at 1272, the original's 1275, measured from a playthrough's shot and simulated); likewise 2048x1536, 2560x1600, 5120x2880, and at 3200x1800 a third of a unit right. A fix drafted (the root centred to a whole unit). The T's stems a unit beside the cross's arms at 2560x1440 are the original's own arithmetic (`DrawTargetAugmentation`'s `int(width*0.5)-1` against the cross window's truncated place), read and not yet run | not yet | a console drawing a weapon's crosshair over a friendly pawn, at 2560x1440 and at 1920x1080 for a control, the middle's columns compared |

## NET: the network

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| NET.8 | A joining client's player while it waits for its pawn: once the server's map is loaded, the original's viewport has a player in the Entry level, and its game engine ticks the Entry level a second time each frame meanwhile, that player's level not being the game's ([the game engine's frame](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#a-levels-tick)); DXEngine's viewport has no player until the pawn arrives (`Engine::LoadClientMap`), draws no world, and ticks the Entry level once | multiplayer ("until its pawn arrives, a joining DXEngine client draws no world") | `TravelJoinConsole` already shows the player's place: the original's `in entry.dx` at its fourth second, DXEngine's logging nothing between `DX.dx` and `DXMP_Cathedral`; the Entry level's time each tick of the wait, and a shot of it, in each |
| NET.9 | On a live server (MTL and ANNA), the player's movement is jittery. The client runs at the original's cap, its net speed over 64 (`Engine::CalcTimeElapsed`): 40 frames a second at the game's 2,600, three 120 Hz refreshes a frame, where a few tenths of a millisecond either way can hold a frame two refreshes or four; the server's corrections go uncounted | not yet | `netspeed 20000` on the same server, the cap past the display's rate: the jitter gone or not; `tools/dxcap.sh live <address>` there, its `ClientAdjustPosition` lines counted |

## REN: the renderer

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| REN.14 | Where two opaque surfaces overlap at one depth, which shows can differ: the gaps between Liberty Island pier's planks, a few hundred pixels of `MeshConsole`'s frame | draw order and movers | `MeshConsole` |
| REN.24 | Liberty Island's pier 1.7% under `D3DDrv`'s frames with the detail textures on and 0.7% over with them off, Paul Denton a level over, `LaserConsole`'s corridor 2% under | brightness | `MeshConsole`, `LaserConsole` |
| REN.28 | A level surface's back-face test takes its texture's own flags, where the original's (`OccludeBsp`, `0x10b17fc9`) takes the surface's and the viewport's alone, the texture's ORed in after (`0x10b181b2`) (`DXEngine/DXEngine/Render/VisibleFrame.cpp:869`): a texture whose flags hold `PF_TwoSided` would draw a one-sided surface from behind in DXEngine alone; no stock texture found so (read) | not yet | none on the stock maps: a test texture made so |

## AUD: the audio

None open.

## ENG: the engine, its input and its config

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| ENG.15 | In a rootless container (distrobox, toolbox: podman's), a pad plugged in after the engine starts is never seen. SDL3 enumerates by libudev, whose monitor drops each message from a sender other than uid 0, and the host's root is the container's `nobody`; SDL watches `/dev/input` itself only in a Flatpak, a Snap or with `/run/host/container-manager`. A pad there at the start is found | ENGINE.md, settings and environment (`Gamepad`) | a pad plugged in beside a listener of the input devices' uevents (each sender's uid), the engine's open devices after; again with `SDL_JOYSTICK_DISABLE_UDEV=1` |

## WIN: the game window

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| WIN.8 | Full screen in a Wayland session, a Screen Resolution chosen does not switch the display: SDL3 emulates the mode (`SDL_SetWindowFullscreenMode`), the frame drawn at its size and scaled to the display's by the compositor, where the original's switches the display's mode | the menus' settings | `getcurrentres` after a choice, against the display's mode (`kscreen-doctor -o`) |

## MAIN: the recreation (deusex-launcher's main)

None open.

## LAPP: the desktop launcher's app, processes and POSIX platform

| ID | What | Said in LAUNCHER.md | Proof |
|---|---|---|---|
| LAPP.6 | In a rootless container, a pad plugged in while DXLauncher runs is never seen: its SDL3 enumerates by libudev too (ENG.15) | not yet | ENG.15's, with the launcher's window open |

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
- **DX:** the binaries for the ConListItems' names, the bleed loop and the laser's maths.
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
