# bugs.md

The bugs found and not yet fixed: the open items of the second sweep, 2026-10-28 (each area of
DXEngine, DXLauncher's branches, the recreation and the workspace read by one agent, and every
finding proven or refuted by another, read-only), and those found since, each with its proof.
The refuted are listed at the end so they are not reported again.

IDs are this list's own, `<section>.<n>`, an item's kept when those before it are fixed; the
first sweep's (`E1.1`, `L3.12`, `X2.L9`) are its own list's, named only in the commits that cite
them. A commit that fixes an item cites it, `(bugs.md VM.1)`, and takes it off this list.

None open: the second sweep's items are fixed, each commit citing its ID, or refuted below.

## Refuted

| Area | Finding | Why refuted |
|---|---|---|
| VM | RadiusActors(None, 300) on the menu map finds one actor more than the original's, 99 against 98 (`DXEngine/DXEngine/VM/Iterator.cpp`, `DXEngine/tools/dxcap/Classes/OpsConsole.uc`) | DX.dx's player rides the menu's flyby, so a count from it is the moment's: 2 s in, the original's player stood at 2769, 308, -98 and DXEngine's at 2752, 299, -92, InterpolationPoint14 (a collision radius of 10) 327 units from the one and 307 from the other, outside 300 plus its radius and inside it, as the original's test has it (engine-dll.md, traces). OpsConsole now counts round the level's first PlayerStart (DXEngine cbc56edd): both engines, twice each, give 96, every `DXOPS:` line alike. |
| DX | ConBindEvents makes every ConListItem with the name None, and a save writes them all as exports of one class, name and outer (`DXEngine/DXEngine/Packages/Engine/Actors/UActor_Conversation.cpp:66`, `DXEngine/DXEngine/Packages/Engine/Actors/UActor_Conversation.cpp:68`) | These items never get into a save. Actor's ConListItems is declared `var transient Object ConListItems;` (gamefiles/System/Engine.u script). PropertyDataBlock::Save (DXEngine/DXEngine/Packages/Core/UObject.cpp:859) skips Transient properties. PackageWriter::WriteObjects (Package/PackageWriter.cpp:116-135) exports only Standalone objects and what they reference. No non-transient script var holds a ConListItem; the only other one is ConListItem.next, which is reached only from the list itself. So no ConListItem export named None is ever written, and nothing gets folded on load. |
| EXT | A text log drops only its oldest expired lines, so a shorter timeout waits behind them (`DXEngine/DXEngine/Packages/Extension/Windows/Text/UTextLogWindow.cpp:73`) | The original's tick (0x10047c30) first subtracts deltaTime from every line's time. It then counts the leading lines whose time is <= 0, stopping at the first that is still positive, and removes that leading run (TArray Remove(0, n)). DXEngine's leading-run erase matches it. extension-dll.md's wording "drops the lines whose time is out" is loose, but the binary drops only the leading run. |
| LPORT | dxl_game_close frees a pending overlapped read's buffer and OVERLAPPED before the read is cancelled (`deusex-launcher/windows-x86_64/src/platform/win32/process.c:225`, `deusex-launcher/windows-x86_64/src/platform/win32/process.c:226`) | The order is wrong (free(g->pending) at process.c:226, then CancelIo at :231, with no wait for completion), but the bad case cannot occur. It needs a read still pending after the process is signalled, which needs another process to hold the line's client end. The launcher passes the client only through a PROC_THREAD_ATTRIBUTE_HANDLE_LIST and closes its own copy (process.c:208). DXEngine's LauncherLine::Open (LauncherLine.cpp:50-54, called from Hello at GameApp.cpp:72) clears HANDLE_FLAG_INHERIT. Its relaunch passes a handle list of the stdio alone (GameApp.cpp:228-249), and the crash uploader inherits nothing (CrashReporter.cpp:187). Windows closes a process's handles before it signals the process object, so the read has completed, with its buffer and status written, before ENDED. |

