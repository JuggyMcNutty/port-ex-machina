# bugs.md

The bugs found and not yet fixed, each with its proof: the third sweep's (every doc of the four
repositories read for what is open, each place DXEngine or the launchers are not the original
that the owner has not chosen, and each check no run has made), the fourth's (what the third
left unread and the first sweep's numbers no commit cites, read area by area against the
binaries), and those found since. A proof is a run of both engines ("Run:"), or a reading
-- of the original's binary or of our code -- with the run that will prove the fix where one
is due ("Read:", "Run due:"). The refuted are listed at the end so they are not reported
again.

IDs are this list's own, `<section>.<n>`, an item's kept when those before it are fixed, each
section's numbers going on from the second sweep's, so no ID names two items; the first sweep's
(`E1.1`, `E10b.15`, `X2.L9`) are its own list's, which no doc keeps: an item here naming one
points at the commits that cite it (`git log --grep`). A commit that fixes an item cites it,
`(bugs.md VM.5)`, and takes it off this list. DXEngine's
[`NATIVES.md`](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/NATIVES.md) cites each
open difference by its ID, and says "by choice" of each the owner has agreed to; each launcher
branch's LAUNCHER.md (main's README) lists its open defects by their IDs under Known defects.

| Section | Area | Open |
|---|---|---|
| [D](#d-for-the-owner) | differences made on purpose, for the owner to rule on or ruled to change | 1 |
| [VM](#vm-the-script-vm-and-the-object-system) | the script VM and the object system | 0 |
| [PKG](#pkg-saving-loading-travel-and-the-loading-screen) | saving, loading, travel and the loading screen | 0 |
| [PHYS](#phys-physics-traces-and-movers) | physics, traces and movers | 0 |
| [AI](#ai-the-npcs) | the NPCs | 0 |
| [DX](#dx-deusexs-natives) | DeusEx's natives | 0 |
| [EXT](#ext-the-window-tree-extension) | the window tree (Extension) | 0 |
| [NET](#net-the-network) | the network | 0 |
| [REN](#ren-the-renderer) | the renderer | 4 |
| [AUD](#aud-the-audio) | the audio | 0 |
| [ENG](#eng-the-engine-its-input-and-its-config) | the engine, its input and its config | 0 |
| [WIN](#win-the-game-window) | the game window | 1 |
| [MAIN](#main-the-recreation-deusex-launchers-main) | the recreation: deusex-launcher's main | 0 |
| [LAPP](#lapp-the-desktop-launchers-app-processes-and-posix-platform) | the desktop launcher's app, processes and POSIX platform | 0 |
| [LCORE](#lcore-the-launchers-core-ini-config-keys-profiles) | the launchers' core (ini, config, keys, profiles) | 0 |
| [LUI](#lui-the-launchers-screens-and-ui-kit) | the launchers' screens and UI kit | 0 |
| [LPORT](#lport-the-launchers-ports-win32-the-smart-pro-the-sysroots) | the launchers' ports (Win32, the Smart Pro, the sysroots) | 0 |
| [X](#x-the-harness-the-scripts-and-ci) | the harness, the scripts and CI | 0 |

## D: for the owner

Differences made on purpose, each the owner's to rule on. A ruling to keep one marks it "by
choice" where it is said and takes it off this list; one to change it stays here, the ruling
said, until the change is made and proven as any other fix. Nothing here changes before the
owner rules.

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| D.49 | Full screen the size is not kept: `FullscreenViewportX` and `Y` stay as configured, 0 the desktop's mode, where the original's viewport keeps each size it is given full screen as the client's full screen size and saves it (`ResizeViewport`, WinDrv.dll `0x1110e2f3`-`0x1110e35b`), so a size asked that the display lacks is saved as the mode it took; DXEngine's 0 is its own | The settings | Read (WIN.18's investigator) |

## VM: the script VM and the object system

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## PKG: saving, loading, travel and the loading screen

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## PHYS: physics, traces and movers

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## AI: the NPCs

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## DX: DeusEx's natives

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## EXT: the window tree (Extension)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## NET: the network

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## REN: the renderer

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| REN.38 | GL's uploads checked by their bytes, by reading (DXEngine 8ce0bc6b): a top mip short of `MipBytes` or past `GL_MAX_TEXTURE_SIZE` the white texel, a short lower level zeros, as Vulkan and D3D11 take them, where GL read past a mod's short DXT1 mip on GL and GL ES | Mods | Read. Run due: an ASan build with a hand-made .utx, a DXT1 texture whose top mip is short, on GL and GL ES |
| REN.39 | Device edges fixed by reading (DXEngine 8ce0bc6b), their runs due: Vulkan without `VK_KHR_sampler_mirror_clamp_to_edge` clamping to the edge (the aarch64 boards' GPUs); the swapchain's choice where the surface lists an sRGB format first; the single-texture bindless array cleared whole; GL's and D3D11's resize no longer flushing every texture (seen only by counting uploads: no frame shows it); D3D11's base and macro pans and its present and read-back samplers |  | Read. Run due: each on its device, D3D11's under Wine |
| REN.43 | The console's `FLUSH`, which `DeusExPlayer.ShowMainMenu` and the brightness menu send, draws no PRECACHING frame after it (`Engine::PrecacheOnFlip` set at a map's load alone), where the original's `UEngine::Flush(1)` (Engine.dll `0x10380fed`) has `D3DDrv`'s Flush set `PrecacheOnFlip` from `UsePrecache` (`0x10002ec9`), so with it on the main menu opens on PRECACHING | Small | Read (REN.39's run planner) |
| REN.44 | The render device's console words are not taken: `GETDETAILTEXTURES`, `TOGGLEDETAILTEXTURES` (which saves the device's config; `URenderDevice::Exec`, Engine.dll `0x10371b8a`), and `D3DDrv`'s `LODBIAS` and `SHOWPOOLS` (`D3DDrv.dll` `0x100061b0`), each logged unrecognized; no stock script sends them | The UI | Read (WIN.19's investigator, [a key's binding](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#a-keys-binding)) |

## AUD: the audio

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## ENG: the engine, its input and its config

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## WIN: the game window

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| WIN.14 | Window edges fixed by reading (DXEngine 957658cd), their runs due: the Win32 window's Alt+F4 closing the Fatal Error window and its full screen putting the window back where it was, on the monitor it is on (Wine passes a sent key to no Win32-backend window here); SDL2 listing the modes of the display the window is on | The UI | Read. Run due: a Win32-backend run on Windows, or a way to key a Wine window; SDL2 on two displays |

## MAIN: the recreation (deusex-launcher's main)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## LAPP: the desktop launcher's app, processes and POSIX platform

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## LCORE: the launchers' core (ini, config, keys, profiles)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## LUI: the launchers' screens and UI kit

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## LPORT: the launchers' ports (Win32, the Smart Pro, the sysroots)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## X: the harness, the scripts and CI

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## Refuted

| Area | Finding | Why refuted |
|---|---|---|
| PHYS | A walking player rests 4.8 over the floor at Liberty Island's start, where the original's rests 4.75, its landing lower, the first frame after the load shorter (`DXEngine/docs/NATIVES.md`, small) | Where it rests follows its landing, so the first frame's length after the load, which is the load's real time on the machine: run again, both engines rest it at 4.25 at the start and after the first walk, 12.2 after the second, 1.70 from the wall (`StandConsole`, two runs of each, every line alike but the moment's places) |
| ENG | A `SpawnNotify` is not asked about an actor spawned while a notification runs, DXEngine's guard against one spawning without end, the original's taken for unread (`DXEngine/DXEngine/Packages/Engine/Actors/UActor.cpp:128`) | The original's has the same guard: `SpawnActor` (`0x10394f60`) holds a flag (`GSpawnNotifying`, `0x10588b48`) over the notifications' walk, and an actor spawned meanwhile hears none (engine-dll.md, spawning) |
| VM | RadiusActors(None, 300) on the menu map finds one actor more than the original's, 99 against 98 (`DXEngine/DXEngine/VM/Iterator.cpp`, `DXEngine/tools/dxcap/Classes/OpsConsole.uc`) | DX.dx's player rides the menu's flyby, so a count from it is the moment's: 2 s in, the original's player stood at 2769, 308, -98 and DXEngine's at 2752, 299, -92, InterpolationPoint14 (a collision radius of 10) 327 units from the one and 307 from the other, outside 300 plus its radius and inside it, as the original's test has it (engine-dll.md, traces). OpsConsole counts round the level's first PlayerStart (DXEngine cbc56edd): both engines, twice each, give 96, every `DXOPS:` line alike. |
| VM | `VisibleActors(None, 300)` round the menu map's first PlayerStart finds one actor in DXEngine and none in the original, every other iterator's count alike (VM.16; `DXEngine/DXEngine/VM/Iterator.cpp`, `DXEngine/tools/dxcap/Classes/OpsConsole.uc`) | The one is the player, riding the menu's flyby, so the count is the moment's, as RadiusActors' was: at the console's 2 s DXEngine's player stood 262 units from the PlayerStart, the original's 306 and 317 uncapped, its flyby ahead on its floor of 5 ms a tick (by choice in NATIVES.md, small), and 220 held to 60 frames a second, where its count is 1 too, the player, as DXEngine's; not hidden and its line clear in every run, every other actor within 300 hidden. The original's iterator (`0x103e5260`) asks what DXEngine's does (engine-dll.md, the iterators, dx-reverse-info e8f95f6). OpsConsole's radius counts leave the player out (DXEngine 4ab33d50): 95, 0 and 0 in both engines, two runs of each, every `DXOPS:` line alike |
| REN | The laser fence's middle beam a fraction of a pixel thinner, missing the top row `D3DDrv` draws (`DXEngine/docs/NATIVES.md`, particles and lasers) | The multisampling's: with DXEngine's default 4x the beams' edge rows are partly lit, and with it off (`DXEngine/tools/dxcap/vulkan-settings.json`) `LaserConsole`'s three beams have `D3DDrv`'s rows, each within 11 levels of 256 of its red. Multisampling by default is DXEngine's, by choice (its [`ENGINE.md`](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/ENGINE.md#rendering)) |
| DX | ConBindEvents makes every ConListItem with the name None, and a save writes them all as exports of one class, name and outer (`DXEngine/DXEngine/Packages/Engine/Actors/UActor_Conversation.cpp:66`, `DXEngine/DXEngine/Packages/Engine/Actors/UActor_Conversation.cpp:68`) | These items never get into a save. Actor's ConListItems is declared `var transient Object ConListItems;` (gamefiles/System/Engine.u script). PropertyDataBlock::Save (DXEngine/DXEngine/Packages/Core/UObject.cpp:859) skips Transient properties. PackageWriter::WriteObjects (Package/PackageWriter.cpp:116-135) exports only Standalone objects and what they reference. No non-transient script var holds a ConListItem; the only other one is ConListItem.next, which is reached only from the list itself. So no ConListItem export named None is ever written, and nothing gets folded on load. |
| EXT | A text log drops only its oldest expired lines, so a shorter timeout waits behind them (`DXEngine/DXEngine/Packages/Extension/Windows/Text/UTextLogWindow.cpp:73`) | The original's tick (0x10047c30) first subtracts deltaTime from every line's time. It then counts the leading lines whose time is <= 0, stopping at the first that is still positive, and removes that leading run (TArray Remove(0, n)). DXEngine's leading-run erase matches it, as extension-dll.md's text logs say. |
| LPORT | dxl_game_close frees a pending overlapped read's buffer and OVERLAPPED before the read is cancelled (`deusex-launcher/windows-x86_64/src/platform/win32/process.c:225`, `deusex-launcher/windows-x86_64/src/platform/win32/process.c:226`) | The order is wrong (free(g->pending) at process.c:226, then CancelIo at :231, with no wait for completion), but the bad case cannot occur. It needs a read still pending after the process is signalled, which needs another process to hold the line's client end. The launcher passes the client only through a PROC_THREAD_ATTRIBUTE_HANDLE_LIST and closes its own copy (process.c:208). DXEngine's LauncherLine::Open (LauncherLine.cpp:50-54, called from Hello at GameApp.cpp:72) clears HANDLE_FLAG_INHERIT. Its relaunch passes a handle list of the stdio alone (GameApp.cpp:228-249), and the crash uploader inherits nothing (CrashReporter.cpp:187). Windows closes a process's handles before it signals the process object, so the read has completed, with its buffer and status written, before ENDED. |
| PKG | The level's time after a save is loaded: at the first tick after the load the original's `Level.TimeSeconds` stood 2.85 s past the time saved, its sleepers' `LatentFloat` 0.40 s less, and DXEngine's 0.17 s past and 0.17 s less, as though the original ran the old level's clock on through the load (`SleepConsole`) | Both engines add the load's frame whole to the level's time and give the actors at most 0.4 s of it ([a level's tick](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#a-levels-tick)): the original's load took 2.87 s under Proton, DXEngine's 0.15 s. `SleepConsole` with the wall clock: the level's time after the load past the time saved by the clock's step over the load, within a millisecond, in both engines (2.865 s against 2.866; 0.149 against 0.148) |
| PHYS | A buoyant crate in the water by Liberty Island's pier comes to rest at the surface some 3 to 6 s in, PHYS_None, flat, a downward velocity kept and no base, where the original's bobs on (`FloatConsole`) | The original's script stops it: Engine.u's `Decoration.ZoneChange` stops a decoration falling slowly back into water from dry, one time in twenty, where no player sees it. The original's own `FloatConsole` run has its other crate, `CrateBreakableMedGeneral1`, at rest from 3.20 s so; DXEngine sends one `ZoneChange` a crossing, and rested one crate in some two dozen re-entries over two runs (DXEngine NATIVES.md, water) |
| PHYS | A spawn of debris refused, `Could not find usable location when trying to spawn`: a broken window's `GlassFragment` on Liberty Island (`DeusExMover.BlowItUp`), a destroyed container's `TrashPaper` on 02_NYC_BatteryPark (`Containers.Destroyed`), the fit testing the filed movers (a playthrough) | The original refuses them at the same spots, unlogged, where DXEngine logs each. `DebrisConsole` (DXEngine 3de10816): of 128 directions round Liberty Island's glass, 111 placed alike and the same 17 refused in both; of 125 spots round Battery Park's `Trashbag1`, which falls out of the world at the map's start in both, 50 placed alike and the same 75 refused. Neither engine's fit sees a filed mover: the original's `AddPolyFragment` gives each filed node no collision hull, and DXEngine fits against the level's own BSP |
| AI | The terrorists Liberty Island's script kills as the statue's mission ends (`Mission01.Timer`: `TakeDamage` at no health, then each one's inventory deleted) leave carcasses that hold their inventory, where the original's are empty (a playthrough) | Both engines leave them empty. `StatueConsole` (DXEngine 1dc75709), `StatueMissionComplete` set at the island's start: each of the 29 goes to `Dying` holding nothing in the timer's tick, and its carcass comes 1.3 s later, empty, still empty after UNATCO HQ and back; every line of the two logs alike but their ticks and times. A state's code runs in its actor's tick alone in both. The one carcass that holds an item is the map's own `TerroristCarcass0`, a mini crossbow, in both |
| PHYS | Swimming out from under the water at some 1,000 frames a second: the original's player gains one float's step a frame and is six times as far out by 0.75 s, where DXEngine's velocity, the move it made, keeps to a step a frame (`SwimConsole`'s d, both engines uncapped) | The swim is the original's; the original's ticks take its floor of 5 ms there, not carried, by choice ([`NATIVES.md`](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/NATIVES.md)), so its game runs some five times fast. `SwimConsole`'s d uncapped: the original's velocity out the move rounded to the floats of its place over exactly 5 ms each tick, 5 float steps more each tick; DXEngine with that floor, a test build, the same velocity and distance out each tick until its speed's cap and the same speed after, its first tick a little over 5 ms |
| DX | `Accessed None` in `AugmentationCannister`'s pickup (`Pickup.Pickup.Frob`, `Pickup(Copy).PickupFunction`), a playthrough | The original's script: `AugmentationCannister.SpawnCopy` ends with a bare `Return` (DeusEx.u), so `Copy` is None and the original's `execContext` (Core.dll `0x101300d0`) logs it and passes over the call as DXEngine does; the canister reaches the inventory, the skipped `PickupFunction` is empty |
| NET | `?Team=128` in the travel URL from a second new game on, logged by `GameInfo.GetIntOption`, a playthrough | The game's own: Main Menu > Multiplayer > Player Setup saves its team choice on opening (`MenuChoice_Team.LoadSetting` ends in `SaveSetting`: `UpdateURL("Team", "128", True)`, 128 the AUTO team), and the original's `UpdateURL` (Engine.dll `0x103b87c0`) and relative travel (`FURL`, `0x103f7b60`) carry it on; nothing reads it in single player |
| DX | `Accessed None` in `MissionIntro.PreTravel` (`Player.SetInstantSoundVolume`) when the intro is skipped within its first second, a playthrough | The original's: `MissionScript.Player` is set by its first `Timer`, a second in, and the skip's travel sends `PreTravel` before it (`SetClientTravel`, `0x1038fa10`); the sound volume was not yet lowered either |
| DX | `Accessed None` in an infolink's end (`ConPlayBase.TerminateConversation`, `con.ClearBindEvents`) as a map exit is taken, a playthrough | The original's script: the infolink had ended and set `con` None, and the travel's `PreTravel` (`DeusExPlayer.PreTravel`, `DataLinkPlay.AbortAndSaveHistory`) ends it a second time during its END TRANSMISSION blink; no `DataLinkPlay` is saved (a transient class), nothing is lost |
| PKG | The belt changes no weapon after a save is loaded, nor does a conversation put one away (a playthrough) | The save held it: a new game begun from inside the game with the pistol drawn had brought the old pistol back beside the new one, the original's bug as much as DXEngine's (a destroyed actor not travelling, by choice in [`NATIVES.md`](https://github.com/JuggyMcNutty/DXEngine/blob/main/docs/NATIVES.md); `NewGameConsole`) |
| EXT | A viewport window's frame takes its canvas frame's corner in the original (`Extension.dll` `0x1004acad`), which a render viewport would move, a conversation's letterbox, where DXEngine's canvas frame is the screen's (EXT.22, from EXT.14's reading) | The canvas's frame is the whole view's while the windows draw: `XRootWindow::PreRender` (`0x1003a760`) pushes the frame it has and gives the scene the render viewport's, and `PostRender` (`0x1003a960`) keeps a copy of that one for the scene's projections and puts the pushed one back (`0x1003aa11`) before `PaintWindows`; so the corner a viewport window adds is (0, 0), as DXEngine's canvas has it |
| DX | `ConBindEvents` binds another of the actors sharing a name than the original's, its walk the level's actors where the original's walks pawns, decorations and players by object index, the last offered winning (`DXEngine/DXEngine/Packages/ConSys/UConversation.cpp:38-68`; ConSys.dll `0x1000fec0`) | Both engines bind the same actor in every stock case tried: `BindOrderConsole` on 06_HongKong_WanChai_Underworld, `RightHandHack`'s RightHandMan TriadRedArrow6 in both, and a Basketball spawned there with the name bound in both; on 08_NYC_Street `DL_Exit`'s and `DL_JockShipWarning`'s Jock BlackHelicopter1 in both (DXEngine's run from the menu map, as the original's). Only the order of the actors bound, listed, differs |
| AI | The script's `ActorReachable` of a navigation point never asks the path network, where the original's answers a pawn standing at a node from it (`DXEngine/DXEngine/Native/NPawn.cpp:294`; Engine.dll `0x103bc1a0`), so an NPC on a node walks straight to an unlinked one | Not as stated: the original answers True for many unlinked nodes a pawn stands by too (`NavReachConsole`: 50 of 87 True, DXEngine 51), its answers and DXEngine's alike but for five pairs, filed as AI.9 |
| AI | Liberty Island's security bots on their patrol sooner: at `NavReachConsole`'s and `SecondWayConsole`'s probe, 1 s after the open by the console's clock, `SecurityBot0` and `SecurityBot1` held a `MoveTarget` in DXEngine (`PatrolPoint19`, `PatrolPoint14`), none in the original (AI.16) | The probe's moment, not the bots': the console's clock takes in the load's frame whole (`UConsole::PreRender`, Engine.dll `0x10379d10`: the viewport's time between two frames shown), the actors 0.4 s of it at most ([a level's tick](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#a-levels-tick)). The original's load of the island took some 1.2 to 1.7 s under Proton, so its probe came at its first tick, every pawn still in `StartUp`'s sleep; DXEngine's took 0.34 s, so its probe came some 0.6 s of the actors' time later, `SecurityBot0`, drawn in the first frame, on its patrol from the second tick and `SecurityBot1` once its sleep (`FRand()+0.2`) had run out, as each run's draw had it. `PatrolStartConsole` at 120 frames a second, by tick and by the actors' time: `SecurityBot0` out of `StartUp` at the second tick in both engines, toward `PatrolPoint19`, `SecurityBot1` as its sleep runs out, toward `PatrolPoint14`. Both consoles probe at the first tick on the island (DXEngine 37baee2a): every `movetarget` None in both engines, the answers and routes alike |

## Not yet read

What no sweep has read, by area: the next sweep's ground.

- **VM:** FixedArray, Map and Pointer past their headers.
- **PKG:** PackageManager's package finding and save folders; two travels asked in one frame
  (`BindOrderConsole` started on its map: the first taken in DXEngine); ENG.28's investigator's leads:
  a package older than version 60 (the original's `OldVersion` question) and one past 100,
  which DXEngine refuses; a map lacking `MyLevel` or an import; a slot whose `SaveInfo` does
  not read; the LOADING frame of a travel in play that fails.
- **PHYS:** Math's mat.h, quaternion.h, FrustumPlanes, hsb and halffloat; physWalking and
  stepUp, the BrushTracker and OverlapTest against the binary; the normal the original's
  cylinder point check gives an actor's contact, and what reads it.
- **AI:** the event manager against the binary.
- **EXT:** UGC's drawing, the ScaleManager, ScrollArea, Tile and Clip windows against the
  binary.
- **NET:** replication against the binary (ReplicateActor, the actor channel's ReceivedBunch,
  NetSerialize, NetRemote), UdpLink, the file channel's tick and destroy.
- **REN:** a mesh's lighting (`SetupForActor`, `Light`), ShadowFromBits, OccludeBsp,
  OccludeFrame and FDynamicSprite, the hand-written fire, water, wet and ice passes, VisibleMesh
  past its flags, RenderCanvas, against Render.dll; `AttachDecal`, Surreal's own code.
- **GL:** the Wayland EGL context (skimmed), ES drivers but Mesa's.
- **SWD3D:** the software device's rasterizer and SIMD in depth; Wine's and Windows' behaviour
  at run time.
- **ENG:** the client's and the render and audio devices' settings read by hand: their
  fallbacks for a key not there against the classes' defaults.
- **AUD:** the utilities E15a's last items touched (`DXEngine/Utils`); the output against
  Galaxy's read, not recorded; whether a sound's update sets only the pan its voice moves to,
  which Galaxy's mixer moves by at most 512 of 32,767 each time it works out the gains
  (`0x10615329`), where `GalaxyMixer` sets a sound's at once (a song's commands and its order
  0 set both, `0x106177aa`, `0x106159aa`).
- **WIN:** a dedicated server's answer to the window's and the device's words, which DXEngine's
  drops before the exec functions and the engine's (`Engine.cpp`, WIN.19's investigator); the
  original's `BRIGHTNESS` word (Engine.dll's strings); Wayland past its timer and keymap (skimmed), the open-folder dialogs, DXWidgets'
  widgets but the push button, the scroll bar and the log view, the canvas and font code past
  the span layout.
- **LCORE:** json.c past its writer, finder_places.c and gamedb.c; mods.c, finder.c and
  servers.c in part; app/browser.c past its first 140 lines; posix net.c (skimmed).
- **LAPP:** the tests, read in part.
- **X:** tools/probes' GLES, fbdev and SDL probes past their headers.
