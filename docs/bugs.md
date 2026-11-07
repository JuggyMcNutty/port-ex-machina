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
| [VM](#vm-the-script-vm-and-the-object-system) | the script VM and the object system | 3 |
| [PKG](#pkg-saving-loading-travel-and-the-loading-screen) | saving, loading, travel and the loading screen | 2 |
| [PHYS](#phys-physics-traces-and-movers) | physics, traces and movers | 4 |
| [AI](#ai-the-npcs) | the NPCs | 5 |
| [DX](#dx-deusexs-natives) | DeusEx's natives | 1 |
| [EXT](#ext-the-window-tree-extension) | the window tree (Extension) | 0 |
| [NET](#net-the-network) | the network | 4 |
| [REN](#ren-the-renderer) | the renderer | 8 |
| [AUD](#aud-the-audio) | the audio | 2 |
| [ENG](#eng-the-engine-its-input-and-its-config) | the engine, its input and its config | 0 |
| [WIN](#win-the-game-window) | the game window | 2 |
| [MAIN](#main-the-recreation-deusex-launchers-main) | the recreation: deusex-launcher's main | 2 |
| [LAPP](#lapp-the-desktop-launchers-app-processes-and-posix-platform) | the desktop launcher's app, processes and POSIX platform | 1 |
| [LCORE](#lcore-the-launchers-core-ini-config-keys-profiles) | the launchers' core (ini, config, keys, profiles) | 2 |
| [LUI](#lui-the-launchers-screens-and-ui-kit) | the launchers' screens and UI kit | 1 |
| [LPORT](#lport-the-launchers-ports-win32-the-smart-pro-the-sysroots) | the launchers' ports (Win32, the Smart Pro, the sysroots) | 2 |
| [X](#x-the-harness-the-scripts-and-ci) | the harness, the scripts and CI | 0 |

## D: for the owner

Differences made on purpose, each the owner's to rule on. A ruling to keep one marks it "by
choice" where it is said and takes it off this list; one to change it stays here, the ruling
said, until the change is made and proven as any other fix. Nothing here changes before the
owner rules.

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| D.47 | A tag of exactly 65,536 bytes is written with its size, where the original's writer gives it size 0 (Core.dll `0x1012a840`'s size codes), so a save of one would not read back. No stock property is that large. Recommended: DXEngine's, by choice |  | Read (PKG.22's investigator) |

## VM: the script VM and the object system

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| VM.25 | Name edges VM.25's first-spelling fix left (DXEngine 046e714c): a map's editor-only name entries are made (`Package/PackageStream.cpp`'s name table), where the original's linker makes no name of one whose flags lack load-for-client or -server and maps it to None (Core.dll `0x10160543`, before the `FName` call at `0x10160553`): 1,230 in the 88 maps, three of them a name's first spelling here (`BArricade`, `supplyroom`, `walkway`); a name read as UTF-8 by `string()` and `GetPropertyText` (`ExpressionEvaluator.cpp:809`, `UObject.cpp:274-285`), a package's names being Latin-1 bytes and those made from a file name, URL, the ini or `StringToName` UTF-8; a package with a name saved 16-bit refused (`PackageStream.cpp:117-118`), the original reading it (`0x101459b0`, made 8-bit, `?` past Latin-1). No stock or mod package has a name past ASCII | Small | Read (VM.25's investigator; its `latent-16bit.diff` drafted). Run due: the three names' spelling after their maps load, both engines |
| VM.29 | An object a package holds has no `Outer` (`VM/ExpressionEvaluator.cpp:355`, a package being no object here), where the original's names the package: `new(None)`'s the package `Transient`, a level's and its actors' their map's, a texture in no group its package's (`Vm29Console`: the probe's, the level's and the player's `Outer` `DX`, `CoreTexMetal.Chainlnk_01`'s `CoreTexMetal`, `new(None)`'s `Transient` in the original, None in DXEngine). No stock script reads one; the ladder probe's test of an outer is PHYS.23 | Small | Run: `Vm29Console`. Drafted: VM.29's investigator's `outer.diff`, each package an object of class `Package`; a save's bytes before and after to be compared |
| VM.30 | A NaN compares unequal (`Native/NObject.cpp:559`, `:1031`; `VM/ExpressionEvaluator.cpp:974`, `:1528`; `Packages/Core/Properties/UProperty.h:198`), where the original's script float `==` and `!=` test the x87's C3 flag alone (Core.dll `0x1013567e`, `0x1013575e`), as do its vector `==` and `Identical` (`0x1016679a`), so a NaN equals anything: a script comparing one takes the other branch, a replicated script float that is NaN is sent each time | Small | Read (NET.17's investigator). Run due: a console comparing a NaN with 0 and with itself, both engines |

## PKG: saving, loading, travel and the loading screen

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| PKG.26 | A tag of a property a class declares again goes to the first of its name the lookup finds (`Packages/Core/UObject.cpp:835-843`), where the original's `FindField` finds the class's own first: `GameReplicationInfo`'s int `Region` takes `Actor`'s struct `Region` tag, which the original skips, logging `Type mismatch in Region of GameReplicationInfo: file 10, class 2` (`original-LoadConsole-20261102-200413`'s log, 56 times in the harness's runs) | Mods | Read (PKG.22's investigator). Run due: `LoadConsole`, the GRI's `Region` after a load, both engines |
| PKG.27 | Config edges PKG.25's fix left: a class's `SaveConfig` does not read its own defaults again (`Packages/Core/UClass.cpp:304-308`, `:352`), where the original's does (`0x10151540`), so a case-only change stays in that class's default for the session; an `INI=` name under five characters or with no `.` fourth or fifth from its end (`INI=MyMod`) is two files to the original, `MyMod` and `MyMod.ini` (GOG DeusEx.exe `0x10915e30`), one to DXEngine and the launchers; `exec`'s file is run line by line whole (`Engine.cpp`, `ExecFile`), where the original's `ExecMacro` takes each by `ParseLine` (`0x10147e60`): `//` a comment, `\|` chaining commands, 255 characters a line; a command word is the first blank-separated word, where `ParseCommand` (`0x10147a20`) also takes `exec"x"`; the URL's `?=x` option from `[DefaultPlayer]` skipped. No stock file reaches them | The command line | Read (PKG.25's investigator) |

## PHYS: physics, traces and movers

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| PHYS.20 | The reach tests' moves pass through the pawn's base and take a mover as blocking by both actors' `bBlockActors` (`UPawn_ReachDeusEx.cpp:64-75`), where the original's walk, fly and swim moves skip the base only with `bIgnoreBases` (all pass 0, `MoveActor`'s filter `0x103995f1`) and block by `IsBlockedBy`: an NPC on a lift's floor tests through it, a flying bird through movers | Moving: wandering and tactical movement | Read. Run due: a pawn based on a lift, `AIDirectionReachable` and `PointReachable` across it |
| PHYS.21 | `JumpLanding`'s steps are raw traces (`UPawn_Path.cpp:540`), the original's three `MoveActor` test moves, held off 2 units and passing pawns and non-static decorations (`jumpLanding` `0x103c2530`): a jumper's landing on a crate or an NPC, some 2 units lower, and the search toward a falling pawn | Moving: wandering and tactical movement | Read. Run due: a jumper's `PointReachable` off a ledge onto a crate |
| PHYS.22 | Line checks' edges and precision: `VisibleActors`' radius by a float square root (`VM/Iterator.cpp:348-349`), the original's squared sum on the x87 (`0x103e53d0`), so a negative radius finds nothing where the original's reaches as far as a positive one; a line under 0.01 clear without its walk (`TraceTest.cpp:614-622`), the original walking it, so from inside solid it finds nothing; `FastWalkNode` in doubles (`TraceRayModel.cpp:101`), the original's plane distances and split points floats (`0x103f33a0`); `NActor::FastTrace` walking end to start (`NActor.cpp:194`); `FindRegion`'s sums in floats, the original's x87 in Z, Y, X order (`0x103f5db0`); `SuggestJumpVelocity`'s accumulator | Implemented, not as the original | Read (VM.16's and the sweep's investigators), against engine-dll.md's traces |
| PHYS.23 | The ladder probe stops at a texture with no outer (`Packages/Engine/Actors/UActor_PhysFlying.cpp:126`), where the original's test of one never passes, a texture in no group having its package for its outer (Engine.dll `0x103ccc40`; `Vm29Console`: `CoreTexMetal.Chainlnk_01`'s `Outer` `CoreTexMetal`): a ladder behind `Chainlnk_01`, `PlatMetlGrate_B`, an `HK_*` or `V_Com_Center` wall is missed, in some ten maps with ladders (02_NYC_Warehouse, the NYC Undergrounds, 09_NYC_ShipFan, 10_Paris_Catacombs, 12_Vandenberg_Cmd and Gas, 14_Oceanlab_Silo, 15_Area51_Entrance, 06_HongKong_WanChai_Market) | Moving: wandering and tactical movement | Read (VM.29's investigator; its `ladder.diff`, or VM.29's `Outer`). Run due: a spot by such a ladder, `LadderConsole`, both engines |

## AI: the NPCs

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| AI.9 | `ActorReachable` of a navigation point answers unlike the original's for 5 of 87 pawn and node pairs on Liberty Island, both ways, every run (MedicalBot1 to a linked node, the original True; MedicalBot5 and 6 to unlinked ones, the original False; Gunther Hermann to an unlinked one, the original True). Cause partly read: the original answers from the path network for a pawn standing at a node (`execactorReachable` `0x103bc1a0`: true for that node or one `CanMoveTo` reaches from it), where DXEngine always walks (`NPawn.cpp:293`, `UPawn_Reach.cpp:16`); the medical bots fit, Gunther does not | Moving: wandering and tactical movement | Run: `NavReachConsole`, two runs of each, each engine's 87 answers alike run to run, 5 apart between them |
| AI.10 | `FindPathToward` of an actor no nearby node reaches answers unlike the original's for 10 of 24 tries on Liberty Island (BumFemale0 a route by PathNode16 in the original, None in DXEngine; Gunther Hermann None against PathNode831), every run. Read: the original's second way marks end points on the goal's list (`findPathToward` `0x103db3f0`, `0x103db938`, `0x103db94d`) and searches from the pawn list's first node, where DXEngine runs both on the pawn's (`UPawn_Path.cpp:758-766`) | Moving: wandering and tactical movement | Run: `SecondWayConsole`, two runs of each, alike run to run |
| AI.11 | `AIPickRandomDestination`'s second try (a multiplier under 1) takes the last step inside the reached distance (`UPawn_AI.cpp:555`), where the original asks with that distance as both its least and most and takes the first step past it (`0x103c8141`-`0x103c8148`): wander and flee spots one step, the pawn's radius, nearer | Moving: wandering and tactical movement | Run: `PickDestConsole`, 14 of 48 picks apart, each the pawn's radius: BumFemale0 180 against 160 (radius 20), Gunther Hermann 242.0 against 217.8 (24.2) |
| AI.13 | `GetPathnodeList` filters by reach and keeps the nearest 32 within 1000 (`UPawn_AI.cpp:626-641`), where the original picks 32 first, once full replacing the nearest entry farther than the new one, then filters, under 1e6 squared (`0x103c6490`): `ReachablePathnodes` (projectile avoidance, `GetOvershootDestination`) where more than 32 nodes lie in reach | Moving: wandering and tactical movement | Read |
| AI.14 | Small path and pick differences: `AIPickRandomDestination` pitched only in water or flying (`UPawn_AI.cpp:542`), the original's unless walking or swimming out of water (`0x103c8041`); `ReachableFromAnchor` tracing anchor to goal (`UPawn_Path.cpp:424`), the original goal to anchor (`0x103da93e`); `ReachablePathnodes` leaving its out variable on the last node, the original None; `FindBestJump`'s log of a drop of 350 or more missing (`0x10481960`) | Moving: wandering and tactical movement | Read |

## DX: DeusEx's natives

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| DX.8 | An actor whose `BindName` is empty is passed over (`Packages/Engine/Actors/UActor_Conversation.cpp:68`), where the original's compare binds it to a conversation with no owner (ConSys.dll `0x1000fec0`). All 1,955 stock conversations have one | Conversations | Read (DX.7's investigator) |

## EXT: the window tree (Extension)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## NET: the network

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| NET.24 | Small URL and travel edges NET.13's fix left: a map loaded again into the level it is already, the original's keeping the same objects (`URLPartsConsole`'s first version: `P.Level == FirstLevel` True), DXEngine making new ones; a URL of another protocol opened in the system's browser by the original (`appLaunchURL`, Engine.dll `0x1038b890`), DXEngine trying to join it as a server; `?load=N` a save's slot in DXEngine, an ordinary browse to the original (DeusEx.dll `0x10010b90`); a URL kept whole, where the original's holds 1,023 characters. No stock path reaches them | Multiplayer | Read (NET.13's investigator) |
| NET.21 | Roles where the original tests them: state code runs for any role from simulated proxy up (`Packages/Engine/Actors/UActor.cpp`, the tick's state code), where the original's runs it with authority or in a simulated state alone (Engine.dll `0x103e5b9c`-`0x103e5ba9`), so a net client runs a proxy's state code the original's does not; the tick's dumb proxy's fall, a server's copy of a client's pawn and `UPlayerPawn::TickInput` test the net mode where the original tests the roles alone (`0x103a23ce`, `0x103a2524`, `0x103a2763`) | Multiplayer | Read (NET.16's investigator; its `roles-optional.diff`, not applied: a standalone player's `RemoteRole` to be checked first) |
| NET.22 | A spectating client looks from rotation 0: the original's server fills a client's `TargetViewRotation`, `TargetEyeHeight` and `TargetWeaponViewOffset` from the pawn it views (Engine.dll `0x103a23e6`-`0x103a249c`), which DXEngine never sets | Multiplayer | Read (NET.16's investigator). Run due: a client viewing another player, both engines |
| NET.23 | Small server and link edges NET.19's and NET.20's fixes left: a player is never made admin where the original's `SpawnPlayActor` sets `bAdmin` with no net driver or with `-alladmin` (Engine.dll `0x10396bd0`), so `admin <command>` does nothing in single player; a `TcpLink`'s socket closes gracefully, where the original's `BindPort` sets `SO_LINGER` on with 0 s, an abortive close (IpDrv.dll `0x10706960`) | Multiplayer | Read (NET.19's investigator) |

## REN: the renderer

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| REN.32 | A light's texels are not held to the rectangle Render.dll lights (`Light/LightmapBuilder.cpp:170-211`; `LE_Cylinder` every texel, `Light/LightEffect.cpp:360-367`), where the original's `SetupLight` gives each light the texels within \|axis\| x sqrt(max(0, 1.05 r^2 - h^2)) of its foot on the surface's plane, clamped to the map, and every shape runs over that alone (`0x10b05fa0`, `0x10b066a2`-`0x10b06865`; the cylinder `0x10b04a80`): a moving cylinder light lights a disc of its radius where the original's lights a square (none on the stock maps); the 425 stock cylinder lights a few texels more at the pool's far edge (`02_NYC_BatteryPark`'s `Light230`, up to 68 of 254); the far band of a non-incidence or shell light (MuzzleFlash, ExplosionLight, the augmentation's Beam, 12 Spotlights) | Lighting | Read. Run due: `LightConsole` with `Effect=17 Spawn=300 View=600`, both at 1024x768, the square against the disc; the sweep's census of the stock cylinder lights' texels |
| REN.33 | An `LE_OmniBumpMap` light lights surfaces (`Light/LightEffect.cpp:39`), where the original's gives an ownerless bump-map light kind 3, which no light map takes (`AddLight` `0x10b08b30`, `0x10b08b85`; `SetupForSurf` `0x10b06c90` merges kinds 0 to 2); render-dll.md's light maps give it the plain shape. No stock light uses it | Lighting | Read. Run due: `LightConsole` with `Effect=15 Spawn=64` |
| REN.34 | A mover's faces all take the mover's zone, its ambient light, pan speeds and fog (`Render/VisibleBrush.cpp:122-126`, `:149`, `:186-189`), where the original files each fragment into the BSP and its node takes its parent's zone on the fragment's side (`AddPolyFragment`, Engine.dll `0x1037bb00`): a mover across zones of different ambient light (`03_NYC_Airfield`'s `BreakableGlass8`, 15 against 5; `09_NYC_Dockyard`'s `DeusExMover5`; `12_Vandenberg_Cmd`'s `DeusExMover2` and `5`) | Draw order and movers | Read. Run due: `GlassConsole` with `GlassName=BreakableGlass8` on `03_NYC_Airfield`, with and without `Flip` |
| REN.35 | Surface flags and textures no stock map reaches: `PF_SmallWavy` not drawn (`RenderSubsystem.cpp:152-170`), the original's U + 8 sin t + 4 cos 2.3t and V + 8 cos t + 4 sin 2.3t; detail and macro textures each locked at its own frame and the macro taken from the base material (`VisibleNode.cpp:62-78`, `:117`, `VisibleBrush.cpp:155-170`), the original's both from `AnimCurrent` as it is, so the Matrix sprite on a macro surface passes an empty texture; the client's `NoLighting` not taken for meshes (`VisibleMesh.cpp:353`, `:653`; `0x10b26291`); a pawn's spotlight aimed by `Rotation` (`LightEffect.cpp:270`), the original's by its view rotation (`0x10b04f80`) | Lighting | Read |
| REN.36 | A realtime texture is converted and sent to the device again at every draw of it in the frame it changed (`UTexture.cpp:103-111`, each draw's TextureInfo carrying the flag, every device sending it whole), its reason gone since one copy per texture (DXEngine 1ccba4c3); on the software device each such draw also forces a mid-frame `DrawRecorded` (`SoftwareRenderDevice.cpp:289`, `:307`); `UTexture.cpp:97-102`'s comment is wrong now. No change to the picture, a cost wherever a fire, water or scripted texture covers several nodes, worst on the software device and the Smart Pro |  | Read. Run due: the uploads counted per frame (the sweep's `realtime-reupload-count.diff`) on 15_Area51_Bunker's flames, then the frame time A/B |
| REN.37 | The devices disagree at the near plane: the software device and D3D11 clip every frame at view z 1 (`SoftwareRenderDevice.cpp:665-672`, `D3D11RenderDevice.cpp:2121`), Vulkan and desktop GL only in a mirror's frame, clamping the depth otherwise (VK.2's fix), GL ES at the near plane. The original likely projects no vertex nearer than 1 (`DrawLodMesh` `0x10b0ea80`, `0x10b0f8db`), how it cuts such a face not read | The software device | Read. Run due: a mesh 0.5 units before the eye on each device, and the original's |
| REN.38 | GL's uploads check a mip's size alone, not its bytes against `MipBytes` (`GLUploadManager.cpp:34`, `:124`), where Vulkan, D3D11 and the software device check them, and `UTexture.cpp:52` pads uncompressed mips alone: a mod's short DXT1 mip reads past its data on GL and GL ES; GL caps at 16,384, not `GL_MAX_TEXTURE_SIZE`, and throws on a texture over it where Vulkan draws white | Mods | Read. Run due: an ASan build with a hand-made .utx |
| REN.39 | Device edges: the Vulkan device requires `VK_KHR_sampler_mirror_clamp_to_edge` (`VulkanRenderDevice.cpp:35`), used for clamped tiles alone, so a GPU without it falls to the software device where GL clamps to the edge (the aarch64 boards); the swapchain takes the surface's first format without B8G8R8A8_UNORM and sRGB-nonlinear (`DXGPU/src/vulkanswapchain.cpp:93`), an sRGB one doubling the gamma; the single-texture bindless path never clears a full array (`VulkanRenderDevice.h:210`); GL flushes its texture cache at each window resize (`GLRenderDevice.cpp:182`); D3D11's base and macro pans share a cache entry (`D3D11RenderDevice.cpp:1584-1588`), E12.4's whole-set check not carried to it, its present and read-back passes binding no sampler, and its scene shader Surreal Engine's (no detail bands, a 1x macro), within the To do's D3D11 parity |  | Read (the sweep's) |

## AUD: the audio

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| AUD.14 | `UseStereo=False` folds the sounds' mix alone (`AudioDevice.cpp:507-514`), the music staying stereo, where Galaxy's whole output is mono: `MusicConsole` with `DXCAP_INI=Galaxy.GalaxyAudioSubsystem:UseStereo=False` recorded the original's tones and songs alike in both channels, a surround channel not cancelled (UNATCO_Music −17.2 dB), where a fold of DXEngine's stereo music, tried, cancelled UNATCO's surround channels (−25.8 dB), the rest within 0.35 dB: Galaxy's mono mix wants reading | Sound | Run (`MusicConsole`, both engines, with `DXCAP_RECORD=1 DXCAP_MUSIC=1`) |
| AUD.16 | A song's loop and sections, which the module rewrite (`DXEngine/Audio/GalaxyMusic.cpp`) cannot give: Galaxy gives each channel the song's own pan, and the song its speed and tempo, again as it enters order 0 (`0x10615710`), so each loop re-centres a channel a pattern panned (LibertyIsland's 2 and 3, eight of HKClub's); a section change silences the voices and plays on from the section, where libopenmpt replays the song to it (`seek.sync_samples`); the music saturates at 16 bits before the sounds are added (`0x106146a0`). Ruled: each made Galaxy's in libopenmpt's player (`DXEngine/Thirdparty/openmpt`), as its pan law is; a song started past order 0 keeps its own pans, speed and tempo, by choice (NATIVES.md's Sound) | Sound | Read: galaxy-dll.md, an IT song. Run due: `MusicConsole` on a stock song through its loop (LibertyIsland's channels 2 and 3) and a section change, both engines |

## ENG: the engine, its input and its config

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|

## WIN: the game window

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| WIN.11 | The Fatal Error window is 1500x800 DIPs centred on the primary screen (`ErrorWindow.cpp:65-72`), so under that (1366x768; 1920x1080 at 150%; 2560x1440 at 200%) its caption, close box and buttons fall off the screen or under the taskbar, and on Windows Alt+F4 and Alt+Space do nothing in an SDL3 window (SDL answers `WM_SYSKEYDOWN` 0) and the log view takes no Esc |  | Read. Run due: `--url=nomap.dx` on a 1280x720-DIP desktop, a shot |
| WIN.12 | Window edges: Alt+Enter back to a window on X11 (SDL3) puts it at the display's corner, `GetFrameGeometry` read right after `ShowNormal` while still full screen (`DXEngine/GameWindow.cpp:36-46`, `sdl3_display_window.cpp:167-180`); SDL3 lists the primary display's modes and size (`sdl3_display_backend.cpp:240`, `:253`) but goes full screen on the window's (`sdl3_display_window.cpp:212`); the X11 paste asks for `STRING`, not `UTF8_STRING`, and serves its own selection as Latin-1 with `?` past it (`x11_display_window.cpp:808`, `:829`, `:1408`), so an Omega copied and pasted comes back `?` (NATIVES.md says UTF-8 both ways; `PasteConsole` passes as wl-copy serves UTF-8 under `STRING`); the dbus file dialog (`dbus_open_file_dialog.cpp:200`, `:227`, `:317`; `dbus_save_file_dialog.cpp:12-22`) passing a null message on a lost bus, unescaping `%20` alone and dropping the save dialog's filters (Save Report, MSVC's alone) | The UI | Read. Run due: `PadConsole` with `axis-keys.txt` and `DXCAP_BACKEND=SDL3`, `xwininfo` after the toggle back |

## MAIN: the recreation (deusex-launcher's main)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| MAIN.12 | The wizard's logo band was never compared with the original: both installs here have `Help/LogoSmall.bmp` (500x77, GOG's of 2021-11-26), so every wizard page shows it in both programs and main's screenshots and its 14 of 15 to the pixel were made without it; main draws the logo whole in a 2-pixel raised edge (`dialog.c:215-224`, `:285-291`), where Wine's modal frame is 3 pixels and its bitmap static likely stretched |  | Read. Run due: `dx-reverse-info/tools/wine/wizard-capture.sh` on goggamefiles against `build/pageshots` |
| MAIN.15 | The wizard's device list stops at 32 devices (`main/src/wizard/wizard.c:27`, `MAX_SHOWN`), where the original's has no cap; a forward goes to the first client socket in the runtime folder, where the original's to the first `IsBrowser` window in the window stacking order, likely the newest client |  | Read (MAIN.13's investigator) |

## LAPP: the desktop launcher's app, processes and POSIX platform

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| LAPP.11 | Only the first launcher holds the instance lock (POSIX `flock`; Windows' mutex, which a second launcher closes), so once the first ends a later start sees no other while another still runs, and reads that one's `Running.ini` as a crash, opening Recovery; DXLauncher's anti-aliasing probe ignores a stop for up to 10 s (`src/app/run.c:653`), the game then started and stopped before it is ready, `Running.ini` left. main and the port branches |  | Read (LAPP.7's investigator) |

## LCORE: the launchers' core (ini, config, keys, profiles)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| LCORE.10 | Run as root, DXLauncher still replaces a read-only ini at the next save: `access()` passes every file for root (`src/platform/posix/fs.c`, `dxl_fs_write_atomic`), and the Smart Pro's user is root (SSH logs in as root). trimui-smartpro |  | Read. Run due: on the device, a read-only `DeusEx.ini`, a setting saved, the file's bytes; whether the card's mount keeps write bits |
| LCORE.13 | Small ini edges LCORE.12's fix left: `dxl_text_bool` takes `Yes` and `No` for User.ini's script-class bools, which DXEngine and the original refuse; main saves a new `INI=` ini to the path as given, so where its folder is there in another case alone the save fails, where the engine makes it in that folder; main's save calls an ini whose link points nowhere read-only |  | Read (LCORE.12's investigator) |

## LUI: the launchers' screens and UI kit

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| LUI.8 | `uiscript`'s `find` focuses its control at once (`src/screens/uiscript.c`), so a script cannot catch a control the d-pad does not reach. A walk by the arrows, tried, selected each list row it passed: `profiles.script` renamed and deleted the wrong profiles. Every port branch |  | Read; the walk tried (LUI.8's investigator's draft), `tests/ui/profiles.script` |

## LPORT: the launchers' ports (Win32, the Smart Pro, the sysroots)

| ID | What | Said in NATIVES.md | Proof |
|---|---|---|---|
| LPORT.6 | Sysroot downloads are checked against no hash: `trimui-smartpro/ports/trimui-smartpro/fetch-sysroot.sh:118` (SDL2's tarball) and `:134` (Mesa's two headers); the workspace's `scripts/lib/common.sh` `dx_fetch_debs` (`:336-364`, no SHA256 nor Release file, cached files taken on `-s`, bookworm's Packages index cached for good) and `dx_fetch_arch_pkg` (`:373-385`): a changed or corrupt download goes into the linux-x86_64, linux-aarch64 and Smart Pro builds' link unnoticed, where every other fetch is SHA256-pinned |  | Read |
| LPORT.9 | Windows' console handler never gets the sign-out or shutdown events in a process that loads user32, so LAUNCHER.md's "the console's handler waits for the stop" is untrue for those two (`windows-x86_64/src/platform/win32/startup.c`) |  | Read (LPORT.8's investigator) |

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

## Not yet read

What no sweep has read, by area: the next sweep's ground.

- **VM:** FixedArray, Map and Pointer past their headers.
- **PKG:** PackageManager's package finding and save folders; two travels asked in one frame
  (`BindOrderConsole` started on its map: the first taken in DXEngine).
- **PHYS:** Math's mat.h, quaternion.h, FrustumPlanes, hsb and halffloat; physWalking and
  stepUp, the BrushTracker and OverlapTest against the binary.
- **AI:** the event manager against the binary.
- **EXT:** UGC's drawing, the ScaleManager, ScrollArea, Tile and Clip windows against the
  binary.
- **NET:** replication against the binary (ReplicateActor, the actor channel's ReceivedBunch,
  NetSerialize, NetRemote), UdpLink, the file channel's tick and destroy.
- **REN:** a mesh's lighting (`SetupForActor`, `Light`), ShadowFromBits, OccludeBsp,
  OccludeFrame and FDynamicSprite, the hand-written fire, water, wet and ice passes, VisibleMesh
  past its flags, RenderCanvas, against Render.dll.
- **GL:** the Wayland EGL context (skimmed), ES drivers but Mesa's.
- **SWD3D:** the software device's rasterizer and SIMD in depth; Wine's and Windows' behaviour
  at run time.
- **AUD:** the utilities E15a's last items touched (`DXEngine/Utils`); the output against
  Galaxy's read, not recorded; whether a sound's update sets only the pan its voice moves to,
  which Galaxy's mixer moves by at most 512 of 32,767 each time it works out the gains
  (`0x10615329`), where `GalaxyMixer` sets a sound's at once (a song's commands and its order
  0 set both, `0x106177aa`, `0x106159aa`).
- **WIN:** Wayland past its timer and keymap (skimmed), the open-folder dialogs, DXWidgets'
  widgets but the push button, the scroll bar and the log view, the canvas and font code past
  the span layout.
- **LCORE:** json.c past its writer, finder_places.c and gamedb.c; mods.c, finder.c and
  servers.c in part; app/browser.c past its first 140 lines; posix net.c (skimmed).
- **LAPP:** the tests, read in part.
- **MAIN:** the pixels against the original's: nothing run (MAIN.12).
- **X:** tools/probes' GLES, fbdev and SDL probes past their headers.