## Coverage

What each finder read, in its own words.

- **VM:** I read VM/Frame.cpp/.h, ExpressionEvaluator.cpp/.h, ExpressionValue.h, StructValue.h,
  ArrayValue.h, Bytecode.cpp/.h, Iterator.cpp/.h, NativeFunc.*, ScriptCall.*, GC/GC.*, EngineGC.cpp,
  Core's UObject.*, UStruct.cpp, UClass.cpp, UState/UFunction/UField/UTextBuffer, NObject.cpp,
  NGC.cpp and NCommandlet.cpp, and checked how PropertyOffsets is used. I compared them with
  core-dll.md, NATIVES.md, the SDK's UnNames.h and the game's script text (strings of the .u files).
  I did not open IDA, because opening it would write into gamefiles/ or reference/, so nothing was
  checked against the binaries. Not reached: Expression.h visitor details, PrintPrettyExpression,
  the property classes' Copy/Compare code.
- **PKG:** Read in full: UnrealURL.cpp, ObjectTravelInfo.cpp, Package.cpp, PackageWriter.cpp,
  PackageStream.cpp, ObjectStream.h, most of PackageManager.cpp, ULevelBase/ULevel.cpp, UModel load
  and save, NGameDirectory and UGameDirectory, NDeusExSaveInfo. In Engine.cpp: Run's travel and save
  loop, ClientTravel, LoadMap, LoadSavedGame/LoadFromSaveFile, SaveGameToSlot, DeusExPreTravel,
  Prune, SaveCurrentLevel, LoginPlayer and open/disconnect. Compared with deusex-dll.md,
  engine-dll.md, core-dll.md, network.md and the Engine.u/DeusEx.u script. Only skimmed:
  IniFile/IniProperty and NameString. Not reached: UBspNodes/UPolys loads, UMesh.
- **PHYS:** Read in full: UActor_Phys*.cpp (all), Collision/TopLevel (CollisionSystem, TraceTest,
  OverlapTest, headers), NActor move/trace natives incl. ParabolicTrace, NZoneInfo, NWarpZoneInfo,
  UWarpZoneInfo, UActor_Base/Touch/Zone, Spawn/Destroy/Move in UActor.cpp, trace iterators in
  VM/Iterator.cpp, UPawn/UPlayerPawn TickRotating. Compared against engine-dll.md (moving, traces,
  touches), game scripts, and Engine.dll in a scratch copy of the IDA backup (UPrimitive::LineCheck,
  execTraceActors, FindBase, IsBasedOn, SingleLineCheck, SetCollision, APawn::performPhysics and
  physicsRotation, IsOverlapping, the ladder code). Not reached: Collision/BottomLevel
  (TraceAABBModel, TraceRayModel, OverlapAABBModel), Math beyond rotator.h, UPawn reach and path
  code.
- **AI:** Read UEventManager.cpp/.h in full, UPawn_AI.cpp, UPawn_Tick.cpp, UPawn_Path.cpp,
  NPawn.cpp, NPlayerPawn/Ext, NScriptedPawn, NNavigationPoint, NGameInfo, NLevelInfo,
  UPlayerPawn.cpp, UActor.cpp, UActor_Animation.cpp, UActor_Touch/Owner, and parts of
  UPawn_ReachDeusEx.cpp. I checked them against engine-dll.md and the game's script, and
  disassembled Engine.dll with objdump at AIProcess 0x10384080, ComputeSenseDetection 0x103837b0 and
  the better-start pass 0x103db0e0 (that pass matches the binary). I did not get to: the
  walk/fly/swim reach moves in detail, AICanSee against the binary, the blend tween's fields in the
  binary, UActor_Base/Zone/Render/Conversation.
