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
| [D](#d-for-the-owner) | differences made on purpose, for the owner to rule on | 29 |
| [VM](#vm-the-script-vm-and-the-object-system) | the script VM and the object system | 1 |
| [PKG](#pkg-saving-loading-travel-and-the-loading-screen) | saving, loading, travel and the loading screen | 0 |
| [PHYS](#phys-physics-traces-and-movers) | physics, traces and movers | 5 |
| [AI](#ai-the-npcs) | the NPCs | 0 |
| [DX](#dx-deusexs-natives) | DeusEx's natives | 0 |
| [EXT](#ext-the-window-tree-extension) | the window tree (Extension) | 4 |
| [NET](#net-the-network) | the network | 1 |
| [REN](#ren-the-renderer) | the renderer | 2 |
| [AUD](#aud-the-audio) | the audio | 0 |
| [ENG](#eng-the-engine-its-input-and-its-config) | the engine, its input and its config | 0 |
| [WIN](#win-the-game-window) | the game window | 0 |
| [MAIN](#main-the-recreation-deusex-launchers-main) | the recreation: deusex-launcher's main | 1 |

## D: for the owner

Differences made on purpose that the owner has not yet ruled on. A ruling to keep one marks it
"by choice" where it is said; one to drop it makes the item the original's, fixed and proven
as any other. Nothing here changes before the owner rules.

| ID | The original | DXEngine | Said in NATIVES.md | Recommended |
|---|---|---|---|---|
| D.1 | on a screen wider than 4:3 keeps its field of view across the width and loses height | keeps the height a 4:3 screen shows and sees more at the sides (Hor+), a viewport window too | the view's width; viewport windows | keep: with the original's angle the intro's "Deus Ex" falls off the bottom at 16:9 |
| D.2 | a mesh's level of detail from its own angle | from the Hor+ angle, so on a wide screen a mesh keeps less detail at the same distance | the view's width; mesh detail | take it from the 4:3 angle, keeping Hor+ |
| D.3 | the root window is the whole viewport, where the stock code gives a cutscene's subtitles no height at 16:9 or wider | the root 4:3, a cutscene letterboxed at 16:9 | the UI | keep |
| D.4 | no multisampling, bloom or HDR | each a setting: 4x multisampling on by default (a laser beam's edge rows partly lit), bloom and HDR off | the software device; ENGINE.md, rendering | keep, the defaults as they are |
| D.5 | an 8-bit frame | the GPU devices' scene buffers of 10 bits a channel | ENGINE.md, rendering | keep |
| D.6 | bilinear sampling from one mip | the GPU devices trilinear, with 8x anisotropy | the software device | keep |
| D.7 | no render scale | `Performance.RenderScale`, 1 by default (0.667 on the Smart Pro) | ENGINE.md, settings and environment | keep |
| D.8 | `D3DDrv` draws the detail textures on hardware that has `BLENDDIFFUSEALPHA` (Wine's Direct3D 7 has not), in three passes, each faded by its polygon's corners and rounded to 8 bits | drawn in one pass, the fade worked out per pixel, no rounding between the bands | the menus' settings; brightness | keep them drawn, the method made `D3DDrv`'s |
| D.9 | `D3DDrv`'s detail passes take a depth bias | no depth bias anywhere | ENGINE.md, rendering | keep: a translucent draw biased by its slope shows through the opaque one |
| D.10 | an edit field's copy, cut and paste in a text buffer of its own | the desktop's clipboard | the UI | keep |
| D.11 | `GetDeusExVersion` is "Mon Mar 19 12:06:14 2001 v1.112fm" | DXEngine's own version, shown under the main menu | implemented, not as the original | keep: a player can tell which engine they run |
| D.12 | a net game's player with no chosen name logs in as `Player` | as `DXEnginePlayer` | multiplayer | keep |
| D.13 | `GetLocalIP` is the first address of the machine's own name, on Linux often a loopback one | the address it reaches the network from | multiplayer | keep |
| D.14 | a fire, water or ice texture with no `MaxFrameRate` steps every frame, faster at a higher frame rate | at most 60 times a second | fire, water and ice textures | keep: the original's pace at 60 frames |
| D.15 | blink and strobe lights at the frame rate (a turn count's lowest bit, a turn each frame); an ambient sound on such a light follows them | on DXEngine's own timing | lighting; sound | keep, at the original's pace at 60 frames |
| D.16 | a flickering light flickers each frame | at most 25 times a second (Surreal Engine's) | lighting | the original's pace at 60 frames |
| D.17 | at least 5 ms a step for the actors (the level's own time takes the frame's), so above 200 frames a second the game runs fast: uncapped, the original drew the dark stretch of the menu map's flyby between the Eidos and Ion Storm logos at some 5,000 frames a second and swept through it in a tenth of a second, some 2 s at 60 frames, where held to 60 the original's flyby and DXEngine's keep within 0.02 s of each other (`IntroConsole`) | the frame's real time | small | keep |
| D.18 | the level's `TimeDilation` not held (the game sets it only through `SetGameSpeed`, 0.1 or more) | held to 0.0025 to 25 | small | keep |
| D.19 | a listen server runs as fast as it draws, losing unreliable calls at high rates | at most 144 frames a second | multiplayer | keep |
| D.20 | with the Sound and Speech sliders equal, both kinds of sound scaled by the slider once more | not carried | sound | carry it |
| D.21 | a 1/256 volume floor | not carried | sound | carry it |
| D.22 | a comment with no end tag reads past the text's end, so `09_EmailMenu_ShipOps`'s listing depends on the memory after it | stops at the end | what the player reads | keep, unless the original's runs show a listing that holds, then that |
| D.23 | a start on `?loadgame=N` logs the player in with the travel info the level was saved with | the player saved, as the game's own load gives it | the command line | keep |
| D.24 | the `.int` files read from `System` alone | from each folder of the paths, then `System` | mods | keep: a mod in its own folders gives its text |
| D.25 | an ini written back whole, CR LF, its comments gone | a changed value changed in its own line, the rest of the file as it was | mods (config files) | keep |
| D.26 | `ReceivedBinary` hands the script other bytes, the count right, and `SendBinary` answers False | the bytes that came; False only when nothing went | multiplayer | keep: no mod can have counted on them |
| D.27 | draws a mesh's faces in face order where its walk of the BSP meets the mesh, after the translucent surfaces behind it | a mesh with an opaque face after a translucent one (`GM_Jumpsuit`, `GM_Suit`, `GFM_TShirtPants`, their carcasses, `ComputerPublic`) drawn whole in face order with the solid actors, so a translucent surface behind it is drawn after its few translucent faces (a troop's 4 modulated ones) | draw order and movers | keep: drawn among the translucent parts by one point's distance, a troop in water could be drawn over the water |
| D.28 | a change to Effects Channels (the Sound menu's) takes the channels at once: lowered, the sounds on channels past it play on out of the driver's reach, a looping one for good; raised, the channels past the start's count play on the music's voices | the sounds past a lowered count stop; a raised count's channels are voices of their own | sound | keep: the original's leaves a stuck sound or breaks the music |
| D.32 | plays a stereo sound, an MP3 or a WAV, as two voices, its channel's and the next channel's, the first 32,768 frames of its ring (0.74 s of a 44,100 Hz one) over and over, its ring never filled again, its lip sync with them, until a sound on its slot stops it; a beep its speaker played with each later line went unheard, and the game crashed at its exit unloading a sound, every run that played one (`LipConsole` with `Speech` on `ConAudioMission01_450`, twice, and on the harness's `CapStereo`; galaxy-dll.md, playing a sound). The stock game has four, all MP3s: `ConAudioHK_Shared_681`, `_1531` and `_1532`, and `ConAudioMission01_450` | plays it once, as a mono MP3's ring has it, a 16-bit WAV too (its 4,096 frames of silence the original's first turn's), an 8-bit one from its start, both its channels at the sound's pan | sound | keep: a defect of Galaxy's, which cuts each of the four lines to its first 0.74 s, repeated, and ends in a crash |

## VM: the script VM and the object system

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| VM.5 | Strings are of bytes, Latin-1, where the original's are UTF-16: a typed character past Latin-1 goes nowhere, a wide string loads with `?` for each such character (a note in a save, every conversation comment of `DeusExConText.u`), and `Chr` keeps the low 8 bits | the UI; implemented, not as the original (small ones) | `TypeConsole` typing U+0100 and past; a note of such characters saved and loaded across both engines; `OpsConsole`'s `Chr` and `Asc` |

## PKG: saving, loading, travel and the loading screen

None open.

## PHYS: physics, traces and movers

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| PHYS.6 | Swimming out from under water at some 1,000 frames a second: the original's player gains one float's step a frame and is six times as far out by 0.75 s | implemented, not as the original | `SwimConsole`'s d, both engines uncapped |
| PHYS.7 | A hinged door crushes a crate 0.2 s into its close, where the original's does at 0.3 s: the crate's spawn fits it 8 units nearer the door, the fit's line toward the door meeting DeusExMover26's invisible semisolid polygon inside the door, which DXEngine files whole and the original's filing drops (PHYS.10) | implemented, not as the original (a mover's move) | `DoorConsole` on UNATCO HQ, with PHYS.10 |
| PHYS.8 | A tie between a mover's own hit and the level's at one plane falls to the mover, where the original's two times, rounded apart, fall either way (`DeusExMover19`'s, `25`'s and `8`'s reversed lines on DXMP_Silo are the level's) | implemented, not as the original | `VisibleConsole` on DXMP_Silo: all 110 lines alike |
| PHYS.9 | The filed polygons are an overlay on the level's BSP: a start inside a filed mover's solid is not in solid, and `TraceTexture` gives the flags of the polygon crossed (`PF_Invisible \| PF_Semisolid` on `DeusExMover26`'s line) where the original's filed node has 0 | implemented, not as the original | `VisibleConsole`, `TraceConsole` |
| PHYS.10 | The movers filed under one filed again are not filed again, and a mover's polygons are filed whole, where the original's tree drops what lands in solid | implemented, not as the original (the movers in a line's way) | `MoverConsole` with two movers filed in turn |

## AI: the NPCs

None open.

## DX: DeusEx's natives

None open.

## EXT: the window tree (Extension)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| EXT.8 | Windows are laid out once a frame, where the original lays each out as it asks: a window made this frame is 10 by 10 until the next; one that shows sits at the margins `SetPos` gives it at once, where a parent placing it again does so the next frame; one a parent placed keeps that place, where the original's goes back to its alignment when the parent's next layout leaves it out | the UI | `MenuConsole`, `ScreenConsole`, `SizeConsole` |
| EXT.9 | The first keyboard focus is seeded in DXEngine's layout pass, after the level's tick, where the original seeds it at its windows' next tick: a conversation's first choice has the focus a frame sooner there | the UI | `ChoiceConsole` |
| EXT.10 | The focus tables pass over no window as clipped away, DXEngine keeping no clip rectangles | the UI | `FocusConsole` with a window clipped out of view |
| EXT.14 | Viewport windows, none of it reached by the game: no `CalcView` event; an origin or watch actor gone leaves the view at the window's fields, not at its last place and turn; a window turned off draws nothing, where the original tiles its default texture; `SetRotation` keeps the watch actor; `SetDefaultTexture` takes no defaults; `bShowWeapons` draws no overlays; `SetViewportActor` adds no network view (`AdditionalViews`); `ConvertVectorToCoordinates` in one, or in a window inside one, projects through the main view; the frame takes the window's whole rectangle, not its clip rectangle | viewport windows | `CameraConsole` with each |

## NET: the network

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| NET.8 | A joining client's player while it waits for its pawn: once the server's map is loaded, the original's viewport has a player in the Entry level, and its game engine ticks the Entry level a second time each frame meanwhile, that player's level not being the game's ([the game engine's frame](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#a-levels-tick)); DXEngine's viewport has no player until the pawn arrives (`Engine::LoadClientMap`), draws no world, and ticks the Entry level once | multiplayer ("until its pawn arrives, a joining DXEngine client draws no world") | `TravelJoinConsole` already shows the player's place: the original's `in entry.dx` at its fourth second, DXEngine's logging nothing between `DX.dx` and `DXMP_Cathedral`; the Entry level's time each tick of the wait, and a shot of it, in each |

## REN: the renderer

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| REN.14 | Where two opaque surfaces overlap at one depth, which shows can differ: the gaps between Liberty Island pier's planks, a few hundred pixels of `MeshConsole`'s frame | draw order and movers | `MeshConsole` |
| REN.24 | Liberty Island's pier 1.7% under `D3DDrv`'s frames with the detail textures on and 0.7% over with them off, Paul Denton a level over, `LaserConsole`'s corridor 2% under: measured again once D.8 is ruled | brightness | `MeshConsole`, `LaserConsole` |

## AUD: the audio

None open.

## ENG: the engine, its input and its config

None open.

## WIN: the game window

None open.

## MAIN: the recreation (deusex-launcher's main)

| ID | What | Said in | Proof |
|---|---|---|---|
| MAIN.8 | Unverified in the original: a second launch handing a running game its command line, and the splash staying up through a real start of the engine | dx-reverse-info's wizard.md | the original under Proton, launched twice |

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

## Not yet read

What no sweep has read, by area: the next sweep's ground, beside the first sweep's numbers no
commit cites ([`bugs-recovered.md`](bugs-recovered.md)).

- **VM:** nothing checked against the binaries; Expression.h's visitors, PrintPrettyExpression,
  the property classes' Copy and Compare.
- **PKG:** IniFile and IniProperty, NameString (skimmed); the loads of UBspNodes and UPolys,
  UMesh.
- **PHYS:** Collision/BottomLevel (TraceAABBModel, TraceRayModel, OverlapAABBModel), Math past
  rotator.h, UPawn's reach and path code.
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
