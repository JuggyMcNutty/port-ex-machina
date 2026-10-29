# bugs.md

The bugs found and not yet fixed: the open items of the second sweep, 2026-10-28 (each area of
DXEngine, DXLauncher's branches, the recreation and the workspace read by one agent, and every
finding proven or refuted by another, read-only), and those found since, each with its proof.
The refuted are listed at the end so they are not reported again.

IDs are this list's own, `<section>.<n>`, an item's kept when those before it are fixed; the
first sweep's (`E1.1`, `L3.12`, `X2.L9`) are its own list's, named only in the commits that cite
them. A commit that fixes an item cites it, `(bugs.md VM.1)`, and takes it off this list.

| Section | Area | Open |
|---|---|---|
| [VM](#vm-the-script-vm-the-object-system-and-the-garbage-collector) | the script VM, the object system and the garbage collector | 1 |
| [EXT](#ext-the-window-tree-and-its-text-extension) | the window tree and its text (Extension) | 1 |
| [REN](#ren-the-renderer-the-frame-the-scene-lighting-textures-and-meshes) | the renderer: the frame, the scene, lighting, textures and meshes | 3 |
| [VK](#vk-the-vulkan-device-and-dxgpu) | the Vulkan device and DXGPU | 1 |
| [AUD](#aud-the-audio) | the audio | 1 |
| [WIN](#win-the-window-system-dxwidgets-and-the-game-window) | the window system: DXWidgets and the game window | 1 |
| [LCORE](#lcore-the-desktop-launchers-core-dxlauncher) | the desktop launcher's core (DXLauncher) | 4 |
| [LAPP](#lapp-the-desktop-launchers-app-processes-and-posix-platform) | the desktop launcher's app, processes and POSIX platform | 5 |
| [LUI](#lui-the-desktop-launchers-ui) | the desktop launcher's UI | 5 |
| [LPORT](#lport-the-windows-and-smart-pro-ports-own-code) | the Windows and Smart Pro ports' own code | 5 |
| [MAIN](#main-the-recreation-deusex-launchers-main) | the recreation: deusex-launcher's main | 4 |

## VM: the script VM, the object system and the garbage collector

### VM.4 RadiusActors(None, 300) on the menu map finds one actor more than the original's

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/VM/Iterator.cpp`, `DXEngine/tools/dxcap/Classes/OpsConsole.uc`
- **What:** OpsConsole's `RadiusActors(None, 300)`, from the player on DX.dx, counts 99 actors in
  DXEngine and 98 in the original; every other iterator's count and the zone census are alike.
  Which actor is the one more was not established. NATIVES.md's edges say the iterators match.
- **Evidence:** OpsConsole on both engines, each started on DX.dx (the original from its menu
  map, DXEngine by `dxe OpsConsole DX.dx`): `DXOPS: RadiusActors(None, 300): 98` against `99`.
  The engine pinned before this round's iterator fixes (0308337f) gives 99 as well.
- **Proof:** the two logs' DXOPS lines diffed: that line alone differs.

## EXT: the window tree and its text (Extension)

### EXT.7 The tool windows draw without their title bar, labels, buttons and list field

- **State:** confirmed, medium (wrong-result)
- **Where:** `DXEngine/DXEngine/Packages/Extension/Windows/`: the windows DeusEx.u's
  `ToolWindow` makes (`CreateToolButton`, `CreateToolLabel`, the list), cause not established
- **What:** The EditFlags cheat screen shows, in the original, its title bar ("Edit Flags"), a
  "Sort By:" label, a white list field and its Add, Edit, Delete and Close buttons. DXEngine
  draws the two radio buttons alone, over a grey field. The other cheat screens built by the
  same `ToolWindow` code (LoadMap, PlayMusic) likely show the same.
- **Evidence:** ToolConsole's shots at 1280x720, the original's through OpenGLDrv and DXEngine's
  on Vulkan: the original's has all of them, DXEngine's the radio buttons alone, before this
  round's window fixes and after them. ToolConsole's row in DXEngine's DEVELOPMENT.md compares
  the frame's corners and edges only.
- **Proof:** the shots side by side.

## REN: the renderer: the frame, the scene, lighting, textures and meshes

### REN.2 A non-dynamic mover's light map is lit at its BrushRaytraceKey place and never rebuilt as
it moves

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXEngine/Light/LightSystem_Light.cpp:30`,
  `DXEngine/DXEngine/Light/LightSystem_Light.cpp:35`,
  `DXEngine/DXEngine/Light/LightSystem_Light.cpp:60`,
  `DXEngine/DXEngine/Light/LightSystem_Light.cpp:153`,
  `DXEngine/DXEngine/Light/LightSystem_Fog.cpp:30`
- **What:** For a mover without bDynamicLightMover, the map's texel world places and the sphere the
  moving lights are gathered in both come from BasePos+KeyPos[BrushRaytraceKey], not from where the
  mover is. Nothing in the !dynamicMover path depends on the mover's location, so the map is never
  rebuilt when it moves. An opened door, or an elevator off its key, keeps the lighting of its key
  place, and a moving light (a flare, the player's light) lights the door's texels where the closed
  door would stand. Its fog map, in contrast, is built at the current Location.
- **Evidence:** GetMoverLightmap builds brushToWorld from KeyPos[key] in the non-dynamic branch and
  passes (brushToWorld*Center) as the light-gathering sphere. GetLightmap's rebuild test, for
  !dynamicMover, is lastStaticUpdate/lastDynamicUpdate/movingSet only (lines 153-201); the movement
  check exists only for dynamicMover (206-214). render-dll.md, light maps: the static map is built
  again '...or, for a mover's surface, after the mover moves, turns or changes leaf'.
- **Verified:** In the non-bDynamicLightMover branch (LightSystem_Light.cpp:30-38) brushToWorld
  comes from BasePos+KeyPos[BrushRaytraceKey] only. The gathering sphere (line 60) uses the same
  matrix. GetLightmap's !dynamicMover path (153-201) never reads mover->Location()/Rotation(), so
  the map never depends on where the mover is. render-dll.md:590-593 says a mover's surface map is
  built again after the mover moves, turns or changes leaf. NATIVES.md Lighting lists no such
  Differs.
- **Proof:** Code path: GetMoverLightmap passes dynamicMover=nullptr for a non-dynamic mover. The
  cache key and rebuild stamps are LMCacheID/ambient, static LastUpdate, the moving set and
  lastDynamicUpdate. None of them changes when Location changes.

### REN.3 bDynamicLight lights never reach a surface's light map as moving lights

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Light/LightSystem_Light.cpp:101`,
  `DXEngine/DXEngine/Light/LightSystem_Light.cpp:520`
- **What:** Also a listed bDynamicLight light is lit as static/animated with shadows instead of as a
  shadowless moving light. How visible this is depends on which stock actors set bDynamicLight,
  which was not established.
- **Evidence:** Line 101: if (!light->bStatic() && !light->bNoDelete())
  entry.Lights.push_back(light). Line 520: if (light->bDynamicLight() || (!light->bStatic() &&
  !light->bNoDelete())) addCandidate. render-dll.md light maps table: moving = 'bDynamicLight, or
  neither bStatic nor bNoDelete; it casts no shadow'; animated = '...not bDynamicLight'. Engine.u:
  'var bool bDynamicLight; // Temporarily treat this as a dynamic light.'
- **Verified:** CollectSurfaceLights (LightSystem_Light.cpp:101) keeps only lights that are neither
  bStatic nor bNoDelete, ignoring bDynamicLight. The mesh path (line 520) and
  render-dll.md:220,578-579 count bDynamicLight as moving. Also, a bDynamicLight light in the
  surface's own list is taken as static or animated with shadow bits (lines 163-178), where the
  original runs it as moving, shadowless. bDynamicLight() is read nowhere else in DXEngine.
- **Proof:** grep bDynamicLight() over DXEngine finds only LightSystem_Light.cpp:520 (mesh) and the
  accessor. The name 'bDynamicLight' (exact) appears in the name tables of many stock maps
  (00_Training, 01_NYC_UNATCOHQ, 02_NYC_* ...). Which actors set it was not established.

### REN.7 Mesh highlights and fog use the main camera's location in mirror, sky and viewport-window
frames

- **State:** confirmed, low (wrong-result)
- **Where:** `DXEngine/DXEngine/Light/LightSystem_Light.cpp:435`,
  `DXEngine/DXEngine/Light/LightSystem_Fog.cpp:127`, `DXEngine/DXEngine/Render/RenderScene.cpp:392`
- **What:** VertexLight's specular term and its fog, and the fog maps' line of sight, take
  engine->CameraLocation. A security camera's feed (DrawViewport, which does not change
  CameraLocation), a mirror's frame and the sky's frame therefore get highlights and fog worked out
  from the player's eye, not the frame's own.
- **Evidence:** InitVertexLight: out.CameraLocation = engine->CameraLocation; GetVertexLight: eye =
  CameraLocation - location. UpdateFogmapTexture: builder.Reaches(light, engine->CameraLocation).
  DrawViewport processes ViewportFrame from 'location' but leaves engine->CameraLocation alone.
  render-dll.md: a mirror's frame walks 'from its own eye (its Coords.Origin)'.
- **Verified:** InitVertexLight sets out.CameraLocation = engine->CameraLocation
  (LightSystem_Light.cpp:435). This drives the highlight eye (LightSystem.h:57) and the fog view
  (LightSystem.h:97), and UpdateFogmapTexture uses it for Reaches/AddLight
  (LightSystem_Fog.cpp:127-129). Only RenderScene.cpp:55-56 (the main frame) is built from
  CameraLocation. DrawViewport (RenderScene.cpp:392) and the mirror and sky frames never set it.
  render-dll.md:740 bases the highlight on the frame's eye.
- **Proof:** grep CameraLocation over Render/ and Light/: it is set nowhere per frame. DrawViewport
  changes CameraFovAngle only.

## VK: the Vulkan device and DXGPU

### VK.2 Vulkan clip distance (or its discard) is on in every frame, not just mirror frames as the
docs say

- **State:** confirmed, low (docs)
- **Where:** `DXEngine/DXEngine/RenderDevice/Vulkan/VulkanRenderDevice.cpp:948`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/FileResource.cpp:41`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/FileResource.cpp:117`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/ShaderManager.cpp:28`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/RenderPassManager.cpp:131`,
  `DXEngine/DXEngine/RenderDevice/RenderDevice.h:30`, `DXEngine/docs/NATIVES.md:535`,
  `DXEngine/docs/ENGINE.md:524`
- **What:** Every frame, the main view included, takes the frame's NearClip. Its default (0,0,1,-1)
  is the near plane at view z=1. So geometry nearer than 1 unit is clipped, and the depth clamp
  enabled 'to avoid clipping the weapon' never acts at the near plane. GL keeps such geometry. On a
  device without shaderClipDistance (the Smart Pro's GE8300, where Vulkan is offered), every scene
  draw is compiled with a discard, which costs a tile-based GPU its hidden-surface removal.
- **Evidence:** SetSceneNode always sets pushconstants.nearClip from Frame->NearClip (default z-1 at
  RenderDevice.h:30). Scene.vert writes gl_ClipDistance[0]=dot(nearClip,p), or under CLIP_DISCARD
  Scene.frag discards when it is <0. CLIP_DISCARD goes into the one scene shader for all pipelines
  (ShaderManager.cpp:28-32). GL turns the clip on only when NearClip != default
  (GLRenderDevice.cpp:1967). NATIVES.md:535-538 says Vulkan discards 'in the mirror's frame alone'.
  ENGINE.md:524 says Vulkan clamps the near plane for 'the weapon close up'.
- **Verified:** VulkanRenderDevice.cpp:948 sets nearClip from Frame->NearClip in every scene frame.
  The default is (0,0,1,-1) (RenderDevice.h:30), and only mirror frames override it
  (VisibleFrame.cpp:1185). Scene.vert (FileResource.cpp:41-45) writes gl_ClipDistance[0]=z_view-1,
  or under CLIP_DISCARD Scene.frag:117-120 discards when that is below 0. CLIP_DISCARD is set once
  for the whole scene shader (ShaderManager.cpp:28-31). GL enables the clip only when NearClip
  differs from the default (GLRenderDevice.cpp:1967-1968). So on Vulkan, depthClampEnable
  (RenderPassManager.cpp:131-133, 'avoid clipping the weapon') never keeps anything nearer than z=1,
  while GL does.
- **Proof:** Path: SetSceneNode -> nearClip = transpose(WorldToView*ObjectToWorld)*(0,0,1,-1) -> dot
  = z_view-1 for every vertex of the main view -> a clip distance below 0 when z_view<1. The frustum
  near is also 1.0 (line 929), so the clamp only acts at the far plane.

## AUD: the audio

### AUD.3 Lip sync is not Galaxy's: 2048-sample rectangular blocks from the sound's start, indexed
by level time, where Galaxy takes 1,024 triangle-windowed samples at the play position every 0.1 s

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Engine/Resources/USound.h:60`,
  `DXEngine/DXEngine/Packages/Engine/Resources/USound.cpp:327`,
  `DXEngine/DXEngine/Packages/Engine/Resources/USound.cpp:400`,
  `DXEngine/DXEngine/Packages/Engine/Subsystems/UDXEngineAudioDevice.cpp:365`,
  `DXEngine/DXEngine/Packages/Engine/Subsystems/UDXEngineAudioDevice.cpp:371`
- **What:** The mouth shapes in conversations come from a different analysis, so the faces move
  differently from the original's. The shapes are precomputed per fixed 2048-sample block with no
  window. The mouth is closed ('X') when the block's peak amplitude is under 0.02, not when the
  strongest frequency is weak. The block is picked by LevelInfo.TimeSeconds since PlaySound (dilated
  game time, starting before the voice does), not by the channel's play position. nextPhoneme is
  written every frame, not every 0.1 s. NATIVES.md lists no difference for lip sync.
- **Evidence:** USound.h:60 LIPSYNC_BLOCK_SIZE = 2048. USound.cpp:337-366 loops over blocks with
  peak < MIN_VOLUME (0.02) giving 'X', then a rectangular FFT. USound.cpp:400 `block = seconds *
  frequency / LIPSYNC_BLOCK_SIZE`. UDXEngineAudioDevice.cpp:365 `elapsedTime =
  LevelInfo->TimeSeconds() - Playing.StartTime`, and :371 writes nextPhoneme every update.
  galaxy-dll.md:236-245: 'Every 0.1 s of a channel's playing time ... 1,024 samples at the play
  position, through a triangle window and an FFT ... X (mouth closed) when even that is weak'.
- **Verified:** USound.h:60 sets LIPSYNC_BLOCK_SIZE to 2048. USound.cpp:337-364 cuts the decoded
  sound into back-to-back 2048-sample blocks from its start, gives 'X' when a block's peak is under
  0.02, and otherwise runs an unwindowed kiss_fftr. USound.cpp:400 picks the block from
  seconds*frequency/2048. UDXEngineAudioDevice.cpp:365 takes those seconds from
  LevelInfo.TimeSeconds minus the StartTime set at line 686, not from the voice's play position, and
  line 371 writes nextPhoneme on every update. galaxy-dll.md:236-245 describes 1,024 samples at the
  play position, triangle-windowed, every 0.1 s of play time, with X when the strongest frequency is
  weak. NATIVES.md and bugs-recovered.md do not mention lip sync differing.
- **Proof:** grep -i 'lipsync|phoneme|lip sync' over DXEngine/docs and bugs-recovered.md finds no
  entry. NATIVES.md:757 'Head turns and lip sync' covers only the blend animations.

## WIN: the window system: DXWidgets and the game window

### WIN.3 The windowed game is placed with its client area at the screen's (0,0), so on Windows its
title bar sits above the screen

- **State:** confirmed, medium (wrong-result)
- **Where:** `DXEngine/DXEngine/Engine.cpp:3024`, `DXEngine/DXEngine/GameWindow.cpp:42`,
  `DXEngine/DXWidgets/src/window/sdl3/sdl3_display_window.cpp:168`,
  `DXEngine/DXWidgets/src/window/win32/win32_display_window.cpp:281`
- **What:** Deterministic on Windows (SDL3 and Win32 backends). On X11 and Wayland the result
  depends on the window manager (Wayland ignores the position). The RE does not document where the
  original places its window.
- **Evidence:** Engine.cpp:3024 `SetFrameGeometry(Rect::xywh(0.0, 0.0, width / dpi, height / dpi))`.
  sdl3_display_window.cpp:168 calls SDL_SetWindowPosition(x,y). SDL3's WIN_AdjustWindowRectWithStyle
  (SDL_windowswindow.c:209-241) starts from rect top=0, runs AdjustWindowRectEx, then does `*y +=
  rect.top`, which is negative for a captioned window. win32_display_window.cpp:273-283 does the
  same through DelayLoadAdjustWindowRectExForDpi. GameWindow.cpp:42-46 reuses GetFrameGeometry's x
  and y.
- **Verified:** Engine.cpp:3024 sets the frame to xywh(0,0,w,h). Win32's SetClientFrame
  (win32_display_window.cpp:270-283) takes the rect as the client area, runs
  AdjustWindowRectExForDpi (which makes rect.top negative for a captioned style) and calls
  SetWindowPos with that rect, so the caption lands above y=0. SDL3's SetClientFrame
  (sdl3_display_window.cpp:156-169) calls SDL_SetWindowPosition(0,0). SDL3's
  WIN_AdjustWindowRectWithStyle does *y += rect.top after AdjustWindowRectEx, and the game window is
  not borderless (flags at :23-29, only a Popup is). GameWindow.cpp:42-46 reuses that x,y when
  leaving full screen.
- **Proof:** Client y=0 -> AdjustWindowRectEx on WS_OVERLAPPEDWINDOW gives rect.top of about -31 ->
  SetWindowPos(hwnd, x<0, y<0, ...) -> the title bar is off the top of the screen.

## LCORE: the desktop launcher's core (DXLauncher)

### LCORE.1 Pad mappings are saved as platform:Linux, so on windows-x86_64 the launcher drops them
at its next start

- **State:** confirmed, medium (platform)
- **Where:** `deusex-launcher/linux-x86_64/src/core/padmap.c:41`,
  `deusex-launcher/windows-x86_64/src/core/padmap.c:41`,
  `deusex-launcher/windows-x86_64/src/screens/padwizard.c:195`
- **What:** Only the launcher's own window loses the mapping. The engine gets it through
  SDL_GAMECONTROLLERCONFIG (run.c:301-309), a hint path that is not filtered by the file loader's
  platform check.
- **Evidence:** padmap.c:41 `dxl_buf_puts(&b, "platform:Linux,");`. SDL3's
  SDL_AddGamepadMappingsFromIO (dep_sdl3 src/joystick/SDL_gamepad.c:2434-2442) adds a line only if
  SDL_PrivateIsGamepadPlatformMatch(platform) is true, and skips lines with no platform field. The
  Windows LAUNCHER.md (lines 601-604) says the mapping is "kept in gamecontrollerdb.txt ... which
  the window loads as it opens its pads".
- **Verified:** windows-x86_64/src/core/padmap.c:41 is identical to linux-x86_64's and always writes
  platform:Linux. windows padwizard.c:195 reloads the file with SDL_AddGamepadMappingsFromFile. The
  bundled SDL3 (windows build dep_sdl3 src/joystick/SDL_gamepad.c:2434-2444) adds a line only when
  SDL_PrivateIsGamepadPlatformMatch(platform) passes, and that compares the field to
  SDL_GetPlatform() ("Windows"). Nothing in the Windows src rewrites the field. The window's pads
  are unmapped after a restart, while controller.c:130 still calls the pad 'the launcher's mapping'.
- **Proof:** SDL_gamepad.c:2392-2402: strncasecmp(platform, SDL_GetPlatform(), len). Line 2441: the
  mapping is added only if that matches. grep finds no other 'platform:' in windows-x86_64/src.

### LCORE.3 The launcher still seeds [Engine.DXEngineWindowSystem] WindowSystem=SDL2, a key the
engine no longer reads or writes

- **State:** confirmed, low (contract)
- **Where:** `deusex-launcher/linux-x86_64/src/core/engine_sections.c:116-119`,
  `deusex-launcher/linux-x86_64/src/core/engine_sections.h:7-9`,
  `deusex-launcher/linux-x86_64/src/core/profile.c:17-20`
- **What:** Every profile gets WindowSystem=SDL2 written into its DeusEx.ini, a Windows profile too,
  and the import copies a DXE-DeusEx.ini's DXEngineWindowSystem section. DXEngine commit d4bc3be2
  removed the key ("Neither read nor written ... The launcher's seed of the key (bugs.md X1.12) is
  for its own round"). The header still says the engine's first clean exit writes it. The key is
  dead and the comment contradicts the engine.
- **Evidence:** engine_sections.c:116-117 `if (!dxl_ini_get(system, "Engine.DXEngineWindowSystem",
  "WindowSystem")) dxl_ini_set(..., "SDL2")`. engine_sections.h:7-9: "its first clean exit writes
  all 40 keys and [Engine.DXEngineWindowSystem] WindowSystem (DXEngine's Engine.cpp...)". `grep -rn
  WindowSystem DXEngine/DXEngine` finds nothing that reads or writes it. bugs-recovered.md:895 cites
  only the engine's half of X1.12.
- **Verified:** engine_sections.c:116-119 still seeds WindowSystem=SDL2, and profile.c:19 lists
  Engine.DXEngineWindowSystem among the engine's sections. DXEngine d4bc3be2 is in HEAD and in the
  pin b6e8d763, and it removed the key: grep finds no DXEngineWindowSystem in DXEngine/DXEngine.
  LAUNCHER.md, as swept in 2b6a0075, says the engine's sections are three, which contradicts the
  code's four and engine_sections.h:7-9. The key is dead and harmless. bugs-recovered cites only the
  engine's half of X1.12.
- **Proof:** git merge-base --is-ancestor d4bc3be2 b6e8d763 -> true. grep -rn DXEngineWindowSystem
  DXEngine/DXEngine -> no hits. tests/test_engine_sections.c:25 still expects SDL2.

### LCORE.4 A stub DeusEx.ini put over Default.ini collapses repeated keys into the base's last line

- **State:** confirmed, low (wrong-result)
- **Where:** `deusex-launcher/linux-x86_64/src/core/ini.c:512-519`,
  `deusex-launcher/linux-x86_64/src/core/profile.c:458-464`,
  `deusex-launcher/linux-x86_64/src/core/profile.c:402-409`
- **What:** dxl_ini_overlay calls dxl_ini_set for each of the source's pairs, and dxl_ini_set
  replaces the last line of that key. If a game DeusEx.ini has no Paths but has repeated keys
  (ServerActors, Suppress, ServerPackages), each of its lines overwrites Default.ini's last line of
  that key in turn. The profile ends up with the base's other lines plus only the stub's last value,
  and its own other values are lost. LAUNCHER.md says the stub's values are put over Default.ini.
- **Evidence:** ini.c:516 `else if (l->kind == LINE_PAIR) dxl_ini_set(dst, section, l->name,
  l->value);`. dxl_ini_set (ini.c:417-426) goes through find_pair, which returns the last
  occurrence. Default.ini has 5 [DeusEx.DeusExGameEngine] ServerActors lines and 10 [Core.System]
  Suppress lines. For example, a stub with ServerActors=A, B and C gives the base's first 4 lines
  plus C, so A and B are lost.
- **Verified:** dxl_ini_overlay (ini.c:512-519) calls dxl_ini_set for each pair. dxl_ini_set
  (ini.c:408-417) uses find_pair, which returns the last match (ini.c:300-310) and replaces it in
  place. So the repeated keys of a stub with no Paths each overwrite Default.ini's last line of that
  key. Only the stub's last value survives, beside the base's other lines. Both callers are
  affected: profile.c:402-409 (the mod's) and profile.c:458-464 (the import). The original has no
  overlay, so this is the launcher's own behaviour falling short of what LAUNCHER.md:113 describes.
- **Proof:** Base [S] K=1,K=2 with overlay K=a,K=b: set(K,a) replaces line K=2 with a, then set(K,b)
  replaces the same last line with b. The result is {1,b}.

### LCORE.5 Profiles' save count counts names the Saves list and the engine do not take

- **State:** confirmed, low (wrong-result)
- **Where:** `deusex-launcher/linux-x86_64/src/core/profile.c:135-146`,
  `deusex-launcher/linux-x86_64/src/core/saves.c:12-28`
- **What:** dxl_profile_count_saves counts every entry, files included, whose name starts with Save
  and a digit in any case, plus quicksave in any case. dxl_saves_list only takes folders named
  exactly Save%04d or QuickSave, the engine's own names. A folder like Save010 or save0001, or a
  stray file Save1.bak, makes Profiles say "N saves" while Saves lists fewer or none and Continue
  has nothing to load.
- **Evidence:** profile.c:141 `dxl_strnicmp(e->d_name, "Save", 4) == 0 && e->d_name[4] >= '0' ...`
  with no folder check. saves.c:17-25 is case-sensitive, requires the canonical Save%04ld name, and
  saves.c:64 requires a folder. profiles.c:52 shows the count.
- **Verified:** profile.c:134-145 counts every directory entry, files included, named Save+digit in
  any case, plus QuickSave in any case. dxl_saves_list (saves.c:12-28, 60-66) takes only folders
  named exactly Save%04d or QuickSave. profiles.c:52-55 shows the count. So stray entries make
  Profiles show more saves than the Saves list holds.
- **Proof:** Save0001.txt: strnicmp matches and the count goes up, but dxl_save_slot_of fails (*end
  != 0). save0002: the count goes up, but strncmp is case-sensitive, so it is rejected. Count 2,
  list 0.

## LAPP: the desktop launcher's app, processes and POSIX platform

### LAPP.1 No window, or a stop by signal, counts as the player quitting a screen and deletes
Running.ini

- **State:** confirmed, low (differs-from-original)
- **Where:** `deusex-launcher/linux-x86_64/src/app/run.c:892`,
  `deusex-launcher/linux-x86_64/src/app/run.c:960`,
  `deusex-launcher/linux-x86_64/src/app/run.c:671`,
  `deusex-launcher/linux-x86_64/src/screens/window.c:860`,
  `deusex-launcher/linux-x86_64/src/screens/window.c:877`
- **What:** Also: on a signal the window opens briefly on Recovery during a logout before it quits.
- **Evidence:** window.c:860 `if (open_window(w, r) != 0) return 0;`, and :877 `if (w->stop &&
  *w->stop) s->ui.in.quit = 1;` gives result 0 (shell.c:933). run.c:894-897 `if (!ui->view(...))
  {... end_without_game(r); goto done;}` with rc 0. run.c:962-966 calls end_without_game after a
  Recovery view that returns 0. LAUNCHER.md: a run that is not clean 'leaves Running.ini', and
  Recovery opens 'at the next start that finds Running.ini'. launch-flow.md removes it only on a
  real wizard cancel.
- **Verified:** window.c:860 returns 0 when open_window fails (SDL_Init/TTF_Init/CreateWindow,
  :697-718), the same value as a quit; :877 turns *stop into ui.in.quit, so shell.c:933 sets result
  0. run.c:892-897 takes any 0 from view() as the player's quit: session saved, end_without_game
  removes Running.ini, rc stays 0. After an unclean run (stopped by signal), run.c:960-966 opens
  Recovery, which quits at once on *stop, then calls end_without_game. LAUNCHER.md:332 says a run
  that is not clean leaves Running.ini; only a player's quit on a screen should take it away.
- **Proof:** Code path: Running.ini present -> decide sets view Recovery -> ui->view -> open_window
  returns -1 -> view returns 0 -> run.c:895-897 dxl_session_save, end_without_game (r->instance
  held, sentinel exists) -> dxl_sentinel_remove; rc 0. A signal during play: dxl_play stops the
  game, played.clean=0 -> run.c:962 view(RECOVERY) -> window opens, loop sets quit at once from
  *stop -> returns 0 -> run.c:966 end_without_game.

### LAPP.2 An inherited SIGCHLD=SIG_IGN makes the launcher wait forever after the engine exits

- **State:** confirmed, low (error-handling)
- **Where:** `deusex-launcher/linux-x86_64/src/platform/posix/process.c:191`,
  `deusex-launcher/linux-x86_64/src/platform/posix/process.c:255`,
  `deusex-launcher/linux-x86_64/src/platform/posix/startup.c:284`
- **What:** reap() reports an end only when waitpid returns the pid. When SIGCHLD is ignored, as a
  parent that ignores it passes on across exec, the kernel reaps the engine itself and waitpid
  returns -1/ECHILD. dxl_game_wait then never returns DXL_GAME_ENDED: dxl_play loops once a second
  forever, the window never comes back, and the run is never recorded. dxl_startup sets
  SIGINT/TERM/HUP/PIPE but never resets SIGCHLD to SIG_DFL.
- **Evidence:** process.c:194-195 `pid_t r = waitpid(g->os->pid, &st, WNOHANG); if (r != g->os->pid)
  return 0;` treats ECHILD like 'still running'. play.c:186 breaks only on DXL_GAME_ENDED.
  startup.c:290-293 has no SIGCHLD reset. Ignored dispositions survive execve on Linux.
- **Verified:** reap() (process.c:191-200) reports an end only when waitpid returns the pid.
  -1/ECHILD is taken as 'still running'. dxl_startup (startup.c:17-26) never resets SIGCHLD, and
  nothing in src/ sets it. When SIGCHLD is ignored, the child is reaped on its own, so dxl_game_wait
  never returns DXL_GAME_ENDED and play.c:186 loops forever, once a second. The --version check
  passes (process.c:428-430: st stays 0, so code 0), so the launcher does reach play. It needs a
  parent that ignores SIGCHLD, which is rare.
- **Proof:** Probe: a process sets SIGCHLD to SIG_IGN and execs itself. The new image forks
  /bin/true and polls waitpid(pid,WNOHANG): 'waitpid=0', then 'waitpid=-1 errno=No child processes'
  every time after. So the ignored disposition survives execve, and the pid is never returned.

### LAPP.3 The 5 s countdown to SIGKILL counts wake-ups, not time

- **State:** confirmed, low (wrong-result)
- **Where:** `deusex-launcher/linux-x86_64/src/app/play.c:187`,
  `deusex-launcher/linux-x86_64/src/app/play.c:191`
- **What:** After the stop, each dxl_game_wait return with no line adds STOPPING_MS (250) to
  since_term, however long it actually waited. A poll cut short by EINTR (repeated Ctrl-C/SIGTERM)
  or by a second start's datagram (DXL_GAME_EXTRA) still counts 250 ms. An engine still quitting
  cleanly, writing its inis, can be SIGKILLed well before 5 s. A run then marked Killed may lose its
  settings.
- **Evidence:** play.c:191-192 `else if (out->stopped && !out->killed && !(ev & DXL_GAME_LINE)) { if
  ((since_term += STOPPING_MS) >= DXL_PLAY_KILL_AFTER_MS)`. dxl_game_wait returns 0 at once on poll
  EINTR (process.c:237-256), since startup.c installs the handlers without SA_RESTART. LAUNCHER.md:
  'SIGKILL 5 s later'.
- **Verified:** play.c:191-192 adds STOPPING_MS for every wake with no line, whatever the time
  spent. A wake can be cut short: poll gets EINTR because the handlers have no SA_RESTART
  (startup.c:21), and dxl_game_wait then returns 0. A forwarded datagram returns DXL_GAME_EXTRA
  early, since the instance still listens while the game stops. Each such wake still counts 250 ms.
  So 20 quick wakes reach DXL_PLAY_KILL_AFTER_MS long before 5 s, against LAUNCHER.md:673-675
  ('SIGKILL 5 s later'). This needs repeated signals or forwarded starts during the stop.
- **Proof:** Code path: stopped=1, poll returns -1 EINTR -> r<0, seen=0, reap 0 -> ev=0 ->
  play.c:191 since_term+=250. The same holds for ev==DXL_GAME_EXTRA.

### LAPP.4 A first run or --game repoint is lost from launcher.ini when the engine check or the mod
check stops the run

- **State:** confirmed, low (error-handling)
- **Where:** `deusex-launcher/linux-x86_64/src/app/run.c:826`,
  `deusex-launcher/linux-x86_64/src/app/run.c:938`,
  `deusex-launcher/linux-x86_64/src/app/run.c:929`,
  `deusex-launcher/linux-x86_64/src/app/session.c:123`
- **What:** Only a first run with no window (dxl-cli --game) leaves profile.ini naming an install
  that launcher.ini lacks, when the engine check stops the run. The window's first run saves in
  close_window (window.c:640). A --game repoint is merely not kept: profile.ini and launcher.ini are
  both unsaved, so they stay consistent.
- **Evidence:** session.c:129-133: remember_install changes s->ini, while profile_save writes
  profile.ini. run.c:938-941 `if (check_engine(r,&v) != 0) { rc = DXL_RUN_EXIT_NOGAME; goto done; }`
  comes before commit()'s dxl_session_save (run.c:691). The import-only path's comment at run.c:831
  names this very state as 'not made'.
- **Verified:** This is real for a first run with no window. first_run -> dxl_session_make_profile
  saves profile.ini with Install=<id> (session.c:130-133), but the [Install.<id>] Path and
  [Launcher] Profile go only into s->ini in memory. check_engine failing (run.c:938-941) jumps to
  done; dxl_session_close (session.c:59-67) does not save. The next start finds the profile, but
  dxl_session_install_path returns NULL, so it exits 4. A repoint alone does not leave the two files
  out of step: repoint's dxl_profile_set is in memory only too, so both changes are lost together. A
  missing mod cannot follow a no-window first run, because --mod needs --import, which saves.
- **Proof:** Code path: dxl-cli --play --game=X with a missing DXL_ENGINE and no profile ->
  first_run (no ui) -> make_profile -> profile_save -> run.c:938 check_engine -1 -> goto done (no
  session_save). Next start: dxl_session_check_install -> path NULL -> -1 -> 'has no install to
  play', exit 4.

### LAPP.5 After a RELAUNCH the launcher keeps the INI=/USERINI= config while the engine reads the
profile's inis

- **State:** confirmed, low (contract)
- **Where:** `deusex-launcher/linux-x86_64/src/app/run.c:975`,
  `deusex-launcher/linux-x86_64/src/app/run.c:464`, `deusex-launcher/linux-x86_64/src/app/run.c:856`
- **What:** open_config picks the ini from INI=/USERINI= in r->words, once. A relaunch replaces
  r->words with the engine's line ('-changevideo', or 'map?...?-server?log=server.log'), which has
  no INI=, and r->config is not reopened. The decision, FirstRun, the seeded sections, the save and
  the reload therefore go to the INI= file, while the engine started with --ini=<profile>/DeusEx.ini
  and no INI= reads the profile's. The original's relaunched DeusEx.exe drops INI= for both sides.
- **Evidence:** run.c:980-984 frees and replaces r->words, sets relaunched, then continues.
  r->config was opened only at run.c:856 (or by dxl_run_use_profile). engine_argv passes
  p->system_ini as --ini (run.c:277), and the relaunch line comes from DeusEx.u's
  `ConsoleCommand("RELAUNCH -changevideo")`.
- **Verified:** open_config (run.c:464-472) picks INI=/USERINI= from r->words, at run.c:856 only.
  The relaunch at run.c:975-984 replaces r->words with the engine's line, which has no INI=, and
  does not reopen r->config. The engine is always passed --ini=<profile>/DeusEx.ini (run.c:277), and
  its --cmdline (r->words) now has no INI=, so it reads the profile's ini. decide, Graphics/Settings
  (settings.c:47-75), commit's seeding, the FirstRun clamp and the save, and the reload all still
  use the INI= file. The original's RELAUNCH starts deusex.exe with only the rest of the line
  (core-dll.md:412-424), so both of its sides go to the default ini. Effect: a -changevideo Graphics
  choice is written to the mod ini, which the relaunched engine does not read.
- **Proof:** Code path: words 'INI=Mod.ini' -> r->config on Mod.ini. Clean end with relaunch
  '-changevideo' -> r->words='-changevideo', continue -> decide(r) and Graphics edit Mod.ini ->
  commit saves Mod.ini -> engine_argv --ini=profile DeusEx.ini, --cmdline '-changevideo'.

## LUI: the desktop launcher's UI

### LUI.1 A Make the profile press left waiting makes a later profile, and starts the game, with no
press

- **State:** confirmed, medium (wrong-result)
- **Where:** `deusex-launcher/linux-x86_64/src/screens/firstrun.c:362`,
  `deusex-launcher/linux-x86_64/src/screens/firstrun.c:366`,
  `deusex-launcher/linux-x86_64/src/screens/firstrun.c:411`,
  `deusex-launcher/linux-x86_64/src/screens/shell.h:324`
- **What:** If Make the profile is pressed while the GPU probe is still running (probe_state 1),
  make_waits is set. Back then leaves the page with make_waits still 1, because nothing clears it.
  The next time Your profile is drawn after the probe has answered, make is forced to 1 on the first
  frame. That can be another install chosen on The game, or a mod's or new profile's import. The
  profile is then made with the default name and choices. On the first run s->result=1 also starts
  the game. The player never pressed Make on that page.
- **Evidence:** firstrun.c:362-369: `if (make && s->probe_state == 1) { s->make_waits = 1; make = 0;
  } if (s->make_waits && s->probe_state != 1) { s->make_waits = 0; make = 1; }`. Back at 411-413
  (dxl_shell_go FIND, or dxl_shell_making_cancel) does not reset make_waits. A grep shows make_waits
  is written only in firstrun.c. The probe is started by this page's first frame (dxl_shell_probe,
  line 259) and runs dxl-cli --probe with a 10 s timeout (window.c:261), so the window to hit this
  is real. Also, if SDL_CreateThread fails (window.c:280), probe_state stays 1 for good and Make
  never acts.
- **Verified:** deusex-launcher/linux-x86_64/src/screens/firstrun.c:362-369 sets make_waits while
  probe_state==1, and only clears it on a later frame of this same page. Back at 411-413 goes to
  dxl_shell_go(FIND) or dxl_shell_making_cancel (profiles.c:127). Neither touches make_waits, and
  neither does dxl_shell_go (shell.c:329-359) or dxl_shell_enter. grep finds it written only in
  firstrun.c. The next draw of the import page once probe_state==2 sets make=1 on its first frame,
  then makes the profile and sets s->result=1 (first run) or goes Home (s->making).
- **Proof:** grep -rn make_waits src: firstrun.c:363,366,367,370 and shell.h:324 only.
  window.c:274-281: when SDL_CreateThread fails, probe_thread is NULL, probe_collect (286) returns
  early and probe_state stays 1. In that case Make only ever sets make_waits.

### LUI.2 Clicking the foot's Cancel hint during a key capture binds LeftMouse instead of cancelling

- **State:** confirmed, low (docs)
- **Where:** `deusex-launcher/linux-x86_64/src/ui/kit.c:460`,
  `deusex-launcher/linux-x86_64/src/screens/shell.c:820`,
  `deusex-launcher/linux-x86_64/src/ui/glyphs.c:236`,
  `deusex-launcher/linux-x86_64/src/ui/controls.c:511`,
  `deusex-launcher/linux-x86_64/src/screens/settings.c:383`
- **What:** While a Keyboard & mouse slot is capturing, the foot shows a clickable Cancel hint.
  LAUNCHER.md says 'a hint clicked does what its key does'. A click on it is never seen as a click,
  though: the kit takes the left button press as the captured input. So the slot is bound to
  LeftMouse, or, since the game's defaults bind LeftMouse to ParseLeftClick|Fire, the 'bound to
  Fire... Instead / Both' dialog comes up, instead of the capture being cancelled.
- **Evidence:** kit.c:460-466: with capture_id set, a MOUSE_BUTTON_DOWN sets
  in.captured=DXL_CAPTURED_MOUSE and breaks, so in.pressed and in.released are never set. dxl_hints
  (glyphs.c:236) depends on dxl_ui_clickable, which needs pressed and released. hints_of
  (shell.c:820-822) still offers {DXL_HINT_BACK, "Cancel"} while capturing. dxl_key_slot
  (controls.c:511-514) returns DXL_SLOT_CAPTURED. captured() (settings.c:387) maps it with
  dxl_key_from_mouse(1) and either binds it or opens the conflict confirm.
- **Verified:** kit.c:460-466: while capture_id is set, a button down sets in.captured=MOUSE and
  breaks before pressed/released are set, so dxl_hints' dxl_ui_clickable (glyphs.c:236) never
  activates. shell.c:820-822 still draws a clickable 'Cancel' hint, and LAUNCHER.md:414-415 says 'a
  hint clicked does what its key does'. A click there reaches dxl_key_slot as DXL_SLOT_CAPTURED and
  captured() maps it to dxl_key_from_mouse(1). LAUNCHER.md:525 says a slot takes the next mouse
  button, so the clash is between the hint and that rule.
- **Proof:** Code path: kit.c:460 capture branch, controls.c:511-514 CAPTURED, settings.c:387
  DXL_CAPTURED_MOUSE, then a bind or the 'keys conflict' confirm.

### LUI.3 Picking a controller from the list chooses the last one of the same name

- **State:** confirmed, low (wrong-result)
- **Where:** `deusex-launcher/linux-x86_64/src/screens/controller.c:286`,
  `deusex-launcher/linux-x86_64/src/screens/controller.c:288`,
  `deusex-launcher/linux-x86_64/src/screens/controller.c:292`
- **What:** With two controllers of the same model, which SDL gives the same name, the Controller
  row's list cannot reach the first one. The pick answer is turned back into an index by comparing
  names with no break, so the last pad of that name is always chosen. Choosing the first pad shows
  the second pad, or changes nothing if the second is already shown. Its model, details, mapping
  wizard and rumble then apply to the wrong pad.
- **Evidence:** controller.c:287-289: `const char *picked = dxl_shell_picked(s, dxl_shell_answer(s,
  which_id)); for (int i = 0; picked && i < count; i++) if (!strcmp(names[i], picked)) index = i;`.
  names[i] is pads[i].name, from SDL_GetGamepadNameForID (line 49), which is the same for identical
  pads. The answer's own index is thrown away.
- **Verified:** controller.c:287-289 loops over every name with no break and keeps the last strcmp
  match. names[i] come from SDL_GetGamepadNameForID (line 49), which is the same for identical
  models. An answer of 0 with two pads of the same name gives index=1, and the answer's own index is
  thrown away.
- **Proof:** Traced by hand: names={"Xbox Controller","Xbox Controller"}, picked=names[0], the loop
  gives i=0 index=0, then i=1 index=1. pad_id becomes pads[1].id.

### LUI.4 After -changevideo's Graphics, Settings opened from Home still plays the game on Done

- **State:** confirmed, low (docs)
- **Where:** `deusex-launcher/linux-x86_64/src/screens/settings.c:544`,
  `deusex-launcher/linux-x86_64/src/screens/settings.c:564`,
  `deusex-launcher/linux-x86_64/src/screens/shell.c:298`
- **What:** In the Graphics view, Esc goes to Home. Settings opened from Home afterwards still
  labels its Done 'Play' and starts the game, because the check uses the view the window was entered
  with, not how the page was reached. Elsewhere LAUNCHER.md has Settings' Done return Home; only
  -changevideo's Graphics has Done as Play.
- **Evidence:** settings.c:544: `int graphics = s->view == DXL_VIEW_GRAPHICS;`. Lines 564-566: `if
  (done && graphics) { dxl_settings_keep(s); s->result = 1; }`. s->view is set only in
  dxl_shell_enter (shell.c:298), so it stays GRAPHICS for the whole view, Home and its pages
  included.
- **Verified:** settings.c:544 tests s->view, which only dxl_shell_enter sets (shell.c:298). In the
  Graphics view, Back goes Home (settings.c:576-577). Settings opened from Home then still labels
  Done 'Play' (547) and sets result=1 (564-566). LAUNCHER.md:361-362 and 866 give Play only to the
  Graphics tab that -changevideo opens; elsewhere Done goes Home. The game starting from Home anyway
  makes this a small inconsistency.

### LUI.5 The log page's reader skips its allocation and ftell checks

- **State:** confirmed, low (error-handling)
- **Where:** `deusex-launcher/linux-x86_64/src/screens/log.c:44`,
  `deusex-launcher/linux-x86_64/src/screens/log.c:59`,
  `deusex-launcher/linux-x86_64/src/screens/log.c:63`
- **What:** load() uses fseek and ftell without checking them. If ftell gives -1, size-from is -1,
  so it mallocs 0 bytes and freads (size_t)-1 into that buffer. log_starts is malloc'd and realloc'd
  with no NULL check, and realloc's result overwrites the old pointer, so running out of memory on a
  large log (up to 4 MB) dereferences NULL. The rest of the launcher uses dxl_xmalloc and
  dxl_xrealloc, and bugs-recovered L2.20 fixed the same unchecked ftell in the last-run tail reader,
  but not here.
- **Evidence:** log.c:44-48: `fseek(f, 0, SEEK_END); long size = ftell(f), from = ...; s->log_text =
  malloc((size_t)(size - from) + 1); ... fread(..., (size_t)(size - from), f)`. Lines 59-65:
  `s->log_starts = malloc(...)`, then `s->log_starts = realloc(...)`, then
  `s->log_starts[s->log_lines++] = ...` with no NULL check.
- **Verified:** log.c:44-48: fseek and ftell are unchecked. If ftell gives -1: size=-1, from=0,
  malloc(0), fread of (size_t)-1, and a write to log_text[n] past a 0-byte buffer. log.c:59-65:
  malloc and realloc of log_starts are unchecked, and realloc overwrites the pointer, so OOM
  dereferences NULL. The rest of src uses dxl_xmalloc/dxl_xrealloc (core/argv.c), and L2.20
  (5b92ec10) fixed the same thing only in the last-run tail. Hard to trigger: the paths are fixed
  log files, and ftell on a regular file rarely fails.

## LPORT: the Windows and Smart Pro ports' own code

### LPORT.1 Windows: a second start waits 30 s before playing beside a -server launcher (or one
whose pipe failed)

- **State:** confirmed, medium (differs-from-original)
- **Where:** `deusex-launcher/windows-x86_64/src/platform/win32/instance.c:140`,
  `deusex-launcher/windows-x86_64/src/platform/win32/instance.c:145`,
  `deusex-launcher/windows-x86_64/src/platform/win32/instance.c:84`,
  `deusex-launcher/windows-x86_64/src/app/run.c:799`,
  `deusex-launcher/windows-x86_64/docs/LAUNCHER.md:306`
- **What:** A primary that holds only the lock has no pipe. That is -server's instance
  (dxl_instance_acquire_lock), or one whose CreateNamedPipeW failed. dxl_instance_other_running
  still finds its mutex, so run.c forwards with HANDOFF_TIMEOUT_MS = 30000. CreateFileW on the
  missing pipe returns ERROR_FILE_NOT_FOUND, which the loop retries with Sleep(20) until the 30 s
  deadline. Only then does the second start go on to play beside the server. POSIX's sendto fails at
  once. The original starts at once when it finds no IsBrowser window.
- **Evidence:** instance.c:145: `if ((e != ERROR_PIPE_BUSY && e != ERROR_FILE_NOT_FOUND) || now >=
  deadline)` fails; otherwise line 151 calls `Sleep(20)` and the loop goes on. acquire(id,0) creates
  the same Local\deusex-launcher.<hash> mutex but no pipe (line 84: `if (listen)`). run.c:799-800
  forwards whenever `busy`. LAUNCHER.md:306-308 says a -server instance "holds the lock alone and
  takes no line ... a second start plays beside it". linux-x86_64 posix/instance.c:184 uses a single
  sendto that fails immediately.
- **Verified:** run.c:663 hold_instance takes dxl_instance_acquire_lock(id) for -server:
  acquire(id,0) creates the Local\deusex-launcher.<hash> mutex and no pipe (instance.c:84). A second
  start without a bypass word gets busy=1 from dxl_instance_other_running (instance.c:120-127, the
  mutex alone), and run.c:799-800 calls forward with HANDOFF_TIMEOUT_MS=30000 (run.c:26). In forward
  (instance.c:140-152), CreateFileW on the missing pipe returns ERROR_FILE_NOT_FOUND, which line 145
  retries with Sleep(20) until the deadline. Only then does it fall through to play.
  LAUNCHER.md:306-308 promises that it plays beside the server. POSIX's sendto fails at once.
- **Proof:** Path: run.c:790 busy=1 -> run.c:800 dxl_instance_forward(...,30000) -> instance.c:141
  CreateFileW fails with ERROR_FILE_NOT_FOUND -> :145 not taken while now<deadline -> :151
  Sleep(20), repeated for about 30 s -> -1 -> run.c:807 open_log and the start goes on.

### LPORT.2 Windows 1903/1909 (the stated floor): a refused TCP connect waits its whole timeout,
reported as 'no answer'

- **State:** confirmed, low (platform)
- **Where:** `deusex-launcher/windows-x86_64/src/platform/win32/net.c:126`,
  `deusex-launcher/windows-x86_64/src/platform/win32/net.c:157`,
  `deusex-launcher/windows-x86_64/ports/windows-x86_64/port.cmake:8`
- **What:** dxl_tcp_connect waits for a non-blocking connect with WSAPoll(POLLWRNORM). Before
  Windows 10 2004 (build 19041), WSAPoll does not report a failed connect: it returns 0 at the
  timeout. The port supports 1903 (10.0.18362). There a refused or unreachable master server or
  server costs the full timeout and is logged as "no answer", not as the error.
- **Evidence:** net.c:126-130: wait_for() is WSAPoll alone. net.c:162-166: ready==0 is taken as "no
  answer". port.cmake:8 sets `DXL_PORT_WINDOWS_MIN 10.0.18362`. Microsoft's WSAPoll fix for
  failed-connect reporting came in Windows 10 2004. On those builds select() with an exceptfds set,
  or WSAConnect plus WSAEventSelect(FD_CONNECT), would report the failure.
- **Verified:** wait_for (net.c:126-130) is WSAPoll alone, and dxl_tcp_connect (net.c:149-170) takes
  ready==0 as 'no answer'. Microsoft's WSAPoll docs say a failed TCP connect is reported
  (POLLHUP|POLLERR|POLLWRNORM) only as of Windows 10 2004. Earlier builds report nothing, so the
  call times out. port.cmake:8 sets the floor at 10.0.18362 (1903). On 1903 and 1909 a refused
  connect therefore costs the whole timeout and is logged as 'no answer'. Not run: there is no 1903
  machine, and Wine does not show the bug.
- **Proof:** net.c:162 `int ready = wait_for(s, POLLWRNORM, timeout_ms);` then :165 `if (ready == 0)
  ... "no answer"`. Microsoft's WSAPoll remarks: 'As of Windows 10 version 2004, when a TCP socket
  fails to connect, (POLLHUP | POLLERR | POLLWRNORM) is indicated.'

### LPORT.3 Windows launcher permanently switches the user's console to code page 65001

- **State:** confirmed, low (platform)
- **Where:** `deusex-launcher/windows-x86_64/src/platform/win32/startup.c:363`,
  `deusex-launcher/windows-x86_64/src/platform/win32/startup.c:370`
- **What:** Location is deusex-launcher/windows-x86_64/src/platform/win32/startup.c:72-80
  (SetConsoleOutputCP at line 79), not 363/370.
- **Evidence:** startup.c:363-371: `__attribute__((constructor)) static void startup_early(void) {
  ... AttachConsole(ATTACH_PARENT_PROCESS) ... SetConsoleOutputCP(CP_UTF8); }`. No
  GetConsoleOutputCP is saved and no restore runs at exit or in dxl_startup_ended.
- **Verified:** The cited lines are wrong: startup.c has 80 lines. The constructor startup_early at
  src/platform/win32/startup.c:72-80 calls SetConsoleOutputCP(CP_UTF8) on every start, and on the
  parent's console after AttachConsole. The output code page belongs to the console, so the user's
  cmd.exe keeps 65001 after the program exits. Nothing in startup.c saves the old code page (no
  GetConsoleOutputCP) or restores it, nor does dxl_startup_ended (line 66-68).
- **Proof:** grep across src/docs: ConsoleOutputCP appears only at startup.c:79. No
  GetConsoleOutputCP or restore anywhere.

### LPORT.4 test_trash_win32 waits for ever under this machine's Wine: its file "can't be sent to Trash"

- **State:** confirmed, medium (test)
- **Where:** `deusex-launcher/windows-x86_64/src/platform/win32/trash.c`,
  `deusex-launcher/windows-x86_64/ports/windows-x86_64/port.sh` (`port_test`)
- **What:** Under `scripts/dx.sh test windows-x86_64` `test_trash_win32` most often never ends
  (two runs of three on 2026-10-29, the third passing): Wine's shell puts up "Confirm file
  deletion", "The item 'C:\users\steamuser\AppData\Local\Temp\dxl-trash-256\stuff\Save0001'
  can't be sent to Trash. Do you want to delete it instead?", which no one answers. The test's
  trash folder (`XDG_DATA_HOME`, `build/windows-x86_64/wine-xdg`) holds that test's earlier moves,
  the last on 2026-10-28; an empty folder in its place waits the same, so the cause is in this
  machine's Wine since then, not the folder's state. The launcher's code did not change in between.
- **Evidence:** the dialog captured on the desktop while the test runs alone (`ctest -R
  '^test_trash_win32$'`, bounded at 60 s and 120 s, both timing out); the prefix's `Temp` and the
  trash folder on one file system; the trash's `files` with `Save0001` and
  `Save0001-3973dd20`, no newer entry, its `info` folder touched by each try and left without a
  new file.
- **Proof:** the run's log and the captured dialog.

### LPORT.5 test_profile and test_ui_shots_1920x1080big fail or hang at times in Wine's parallel runs

- **State:** confirmed, low (test)
- **Where:** `deusex-launcher/windows-x86_64/ports/windows-x86_64/port.sh` (`port_test`),
  `deusex-launcher/windows-x86_64/tests/CMakeLists.txt`
- **What:** Of three runs of the Wine suite on 2026-10-29, the launcher's code unchanged between
  them, one had `test_profile` fail after 0.03 s and another had `test_ui_shots_1920x1080big` never
  end, each passing in the other runs. Alone, both pass. So a test started beside others under
  this machine's Wine can fail or hang, which LPORT.4's dialog does not explain. A failure that
  fast looks like the program not starting rather than a check failing; its output was not kept.
- **Evidence:** run 1 (`scripts/dx.sh test windows-x86_64`): `test_profile` Failed in 0.03 s, the
  other 50 passed. Run 2 (`port_test`'s steps, ctest bounded at 900 s): `test_trash_win32` alone
  never ended. Run 3 (the same): `test_trash_win32` and `test_ui_shots_1920x1080big` never ended.
  Alone (`ctest -R`): `test_profile` passed in 0.47 s, `test_ui_shots_1920x1080big` in 24.63 s.
- **Proof:** the three runs' logs; the next one to fail with `--output-on-failure` should say why.

## MAIN: the recreation: deusex-launcher's main

### MAIN.1 Renderer page's detection run gets a mangled command line: wrong DescFlags section, a log
named Detected.log"

- **State:** confirmed, medium (differs-from-original)
- **Where:** `deusex-launcher/main/src/wizard/wizard.c:27`,
  `deusex-launcher/main/src/wizard/wizard.c:305`, `deusex-launcher/main/src/core/argv.c:5`,
  `deusex-launcher/main/src/core/cmdline.c:28`, `deusex-launcher/main/src/core/policy.c:47`,
  `deusex-launcher/main/src/launch/launch.c:88`, `deusex-launcher/main/src/core/renderdev.c:182`
- **What:** The detection run starts the launcher with DETECT_FLAGS as one argv word. The child's
  dxl_cmdline_join quotes that word after its first '=', so the child's test device becomes
  "D3DDrv.D3DRenderDevice log=Detected.log" and its log becomes `Detected.log"`. DescFlags=2 then
  goes to a junk section, `[D3DDrv.D3DRenderDevice log=Detected.log]`, and the real
  `[D3DDrv.D3DRenderDevice]` is never marked incompatible. With a DeusEx.ini where D3D already has
  DescFlags=1 (written by the original's detection), Direct3D stays certified, is chosen by default
  and the Driver page is shown, instead of Software.
- **Evidence:** dxl_argv_line passes the line as argv[1]. quoted_word: the word holds a space, no
  quote and no leading blank, so it is quoted after the '='. In a scratch build, joining
  {"DeusEx","testrendev=D3DDrv.D3DRenderDevice log=Detected.log"} prints: joined
  `testrendev="D3DDrv.D3DRenderDevice log=Detected.log"`, test_rendev=`D3DDrv.D3DRenderDevice
  log=Detected.log`, LOG=`Detected.log"`. The original's ShellExecute gives two tokens (wizard.md
  Renderer; ini-keys.md DescFlags). renderdev.h says every device tested stays incompatible.
  test_launch.c:289 passes two argv words, so the tests miss this.
- **Verified:** wizard.c:305 passes DETECT_FLAGS to dxl_child_start. posix/process.c:71
  dxl_argv_line makes it argv[1], one word. The child's launch.c:308 dxl_cmdline_join runs it
  through quoted_word (cmdline.c:28): the word holds a space, no quote and no leading blank, so it
  is quoted after its '='. policy.c:47 then takes the device as the whole quoted text,
  renderdev.c:182 writes DescFlags=2 under that junk section, and log_path_of gets
  LOG=`Detected.log"`. test_launch.c:289 passes two argv words, so the tests never take this path.
- **Proof:** A scratch build of cmdline.c with argv {DeusEx, DETECT_FLAGS} printed: joined
  [testrendev="D3DDrv.D3DRenderDevice log=Detected.log"], testrendev [D3DDrv.D3DRenderDevice
  log=Detected.log], LOG [Detected.log"].

### MAIN.2 CD check rebuilds System/ by literal name: a lowercase 'system' folder fails the check
forever

- **State:** confirmed, low (wrong-result)
- **Where:** `deusex-launcher/main/src/core/install.c:14`,
  `deusex-launcher/main/src/launch/launch.c:189`
- **What:** cd_check passes dirname(base) and dxl_install_cd_ok joins "System" back on, instead of
  using the launcher's own folder. If that folder is not spelled 'System' (for example an
  innoextract --lowercase GOG install, with 'system'), the path <game>/System/..\ does not exist and
  resolve_ci cannot open it. Every launch then shows the Insert-CD prompt, which no OK can satisfy,
  and Cancel leaves Running.ini. The game cannot start, although the shipped CdPath ..\ or ..\GOTY_1
  is present.
- **Evidence:** install.c:14: sys = dxl_path_join(game_dir, "System"); base = join(sys, rel).
  dxl_path_resolve_ci("<g>/System/..","Textures"): the exact stat fails because the System component
  is missing, and opendir fails, so it returns NULL. Everywhere else the launcher uses r->base and
  resolves names in any case (config.c leaf_path, sentinel, splash). launch-flow.md §8 says the
  shipped CdPath always passes.
- **Verified:** install.c:14 joins a literal "System" onto dirname(base) instead of using base.
  dxl_path_resolve_ci(<g>/System/.., "Textures/Palettes.utx") fails when the folder on disk is
  'system'. On the original's NTFS the case does not matter, so its shipped CdPath always passes.
  Steam's and GOG's installs ship 'System', so only a re-cased install (for example innoextract
  --lowercase) hits this. That makes it an edge case: low.
- **Proof:** Probe, <tmp>/system plus <tmp>/Textures/Palettes.utx (non-empty):
  dxl_install_cd_ok(<tmp>,"..\\") returned 0. After renaming the folder to System it returned 1.

### MAIN.3 run-game.sh names DeusEx.ini and User.ini in exact case while the launcher uses whatever
case is on disk

- **State:** confirmed, low (contract)
- **Where:** `deusex-launcher/main/system/run-game.sh:32`,
  `deusex-launcher/main/src/core/config.c:21`, `deusex-launcher/main/src/core/config.c:71`,
  `deusex-launcher/main/src/core/config.c:78`
- **What:** The launcher opens <Package>.ini and User.ini through dxl_path_resolve_ci ('Installs
  vary in case'). It writes FirstRun, GameRenderDevice and the Detail settings there, then starts
  the engine with --ini="$here/$pkg.ini" --userini="$here/User.ini" in exact case. On an install
  holding deusex.ini or user.ini, the engine reads and writes different files from the ones the
  launcher configured, so the wizard's choices never reach the game. This breaks the README's
  promise that the engine plays the launcher's package ini.
- **Evidence:** config.c leaf_path: dxl_path_resolve_ci(dir, "DeusEx.ini"), else the joined name.
  run-game.sh:32: --ini="$here/$pkg.ini" --userini="$here/User.ini". The README's 'The game and the
  launcher' says run-game.sh starts the engine with the launcher's package's ini.
- **Verified:** config.c:21-30 leaf_path resolves <Package>.ini and User.ini in any case.
  run-game.sh:32 passes --ini="$here/$pkg.ini" --userini="$here/User.ini" in exact case. DXEngine
  PackageManager.cpp:1128-1136 and 1151-1159 test with fs::exists, which is exact on Linux. When the
  file is missing the engine seeds it from Default.ini or DefUser.ini and writes it at that
  exact-case name. So on an install holding only deusex.ini or user.ini, the engine plays a fresh
  file and never sees the wizard's choices.
- **Proof:** Read path: dxl_config_path gives .../deusex.ini, while run-game.sh passes
  .../DeusEx.ini. The engine's fs::exists(.../DeusEx.ini) is false, so it takes the Default.ini
  branch (PackageManager.cpp:1132-1136).

### MAIN.4 'Another instance' is probed then released before the lock is taken; a failed acquire is
ignored

- **State:** confirmed, low (race)
- **Where:** `deusex-launcher/main/src/launch/launch.c:332`,
  `deusex-launcher/main/src/launch/launch.c:366`, `deusex-launcher/main/src/launch/launch.c:373`
- **What:** other is taken from a flock that is tried and released at line 332. The lock itself is
  taken only at line 366, after the config, the strings and the splash (SDL start-up) are done. Two
  launches started close together can both see other=0. Both then empty the log (dxl_log_open_new),
  and with a stale Running.ini both open RecoveryMode. The second's dxl_instance_acquire returns
  NULL, but that result is never folded into other_instance. The original's CreateMutex check is
  atomic (ERROR_ALREADY_EXISTS).
- **Evidence:** launch.c:332 int other = dxl_instance_other_running(); instance.c:137-138 takes
  and releases the lock. Line 366 r.inst = ... acquire, and its NULL result is not used for other.
  Line 373 .other_instance = other. launch-flow.md §5.3: the mutex is made, and finding it there
  already records another live instance.
- **Verified:** launch.c:332 sets other from dxl_instance_other_running, which takes the flock and
  releases it at once (instance.c:137-140). The lock is taken only at launch.c:366, after the config
  load and the splash. A NULL result from dxl_instance_acquire is never folded into other, and line
  373 passes other unchanged. In the original, CreateMutex at InitEngine step 3 is atomic: the
  second process sees ERROR_ALREADY_EXISTS (launch-flow.md §5.3). Two launches inside that window
  both get other=0. Both then empty the log, and with a stale Running.ini both take the crash path.
  The window is narrow, so low.
- **Proof:** launch.c: nothing after line 366 reads !r.inst to set other or in.other_instance. other
  is assigned only at line 332.

## Refuted

| Area | Finding | Why refuted |
|---|---|---|
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