- **DX:** I read all of this area's code: NFlagBase plus UFlagBase, NConversation/UConversation,
  RandomLabel, ConAudioList, UActor_Conversation (ConBindEvents), the DeusExTextParser,
  DeusExPlayer, GameDirectory, SaveInfo, ScriptedPawn tick and alliances, the particle and laser
  iterators, and NComputerWindow plus UComputerWindow's queue. I compared them with deusex-dll.md,
  consys-dll.md, deusextext-dll.md, extension-dll.md's flags section, NATIVES.md and the game script
  pulled from the .u files with strings. Most of it matches. IDA was denied, so I could not check
  the binaries for the ConListItem naming, the bleed loop or the laser maths.
- **EXT:** I read the Extension package code: UWindow, URootWindow, UModal, UTabGroup, URadioBox,
  UClip, UScrollArea, UScale (first 620 lines), UScaleManager, UTile, UBorder, UViewport, UList,
  UEdit, UText, ULargeText, UTextLog, UButton, UToggle, UCheckbox, UGC, UExtString and UFlagBase,
  plus NWindow, NRootWindow, NExtString, NExtensionObject and NTextWindow. I also read
  RenderWindow.cpp (the GC stack) and checked each finding against extension-dll.md, NATIVES.md and
  the game's script in DeusEx.u and Extension.u. Not reached: UComputerWindow, the rest of
  UScaleWindow (geometry and drawing), the other natives' wrappers in detail. I opened no IDA
  database, so behaviour the RE docs do not cover stays unconfirmed.
- **NET:** Read all of Network/ (NetBits, NetConnection, NetChannel, NetDriver, NetServer,
  NetClient, NetPackageMap, NetSerialize, NetRemote), Packages/IpDrv (InternetLink, TcpLink,
  UdpLink), Native/N*Link.cpp, and Engine.cpp's net parts (main loop, BeginConnect,
  TickPendingLevel, LoadClientMap, HandleClientPlayer, CloseNetDriver, Listen, PreLogin,
  SpawnPlayActor, GETPING/NETSPEED). Compared them with network.md, ipdrv-dll.md, NATIVES.md, the
  SDK's IpDrv headers and the game's script strings (DeusEx.u, IpServer.u, UBrowser.u). Not reached:
  UnrealURL parsing, the package-cache code (FindPackageFile/UsePackageFile), and anything verified
  only with IDA.
- **REN:** Read in full: Render (VisibleFrame, VisibleMesh,
  VisibleActor/Sprite/Node/Brush/Decal/IteratorItem, RenderCanvas/Scene/Subsystem/Window), Light
  (LightSystem*, LightmapBuilder setup/merge, LightmapAtlas, Shadowmap, LightActorTree, Fog),
  Textures (UTexture, fire/water/wet/ice/scripted, most of FireEngine), Mesh load, NCanvas,
  NScriptedTexture, NDecal, UDecal. Compared against render-dll.md, fire-dll.md, engine-dll.md and
  NATIVES.md. Only skimmed: BspClipper rasterizer, LightEffect shapes other than NoneEffect,
  FireEngine spark kinds. IDA not used.
- **VK:** Read in full: VulkanRenderDevice.cpp/.h, CommandBufferManager, DescriptorSetManager,
  TextureManager, UploadManager, TextureUploader, RenderPassManager, FramebufferManager,
  SceneTextures, Buffer/Sampler/ShaderManager, FileResource.cpp (all shaders), RenderDevice.cpp/.h;
  DXGPU vulkandevice, vulkanswapchain, vulkaninstance, and most of vulkanbuilders (device pick,
  pipelines, barriers, submit, descriptors, images, samplers). Compared against d3ddrv-dll.md,
  ENGINE.md and NATIVES.md. Checked the frame-in-flight sync, layouts, bloom chain and uploads; they
  look sound. Not reached: the vulkanobjects.h command wrappers line by line, GLSLCompiler::Compile,
  VulkanSurface and the widget surface code.
