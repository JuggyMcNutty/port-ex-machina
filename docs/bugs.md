# bugs.md

The bugs found and not yet fixed, each with its proof: the third sweep's (every doc of the four
repositories read for what is open, each place DXEngine or the launchers are not the original
that the owner has not chosen, and each check no run has made), the fourth's (what the third
left unread and the first sweep's numbers no commit cites, read area by area against the
binaries), the fifth's (what the fourth left unread, read area by area against the binaries and
our code, each finding a run could reach run in both engines), and those found since. A proof is
a run of both engines ("Run:"), or a reading -- of the original's binary or of our code -- with
the run that will prove the fix where one is due ("Read:", "Run due:"). The refuted are listed
at the end so they are not reported again.

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
| [NET](#net-the-network) | the network | 2 |
| [REN](#ren-the-renderer) | the renderer | 3 |
| [AUD](#aud-the-audio) | the audio | 0 |
| [ENG](#eng-the-engine-its-input-and-its-config) | the engine, its input and its config | 0 |
| [WIN](#win-the-game-window) | the game window | 5 |
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
| D.81 | `PREFERENCES` opens nothing and is logged (`Engine.cpp`, `WindowExec` and `LauncherExec`), where the original's opens its Advanced Options window, out of full screen first (WinDrv.dll `0x11106d90`; the launcher's `GLaunchExec`, `WConfigProperties` captioned `AdvancedOptionsTitle`): the config's classes and their properties in a tree, edited in place ([launch-flow.md](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/launch-flow.md)). Ruled: build the Advanced Options window, `PREFERENCES` opening it as the original's does (the server's clipboard and `OBJ GARBAGE` kept by choice) | The command line | Run: `WindowWordsConsole` |

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
| NET.38 | A DXEngine server sends no client `SimFallServer`'s `CapSimFall` -- hidden, `DT_None`, always relevant, a simulated proxy with `bSimFall`, spawned 3 s after the player is in, its `Physics` changed every 2 s --, neither the original's client nor its own, on the builds before NET.31's fix and after (`Network/NetServer.cpp`'s due list and channels; the cause not read), where the original's server sends it | Multiplayer | Run: `SimFallServer` with `JoinConsole`: the original's server's `CapSimFall` logged by a DXEngine client at 3.1 s; a DXEngine server's by neither client, before and after |
| NET.39 | A player's `bBadConnectionAlert` on a DXEngine server is on for its first second in the game, then off, where the original's is off from its first half second: the test is the original's (Engine.dll `0x10404e1c`-`0x10404ef0`), what goes into it in the first period with the player not (the acks of packets sent while the client loaded the map, or the player's channel open sooner, not read). No stock script reads it | Multiplayer | Run: `AlertServer` with `JoinConsole`: the original's False at 0.5 s and after; DXEngine's True at 0.5 s and 1.0 s, then False |

## REN: the renderer

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| REN.39 | Device edges fixed by reading (DXEngine 8ce0bc6b), their runs due: the single-texture bindless array cleared whole; D3D11's base and macro pans and its present and read-back samplers |  | Read. Run due: the array's and the dither's with a case the build before fails (`DXE_DEBUG_BINDLESS_FILL=10` on the main menu, and the dither on `TileConsole` under Wine, drew the same before as after); D3D11's pans against Vulkan's once its picture matches. Proven: a GPU without `VK_KHR_sampler_mirror_clamp_to_edge` keeping the Vulkan device, its clamped tiles the same to the level, where before the software device drew; GL's and D3D11's resize sending no texture again, where before each size sent some 95; a surface listing `R8G8B8A8_SRGB` first (SDL3's Wayland window, its list cut to that and `R8G8B8A8_UNORM`) given the UNORM format, its picture the usual one's, where before the sRGB format washed it out |
| REN.62 | A decal counts as drawn on a node the original's walk saves no polygon of (`Render/VisibleFrame.cpp`, `ProcessNodeSurface`; `Render/VisibleDecal.cpp`, `StampDecals`), where the original stamps one only where `ClipDecal` leaves some of it in a polygon `OccludeBsp` saved, any span of which shows (`0x10b173d0`; DrawFrame's loop `0x10b1b0e6`-`0x10b1b2f0`): the nodes' span tests part, so a few decals out of sight live on past the second `DeusExDecal.Timer` waits | Out of sight | Run: `DecalSeenConsole` on the island: of the 29 marks the eye's line to is blocked, on the sea floor past the dock, DXEngine counts 23 and the original 15, its 15 among them, the 8 more at 800 and 1,000 units out |
| REN.71 | A mover's face turned from where it lay is drawn darker than the original's: Liberty Island's `DeusExMover10` opened 90 degrees, its face's light map built again where it now is (`Light/LightSystem_Light.cpp`, `GetMoverLightmap`), where the original's face keeps the light it had, or is lit otherwise (not read; render-dll.md, light maps). Every door that turns toward the shade | Lighting | Run: `MoverDecalConsole`'s shot 2, the opened door's face 0.039 of white in DXEngine, before and after the decal fixes, against the original's 0.092, the wall beside 0.130 in both; shut, 0.094 and 0.094 |

## AUD: the audio

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## ENG: the engine, its input and its config

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## WIN: the game window

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| WIN.29 | `STAT` toggles the renderer's panels a word, as the original's (`Engine.cpp`, `RendererExec`), but draws Fps's line alone (`Render/RenderCanvas.cpp`, `DrawStats`), where the original draws each of the nine panels its `DrawStats` has that is on (Render.dll `0x10b13a50`): Global, Net, Hardware, Mesh, Zone, Occlusion, Game and Cache list the original's own timings, connection counts and caches, which DXEngine does not keep so; the other words' panels draw nothing in either. Typed only | Small | Read (`ViewWordsConsole`: the FPS line alike in both engines) |
| WIN.30 | DXEngine has no demo driver (`Engine.cpp`, `LevelExec`): a named `DEMOREC` or `DEMOPLAY` answers "Demo recording failed: DXEngine has no demo driver" or "Demo playback failed: ...", `STOPDEMO` nothing, `[Engine.Engine] DemoRecordingDevice` unread, where the original's level records with its demo driver, single player too (`InitListen` `0x10407870`, a CLIENTDEMO with an `Engine.DemoRecSpectator`), plays a demo back and answers each word (`UDemoRecDriver::Exec` `0x104083c0`). Typed only | Small | Run: `DemoWordsConsole` on the island: the original's "Demo recording started to DxCapDemo.dem", "Demo recording currently active: DxCapDemo.dem" for `DEMOREC` and `DEMOPLAY`, "Demo DxCapDemo.dem stopped (369 frames)"; DXEngine's failures, the bare words' "You must specify a filename" alike |
| WIN.31 | The engine's words that list the original's own tables are taken by nothing (`Engine.cpp`, `SystemExec`, `ObjectExec`): `CONFIGHASH` (`USystem::Exec`, Core.dll `0x1016b790`), `DUMPNATIVES` and `OBJ`'s `REFS`, `HASH`, `DEPENDENCIES`, `VFHASH` and `LINKERS` (`StaticExec` `0x10153110`); `DUMPCACHE` lists nothing (`FMemCache::Exec` `0x10125500`), DXEngine having no memory cache; `OBJ LIST`'s bytes are each class's properties', the original's what its serializer walks of each object. Typed only | Small | Run: `EngineWordsConsole` from the menu map: the original's `CONFIGHASH` 153 characters, `DUMPCACHE` 478, `OBJ LINKERS` 2,880, `OBJ HASH` 38, `OBJ REFS` 62,118; DXEngine '' for each |
| WIN.35 | The engine's own Wayland window (a dev build's) types text through no input method (`text-input-v3`; `DXWidgets/src/window/wayland/wayland_display_backend.cpp`), its compose sequences alone, so a language typed through one, Chinese or Japanese, types nothing in it; SDL3's window takes one. No port builds it |  | Read |
| WIN.38 | Galaxy's console words are taken by nothing, DXEngine's audio device having no words of its own: `MUSICORDER`, a jump to the song's order, logged "Galaxy order"; `ASTAT AUDIO` and `ASTAT DETAIL`, each channel's sound, volume, pitch, radius and priority drawn over the view; `CDTRACK` and `CDVOLUME`; `RECORDSOUND` and `ENDRECORDSOUND`, the microphone at 8 kHz saved as `Test.wav` and played at the player (Galaxy.dll `Exec` `0x10607070`, `PostRender` `0x10608570`); nor the original launcher's `EDITACTOR`, an actor's property inspector (DeusEx.exe's `GLaunchExec`), whose `MPLAYER` and `HEAT` reach dead services (`0x109142C0`). Typed only | Small | Read (WIN.31's investigator) |

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
| AUD | A sound's update sets only the pan its voice moves toward, which Galaxy's mixer moves by at most 512 of 32,767 each time it works out the gains (`0x10615329`), where `GalaxyMixer` sets a sound's at once (the lead Not yet read named) | Galaxy never glides a pan: every write of a voice's target pan writes its current pan the same (`glxStartSample` `0x106125e6`, `glxControlSample` `0x1060d2ec`-`0x1060d30a`, the 3D control `0x1060d678`, `glxInit` `0x1060e04a`, order 0 `0x106159aa`, the 8xx command `0x106177aa` and the rest), the update's own write of the target alone (`0x1060a333`) coming after `glxControlSample` set both (`0x1060a322`): a moving sound's pan steps once a frame in both engines (galaxy-dll.md, the mixer) |
| PHYS | The normal the original's cylinder point check gives an actor's contact (the lead Not yet read named) | Nothing reads it: `UPrimitive::PointCheck` of a cylinder (`0x103d87e0`) makes it, but the encroachment check keeps the actor alone and the actors' point check (`0x103585b0`) is reached only through `FindSpot` with actors off at a spawn, a far move and a landing (`0x103951ae`, `0x10398d13`, `0x103cf454`); DXEngine's is 0, its one caller `FindSpot` with actors off too |
| PKG | `BindOrderConsole` started on its map takes the first of its two travels in DXEngine, the original the other (the lead Not yet read named) | Its second travel is asked during the first's SAVING paint, dropped by both engines' browse; two travels asked in one frame end with the last in both (`SetClientTravel` `0x1038fa10`), `ServerTravel`'s first in both (`LoadTravelConsole`: one asked during SAVING dropped in both); what differs is a travel asked during LOADING, PKG.34 |
| PKG | A package older than version 60 asks the original's `OldVersion` question, its No refusing it, where DXEngine refuses it outright (the lead Not yet read named) | A map of version 59 stops both: the original asserts in its file reader (`InPos<=Size`, `FArchiveFileReader::Seek`) before any question, DXEngine refuses the version (`LocalMapConsole` on `badpackage2.py`'s `--version 59`) |
| PKG | A map importing a sound that is not there is refused by the original's map check, which DXEngine loads with the sound None | Both load it: `LocalMapConsole` on 02_NYC_Bar with its sound `Sink` renamed starts on the map in both engines |
| EXT | A masked button's texture is drawn opaque by DXEngine where the original's window takes its background style, so colour 0 shows the box behind (`MenuUIScrollAreaWindow`'s arrows, `MenuUITabButtonWindow`) | Colour 0 of a masked button is black in both: `BgStyleConsole`'s masked buttons at 13, 13 black in the original through OpenGLDrv and D3DDrv alike, as in DXEngine; only the translucent style differs, EXT.28 |
| REN | The console's `FLUSH`, which `DeusExPlayer.ShowMainMenu` and the brightness menu send, draws no PRECACHING frame after it, where the original's `UEngine::Flush(1)` has `D3DDrv`'s Flush set `PrecacheOnFlip` from `UsePrecache` (`0x10002ec9`), so with it on the main menu opens on PRECACHING (REN.43) | Not so in the original: `RenderWordsConsole` with `DXCAP_PRECACHE=1`, the level's action at the console's ticks Precaching at the map's first and at none of the five after `FLUSH` or `BRIGHTNESS`, as in DXEngine |
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
| NET | After a value too big for a bunch a DXEngine server sends that property no more (the first wave's NET.32) | `CapBigString` logged a change of the string's length alone, and `xyz` is as long as `abc`: logging each change of its text, the original's client of DXEngine's server gets `abc`, then `xyz` at 7 s, and DXEngine's of the original's `abc`, '' at 5 s, `xyz` at 9 s; the overflow's one difference is D.56's |
| ENG | A key held across a map load is not pressed again, the new level's pawn getting none of its `Button` bytes until it is pressed again, where the original's `LoadMap` resets the input and its poll presses each key held (`UInput::ResetInput` `0x10394910`, WinDrv `0x11109853`-`0x111098d3`) | `SideKeyConsole` with Left Shift, W, X and Space held over a reload of Liberty Island: the new pawn's `bRun` and `bDuck` 1 in DXEngine as in the original, from the level's first logged tick |

## Not yet read

What no sweep has read, by area: the next sweep's ground.

- **VM:** a bytecode object constant naming a top-level package (DXEngine's None,
  `Package/Package.cpp:269-270`).
- **PKG:** `UModel`'s reads up to version 61 (`0x103acaa0`) and `0x103a9b20`'s element format;
  Fire.dll's serializers' version tests; version 69 (Revision's 40 packages); Core's parse of a
  malformed GUID; the original server's answer to a `HAVE` of a newer generation; the
  original's failed join on screen against DXEngine's "Connection failed"; a slot's save of
  Liberty Island, 7,985 KB in DXEngine against the original's 7,865 (`MissingSlotConsole`'s
  `GetSaveDirectorySize(9)`): what the 120 KB more are.
- **PHYS:** a static or immovable actor's move, the original's `MoveActor` answering false at
  once, its hit as it was (`0x103990e0`, read), DXEngine's a hit at time 0; the velocity remade from
  a move by the x87's reciprocal of the tick (the walk's end `0x103cbd7d`, X from the unrounded
  values, Y and Z by the reciprocal stored as a float; the fall's hand-off `0x103cbc3b`; flying,
  falling, projectiles; rolling by Core's reciprocal), DXEngine's divided; `SetLocation`'s touches
  waiting for `bBegunPlay` (`UActor_Phys.cpp:193`), the original's script `SetLocation`
  (`0x103e3120`) asking for them always; the player's start on Liberty Island at z -256.20 in
  DXEngine, -256.25 in the original (`HashOrderConsole`'s and `WallHitConsole`'s first lines).
  a mover filed at 64 bits at a load and a save; the trace's other sums at 24 bits, still in
  doubles (`Collision/TopLevel/TraceTest.cpp`'s `CutLine`, `PullTime` and `BoxCheckTime`), and a
  mover's own brush walk's float order (the original's coords path from `0x103f3593`, its
  planes turned per node, ours turned first); `FinishMove`'s touch filter, its riders' order and
  its missing encroachment check, met outside the ground.
- **EXT:** the edit field's mouse handlers, `Tick`, word moves and `FilterString`;
  `XLargeTextWindow::ConfigurationChanged` and its preferred size; the GL and D3D11 devices'
  tile samplers past their clamp.
- **NET:** the due loop's `iFirstDynamicActor` test (the level's `+0x110`) against
  `Network/NetServer.cpp:505`; `FlushNet`'s loss, duplication and lag simulation; the
  connection's constructor (`0x10401490`), `ReceiveFile` and `SendPackageMap`; Core's
  `UPackageMap::Compute` for generation 0; Wine's `select` on an unconnected TCP socket;
  InternetLink's tick, `StringToIpAddr`, UdpLink's tick and sends.
- **REN:** a plain mesh's lights, picked by the original's `DrawMesh` for the actor drawn, not its
  light sink; a held `SelectedItem`'s ambient light, the original's from its holder's zone; a mesh's
  normals in a mirror's frame, the original's from its mirrored view space; `AttachDecal`'s corner
  order (from `0x103e79a5`); `OccludeBsp`'s portal surfaces in a
  zone-0 frame; the face draw's side and near clip, `ClipBspSurf` and its 3DNow! copy; whether
  the stock doorways hold zone portals behind their doors (REN.60's reach); Fire.dll's `.data`
  passes, `RedrawSparks`' cases 13 to 28 and 33 to 43, `DrawFlashRamp`, `SetWaveLight` and the
  ice blits; the canvas line drawer's tile past its `Z` (`0x10377170`); DX.dx's ElectricityEmitter's fog.
- **GL:** SDL's WGL code and SDL2's sources (inferred from SDL3's); vendor drivers at run time;
  D3D11's HDR on an HDR display; DXGI's `ResizeBuffers` at 0x0 under Wine.
- **ENG:** SoftDrv's own use of its settings; the menus' pointer speed, an absolute
  `MousePosition` against raw deltas; UInput's words past `KEYNAME` and `KEYBINDING`; WinDrv's
  client and viewport words past WIN.23; Galaxy's `Exec`; DeusEx.exe's `GLaunchExec`.
- **AUD:** whether Galaxy's plain routine sends a voice to the reverb; its 3DNow! routine; the
  vibrato's depth and Q's timing past the stock songs.
- **WIN:** SDL3's screen keyboard on the Steam Deck, the game window starting text input as it
  is made (`DXWidgets/src/window/sdl3/sdl3_display_window.cpp:86`).
- **LCORE:** a server's ping after a second ask, taken from the last ask, against how the
  game's own browser measures it; `trash.c`.
- **LAPP:** `test_ui_controls.c` past its first 480 lines.
- **LUI:** the list widget past its use on the servers page.