- **GL:** I read all of GLRenderDevice.cpp/.h, GLTextureManager, GLUploadManager, GLTextureUploader,
  GLHandles, GLCachedTexture and GLFileResource (the shaders), plus their callers in
  RenderSubsystem, URootWindow and Engine::TakeScreenshot. I diffed the scene draws, flash, present
  and bloom against the Vulkan device, and checked blending and gamma against d3ddrv-dll.md. On Mesa
  I compiled and linked every shader variant as GLSL 4.20 and ES 3.20 (all clean) and tested
  glDrawBuffers(GL_BACK) (no error). I did not reach gl_load.c beyond its loader entry, the
  DXWidgets GL context code, or ES driver behaviour other than Mesa's.
- **SWD3D:** I read all of Software/ (device, commands, setup, rasterizer, pipelines, SIMD, sampler,
  reference, present, workers, textures) and D3D11/ (device, texture manager, upload manager,
  uploaders, HLSL). I compared them with the Vulkan and GL uploaders, Vulkan's SetSceneNode, ClearZ
  and pipelines, d3ddrv-dll.md, ENGINE.md and NATIVES.md, and the E11b/E13/E14 fixes. I did not
  reach the widgets' PresentBitmap backends, the D3D11 bloom and present HLSL beyond a skim, and
  Wine or Windows runtime behaviour. Nothing was run.
- **AUD:** Read in full: Audio/AudioDevice.cpp, AudioSource.cpp, GalaxyMixer.cpp/.h,
  UDXEngineAudioDevice.cpp/.h, USound.cpp/.h, UMusic.cpp. Also read their callers in Engine.cpp
  (init, map load, StopEffects), NActor.cpp (PlaySound and the volume natives), UActor HearSound,
  UPawn ClientHearSound and UWindow::PlaySound. Compared them with galaxy-dll.md,
  engine-dll.md#sounds, NATIVES.md#sound, the game's ini and Engine.u's LipSynch, and checked
  bugs-recovered E15a/E5a. Not run or built. Not checked: OpenAL/OpenMPT render quality against
  Galaxy's output, or the MSVC/Windows env path for -nosound.
- **ENG:** Read in full: GameApp.cpp, MainGame.cpp, LauncherSettings.cpp/.h, GamepadInput.cpp,
  UDXEngineClient.cpp/.h, CommandLine, LauncherLine, OriginalCommandLine, GameFolder, the Native
  stubs, Engine.cpp's ctor, Run loop, CalcTimeElapsed, ConsoleCommand, the input path and window
  callbacks, Engine.h, Utils File/Logger/Convert/StrTools/UTF8/UTF16/Random/Exception. Compared
  against engine-dll.md (key bindings, level tick), windrv-dll.md, launch-flow.md, network.md,
  NATIVES.md and ENGINE.md. Did not open IDA, which would write next to the game files.
  CrashReporter (MSVC only), Array.h, JsonValue parse, MD5, SHA1 and PEIcon got only a skim.
- **WIN:** Read GameWindow.cpp/.h, UI/ErrorWindow and WidgetResourceData, window.cpp, the SDL3 and
  SDL2 backends in full, the X11 connection and most of its window, Win32's message handler and
  SetClientFrame, core/widget.cpp's window dispatch and geometry. I followed the callers in
  Engine.cpp (OpenWindow, TickWindow, OnWindow*, setres, togglefullscreen), GameApp, URootWindow's
  key routing, GamepadInput, and the vendored SDL3 source. I read Wayland only for its timer fix,
  and did not reach the dbus or Win32 file dialogs, DXWidgets' widgets beyond the error window's, or
  the canvas and font code.
- **LCORE:** I read all of linux-x86_64/src/core: ini, profile, config, sentinel, policy, cmdline,
  argv, engine_args, engine_sections, settings, engine_settings, json, vdf, upkg, upkg_assets,
  saves, shotlist, gamespy, servers, mods, finder, finder_places, install, gamedb, lastrun, padmap,
  bindings, renderers, advice, bigscreen and log, and skimmed keys, theme_colors, padcurve and
  settings_table. I checked them against the engine (UE1GameDatabase, LauncherSettings,
  PackageManager, OriginalCommandLine, U*Device headers), SDL3's SDL_gamepad.c, DeusEx.u's script
  and Default.ini. I did not check sha1.c, keys.c's scancodes against DXWidgets, or the platform/
  and app/ code beyond the callers.
- **LAPP:** Read in full: src/app/{run,play,session,browser}.c, src/main.c, src/cli_main.c, and
  every file in src/platform/posix (process, instance, wait, startup, os, fs, folders, net, trash,
  desktop, gpu_probe). Checked against LAUNCHER.md, policy.c, cmdline.c, engine_args.c, window.c's
  view/listener, the engine's LauncherLine/Relaunch, launch-flow.md section 1 and DeusExLocalLink's
  script. The aarch64 diff is only the SDL2 backend name, which matches ENGINE.md. Not reached:
  src/screens beyond window.c, src/core beyond the files named, the tests.
- **LUI:** I read all of src/ui (kit, controls, widgets, focus, text, osk, thumbs, audio, theme,
  draw, art in part) and src/screens (shell, window, firstrun, home, profiles, mods, saves, shots,
  servers, dialogs, settings, controller, padwizard, recovery, log, about). I followed the calls
  into session.c, run.c, install.c, profile.c, bindings.c, engine_settings.c and saves.c, and
  checked against LAUNCHER.md and bugs-recovered.md. I skimmed uiscript.c, which is test-only, and
  did not read pad3d.c, padmesh.c, icon.c or the rest of glyphs.c beyond dxl_hints.
- **LPORT:** I read all of win32/*.c (process, instance, fs, trash, os, wait, startup, desktop,
  folders, net, gpu_probe), finder_places_win32.c, Windows' shared-code diff (run.c, main.c,
  settings, renderers, cli), the manifest and toolchain. For the Smart Pro I read gamehook, cpumode,
  delete.c, the run.c/session/screens/settings/window diffs, posix gpu_probe, SDL_fbvideo.c,
  launch.sh and port-hooks.sh, with both LAUNCHER.md files and the port READMEs. I checked the
  engine's LauncherLine.cpp side. I skimmed the tests, fetch-sysroot.sh and profile-map.sh, and did
  not verify vdf unescaping.
- **MAIN:** Read all of deusex-launcher/main/src (launch, policy, cmdline, config, ini, instance,
  sentinel, migrate, safemode, renderdev, detail, install, localize, paths, peicon, log, process,
  winmain, wizard, msgbox, splash; gui.c and dialog.c skimmed), system/run-game.sh, and the relevant
  tests and livecheck.py. Compared them with launch-flow.md, wizard.md, cli-flags.md and
  ini-keys.md, and with DXEngine's LauncherLine, Relaunch and GameFolder. I proved finding 1 by
  compiling cmdline.c and policy.c in scratch. I did not deeply audit the GUI drawing code (gui.c
  text rendering, dialog.c layout) or the pixel accuracy.
- **X:** Read: port-ex-machina scripts (dx.sh, engine.sh, launcher.sh, recreation.sh, ci.sh,
  ci-apt-install.sh, wine.sh, check-docs.sh, lib/common.sh), both workflows and the callers'
  build.yml; DXEngine GameApp, MainGame, CommandLine, OriginalCommandLine, GameFolder,
  UE1GameDatabase, LauncherLine, LauncherSettings, Engine Relaunch/OnLauncherLine, PackageManager
  ini loading, cmake/version.cmake, CMakeLists zip part, dxcap.sh, host-tools.sh; launcher gamedb,
  install.c, engine_args, settings tables, play.c, posix process.c, win32 quoting. Skimmed only:
  natives_audit.py, dxcap/*.py, tools/probes, check-abi.sh.
