# bugs.md

The second sweep's list, 2026-10-28: each area of DXEngine, DXLauncher's branches, the recreation
and
the workspace read by one agent, and every finding proven or refuted by another, independently,
read-only (no game or harness runs). 84 confirmed, 0 uncertain; 3 refuted, listed at the end
so they are not reported again.

IDs are this list's own, `<section>.<n>`; the first sweep's (`E1.1`, `L3.12`, `X2.L9`) are its
own list's, named only in the commits that cite them. A commit that fixes an item
cites it: `(bugs.md VM.1)`.

| Section | Area | Confirmed | Uncertain | Refuted |
|---|---|---|---|---|
| [VM](#vm-the-script-vm-the-object-system-and-the-garbage-collector) | the script VM, the object system and the garbage collector | 3 | 0 | 0 |
| [PKG](#pkg-packages-loading-travel-and-saves) | packages, loading, travel and saves | 5 | 0 | 0 |
| [PHYS](#phys-collision-physics-and-movement) | collision, physics and movement | 4 | 0 | 0 |
| [AI](#ai-actors-pawns-ai-navigation-animation-and-the-event-manager) | actors, pawns, AI, navigation, animation and the event manager | 5 | 0 | 0 |
| [DX](#dx-deus-exs-own-natives-the-game-conversations-flags-and-texts) | Deus Ex's own natives: the game, conversations, flags and texts | 1 | 0 | 1 |
| [EXT](#ext-the-window-tree-and-its-text-extension) | the window tree and its text (Extension) | 6 | 0 | 1 |
| [NET](#net-the-network) | the network | 3 | 0 | 0 |
| [REN](#ren-the-renderer-the-frame-the-scene-lighting-textures-and-meshes) | the renderer: the frame, the scene, lighting, textures and meshes | 7 | 0 | 0 |
| [VK](#vk-the-vulkan-device-and-dxgpu) | the Vulkan device and DXGPU | 2 | 0 | 0 |
| [GL](#gl-the-opengl-and-opengl-es-device) | the OpenGL and OpenGL ES device | 2 | 0 | 0 |
| [SWD3D](#swd3d-the-software-and-d3d11-devices) | the software and D3D11 devices | 5 | 0 | 0 |
| [AUD](#aud-the-audio) | the audio | 3 | 0 | 0 |
| [ENG](#eng-the-main-loop-startup-settings-input-and-the-engines-utilities) | the main loop, startup, settings, input and the engine's utilities | 6 | 0 | 0 |
| [WIN](#win-the-window-system-dxwidgets-and-the-game-window) | the window system: DXWidgets and the game window | 5 | 0 | 0 |
| [LCORE](#lcore-the-desktop-launchers-core-dxlauncher) | the desktop launcher's core (DXLauncher) | 5 | 0 | 0 |
| [LAPP](#lapp-the-desktop-launchers-app-processes-and-posix-platform) | the desktop launcher's app, processes and POSIX platform | 5 | 0 | 0 |
| [LUI](#lui-the-desktop-launchers-ui) | the desktop launcher's UI | 5 | 0 | 0 |
| [LPORT](#lport-the-windows-and-smart-pro-ports-own-code) | the Windows and Smart Pro ports' own code | 3 | 0 | 1 |
| [MAIN](#main-the-recreation-deusex-launchers-main) | the recreation: deusex-launcher's main | 5 | 0 | 0 |
| [X](#x-across-the-repositories-the-launcher-engine-contract-the-scripts-ci-the-harness) | across the repositories: the launcher-engine contract, the scripts, CI, the harness | 4 | 0 | 0 |

## VM: the script VM, the object system and the garbage collector

### VM.1 A native's number past the last one registered is read off the end of NativeByIndex

- **State:** confirmed, low (crash)
- **Where:** `DXEngine/DXEngine/VM/Frame.cpp:335`, `DXEngine/DXEngine/VM/NativeFunc.cpp:19`,
  `DXEngine/DXEngine/VM/NativeFunc.cpp:43`
- **What:** CallNative indexes NativeByIndex[func->NativeFuncIndex] without a bounds check.
  NativeByIndex only grows to the highest number DXEngine registers (4003). FuncByIndex grows to any
  number a loaded class declares (up to 65535). So a mod class declaring native(N) with N > 4003 and
  no handler reads past the vector's end and calls a garbage std::function. NATIVES.md says it
  should instead stop with 'Unknown native function ...'.
- **Evidence:** Frame.cpp:335 `auto& callback =
  NativeFunctions::NativeByIndex[func->NativeFuncIndex]; if (callback) ... else
  Exception::Throw("Unknown native function "...)`. Utils/Array.h:189 operator[] is unchecked
  `_data[pos]`. NativeFunc.cpp:19 resizes NativeByIndex only in RegisterHandler, while
  NativeFunc.cpp:43 resizes FuncByIndex for every declared index. ExpressionEvaluator.cpp:830 checks
  FuncByIndex's bounds, so the call reaches Frame::Call and then CallNative. The highest registered
  number is 4003 (NDebugInfo.cpp). NATIVES.md 'Stops the game': an unknown native exits with its
  message; commit a851fe08 meant 'a native index past the table' to be a script error.
- **Verified:** NativeByIndex is resized only in RegisterHandler (NativeFunc.cpp:19), so its size is
  4004 (highest registration NDebugInfo.cpp:11, 4003). FuncByIndex grows for any declared index
  (NativeFunc.cpp:43). UFunction::Load reads NativeFuncIndex as a uint16 (UFunction.cpp:10).
  Frame.cpp:335 indexes NativeByIndex[func->NativeFuncIndex] with no size check, and
  Array::operator[] (Utils/Array.h:189) does not check bounds. The bounds-checked path is
  GlobalFunctionExpression (ExpressionEvaluator.cpp:830), and it checks only FuncByIndex. A native
  with index 4004..65535 and no handler reads past the vector instead of reaching the 'Unknown
  native function' throw.
- **Proof:** grep of the registrations: max 4003. Frame.cpp:335 `auto& callback =
  NativeFunctions::NativeByIndex[func->NativeFuncIndex];` with no size() test. NativeByIndex has no
  other resize.

### VM.2 Clamp and FClamp answer the lower bound when the bounds are inverted; UE1's Clamp answers
the upper

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Native/NObject.cpp:329`, `DXEngine/DXEngine/Native/NObject.cpp:591`,
  `DXEngine/DXEngine/Math/vec.h:347`
- **What:** The int clamp is vec.h:347. FClamp uses the generic template at vec.h:344
  (std::max(std::min(v,max),min)), which gives the same Min-wins result.
- **Evidence:** vec.h:347: `if (maxval < val) val = maxval; if (minval > val) val = minval; return
  val;`. NObject::Clamp and FClamp call it directly. UE1's Core/Inc/UnTemplate.h Clamp is `X<Min ?
  Min : X<Max ? X : Max` (the SDK's headers zip holds it). The script call sites are in Engine.u
  (PlayerPawn ClientUpdatePosition's SetCollisionSize FClamp) and DeusEx.u (FrobDisplayWindow).
- **Verified:** vec.h:344 (generic, max(min(v,max),min)) and vec.h:347 (int) both return Min when
  Min>Max. The SDK's UnTemplate.h:108-110 Clamp is `X<Min ? Min : X<Max ? X : Max`, which returns
  Max when X>=Min>Max. Core.dll's execClamp has the same order: cmp X,Min jl -> Min; cmp X,Max jge
  -> Max; else X. NObject.cpp:331/593 call clamp directly. NATIVES.md does not list this difference.
- **Proof:** Core.dll execClamp thunk 0x10101c5d -> 0x10134be0: `cmp eax,ecx; jl` (X<Min gives Min),
  then `cmp eax,ecx; jge` (X>=Max gives Max). UnTemplate.h:110. For V=0, Min=0, Max=-1: original -1,
  DXEngine 0.

### VM.3 A dynamic cast whose class import did not load dereferences null

- **State:** confirmed, low (crash)
- **Where:** `DXEngine/DXEngine/VM/ExpressionEvaluator.cpp:491`
- **What:** DynamicCastExpression reads expr->Class->Name without checking for null. Commit a851fe08
  made a FinalFunction import that failed to load a script error rather than a crash, but
  class<X>(obj) still segfaults when X's import read as null. This happens with a mod or package
  whose import is missing. MetaCast survives a null class; DynamicCast does not.
- **Evidence:** ExpressionEvaluator.cpp:490-492: `UObject* value = Value(expr->Value).ToObject(); if
  (value && !value->IsA(expr->Class->Name)) value = nullptr;`. Bytecode.cpp:355 sets `expr->Class =
  stream->ReadObject<UClass>()`, which can be null for an import that did not load: the case
  FinalFunctionExpression guards at ExpressionEvaluator.cpp:792.
- **Verified:** Bytecode.cpp:355 sets expr->Class = ReadObject<UClass>(), which is
  Cast<T>(GetUObject(ref)) (UObject.h:456). Cast passes null through (UObject.h:393). An import
  whose package exists but lacks the object makes FindObjectReference return 0 (Package.cpp:415),
  and GetUObject(0) returns nullptr (Package.cpp:261). ExpressionEvaluator.cpp:491 then dereferences
  expr->Class->Name whenever the value is non-null. FinalFunctionExpression guards the same case
  (line 792); MetaCast compares pointers and survives a null class.
- **Proof:** Path: Package.cpp:257 GetPackage(pkg)->GetUObject(class,name,group) ->
  FindObjectReference returns 0 -> GetUObject(0) returns nullptr -> ReadObject null ->
  ExpressionEvaluator.cpp:491 null->Name.

## PKG: packages, loading, travel and saves

### PKG.1 Package::GetExportName reads the wrong export and overwrites the export table through a
reference

- **State:** confirmed, medium (memory)
- **Where:** `DXEngine/DXEngine/Package/Package.cpp:570`,
  `DXEngine/DXEngine/Package/Package.cpp:575`, `DXEngine/DXEngine/Network/NetPackageMap.cpp:74`
- **What:** The overwritten slot is the next export (objref), and only when that next export has an
  outer. The logged name is the next export's.
- **Evidence:** Package.cpp:570 `ExportTableEntry& entry = ExportTable[objref];` (callers pass
  exportIndex+1, NetPackageMap.cpp:74). Line 575 `entry = ExportTable[entry.ObjOuter - 1];` assigns
  through the reference. A map actor's ObjOuter is the MyLevel export, so slot exportIndex+1 becomes
  a copy of MyLevel's entry. A later GetUObject or FindObjectReference on that slot loads or matches
  the Level instead of that export. The only caller is the log line on the 'map actor no longer
  there' path.
- **Verified:** Package.cpp:570 indexes ExportTable[objref], but GetExportEntry and GetUObject
  (Package.cpp:206, 433) take objref-1. NetPackageMap.cpp:74 passes exportIndex+1, so the code reads
  slot exportIndex+1, the next export, and reads out of bounds for the last one. `entry` is a
  reference, so line 575 `entry = ExportTable[entry.ObjOuter - 1]` copies the outer's entry over
  that live slot. The only call is on the server's path for a destroyed map actor
  (NetPackageMap.cpp:65-75).
- **Proof:** Path: IndexToObject, then GetLoadedExport(i)==null and the class is Actor, then
  GetExportName(i+1). That reads ExportTable[i+1], whose ObjOuter is MyLevel for a map actor that
  follows, and the copy overwrites ExportTable[i+1] with MyLevel's entry. A later reference to
  export i+1, once it is freed, then sees class Level (not Actor) and GetUObject loads MyLevel's
  data in its place.

### PKG.2 ?restart gives the player back the travel items it arrived with; the original reloads with
?loadonly and carries none

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXEngine/Engine.cpp:495`, `DXEngine/DXEngine/Engine.cpp:499`,
  `DXEngine/DXEngine/Engine.cpp:1845`
- **What:** The NextURL ?restart branch has the same defect (Engine.cpp:459-468).
- **Evidence:** Engine.cpp:499-500 `auto restartTravelInfo = Level->TravelInfo; LoadMap(restartURL,
  restartTravelInfo);`. Line 1845 accepts the travel when TravelType == TRAVEL_Relative, which
  ClientTravel set. deusex-dll.md (Browse): '?restart: empties Current, then reloads the current map
  (?loadonly)'. engine-dll.md (SpawnPlayActor): travelling items' TravelPreAccept 'none for
  ?loadonly'. Also, bItems is false in the script call.
- **Verified:** DeusExPlayer.RestartLevel calls ResetPlayer, then Super.RestartLevel, which is
  ClientTravel("?restart", TRAVEL_Relative, false) (Engine.u). Engine.cpp:813 sets TravelType to
  Relative. Engine.cpp:495-502 reloads LastURL with Level->TravelInfo, the arrival info stored at
  Engine.cpp:1053. LoginPlayer (1784, 1845-1858) then Accepts it. The original's ?restart reloads
  with ?loadonly (deusex-dll.md:183), and SpawnPlayActor gives no travelling items for ?loadonly
  (engine-dll.md:1549). NATIVES.md lists no such difference.
- **Proof:** Code path: ClientTravel("?restart") sets ClientTravelInfo.TravelType=Relative. Run
  takes the restart branch, then LoadMap(LastURL, Level->TravelInfo), then LoginPlayer, then
  ActorTravelInfo::Accept with the arrival travel info.

### PKG.3 TRAVEL_Partial (console open) keeps the last URL's server host, so 'open <map>' on a net
client rejoins the server

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXEngine/Engine.cpp:795`, `DXEngine/DXEngine/UnrealURL.cpp:20`,
  `DXEngine/DXEngine/Engine.cpp:1049`
- **What:** A Partial travel is built with the merge constructor, which keeps the base's Host, Port
  and Protocol when the new URL names no host. On a net client, LastURL is the server's URL (LoadMap
  stores the pending join's URL). 'open DX' or 'open 01_NYC_UNATCOIsland' therefore takes Run's Host
  branch and calls BeginConnect to the same server, instead of leaving it and loading the local map.
- **Evidence:** Engine.cpp:795 `ClientTravelInfo.URL = UnrealURL(LastURL, url.ToString());`.
  UnrealURL.cpp:11-25 copies the base whole and replaces Host only when next.Host is non-empty.
  Engine.cpp:1049 `LastURL = url;` runs for the client map too (LoadClientMap passes pending->URL,
  which has a Host). Engine.u's ETravelType: `TRAVEL_Partial, // Partial (carry name, reset
  server)`: a partial travel keeps the options and drops the server.
- **Verified:** Engine.cpp:795 builds a Partial travel with UnrealURL(LastURL, url).
  UnrealURL.cpp:10-25 copies the whole base and keeps Host, Port and Protocol unless next names a
  host. On a net client, LastURL is set at Engine.cpp:1049 from LoadClientMap's pending->URL
  (1969-1971), which carries the server's host. Run (523-528) then calls BeginConnect. Engine.u says
  TRAVEL_Partial is 'carry name, reset server'. UE1's FURL copies only the base's options for
  Partial.
- **Proof:** On a client, 'open DX' runs the console check (2473-2481), then ClientTravel(Partial),
  so ClientTravelInfo.URL.Host is the server's. Run's Host branch then calls BeginConnect to the
  same server with map DX.

### PKG.4 Travel to a map that is not there throws after the old level is unloaded and ends the
engine

- **State:** confirmed, medium (error-handling)
- **Where:** `DXEngine/DXEngine/Engine.cpp:1035`, `DXEngine/DXEngine/Package/PackageManager.cpp:390`
- **What:** Reached only by a map name that is not there (a mod's exit, a script's URL, --url). The
  stock game's exits all exist.
- **Evidence:** Engine.cpp:1019 UnloadMap(), then 1035 packages->LoadMap(url.Map).
  PackageManager.cpp:390 returns GC::Alloc<Package>(..., absolute_path) without checking that the
  file exists. Package::ReadTables uses GetStream, and File::open_existing throws. Only the
  console's open checks the map list (Engine.cpp:2477). ClientTravel and NextURL do not.
- **Verified:** Engine.cpp:1006-1008 sends NotifyLevelChange and calls UnloadMap, then 1035 opens
  the package. PackageManager.cpp:390 allocates a Package for a path that does not exist, and
  Package's constructor runs ReadTables, then GetStream, which throws. No catch around Run's travel
  branches turns that into a failed browse. engine-dll.md:1690-1694: the original checks the new
  map's package (LOAD_Verify) before NotifyLevelChange and before the old level goes, so a missing
  map fails the browse. Only the console's open checks the map list (2475-2484).
- **Proof:** ClientTravel("NoSuchMap", Relative, true) makes ClientTravelInfo.URL.Map non-empty. Run
  line 539 calls LoadMap, then UnloadMap, then packages->LoadMap, then GC::Alloc<Package>, then
  ReadTables, and the open throws with Level already unloaded.

### PKG.5 LoadSaveFile always re-sets the extension in lower case, defeating FindCurrentSave's
any-case match

- **State:** confirmed, low (platform)
- **Where:** `DXEngine/DXEngine/Package/PackageManager.cpp:624`,
  `DXEngine/DXEngine/Package/PackageManager.cpp:645`
- **What:** Only files the engine did not write itself (it writes lower case) are hit, so this is an
  edge case.
- **Evidence:** PackageManager.cpp:624 `fullPath.extension().string() != GetSaveExtension()`.
  LoadFileExtensions (1183) stores SaveExt without a dot, so the test is always true.
  FindCurrentSave (645) matches the extension with equals_ignore_case and returns `Save/Current`'s
  path plus the real filename. This contradicts LoadMap's comment that the save is 'found whatever
  case'.
- **Verified:** PackageManager.cpp:624 compares extension() (".dxs", with its dot) to
  GetSaveExtension() ("dxs", from SaveExt at 1183), so the test is always true and replace_extension
  always runs. FindCurrentSave (645-653) returns the on-disk spelling in any case. On Linux,
  Current/X.DXS becomes Current/X.dxs, fs::exists fails and nullptr comes back. Engine.cpp:1025-1036
  then loads the map fresh from Maps.
- **Proof:** fs::path("Current/Foo.DXS").extension() is ".DXS", which is never equal to "dxs", so
  replace_extension("dxs") gives Current/Foo.dxs, a file not on a case-sensitive file system.

## PHYS: collision, physics and movement

### PHYS.1 TraceActors/TraceTexture/TraceVisibleActors widen a box Extent to a cylinder of radius
|Extent.xy| (sqrt2 x Extent.X)

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXEngine/VM/Iterator.cpp:436`, `DXEngine/DXEngine/VM/Iterator.cpp:441`
- **What:** DeusExMultiLineCheck sets radius = length(extent.xy()). An Extent (r,r,h) is therefore
  traced with radius r*1.414, against the level's AABB and as extra radius on every actor's
  cylinder. The original adds Extent.X. ScriptedPawn.AISafeToShoot passes extent*3 of the
  projectile's cylinder to TraceActors, so NPCs with projectile weapons see walls and allies through
  a box about 41% too wide and call more shots unsafe.
- **Evidence:** Iterator.cpp:436 `float radius = length(extent.xy());`, then
  Collision.Trace(start,end,height,radius,...). Engine.dll UPrimitive::LineCheck 0x103d8af0: the
  cylinder test uses `a6.X + CollisionRadius` and `a6.Y + CollisionRadius`, each axis on its own.
  execTraceActors 0x103e4bc0 passes the script's Extent to MultiLineCheck unchanged. UActor::Trace
  -> CollisionSystem::TraceFirstHit (CollisionSystem.cpp:66) uses extents.x, as the original does.
  DeusEx.u: `AISafeToShoot(hitActor, projEnd, projStart, extent*3)`, where extent =
  (CollisionRadius, CollisionRadius, CollisionHeight).
- **Verified:** Iterator.cpp:436 passes radius=length(extent.xy()) to Collision.Trace.
  TraceTest.cpp:32/73 makes the level box {radius,radius,height}, and :450/494 adds radius to each
  actor's CollisionRadius. TraceFirstHit (CollisionSystem.cpp:66) passes extents.x to the same
  parameter. The original UPrimitive::LineCheck 0x103d8af0 uses a6.X+CollisionRadius as the cylinder
  radius in its quadratic, with a6.Y used only in the AABB reject. DeusEx.u AISafeToShoot(...,
  extent*3), extent=(R,R,H), reaches this path.
- **Proof:** IDA decompile 0x103d8af0: v49 = a6.X + CollisionRadius; then v49 = dx*dx+dy*dy -
  v49*v49, so the radius is Extent.X alone. DXEngine: radius = |(r,r)| = 1.414r.

### PHYS.2 FindBase's 'based on it' test is reversed: it skips the floor the actor itself stands on,
not a floor that stands on the actor

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Engine/Actors/UActor_Phys.cpp:147`,
  `DXEngine/DXEngine/Packages/Engine/Actors/UActor_Base.cpp:107`
- **What:** In DXEngine, X->IsBasedOn(Y) walks Y's base chain looking for X, so it means 'Y is based
  on X' (the reverse of UE1's). The other callers are written for that meaning. FindBase calls
  hit.Actor->IsBasedOn(this), which asks whether this actor is based on the hit, where the original
  asks whether the hit is based on this actor. Result: an actor based through a chain on what it
  finds below is not re-based onto it, and a floor actor that stands on the caller is passed to
  SetSupportBase/SupportActor, where the original returns.
- **Evidence:** UActor_Base.cpp:107-116: `for (cur = other; cur; cur = cur->ActorBase()) if (cur ==
  this) return true;`. Engine.dll AActor::IsBasedOn 0x1034c8d0 walks from `this` up `Base` looking
  for the argument, and AActor::FindBase 0x103c9470 does `if (Base != Hit.Actor && Hit.Actor &&
  Hit.Actor->IsBasedOn(this)) return;`. So the original asks whether the hit stands on this actor;
  DXEngine's identical-looking call asks the opposite. The code comment at UActor_Phys.cpp:134-136
  says 'unless that is based on it'.
- **Verified:** DXEngine IsBasedOn (UActor_Base.cpp:107-116) walks the argument's base chain looking
  for this. The original AActor::IsBasedOn 0x1034c8d0 walks this's chain looking for the argument.
  The original FindBase 0x103c9470 calls IsBasedOn(hit, this), meaning the hit stands on this.
  DXEngine's hit->IsBasedOn(this) (UActor_Phys.cpp:147) means this stands on the hit, the reverse.
  SetBase's cycle guard (UActor_Base.cpp:73) is written for DXEngine's meaning, so nothing crashes:
  the base cycle is refused there, but SupportActor still fires on a floor that rides on the caller.
- **Proof:** IDA: IsBasedOn: while(this!=a2) this=this->Base. FindBase: if(Base!=hit){ if(hit &&
  IsBasedOn(hit,this)) return; SetSupportBase(this,hit);}

### PHYS.3 An NPC's turn runs every tick and a player's is gated; the original gates the NPC's
(Rotation != DesiredRotation or RotationRate.Roll > 0) and always runs the player's

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Engine/Actors/Pawn/UPawn_Tick.cpp:174`,
  `DXEngine/DXEngine/Packages/Engine/Actors/Pawn/UPawn_Tick.cpp:192`,
  `DXEngine/DXEngine/Packages/Engine/Actors/Pawn/UPlayerPawn.cpp:67`,
  `dx-reverse-info/engine-dll.md:351`
- **What:** UPawn::TickRotating calls DeusExPhysicsRotation every tick. That forces
  bRotateToDesired=true and bFixedRotationDir=false, and zeroes a roll, even when the NPC already
  faces DesiredRotation. The original skips physicsRotation for a non-player whose Rotation equals
  DesiredRotation (raw ints) and whose RotationRate.Roll <= 0, so in that case it keeps the flags
  and any roll. For a player, the original calls it every tick (roll zeroed whenever non-zero),
  while DXEngine gates it. engine-dll.md states the gate the wrong way round.
- **Evidence:** Engine.dll APawn::performPhysics 0x103c9a70, at 0x103c9ba3-0x103c9bfd: the class
  chain loop `cmp eax, APlayerPawn; jz 0x103c9bff` jumps straight to the physicsRotation call.
  Otherwise it compares Rotation with DesiredRotation (+120h..128h against +208h..210h) and tests
  RotationRate.Roll (+204h) `jle` to skip. In APawn::physicsRotation 0x103c9f30 the same IsA test
  jumps past the turning, so 'found' means a player. engine-dll.md:351 says 'a player's only when
  its rotation is not the desired one'.
- **Verified:** At 0x103c9ba3 APawn::performPhysics walks the class chain; an APlayerPawn jumps to
  0x103c9bff, so physicsRotation is called every tick. Any other pawn compares Rotation +120..128
  with DesiredRotation +208..210 and skips when they are equal and RotationRate.Roll (+204) <= 0
  (jle 0x103c9c20). DXEngine is the other way round: UPawn::TickRotating (UPawn_Tick.cpp:174-180)
  always calls DeusExPhysicsRotation, which forces the flags and zeroes roll, and
  UPlayerPawn::TickRotating (UPlayerPawn.cpp:67-76) gates the player. engine-dll.md:351 states the
  gate for the wrong pawn.
- **Proof:** Disasm 0x103c9baa: cmp eax, APlayerPawn::PrivateStaticClass; jz 0x103C9BFF (the
  physicsRotation call at 0x103c9c1b). 0x103c9bc5-0x103c9bfd: the Rotation/DesiredRotation compares
  and the Roll jle to 0x103C9C20.

### PHYS.4 RE doc: Trace passes over the tracer's owners, not 'one it owns'

- **State:** confirmed, low (docs)
- **Where:** `dx-reverse-info/engine-dll.md:1247`,
  `DXEngine/DXEngine/Collision/TopLevel/CollisionSystem.cpp:68`
- **What:** engine-dll.md says Trace's first hit is 'not the tracing actor or one it owns'. The
  binary, like DXEngine's TraceFirstHit (tracingActor->IsOwnedBy(hit.Actor)), passes over the tracer
  and every actor up its Owner chain, which is how a weapon's trace skips its owning pawn. The doc
  states the relationship backwards; the code is right.
- **Evidence:** Engine.dll ULevel::SingleLineCheck 0x1039aed0: `for (i = SourceActor; i; i =
  i->Owner) if (i == Hit.Actor) skip;`. CollisionSystem.cpp:68 does the same through
  UActor::IsOwnedBy (UActor_Owner.cpp:47-56, which walks this->Owner).
- **Verified:** ULevel::SingleLineCheck 0x1039aed0 skips a hit when it finds the hit actor while
  walking from the source actor up its Owner chain, i.e. the tracer or one of its owners.
  engine-dll.md:1247 says 'one it owns', which is the reverse. DXEngine's TraceFirstHit uses
  tracingActor->IsOwnedBy(hit.Actor), and IsOwnedBy (UActor_Owner.cpp:47-56) walks this->Owner,
  which matches the binary. Only the doc is wrong.
- **Proof:** IDA decompile 0x1039aed0: for (i = a3; i; i = i->Owner) if (i == hit->Actor) skip.

## AI: actors, pawns, AI, navigation, animation and the event manager

### AI.1 A destroyed sender goes on giving off its AIStartEvent level until CleanupDestroyed, up to
128 destructions later

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Engine/UEventManager.cpp:499-517`,
  `DXEngine/DXEngine/Packages/Engine/UEventManager.cpp:368-371`,
  `DXEngine/DXEngine/Packages/Engine/Resources/Level/ULevel.cpp:280-297`
- **What:** Tick copies each sender's Current into the new slot whatever its actor's state. Current
  is zeroed only in ActorDestroyed, which CleanupDestroyed calls once 128 actors are on
  FirstDeleted. Until then a destroyed DeusExProjectile (AIStartEvent 'Projectile' in PostBeginPlay,
  never ended) or a removed carcass ('Carcass') still raises its full level at its last spot. NPCs
  keep weighing it, run its score callback and get Begin/Pulse calls for an actor that is gone.
- **Evidence:** Engine.dll AIProcess ring step at 0x103844fb-0x1038451f: for each sender it calls
  the actor's vtable +0x2c (UObject::IsPendingKill per UnObjBas.h's order: QI, AddRef, Release,
  dtor, ProcessEvent, ProcessState, ProcessRemoteFunction, Modify, PostLoad, Destroy, Serialize,
  IsPendingKill). When true it zeroes the current levels (+0x13c..+0x148) before copying them into
  the slot. engine-dll.md step 7 says the same: '0 for an actor being destroyed'. DXEngine line 504
  copies with no bDeleteMe test.
- **Verified:** In the original, vtable +0x2c is IsPendingKill: UnObjBas.h's order is QI, AddRef,
  Release, dtor, ProcessEvent, ProcessState, ProcessRemoteFunction, Modify, PostLoad, Destroy,
  Serialize, IsPendingKill, so 12*4=0x2c. When it returns true, 0x10384507-0x10384519 zero
  +0x13c..+0x148 before the copy at 0x1038451f. DXEngine's UEventManager.cpp:504 copies Current with
  no test. Current is cleared only in ActorDestroyed (205), which ULevel.cpp:297 calls from
  CleanupDestroyed. That runs only once DeletedCount>=128 (283) or when forced. UActor::Destroy
  (UActor.cpp:283-289) only chains the actor. ProcessReceiver:370 has no bDeleteMe test either.
- **Proof:** objdump 0x103844fb: mov 0x2c(%esi),%ecx; call *0x2c(%edx); je 0x1038451f; mov
  %ebx,0x13c..0x148(%esi). DeusEx.u: AIStartEvent('Projectile', EAITYPE_Visual) with no matching
  AIEndEvent. Carcass has its AIEndEvent only on some paths.

### AI.2 An inventory sender's size scale is applied after AICanSee instead of to the visual value
passed in, and not at all for non-pawn receivers

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Engine/UEventManager.cpp:249-274`
- **What:** There is one more edge: when the owner's H*R is 0, the original takes sqrt(item H*R)
  undivided, not 1. For a non-pawn receiver the original still traces to the item's own location:
  only the visual number is scaled.
- **Evidence:** ComputeSenseDetection 0x103837b0: the inventory test and sqrt run whatever the
  receiver's class (0x1038380e-0x1038388c). 0x103838ae-0x103838c6 stores peak.visual*scale both as
  the result and as the argument. 0x103838e7-0x103838fc pushes the receiver's four checks, that
  product and the owner, then calls AICanSee (0x10301357). The non-pawn trace path
  (0x1038390e-0x103839ca) keeps the scaled value on a clear line. It divides only when the owner's
  H*R is nonzero, with no test of the item's size.
- **Verified:** The original's ComputeSenseDetection runs the Inventory test (class 0x10576f70) for
  every receiver: the pawn test only sets -0x14. When the owner's H*R is nonzero it takes
  sqrt(itemHR/ownerHR), so an item of size 0 gives a scale of 0. At 0x103838ae-0x103838c6 it stores
  visual*scale both to *visibility and to the argument slot. 0x103838f6-0x103838fc pushes that
  product and edi (the owner) to AICanSee. The non-pawn path keeps the stored scaled value when the
  trace is clear. DXEngine scales only when pawn!=null (line 250) and multiplies after AICanSee
  (265), and AICanSee subtracts VisibilityThreshold and clamps (UPawn_AI.cpp:398-402).
- **Proof:** objdump 0x103838ae: flds -0x40; fmuls 0x8(%ebp); fsts 0x8(%ebp); fstps (%ebx). Then at
  0x103838f6: mov 0x8(%ebp),%eax; push %eax; push %edi; call 0x10301357.

### AI.3 A non-pawn receiver hears a 0-radius audio event within 800 units, where the original hears
nothing

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Engine/UEventManager.cpp:284-287`
- **What:** For a receiver that is no pawn, DXEngine turns a peak AudioRadius of 0 into 800. The
  original compares the squared distance with the radius squared as it is, so a radius of 0 is never
  heard. The 800 default belongs to the pawn's AICanHear alone.
- **Evidence:** 0x10383a08-0x10383a5e: the squared distance between the receiver's and the sender's
  actor locations is compared with radius*radius (-0x38), volume = audio when less, else 0.0, with
  no default. engine-dll.md: 'for another actor, the audio number within the radius'.
- **Verified:** At 0x10383a08-0x10383a5e the original compares |sender.Location-receiver.Location|^2
  with radius*radius (-0x38). When it is less, volume is audio; otherwise 0.0 (0x10429838 holds
  0.0). There is no 800 default, so a radius of 0 is never heard. DXEngine's UEventManager.cpp:285
  swaps a radius of 0 for 800. A radius of 0 can come from script such as AISoundLevel*256 or
  blastRadius*24 with a zero factor, so this is an edge case.
- **Proof:** objdump 0x10383a42: flds -0x38; fmuls -0x38; fcompp; je ->fldl 0x10429838 (0.0), else
  flds -0x3c (audio). No 800 constant anywhere in the block.

### AI.4 A pawn's latent move/turn/strafe is polled before its animation and script Tick, not in
ProcessState after them

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Engine/Actors/Pawn/UPawn_Tick.cpp:20-90`,
  `DXEngine/DXEngine/Packages/Engine/Actors/UActor.cpp:389-464`
- **What:** UPawn::Tick runs the MoveTo/MoveToward/StrafeTo/TurnTo/WaitForLanding polls before
  UActor::Tick, so before the animation pass and the script Tick/PlayerTick. In the original the
  pending latent action ticks at the start of ProcessState, which follows the script Tick. A Tick,
  AnimEnd or notify that does a GotoState or changes Destination/MoveTarget still gets a frame of
  the old move's acceleration. LongFall is also sent before Tick instead of after it.
- **Evidence:** engine-dll.md, a level's tick: an actor's tick runs 'its animation; then its script
  Tick, its state code, ...', and state code (ProcessState 0x103e5b60) says 'a latent action pending
  ticks first; then bytecodes run'. In DXEngine the polls come first at UPawn_Tick.cpp:20-88, and
  UActor::Tick (animation line 391, Tick event line 443, StateFrame->Tick line 462) runs after them.
- **Verified:** Engine.dll exports no APawn::Tick, only AActor::Tick. engine-dll.md:1586-1590 gives
  the order: animation, then script Tick, then state code. ProcessState 0x103e5b60 calls
  GNatives[frame->LatentAction] at 0x103e5c0c-0x103e5c13 before the bytecodes run. In DXEngine,
  UPawn::Tick (UPawn_Tick.cpp:19-86) polls MoveTo, MoveToward, StrafeTo, TurnTo and WaitForLanding
  (with LongFall) before UActor::Tick (88). UActor::Tick runs TickAnimation (391), then Tick or
  PlayerTick (439-444), then StateFrame->Tick (460).
- **Proof:** objdump 0x103e5bfb: mov 0xc(%esi),%eax; mov 0x24(%eax),%ecx; ... call *(%eax,%edx,4)
  with GNatives at 0x10599a38, ahead of the bytecode loop. objdump -p shows only ?Tick@AActor@@.

### AI.5 engine-dll.md says the better-start threshold is the deeper cut; the binary and code take
the shallower

- **State:** confirmed, low (docs)
- **Where:** `dx-reverse-info/engine-dll.md:941`,
  `DXEngine/DXEngine/Packages/Engine/Actors/Pawn/UPawn_Path.cpp:531`
- **What:** The doc says a candidate must be 'cheaper by 15% (or 150, whichever cuts deeper)', which
  reads as min(0.85*total, total-150). The binary takes max(ftol(total*0.85), total-150), the
  smaller cut, and so does the code (std::max). The code is right and the doc says otherwise.
- **Evidence:** Engine.dll 0x103db207-0x103db226: eax=ftol(endTotal*0.85); ecx=endTotal-0x96; if
  eax<ecx eax=ecx; candidate if total<eax. UPawn_Path.cpp:531 uses std::max.
- **Verified:** The binary computes eax=ftol(endTotal*0.85) (0x104325a8 holds 0.85) and
  ecx=endTotal-150, keeps the larger, and makes the node a candidate when total<that. That is the
  more lenient threshold, the shallower cut. UPawn_Path.cpp:531 uses std::max and matches it.
  engine-dll.md:941's 'whichever cuts deeper' says the opposite.
- **Proof:** objdump 0x103db207-0x103db226: fildl -0x1c; fmull 0x104325a8 (=0.85); call ftol; lea
  -0x96(%edx),%ecx; cmp %ecx,%eax; jge; mov %ecx,%eax; cmp %eax,%ebx; jge skip.

## DX: Deus Ex's own natives: the game, conversations, flags and texts

### DX.1 The save listing passes over Save* folders whose fifth character is not a digit, such as
Save-003 from `savegame -3`

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/DeusEx/UGameDirectory.cpp:228`,
  `DXEngine/DXEngine/Engine.cpp:1395`
- **What:** Also a minor point: DXEngine's prefix compare is case-sensitive, while the original's
  Windows Save* match ignores case.
- **Evidence:** UGameDirectory.cpp:228: `name.compare(0, 4, "Save") == 0 && name[4] >= '0' &&
  name[4] <= '9'`. Engine.cpp:1395-1402: "any other number a Save%04d, a negative one too (the
  console's `savegame -3` writes Save-003 ...)". deusex-dll.md, the save directory: "saves
  (GetSaveGamesDirectory, 0x10017520): the Save* directories."
- **Verified:** UGameDirectory.cpp:228 keeps a directory only when name[4] is a digit, so Save-003
  is left out. The original's GetSaveGamesDirectory (0x10017520) lists the `Save*` directories
  (deusex-dll.md, the save directory). DeusEx.dll's pattern string is `%s\%s*` with `Save`. Its
  disassembly shows only a directory FindFiles call and no character check on the names.
  Engine.cpp:1395-1402 and NATIVES.md (saving) both say `savegame -3` writes Save-003, which
  DXEngine then never lists. This is not among the Differs and has no fix in bugs-recovered.md. Only
  a console `savegame` with a negative slot reaches it.
- **Proof:** strings -el DeusEx.dll gives `%s\%s*` and `Save`. objdump of 0x10017520-0x10017770
  shows FindFiles through a vtable (call *0x28(%eax)) and no comparison of the names' characters.

## EXT: the window tree and its text (Extension)

### EXT.1 BorderWindow always draws a white debug outline over itself

- **State:** confirmed, medium (wrong-result)
- **Where:** `DXEngine/DXEngine/Packages/Extension/Windows/UBorderWindow.cpp:127`,
  `DXEngine/DXEngine/Packages/Extension/Windows/UWindow.cpp:2376`
- **What:** UBorderWindow::DrawWindow ends with an unconditional DrawDebugBox(gc). That call draws
  four white 1-pixel lines round the window's rectangle with Draw2DLine, outside the GC's clip and
  with no debug-mode check. Every BorderWindow shows the white frame: the frame of the EditFlags,
  LoadMap and PlayMusic cheat screens (ToolWindow.CreateToolWindowContainer makes a BorderWindow)
  and ToolBorderWindow. The original draws no such lines.
- **Evidence:** UBorderWindow.cpp:127 `DrawDebugBox(gc);` comes right after gc->DrawBorders and
  UWindow::DrawWindow. DrawDebugBox (UWindow.cpp:2376) calls engine->render->Draw2DLine(vec4(1.0f),
  ...) four times. The only other use of DrawDebugBox is commented out (UWindow.cpp:2259 `//
  DrawDebugBox(gc);`), so this one is a debugging leftover. DeusEx.u: `winBorder =
  BorderWindow(winParent.NewChild(Class'BorderWindow'))` in CreateToolWindowContainer.
- **Verified:** UBorderWindow.cpp:127 calls DrawDebugBox(gc) with no condition after DrawBorders and
  UWindow::DrawWindow. UWindow.cpp:2376-2382 draws four white vec4(1) Draw2DLine edges round the
  window. The only other call is commented out (UWindow.cpp:2259). The line came from Surreal Engine
  (git log shows only ea8fad80). Nothing in NATIVES.md or extension-dll.md describes such an
  outline. Both DeusEx.u CreateToolWindowContainer (BorderWindow) and ToolBorderWindow (which
  expands BorderWindow) reach it.
- **Proof:** DeusEx.u strings: `winBorder = BorderWindow(winParent.NewChild(Class'BorderWindow'));`
  and `class ToolBorderWindow expands BorderWindow;`. grep shows DrawDebugBox called only at
  UBorderWindow.cpp:127.

### EXT.2 Window.winParent is never set, so a script reading it gets None

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Extension/Windows/UWindow.cpp:1249`,
  `DXEngine/DXEngine/Packages/Extension/Windows/UWindow.h:396`
- **What:** Extension.u declares `var const window winParent; // Parent window; NULL if this is
  root`, and the game's script reads it. DXEngine keeps the parent only in the private parentOwner:
  NewChild sets parentOwner and never writes winParent, and nothing else does.
  SpinningLogoWindow.Tick bounces its logo off `winParent.width` and `winParent.height`. Its parent
  is the QuotesWindow, the credits' quotes easter egg (bQuotesEnabled) and the ShowQuotesWindow
  cheat. There each of the 10 logos reads None (an Accessed None every tick), sees a parent 0 by 0,
  and jitters at the top-left corner instead of bouncing round the screen.
- **Evidence:** grep finds winParent in DXEngine only in PropertyOffsets and the accessor at
  UWindow.h:396; it is never assigned. UWindow.cpp:1249 `child->parentOwner() = this;` is the only
  parent link NewChild makes. DeusEx.u SpinningLogoWindow.Tick: `if (newX > winParent.width-width)
  newX = 2*(winParent.width-width) - newX;`, and the same for height. QuotesWindow.CreateLogo makes
  10 of them with NewChild.
- **Verified:** The SDK's ExtWindow.h has `XWindow *parent; // Parent window; NULL if this is root`,
  which is script winParent, separate from parentOwner. DXEngine declares the accessor winParent()
  (UWindow.h:396) but never assigns it. NewChild (UWindow.cpp:1248) sets only parentOwner.
  SpinningLogoWindow.Tick reads winParent.width and winParent.height, so in DXEngine both read None.
- **Proof:** grep -rn winParent DXEngine/DXEngine: only PropertyOffsets.h:3300,
  PropertyOffsets.cpp:3979, UWindow.h:396. DeusEx.u: `if (newX > winParent.width-width)`;
  QuotesWindow: `winLogo = SpinningLogoWindow(NewChild(Class'SpinningLogoWindow'))`.

### EXT.3 A list measuring a field pushes the GC with GetGC and never pops it

- **State:** confirmed, low (resource-leak)
- **Where:** `DXEngine/DXEngine/Packages/Extension/Windows/UListWindow.cpp:217`,
  `DXEngine/DXEngine/Render/RenderWindow.cpp:162`
- **What:** The leak is bounded at 64 entries. The visible effect is the GC's state left switched to
  the list's (colours, style, alignment) after the call, not unbounded growth.
- **Evidence:** UListWindow.cpp:217 `UGC* gc = UObject::Cast<UGC>(GetGC());`; the function restores
  only the fonts and special text by hand, with no ReleaseGC or PopWindowGC. UWindow::GetGC
  (UWindow.cpp:657) runs `engine->render->PushWindowGC(this)`. RenderWindow.cpp: PushWindowGC pushes
  a closure and calls ResetWindowGC; its comment on the 64 cap reads 'A script that never releases'.
- **Verified:** MeasureText (UListWindow.cpp:217) calls GetGC, which runs PushWindowGC(this)
  (UWindow.cpp:659) and resets the shared GC to the list's state. It restores only fonts and special
  text by hand and never calls ReleaseGC/PopWindowGC, so one entry leaks per call (lines 288, 726).
  The stack is capped at 64 (RenderWindow.cpp:183), so it does not grow without end. The GC is left
  with the list's colours and style until the next ResetWindowGC, so a script holding a GC across an
  AddRow sees them.
- **Proof:** PushWindowGC pushes a closure then ResetWindowGC(window,...). PopWindowGC is reached
  only from UWindow::ReleaseGC (UWindow.cpp:1369). Neither is in MeasureText.

### EXT.4 IsActorValid counts a destroyed actor as valid, where Add and Remove pass it over

- **State:** confirmed, low (wrong-result)
- **Where:** `DXEngine/DXEngine/Packages/Extension/Windows/UWindow.cpp:794`,
  `DXEngine/DXEngine/Packages/Extension/Windows/UWindow.cpp:803`
- **What:** Also differs: the original compacts deleted and None entries out of actorList on every
  call. In DXEngine a destroyed actor's entry stays in a slot, nulled at the latest by
  CleanupDestroyed, and takes up room toward the 32-entry cap.
- **Evidence:** UWindow.cpp:242 and 1385: `if (actor && actor->bDeleteMe()) actor = nullptr;` in Add
  and Remove. UWindow.cpp:803: `if (refs[i].Actor == refActor) return true;` with no such check.
  DeusEx.u: `if ((Player.Level.TimeSeconds - lastTargetTime < 0.5) && IsActorValid(lastTarget))
  target = lastTarget;`.
- **Verified:** In the original, Add, Remove and IsActorValid (Extension.dll 0x10037f50, 0x10038060,
  0x10038130) all use one lookup at 0x10038400. That lookup sets an entry to None when its actor has
  byte 0x28 bit 0x80 set (bDeleteMe per the SDK's AActor bitfields), drops None entries, resets the
  count, and only then matches. So the original's IsActorValid answers False for a destroyed actor.
  DXEngine's IsActorValid (UWindow.cpp:801-805) compares raw pointers, and its Add/Remove never
  compact the list. CleanupDestroyed nulls the list only once 128 actors are deleted
  (ULevel.cpp:282).
- **Proof:** objdump 0x10038422-0x10038468: `test BYTE PTR [esi+0x28],0x80; je; mov DWORD PTR
  [edx],0x0` then compaction and `mov [ecx+0x998],ebx`. 0x10038130: `call 0x10038400; test eax,eax;
  jl` gives 0, else 1.

### EXT.5 NewChild makes any ModalWindow a modal, even one not under the root

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Extension/Windows/UWindow.cpp:1263`
- **What:** The original makes a ModalWindow a modal (type 2) only as a child of the root; anywhere
  else it logs 'Modal window must be a child of Root!' and the window stays a tab group (1).
  DXEngine sets windowType 2 for every UModalWindow, whatever its parent. A nested modal then breaks
  input. FindWindowByPoint passes over windowType >= 2 children, so no click reaches it.
  IsTraversable finds it as the nearest modal ancestor, which is never the root's topmost, so
  nothing inside it takes the focus. CollectTabGroupTable skips it. No stock screen nests one; a
  mod's would be dead.
- **Evidence:** extension-dll.md, Keyboard focus: 'A modal that is not the root's child logs "Modal
  window must be a child of Root!" and stays 1.' UWindow.cpp:1263: `else if
  (UObject::TryCast<UModalWindow>(child)) child->windowType() = 2;` with no check of `this`.
  URootWindow.cpp FindWindowByPoint: `if (child->windowType() < 2)`.
- **Verified:** The original XModalWindow::Init (0x10036c00) sets type 2 only when the parent is
  null or the parent's type byte is 3. Otherwise it logs "Modal window must be a child of Root!" and
  keeps the tab group's 1. DXEngine's NewChild (UWindow.cpp:1262) sets 2 for every UModalWindow.
  URootWindow.cpp:456 FindWindowByPoint skips type >= 2 children, and IsTraversable
  (UWindow.cpp:945-958) takes such a window as the first modal ancestor. No stock script nests one:
  the only DeusExBaseWindow made by NewChild is ConWindowActive, under rootWindow.
- **Proof:** objdump 0x10036c31: `cmp edi,ebx; jne; mov [esi+0x28],2` / `cmp BYTE PTR
  [edi+0x28],0x3; jne -> log`. strings -el Extension.dll: 'Modal window must be a child of Root!'.

### EXT.6 ExtString.GetNextTextPart empties its argument at the end, against its own comment

- **State:** confirmed, low (contract)
- **Where:** `DXEngine/DXEngine/Packages/Extension/UExtString.cpp:40`,
  `DXEngine/DXEngine/Packages/Extension/UExtString.cpp:43`
- **What:** Only a docs mismatch. The code matches the original, which also empties the argument
  when the length is a multiple of 239. The comment and extension-dll.md's 'its argument left as it
  was' should say that, rather than the code changing.
- **Evidence:** UExtString.cpp:36 comment: 'The next part, or 0 once none is left, the argument left
  as it was.' Line 40: `if (page > len / 239) return 0;`. Line 42: `count = std::min(len - 239 *
  page, 239)` gives 0 at len=478, page=2. Line 43 then assigns the empty substr. extension-dll.md,
  Small: 'once none is left gives 0, its argument left as it was.'
- **Verified:** The code does as the finding says: at len=478, page 2 passes `page > len/239`
  (UExtString.cpp:40), count is 0, and line 43 sets outText to "". The original's
  execGetNextTextPart (0x10044ad0) does the same: it zeroes the argument only when page > (len)/239,
  and otherwise assigns Mid(239*page, min(len-239*page, 239)), an empty string here. So the
  behaviour matches the original. What is wrong is the comment at UExtString.cpp:36 and
  extension-dll.md line 1262.
- **Proof:** objdump 0x10044b53-0x10044bb4: the page is compared with len/239 (magic divide). If not
  greater, `imul eax,esi,0xef; sub edx,eax`, clamped to 0xef, then Mid and assign to the out string.

## NET: the network

### NET.1 UdpLink.BindPort that fails leaves a blocking socket, so the next tick's recvfrom freezes
the game on Linux

- **State:** confirmed, medium (crash)
- **Where:** `DXEngine/DXEngine/Packages/IpDrv/UUdpLink.cpp:87`,
  `DXEngine/DXEngine/Packages/IpDrv/UUdpLink.cpp:102`,
  `DXEngine/DXEngine/Packages/IpDrv/UUdpLink.cpp:104`,
  `DXEngine/DXEngine/Packages/IpDrv/UUdpLink.cpp:46`,
  `DXEngine/DXEngine/Packages/IpDrv/UUdpLink.cpp:117`
- **What:** BindPort makes the socket (line 87) but only calls SetNonBlocking after a successful
  bind (line 104). If the bind fails, handle is still valid and the socket stays blocking and
  unbound. On the next tick in event mode, Tick calls recvfrom with no timeout (lines 46, 117). On
  Linux (both Linux ports and the Smart Pro) that call never returns, so the main thread hangs.
  ReadText and ReadBinary hang the same way. Windows gives WSAEINVAL at once.
- **Evidence:** The game's script expects BindPort to fail sometimes and keeps the actor ticking.
  DeusEx.u DeusExServerPing state Binding: `if( BindPort(2000, true) == 0 ) ...
  GotoState('BindFailed')`, which waits BindRetryTime and tries again. IpServer.u UdpServerQuery
  logs 'Port failed to bind.' and returns. ipdrv-dll.md (UdpLink) says the original's BindPort makes
  the socket non-blocking first and then binds. ReceiveMode defaults to RMODE_Event
  (UInternetLink::InitNativeDefaults).
- **Verified:** UUdpLink.cpp:87 makes a blocking socket, and :98-103 return on a failed bind before
  SetNonBlocking at :104, so handle stays valid. Tick (:38-46) in RMODE_Event calls Receive, which
  is a recvfrom with no flags (:117). ReadText/ReadBinary (:156, :130) do the same. ipdrv-dll.md:90
  says the original sets non-blocking when it makes the socket. BindSocket (UInternetLink.cpp:249)
  only tries the 20 ports; it never changes the blocking mode.
- **Proof:** Probe in scratch NET-verify-0/p.c: a blocking UDP socket whose bind fails (port already
  taken), then recvfrom -> 'BLOCKED (alarm fired)' after 2 s on this Linux host.

### NET.2 TcpLink's connectFailed is never cleared, so after one failed connect on Linux a later
Open never raises Opened

- **State:** confirmed, low (wrong-result)
- **Where:** `DXEngine/DXEngine/Packages/IpDrv/UTcpLink.h:64`,
  `DXEngine/DXEngine/Packages/IpDrv/UTcpLink.cpp:194`,
  `DXEngine/DXEngine/Packages/IpDrv/UTcpLink.cpp:214`,
  `DXEngine/DXEngine/Packages/IpDrv/UTcpLink.cpp:335`,
  `DXEngine/DXEngine/Packages/IpDrv/UTcpLink.cpp:352`
- **What:** When a connect fails, CheckConnectionAttempt sets connectFailed = true (line 214).
  Nothing resets it: not Close(), which closes the socket and goes back to Initialized, not BindPort
  or EnsureSocket, and not Open(). On the same link, Close() then BindPort() then Open(), or simply
  Open() again, makes a new connect. CheckConnectionAttempt then returns at line 194 every tick, so
  Opened never comes, even when the server is up. The link waits until the script times out.
- **Evidence:** `grep connectFailed` finds only the declaration (h:64), the test (cpp:194) and the
  set (cpp:214). The header comment calls it the memory of 'a connect that failed', but nothing ties
  it to that one connect. UBrowser.u UBrowserHTTPClient.Browse runs DoBind (BindPort plus Open)
  again on the same actor after SetError has closed it, so the script expects a link to be reused.
  On Windows a failed connect never turns writable (ipdrv-dll.md), so this only shows on Linux.
- **Verified:** grep finds connectFailed only at UTcpLink.h:64 (decl), cpp:194 (test), cpp:214
  (set). Close() in STATE_Connecting (cpp:364-375) closes handle and resets the state but not
  connectFailed. EnsureSocket (cpp:52) and Open (cpp:335) don't reset it either. A new Open
  therefore sets STATE_Connecting, and CheckConnectionAttempt returns at :194 on every tick, so
  Opened never comes. Only Linux can reach :214, because on Windows a failed connect never turns
  writable.
- **Proof:** Path: Open(closed port) -> Tick -> select writable, SO_ERROR=ECONNREFUSED ->
  connectFailed=true (:214). Close() -> handle closed, Initialized. Open(live server) ->
  EnsureSocket makes a new socket -> STATE_Connecting -> Tick -> :194 returns. Opened is never
  raised.

### NET.3 Vector NetSerialize adds the bias to a coordinate that may sit near INT_MAX: signed
overflow

- **State:** confirmed, low (undefined-behaviour)
- **Where:** `DXEngine/DXEngine/Network/NetSerialize.cpp:184`,
  `DXEngine/DXEngine/Network/NetSerialize.cpp:188`, `DXEngine/DXEngine/Network/NetSerialize.cpp:190`
- **What:** Round() caps a coordinate at plus or minus 2147483520, the fix for no-number and
  past-an-int values. The bit count is then capped at 15, so bias is 65536. For x above INT_MAX -
  65536, the expression `x + bias` (lines 190-192, both int) overflows a signed int, which is
  undefined behaviour. The E8.14 fix keeps a huge replicated Location or Velocity inside an int but
  not inside this sum.
- **Evidence:** int x = Round(v.x) can be 2147483520. bits = clamp(CeilLogTwo(1+largest),1,16)-1 =
  15, so bias = 1<<16. 2147483520 + 65536 > 2147483647. The cast to uint32_t comes after the signed
  add.
- **Verified:** Round (NetSerialize.cpp:40-47) clamps to +/-2147483520. With largest=2147483520,
  CeilLogTwo gives 31, the clamp makes it 16, so bits=15 and bias=65536 (:186-188). The int add
  x+bias at :190-192 then overflows before the cast to uint32_t. In practice it wraps, and the value
  is out of the field's range in any case, but it is UB in the language.
- **Proof:** g++ -fsanitize=undefined on a copy of Round/CeilLogTwo/bias with v.x=3e9: 'runtime
  error: signed integer overflow: 2147483520 + 65536 cannot be represented in type int'.

## REN: the renderer: the frame, the scene, lighting, textures and meshes

### REN.1 Water drops still write past the half-size height fields (E5b.18's fix is incomplete)

- **State:** confirmed, medium (memory)
- **Where:** `DXEngine/DXEngine/Packages/Engine/Resources/Textures/FireEngine.cpp:696`,
  `DXEngine/DXEngine/Packages/Engine/Resources/Textures/FireEngine.cpp:697`,
  `DXEngine/DXEngine/Packages/Engine/Resources/Textures/FireEngine.cpp:709`,
  `DXEngine/DXEngine/Packages/Engine/Resources/Textures/FireEngine.cpp:769`,
  `DXEngine/DXEngine/Packages/Engine/Resources/Textures/FireEngine.cpp:782`,
  `DXEngine/DXEngine/Packages/Engine/Resources/Textures/UWaterTexture.cpp:13`,
  `DXEngine/DXEngine/Packages/Engine/Resources/Textures/UWaterTexture.cpp:26`
- **What:** cell should mask X with hmask and Y with vmask and shift by log2(width) taken from the
  width. As it stands, any drop at Y >= VSize/2 writes up to about USize*VSize/2 bytes past the heap
  buffer.
- **Evidence:** UWaterTexture::Prepare: Fields.resize(w*h/2). RedrawDrops: cell = (x & UMask) + ((y
  & VMask) << UBits), so the largest value is w*h-1, and second[at] = Fields[w/2 + at]. Only
  RandomMover, Whirly, Rain and AreaClamp use hmask/vmask (UMask>>1). WaterPass reads the fields as
  rows = vsize/2 of usize bytes. Commit 103181d6 (E5b.18) claimed 'drops stay within it' and 'the
  row shift comes from the width', but it changed the shift only for fire (UFireTexture.cpp computes
  ubits). Water still takes mipmap.UBits.
- **Verified:** UWaterTexture.cpp:12 sizes Fields to w*h/2: vsize/2 rows of usize bytes, as
  WaterPass reads them (FireEngine.cpp:847-851, first at y*usize+x, second at +half). cell() at
  FireEngine.cpp:697 masks with the full UMask/VMask and shifts by the package's UBits
  (UWaterTexture.cpp:25), and second adds usize/2. So a drop with Y >= VSize/2 writes past the
  buffer. 103181d6 only computes ubits for fire; for water it only added the full-size masks.
- **Proof:** 256x256: cell(255,255)=255+(255<<8)=65535, plus second's 128 gives 65663, against
  Fields.size()=32768. The direct users of 'at' or of an unmasked axis (cases 0-3, 5, 8/12 Y, 9/13
  X, 18/19) all reach it. An X >= USize/2 also lands in the second field.

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

### REN.4 NEON NoneEffect still gives NaN for a texel at the light's point (E10b.4 fix missed the
NEON lanes)

- **State:** confirmed, low (undefined-behaviour)
- **Where:** `DXEngine/DXEngine/Light/LightEffect.cpp:179`,
  `DXEngine/DXEngine/Light/LightEffect.cpp:180`, `DXEngine/DXEngine/Light/LightEffect.cpp:187`,
  `DXEngine/DXEngine/Light/LightmapBuilder.cpp:253`
- **What:** On aarch64 (linux-aarch64, the Smart Pro) the NEON path computes rcpdist = 1/len with no
  len>0 guard. At len 0 that is 0*inf = NaN. inRange is true (0 < 1), so a NaN goes into the
  illumination map, and AddLightContributionDX converts it with (int)(NaN+0.5f), which is undefined
  behaviour; ARM gives 0, a dark texel where the original takes the angle as 1. The SSE2 and scalar
  paths got the guard; NEON did not.
- **Evidence:** Lines 179-197: len = vsqrtq; rcpdist = vdivq_f32(one, len); Lx*=rcpdist...; value
  stored and masked only by inRange. The SSE2 path (lines 124-133) masks with lenmask and forces the
  angle to 1. Commit c75839a5 says 'NoneEffect (its SSE2 and plain paths)' and touched only those.
  USE_NEON is defined in Precomp.h:39.
- **Verified:** The NEON block in LightEffect.cpp (rcpdist = vdivq_f32(one, len), about line 180)
  has no len>0 mask and no angle-forced-to-1. The SSE2 block has both (lenmask). At len 0: Lx*inf =
  0*inf = NaN, so the angle is NaN. inRange is true, so the NaN is stored. LightmapBuilder.cpp:253
  then does (int)(NaN+0.5f), which is UB. USE_NEON is defined for __ARM_NEON (Precomp.h:39), that is
  aarch64 builds.
- **Proof:** IEEE: 1/0=inf, 0*inf=NaN; vabsq(NaN)=NaN; NaN*shadow=NaN; vand with all-ones inRange
  keeps the NaN.

### REN.5 Warp zone clip plane: the normal is turned the opposite way to the point

- **State:** confirmed, low (wrong-result)
- **Where:** `DXEngine/DXEngine/Render/VisibleFrame.cpp:1219`,
  `DXEngine/DXEngine/Render/VisibleFrame.cpp:1224`,
  `DXEngine/DXEngine/Render/VisibleFrame.cpp:1230`, `DXEngine/DXEngine/Render/VisibleFrame.cpp:1113`
- **What:** WarpLocationToOtherSide and WarpRotationToOtherSide apply 'rotate', then the other
  side's transpose. WarpNormalToOtherSide applies the transpose first, then the other side's
  'rotate'. For any warp zone whose two sides are not turned alike, the portal plane built from
  these is wrong, and the warped frame is clipped by a tilted plane. No stock Deus Ex map has a
  WarpZoneInfo; a mod's warp is affected.
- **Evidence:** Location: p = rotate*(p-origin); p = transpose(rotateOther)*p + originOther. Normal:
  n = transpose(rotate)*n; n = rotateOther*n. These are the inverse rotations of each other unless
  the matrices are symmetric. DrawPortals uses both to make portalPlane(n, -dot(p,n)).
- **Verified:** WarpLocationToOtherSide maps p by Ro^T R (VisibleFrame.cpp:1219-1240), and so does
  WarpRotationToOtherSide. WarpNormalToOtherSide (1246-1262) applies R^T, then Ro: Ro R^T, which is
  the inverse rotation. A rigid rotation maps normals by the same matrix as directions, so the
  portal plane (line 1120) is wrong whenever the sides' rotations differ (not equal or 180 degrees
  apart in yaw). No stock map holds a WarpZoneInfo.
- **Proof:** Yaw-only example: R=rot(90), Ro=rot(0). Points turn by +90, the normal by -90, so the
  plane's normal is not perpendicular to the warped portal. strings over gamefiles/Maps/*.dx finds
  no WarpZoneInfo name in any map.

### REN.6 Textures stop stepping on frames where the world is not drawn

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Render/RenderScene.cpp:30`,
  `DXEngine/DXEngine/Render/RenderSubsystem.cpp:75`,
  `DXEngine/DXEngine/Render/RenderSubsystem.cpp:113`,
  `DXEngine/DXEngine/Packages/Engine/Resources/Textures/UTexture.cpp:110`
- **What:** TextureFrameCounter is advanced only in DrawScene. With the root window's rendering off
  (pause menus over the snapshot: ShowSnapshot calls EnableRendering(False); the credits), or with
  bNoDrawWorld set, UpdateTexture sees tex->FrameCounter == TextureFrameCounter and never steps a
  texture drawn only by the windows or the canvas, so animated and fractal UI textures freeze.
  ChangedThisFrame then answers true on every draw, so a texture changed once is re-uploaded every
  frame.
- **Evidence:** RenderSubsystem::DrawGame calls DrawScene only when bRender is on. UpdateTexture: if
  (tex->FrameCounter != TextureFrameCounter) {Update...}. fire-dll.md, stepping: the original's
  UTexture::Update ticks by the texture's own InternalTime against the current time, 'once for each
  new time', whether or not a 3D scene was drawn.
- **Verified:** TextureFrameCounter++ happens only in DrawScene (RenderScene.cpp:30). DrawGame skips
  DrawScene with bNoDrawWorld or rootWindow bRender off (RenderSubsystem.cpp:67-79), and
  PostRenderFlash still draws the windows. UpdateTexture (113) then never re-steps a texture already
  stamped with the counter. In the original, Update ticks once for each new time
  (fire-dll.md:30-32), and DXEngine's own clock, LevelTime, keeps advancing (Engine.cpp:328,432).
  ChangedThisFrame also returns true every draw once ModifiedFrame equals the frozen counter.
- **Proof:** Code path: with bRender false across frames, TextureFrameCounter stays constant, so
  tex->FrameCounter == TextureFrameCounter after the first draw and Update is skipped.

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

### VK.1 ReadPixels reads a GPU_TO_CPU staging buffer without a host barrier or invalidate

- **State:** confirmed, medium (platform)
- **Where:** `DXEngine/DXEngine/RenderDevice/Vulkan/VulkanRenderDevice.cpp:821`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/VulkanRenderDevice.cpp:840`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/VulkanRenderDevice.cpp:847`,
  `DXEngine/DXGPU/include/dxgpu/vulkanobjects.h:506`
- **What:** The spec is broken on every device. Wrong pixels appear only where VMA picks a cached,
  non-coherent type, as some PowerVR and Adreno drivers expose. Desktop x86 GPUs and lavapipe expose
  coherent types, so they are likely unaffected. The effect is a wrong snapshot or screenshot image,
  not corrupt save data.
- **Evidence:** The barrier after copyImageToBuffer only moves the image back to SHADER_READ
  (VulkanRenderDevice.cpp:840-842). There is no AddMemory(TRANSFER_WRITE->HOST_READ) to stage HOST.
  VulkanBuffer::Map is a plain vmaMapMemory. VMA's GPU_TO_CPU case (vk_mem_alloc.h:3594-3596) sets
  required HOST_VISIBLE and preferred HOST_CACHED. Spec: a fence's memory dependency covers only
  device accesses, so host visibility needs a barrier to HOST, and non-coherent memory also needs
  vkInvalidateMappedMemoryRanges.
- **Verified:** ReadPixels (VulkanRenderDevice.cpp:819-850) allocates its staging buffer with
  VMA_MEMORY_USAGE_GPU_TO_CPU, which requires HOST_VISIBLE and prefers HOST_CACHED, not
  HOST_COHERENT (vk_mem_alloc.h:3594-3596). After copyImageToBuffer, the only barrier is the image's
  transition back to SHADER_READ (840-842). Neither the device code nor SubmitCommands has a
  TRANSFER_WRITE->HOST_READ barrier, and Map is a bare vmaMapMemory (vulkanobjects.h:506-511) with
  no vmaInvalidateAllocation anywhere. By the spec, a fence wait does not make device writes visible
  to the host.
- **Proof:** grep for HOST_READ or vmaInvalidate in RenderDevice/Vulkan and DXGPU/src: no hits,
  apart from debugFullPipelineBarrier, which ReadPixels never calls.

### VK.2 Vulkan clip distance (or its discard) is on in every frame, not just mirror frames as the
docs say

- **State:** confirmed, low (docs)
- **Where:** `DXEngine/DXEngine/RenderDevice/Vulkan/VulkanRenderDevice.cpp:939`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/FileResource.cpp:41`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/FileResource.cpp:117`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/ShaderManager.cpp:28`,
  `DXEngine/DXEngine/RenderDevice/Vulkan/RenderPassManager.cpp:131`,
  `DXEngine/DXEngine/RenderDevice/RenderDevice.h:30`, `DXEngine/docs/NATIVES.md:513`,
  `DXEngine/docs/ENGINE.md:523`
- **What:** Every frame, the main view included, takes the frame's NearClip. Its default (0,0,1,-1)
  is the near plane at view z=1. So geometry nearer than 1 unit is clipped, and the depth clamp
  enabled 'to avoid clipping the weapon' never acts at the near plane. GL keeps such geometry. On a
  device without shaderClipDistance (the Smart Pro's GE8300, where Vulkan is offered), every scene
  draw is compiled with a discard, which costs a tile-based GPU its hidden-surface removal.
- **Evidence:** SetSceneNode always sets pushconstants.nearClip from Frame->NearClip (default z-1 at
  RenderDevice.h:30). Scene.vert writes gl_ClipDistance[0]=dot(nearClip,p), or under CLIP_DISCARD
  Scene.frag discards when it is <0. CLIP_DISCARD goes into the one scene shader for all pipelines
  (ShaderManager.cpp:28-32). GL turns the clip on only when NearClip != default
  (GLRenderDevice.cpp:1967). NATIVES.md:513-516 says Vulkan discards 'in the mirror's frame alone'.
  ENGINE.md:523 says Vulkan clamps the near plane for 'the weapon close up'.
- **Verified:** VulkanRenderDevice.cpp:939 sets nearClip from Frame->NearClip in every scene frame.
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

## GL: the OpenGL and OpenGL ES device

### GL.1 After a skipped frame with a screen flash, the next frame draws a negative index count and
Unlock throws "Drawing failed"

- **State:** confirmed, high (crash)
- **Where:** `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:1879`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:1144`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:1219`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:1181`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:2147`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:2152`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:2244`
- **What:** The skipped frame's flash draw is worse than stale data: the bound VAO is PPStepLayout,
  which has no element buffer (989-994), and the core or ES context then raises
  GL_INVALID_OPERATION. Nothing reads that error. The next Lock's resize picks it up at its first
  ThrowIfGLError and fails again. FlashScale never refreshes while frames are skipped, so the scene
  buffers may never recover (needs a run to confirm). Each skipped frame adds 6 to the start.
- **Evidence:** EndFlash (1879) calls SetPipeline, ReserveVertices and DrawBatches with no IsLocked
  check, while every Draw*, ClearZ and TakeBloom check it. ReserveVertices only checks
  SceneVertices, which still points at the staging arrays from the last good frame. DrawBatches ends
  with Batch.SceneIndexStart = SceneIndexPos (2147). Unlock's early return at 1219-1220 skips the
  reset at 1227. Lock's MapVertices(true) (1181) zeroes GLSceneVertexPos, SceneIndexPos and
  Uploaded*, not Batch.SceneIndexStart. AddDrawBatch then pushes start 6, end 0, so icount = 0-6 as
  size_t (2152) and (GLsizei)icount = -6 (2244). ThrowIfGLError("Drawing failed") (1225) throws. The
  skipped frame's upload is also skipped, since 4 < UploadedVertexPos, so its flash quad draws stale
  indices.
- **Verified:** Lock skips the frame at GLRenderDevice.cpp:1144 before it sets FlashScale and
  FlashFog (1149), so they keep the last good frame's flash. RenderSubsystem.cpp:82 still calls
  EndFlash, which has no IsLocked check (1879). DrawBatches returns early, then SetPipeline and
  SetDescriptorSet(0) run (nulltex was set at 1115). ReserveVertices sees SceneVertices non-null,
  UseVertices sets SceneIndexPos to 6, and DrawBatches leaves Batch.SceneIndexStart at 6 (2147).
  Unlock returns at 1219 before the reset at 1227. The next good Lock's MapVertices(true)
  (1088-1091) zeroes SceneIndexPos but not Batch.SceneIndexStart, so AddDrawBatch (2110) queues
  [6,0) and glDrawElements gets (GLsizei)-6.
- **Proof:** Path traced by hand. Next good frame: DrawBatches early-out (2125) fails because 6 !=
  0. AddDrawBatch sets SceneIndexEnd=0 with start 6. DrawEntry: icount=(size_t)(0-6),
  (GLsizei)icount=-6, which is GL_INVALID_VALUE. Unlock 1225 ThrowIfGLError("Drawing failed") throws
  std::runtime_error. DrawGame (Engine.cpp:432) has no catch around it.

### GL.2 ReadPixels dereferences null framebuffers after a failed resize, crashing a screenshot,
save or menu snapshot

- **State:** confirmed, high (crash)
- **Where:** `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:1767`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:1782`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:1818`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.cpp:260`,
  `DXEngine/DXEngine/RenderDevice/OpenGL/GLRenderDevice.h:88`
- **What:** The crash needs a throw before PPFramebuffer[0] and [1] exist. A throw at a blur level
  does not crash. The callers are Engine.cpp:1467 and URootWindow.cpp:53.
- **Evidence:** Line 260, SceneBuffers.Width = width, comes before the allocations that can throw.
  GLRenderDevice.h:88 makes LastFrameSize return SceneBuffers.Width and Height, so
  RenderSubsystem::ReadLastFrame's width/height > 0 check passes. FrameDrawn = true is set
  unconditionally (RenderSubsystem.cpp:89). ReadPixels has no check:
  `SceneBuffers.PPFramebuffer[1]->Handle` (1782) with GammaCorrectScreenshots, else `fb->Handle`
  with fb = PPFramebuffer[0].get() (1818-1819). Callers: Engine.cpp:1457 (save),
  Engine::TakeScreenshot, URootWindow.cpp:35.
- **Verified:** ResizeSceneBuffers resets every buffer (243-258) and sets Width and Height at
  260-262. Only then does it allocate, and it can throw from ThrowIfGLError (300, 316, 330, 360,
  369) or from an incomplete framebuffer (351). Lock catches the throw and returns (1132-1145).
  RenderSubsystem sets FrameDrawn=true anyway (RenderSubsystem.cpp:89). LastFrameSize
  (GLRenderDevice.h:88) returns the new Width and Height, which are above 0. ReadLastFrame
  (RenderSubsystem.cpp:92-102) then calls ReadPixels, which dereferences PPFramebuffer[1]->Handle
  (1782) or fb->Handle (1818-1819) with no check.
- **Proof:** Path traced: a throw at the ColorBuffer, DepthBuffer, PPImage, Framebuffer or
  PPFramebuffer step leaves PPFramebuffer[0] and [1] null, with Width > 0. The callers are
  Engine.cpp:1467 (the save's and shot's snapshot) and URootWindow.cpp:53 (the menu snapshot), not
  1457 and 35. A throw only at a bloom level leaves the PP framebuffers whole, so it does not crash.

## SWD3D: the software and D3D11 devices

### SWD3D.1 D3D11 P8 uploader reads through a null palette (the E11b.7 fix was never applied to
D3D11)

- **State:** confirmed, medium (crash)
- **Where:** `DXEngine/DXEngine/RenderDevice/D3D11/D3D11TextureUploader.cpp:40`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11TextureUploader.cpp:53`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11TextureUploader.cpp:65`
- **What:** Some P8 textures have no palette (TextureInfo.Palette left null), such as a scripted,
  ice or wet texture copied from a palette-less source in a mod. When the D3D11 device draws one,
  its P8 uploader indexes palette[idx] through the null pointer and the game crashes on Windows when
  Settings.json Type is D3D11. This affects the whole-texture and rectangle uploads, masked or not.
- **Evidence:** D3D11TextureUploader_P8::UploadRect has no null check: `*Ptr++ = palette[idx]`.
  DXEngine 5d77968e (E11b.7) added a grey-level palette when it is null to
  Vulkan/TextureUploader.cpp:98 and OpenGL/GLTextureUploader.cpp:96, with the comment 'the palette
  was read through null'. The software device draws such a texture in grey levels
  (SoftwareTextureUploader.cpp:112-115). RenderCanvas.cpp:197, VisibleDecal.cpp:41 and
  VisibleCorona.cpp:44 set Palette only `if (Tex->Palette())`. D3D11 is reachable:
  LauncherSettings.cpp:28.
- **Verified:** D3D11TextureUploader_P8::UploadRect (D3D11TextureUploader.cpp:40-69) indexes
  palette[idx] with no null check. UploadManager passes Info.Palette straight through
  (D3D11UploadManager.cpp:118, 149). RenderCanvas.cpp:197/224 set Palette only when Tex->Palette()
  exists, so it can stay nullptr (RenderDevice.h:110). git show 5d77968e --stat shows E11b.7 touched
  only GLTextureUploader.cpp and Vulkan/TextureUploader.cpp. Severity medium, not high: no launcher
  offers D3D11, so it is reached only by a hand-set Settings.json Type D3D11
  (LauncherSettings.cpp:28).
- **Proof:** 5d77968e: 2 files changed, GL and Vulkan uploaders only. D3D11 P8 lines 53 and 65
  dereference palette unconditionally.

### SWD3D.2 D3D11 texture upload trusts the mip chain: short mips are over-read, and a misfit chain
makes CreateTexture2D throw mid-draw

- **State:** confirmed, medium (memory)
- **Where:** `DXEngine/DXEngine/RenderDevice/D3D11/D3D11UploadManager.cpp:24`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11UploadManager.cpp:72`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11UploadManager.cpp:79`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11UploadManager.cpp:137`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11UploadManager.cpp:149`
- **What:** D3D11's UploadTexture never checks a mip's bytes against its size, and MipLevels is
  Info.NumMips unclamped. A package's mip that is short of its bytes is read past its end (a heap
  over-read). A chain with more levels than log2 of the size makes CreateTexture2D return
  E_INVALIDARG, and ThrowIfFailed then throws out of GetTexture in the middle of a draw call. A
  level whose mip is not the level's size gets a box outside its subresource.
- **Evidence:** UploadTexture checks only width and height >= 1 and the size limit, then sets
  `texDesc.MipLevels = mipcount` (=Info.NumMips) and calls ThrowIfFailed. UploadData skips only
  `Mip->Data.empty()` and copies mipwidth x mipheight from Mip->Data with each mip's own dims.
  Vulkan's fix 3e2ddf2b (E11b.1/E11b.2) makes a top mip short of SourceBytes the white texel
  (UploadManager.cpp:50), clamps the levels to the chain (UploadManager.cpp:58-62) and writes zeros
  for a misfit level (UploadManager.cpp:140). The software device checks ValidMip
  (SoftwareTextureUploader.cpp:49-52).
- **Verified:** UploadTexture (D3D11UploadManager.cpp:24-37) never checks Mips[0].Data.size()
  against its size, and sets MipLevels = Info.NumMips unclamped (27, 72), followed by ThrowIfFailed
  (79). UploadData (137-160) skips only empty mips, then copies each mip at its own Width/Height,
  with no size or fit check. Vulkan's UploadManager has all three guards (the SourceBytes check,
  maxLevels clamping, the 'fits' zeros). D3D11 got none of them.
- **Proof:** Vulkan/UploadManager.cpp: `Info.Mips[0].Data.size() < SourceBytes(...)`, `mipcount =
  std::min(mipcount, maxLevels)`, `bool fits = ...`. None of these is in D3D11UploadManager.cpp.

### SWD3D.3 D3D11 starts a frame on a NO_OVERWRITE mapping after any Flush between frames,
overwriting vertices the GPU may still read

- **State:** confirmed, low (race)
- **Where:** `DXEngine/DXEngine/RenderDevice/D3D11/D3D11RenderDevice.cpp:2164`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11RenderDevice.cpp:2174`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11RenderDevice.cpp:1305`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11RenderDevice.cpp:1347`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11RenderDevice.cpp:1401`,
  `DXEngine/DXEngine/RenderDevice/D3D11/D3D11RenderDevice.cpp:384`
- **What:** Unlock unmaps the scene buffers and resets the vertex and index positions to 0. A Flush
  outside a frame runs DrawBatches(), which ends with MapVertices(false) and maps both buffers with
  WRITE_NO_OVERWRITE. Such a Flush comes from SetRes inside Lock on every window resize, from
  RenderSubsystem::FlushDevice or OnMapLoaded, or from the 'flush' command. Lock's MapVertices(true)
  then does nothing because the buffers are already mapped, so the new frame writes from offset 0
  without DISCARD while the last frame's draws over the same range may still be in flight (up to 3
  buffers). The geometry of the frame after a resize or flush can come out corrupted.
- **Evidence:** DrawBatches always calls `MapVertices(nextBuffer)`, which is false from Flush.
  MapVertices maps only `if (!SceneVertices)`, with `nextBuffer ? WRITE_DISCARD :
  WRITE_NO_OVERWRITE`. The order is Lock -> SetRes (1347) -> Flush(1) (384) -> DrawBatches ->
  NO_OVERWRITE map, then `MapVertices(true)` at 1401 is a no-op. Unlock (1440-1444) zeroes
  D3D11SceneVertexPos and SceneIndexPos. The GL device returns early from DrawBatches when nothing
  is queued (GLRenderDevice.cpp:2125).
- **Verified:** Flush (D3D11RenderDevice.cpp:1296-1303) calls DrawBatches() with nextBuffer=false
  (header default, .h:269). DrawBatches maps both buffers at 2174 whether or not IsLocked. Unlock
  (1439-1444) unmaps and zeroes the positions. The path Lock -> SetRes (1347) -> Flush(1) (384), or
  a Flush between frames, leaves the buffers mapped NO_OVERWRITE. Lock's MapVertices(true) at 1401
  then skips them because SceneVertices is non-null, so the frame writes from 0 with no DISCARD.
  That breaks NO_OVERWRITE's contract while the earlier frame's draws over that range may still be
  in flight. Low: it hits one frame after a flush, on a device no launcher offers.
- **Proof:** MapVertices maps only `if (!SceneVertices)`. DrawBatches(false) -> MapVertices(false)
  -> D3D11_MAP_WRITE_NO_OVERWRITE. No IsLocked guard in Flush or DrawBatches.

### SWD3D.4 Software device: a window over 8000 px is cropped, not scaled; the E13.4 fix fits the
frame but not the game's viewport

- **State:** confirmed, low (wrong-result)
- **Where:** `DXEngine/DXEngine/RenderDevice/Software/SoftwareRenderDevice.cpp:84`,
  `DXEngine/DXEngine/RenderDevice/Software/SoftwareRenderDevice.cpp:86`,
  `DXEngine/DXEngine/RenderDevice/Software/SoftwareRenderDevice.cpp:667`,
  `DXEngine/DXEngine/RenderDevice/Software/SoftwareSetup.cpp:224`,
  `DXEngine/DXEngine/GameWindow.cpp:24`
- **What:** Lock shrinks the frame to fit 8000 px, but the game still lays out its scene nodes and
  canvas at GetRenderWidth/Height (GameWindow::GetPixelWidth, RenderDeviceCanvas::begin). In a
  10000-wide window the frame is 8000 wide while SetSceneNode's viewport is 10000 wide. The scissor
  and the 8000 guard-band clip drop the right and bottom fifth of the view and HUD, and the present
  stretches the remaining 80% by 1.25 over the window. ENGINE.md says such a window 'is drawn
  smaller and scaled up'.
- **Evidence:** Lock: `fit = min(1, 8000/max(w,h)); Frame.Resize(w*fit, h*fit)`. SetSceneNode copies
  Frame->X/Y/XB/YB unscaled into State. Scissor clips to `min(ViewportX+ViewportWidth,
  frame.Width)`. ClipPlanes clips at ±8000 px of the origin. GameWindow::GetPixelWidth returns
  device->GetRenderWidth(), and RenderDevice.cpp:141-146 sets the canvas frame to GetRenderWidth().
  Present.Prepare scales the 8000 frame to the window (scale 1.25).
- **Verified:** Lock (SoftwareRenderDevice.cpp:84-86) shrinks only Frame. GetRenderWidth
  (RenderDevice.cpp:119) is non-virtual, at native*RenderScale, and is what
  GameWindow::GetPixelWidth and the canvas see. SetSceneNode (667-670) copies Frame->X/XB unscaled.
  SoftwareSetup.cpp:96-98 maps NDC to that viewport, and Scissor (226-229) clips to frame.Width. So
  in a 10000-wide window, game x>=8000 is dropped and the rest is stretched. ENGINE.md:559 claims it
  is scaled. 5018e71d changed only Frame.Resize. Low: an edge case (a window over 8000 px).
- **Proof:** 5018e71d diff: only Frame.Resize changed. Nothing scales State.Viewport* by the fit
  factor.

### SWD3D.5 Software device: a refused background thread throws std::system_error out of Lock or a
draw (the E13.9 fix covers only the pool)

- **State:** confirmed, low (error-handling)
- **Where:** `DXEngine/DXEngine/RenderDevice/Software/SoftwareWorkers.cpp:227`,
  `DXEngine/DXEngine/RenderDevice/Software/SoftwareWorkers.cpp:228`,
  `DXEngine/DXEngine/RenderDevice/Software/SoftwareRenderDevice.cpp:47`,
  `DXEngine/DXEngine/RenderDevice/Software/SoftwareRenderDevice.cpp:179`
- **What:** The unguarded thread creation is at
  DXEngine/DXEngine/RenderDevice/Software/SoftwareWorkers.cpp:157-158 (Start), not lines 227-228.
- **Evidence:** Start: `if (!BackgroundThread.joinable()) BackgroundThread = std::thread(...)` with
  no catch. The constructor (SoftwareWorkers.cpp:109-119) catches std::system_error for the pool.
  Streaming is set at SoftwareRenderDevice.cpp:47 from Threads() alone. LaunchWave (179) is called
  from Lock (97) and AddDraw (352).
- **Verified:** SoftwareWorkers::Start creates BackgroundThread with std::thread and no try/catch.
  The pool's own creation, in the constructor at SoftwareWorkers.cpp:39-49, catches
  std::system_error. Streaming depends only on Threads() (SoftwareRenderDevice.cpp ~46), and
  LaunchWave calls Workers->Start from Lock and AddDraw. Its line numbers are wrong: Start is at
  SoftwareWorkers.cpp:155-158, not 227-228.
- **Proof:** SoftwareWorkers.cpp:157-158: `if (!BackgroundThread.joinable()) BackgroundThread =
  std::thread([this]() { Background(); });`, unguarded.

## AUD: the audio

### AUD.1 Every tracker song plays with the second half of each 256-float chunk zeroed: OpenMPT's
ReadSamples returns frames, and the caller reads that count as floats

- **State:** confirmed, high (wrong-result)
- **Where:** `DXEngine/DXEngine/Audio/AudioSource.cpp:400`,
  `DXEngine/DXEngine/Audio/AudioDevice.cpp:379`, `DXEngine/DXEngine/Audio/AudioDevice.cpp:381`,
  `DXEngine/DXEngine/Packages/Engine/Subsystems/UDXEngineAudioDevice.cpp:184`,
  `DXEngine/Thirdparty/openmpt/libopenmpt/libopenmpt.hpp:813`
- **What:** The music thread asks for 256 floats (128 stereo frames) per chunk.
  OpenMPTAudioSource::ReadSamples renders 128 frames (256 floats) but returns
  read_interleaved_stereo's frame count, 128. The caller treats got=128 as floats and zero-fills
  chunk[128..256). So frames 64-127 of every chunk become silence while the song still advances 128
  frames. Every stock song is a tracker module, so all music plays half-gated at about 344 Hz (a
  buzz), and half of the song's audio is thrown away.
- **Evidence:** AudioSource.cpp:400 `return module->read_interleaved_stereo(GetFrequency(), samples
  / GetChannels(), output);`. libopenmpt.hpp:811-813: count is 'audio frames to render per channel',
  '\return The number of frames actually rendered.' AudioDevice.cpp:379-381: `got =
  ReadSamples(chunk, musicBufferSize); if (got < musicBufferSize) std::fill(chunk + got, chunk +
  musicBufferSize, 0)`, with musicBufferSize 256 (InitDevice). The other sources return floats: Mp3
  returns samples, Wav frames*channels, Ogg frames*channels. The fill came from 2134e729 (E15a.3),
  whose message assumed that 'tracker modules ... never read short'.
- **Verified:** UDXEngineAudioDevice.cpp:184 creates the device with musicBufferSize 256 floats.
  AudioDevice.cpp:379 calls ReadSamples(chunk, 256). AudioSource.cpp:400 asks
  read_interleaved_stereo for 256/2=128 frames and returns its result, which libopenmpt.hpp:813
  documents as frames. So got=128 and AudioDevice.cpp:380-381 zero the floats chunk[128..256), which
  are stereo frames 64-127. The Mp3 source (73-98) returns samples, and Ogg (329) returns
  frames*channels, so only the tracker path miscounts. The bug came in with E15a.3's fill
  (2134e729), so it is not a re-report of a fixed item.
- **Proof:** AudioSource.cpp:400 `return module->read_interleaved_stereo(GetFrequency(), samples /
  GetChannels(), output);` and libopenmpt.hpp:813 `\return The number of frames actually rendered.`
  AudioDevice.cpp:380 `if (got < (size_t)musicBufferSize) std::fill(chunk + got, chunk +
  musicBufferSize, 0.0f);` with 256 at UDXEngineAudioDevice.cpp:184.

### AUD.2 An obstructed sound whose actor is gone, and speech, jump to obstruction 0 at once; Galaxy
lets the time shrink

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Packages/Engine/Subsystems/UDXEngineAudioDevice.cpp:461`,
  `DXEngine/DXEngine/Packages/Engine/Subsystems/UDXEngineAudioDevice.cpp:463`
- **What:** Only the actor-gone case is audible: speech's time is never raised, so zeroing it
  matches the original in effect.
- **Evidence:** UDXEngineAudioDevice.cpp:461-465: `if (!Playing.Actor || (Playing.Id & 14) ==
  SLOT_Talk * 2) { Playing.ObstructionTime = 0.0f; return; }`. galaxy-dll.md:177-178 (Sounds behind
  walls): 'Speech and a sound whose actor has gone run no line: their time only shrinks.' The
  obstruction factor is max(1-2t, 0.33) (line 448).
- **Verified:** UDXEngineAudioDevice.cpp:461-464 sets ObstructionTime=0 and returns when the actor
  is gone or the slot is speech. galaxy-dll.md:177-178 says these run no line and their time only
  shrinks. Line 706 sets Actor to nullptr when the actor is destroyed, so the condition occurs, and
  a muffled sound (volume down to 0.33, line 448) jumps to full volume in one frame instead of
  fading back over up to 0.5 s. NATIVES.md's Sound Differs (lines 1124-1145) does not list this. For
  speech the effect is nil in practice, because its time never grows.
- **Proof:** Lines 461-464: `if (!Playing.Actor || (Playing.Id & 14) == SLOT_Talk * 2) {
  Playing.ObstructionTime = 0.0f; return; }`; line 498 has the shrink branch, which the early return
  skips.

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
  entry. NATIVES.md:736 'Head turns and lip sync' covers only the blend animations.

## ENG: the main loop, startup, settings, input and the engine's utilities

### ENG.1 Two keys on one input axis do not add up: the later press replaces the earlier, and
releasing either key stops the axis

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXEngine/Engine.cpp:3473`, `DXEngine/DXEngine/Engine.cpp:3293`,
  `DXEngine/DXEngine/Engine.cpp:2877`, `DXEngine/DXEngine/Engine.h:333`
- **What:** activeInputAxes keeps one entry per axis name, holding one key. Hold W (MoveForward,
  aBaseY +300), then tap S (aBaseY -300) and let S go. The S release erases the aBaseY entry, so the
  player stops while W is still held. OS key repeat does not resume W, because only the last key
  pressed repeats. The same happens for W with Up, A with D, Jump with Duck (both aUp), and a stick
  with a key on the same axis. In the original each held key's binding runs again every frame and
  the values add, so W with S cancels and W resumes once S is let go.
- **Evidence:** Line 3473: activeInputAxes[args[1]] = { speed * delta, key }; overwrites the entry.
  Line 3293: a release erases the entry only when it->second.Key == key. DefUser.ini binds W and Up
  to MoveForward, S and Down to MoveBackward, and Jump and Duck both to aUp. engine-dll.md 'A key's
  binding', UInput::ReadInput 0x10394580: each key still down gets its binding again as held, and
  with -1 every input float is set to 0, so held keys accumulate into a float that is cleared each
  tick. NATIVES.md 'Input axes' lists no Differs for this.
- **Verified:** Engine.h:333 activeInputAxes is std::map<std::string,ActiveInputAxis>, one
  {Value,Key} per axis name. Engine.cpp:3473 overwrites it on every press, and :3291-3301 on a
  release erases every entry whose Key is the released key, setting the axis to 0. UpdateInput
  (:2876-2886) writes each entry's Value as it stands, with no sum. engine-dll.md 1499-1506 says the
  original runs each held key's binding again on every tick and clears the floats with -1, and UE1's
  AXIS adds to the float. NATIVES.md 1593-1598 lists no Differs for this.
- **Proof:** Hold W: aBaseY={+300*20,W}. Press S: aBaseY={-300*20,S}. Release S: the Key==S entry is
  erased and aBaseY is set to 0, so nothing moves although W is still down. W's key repeat stopped
  when S was pressed, so no new W press arrives. Releasing W while Up is still held does the same.

### ENG.2 GameApp's error handler reads the global commandline after the CommandLine it points to
has been destroyed

- **State:** confirmed, medium (memory)
- **Where:** `DXEngine/DXEngine/GameApp.cpp:54`, `DXEngine/DXEngine/GameApp.cpp:55`,
  `DXEngine/DXEngine/GameApp.cpp:131`
- **What:** cmd is a local inside the try block, and the global commandline points to it. When any
  startup or Engine::Run exception reaches the catch, the stack unwind has already destroyed cmd,
  whose std::map nodes are freed. The catch then calls commandline->HasArg("-n", "--no-launcher"),
  which reads freed memory. The result is undefined behaviour on every fatal error. It can crash in
  the error path, or it can wrongly open the modal error window on a headless rig that passed
  --no-launcher.
- **Evidence:** GameApp.cpp:54 has `CommandLine cmd(args);` and :55 has `commandline = &cmd;`, both
  inside try{}. :131 is inside catch: `if (widgets && (!commandline || !commandline->HasArg("-n",
  "--no-launcher")))`. The pointer is never reset, so the !commandline guard does not help. C++
  destroys a try block's automatic objects before its handler runs.
- **Verified:** GameApp.cpp:54-55 build `CommandLine cmd` inside try{} and point the global
  commandline at it. Nothing resets the pointer. Any throw after widgets=true (:91), whether from
  GameFolderSelection, Engine's constructor or Run, reaches the catch at :131, which calls
  commandline->HasArg on the destroyed object. The Args std::map has already been destroyed and its
  nodes freed, so this is undefined behaviour on every fatal error.
- **Proof:** A probe of the same pattern under g++ -fsanitize=address reports: ERROR:
  AddressSanitizer: stack-use-after-scope in std::_Rb_tree::find from CL::Has, called in the catch.

### ENG.3 An exception from an engine command run by a key binding, such as SHOT failing to write
its file, ends the game

- **State:** confirmed, medium (error-handling)
- **Where:** `DXEngine/DXEngine/Engine.cpp:3485`, `DXEngine/DXEngine/Engine.cpp:1474`,
  `DXEngine/DXEngine/Engine.cpp:1518`, `DXEngine/DXEngine/Engine.cpp:664`
- **What:** A key's engine command goes from InputCommand to ConsoleCommand outside any native, so
  nothing catches it. The exception travels up through the window's event dispatch and out of
  Engine::Run, and the game stops with the fatal error window. A realistic trigger is DefUser.ini's
  GreyStar=Shot when no --userdir is given, which is main's recreated launcher. TakeScreenshot then
  writes into the game's System folder, and File::create_always throws there if the folder is
  read-only or the disk is full (Directory::create can throw too). EXEC= lines at startup and
  OnLauncherLine's Open run through the same uncaught path. The same commands typed in the console
  are caught by Frame::CallNative.
- **Evidence:** Engine.cpp:3485 calls ConsoleCommand(viewport->Actor(), ...) from InputCommand,
  which is reached from OnWindowKeyDown during GameWindow::ProcessEvents (GameWindow.cpp:179, no
  catch). TakeScreenshot :1518 calls File::create_always, which throws 'Could not create'
  (File.cpp). Only CallNative (VM/Frame.cpp:344-361) catches std::exception. The original's SHOT
  that cannot open its file writes nothing and carries on.
- **Verified:** The path is UpdateInput:2869 TickWindow, then :3019 GameWindow::ProcessEvents, the
  backend's dispatch, GameWindow::OnKeyDown:133, Engine::OnWindowKeyDown:3134, InputEvent,
  InputCommand:3485, ConsoleCommand, 'shot' at :2360-2362, then TakeScreenshot:1518
  File::create_always, which throws 'Could not create' (File.cpp:225). With --userdir,
  Directory::create at :1474 can throw too. None of these has a try/catch: Engine.cpp's only ones
  are at 637, 1555, 2574, 2597, 2636, 2691 and 2919, and the DXWidgets window code catches only in
  backend creation. The exception leaves Run for GameApp's catch, which shows the fatal window.
  main's launcher (launch.c:281) passes no --userdir, so the shot goes to the System folder.
- **Proof:** Exact path traced above. The trigger is DefUser.ini's GreyStar=Shot. No exec function
  named Shot exists, so ExecCommand misses and the line goes to ConsoleCommand.

### ENG.4 The engine's own SWITCHLEVEL shadows PlayerPawn's exec SwitchLevel and lets a net client
load a map locally

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Engine.cpp:2534`, `DXEngine/DXEngine/Engine.cpp:2562`
- **What:** ConsoleCommand handles 'switchlevel' before ExecCommand reaches the pawn's exec
  functions. It sets LevelInfo->NextURL directly, skipping Engine.u's PlayerPawn.SwitchLevel, which
  runs Level.ServerTravel only for an admin, standalone or a listen server. Typed on a net client,
  DXEngine's main loop then loads the map on the client itself, while the original does nothing. In
  standalone it also skips LevelInfo.ServerTravel and Game.ProcessServerTravel.
- **Evidence:** Engine.u script: `exec function SwitchLevel( string URL ) if( bAdmin ||
  Level.NetMode==NM_Standalone || Level.netMode==NM_ListenServer ) Level.ServerTravel( URL, false
  );`. engine-dll.md: UPlayer::Exec offers the player pawn's exec functions before the engine's
  Exec. network.md's list of UGameEngine::Exec words has no SWITCHLEVEL. NATIVES.md's Differs covers
  only the viewport's own words.
- **Verified:** Engine.cpp:2534 handles 'switchlevel' inside ConsoleCommand, before the ExecCommand
  fallback at :2765, so the console's text (NConsole.cpp:16) and a key's line never reach
  PlayerPawn's exec SwitchLevel. It sets LevelInfo->NextURL at :2562 with no check of net mode or
  admin. The main loop at :445-490 then loads NextURL with no client guard. Engine.u's exec
  SwitchLevel calls Level.ServerTravel only when bAdmin, standalone or a listen server. No DLL of
  the game has the wide string SWITCHLEVEL, so the original has no native command of that name.
- **Proof:** `strings Engine.u`: exec function SwitchLevel( string URL ) if( bAdmin ||
  Level.NetMode==NM_Standalone || Level.netMode==NM_ListenServer ) Level.ServerTravel( URL, false ).
  `strings -el` finds SWITCHLEVEL 0 times in Engine.dll and every other System/*.dll and *.exe,
  while SERVERTRAVEL, DISCONNECT and RECONNECT are each found once.

### ENG.5 Key names and alias names are matched case-sensitively, unlike the original's name lookups

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/Engine.cpp:2661`, `DXEngine/DXEngine/Engine.cpp:2445`,
  `DXEngine/DXEngine/Engine.cpp:3259`, `DXEngine/DXEngine/Engine.cpp:3264`,
  `DXEngine/DXEngine/Engine.h:322`
- **What:** keybindings and inputAliases are std::map<std::string,...>, which compare
  case-sensitively. 'set input w MoveForward' stores the key 'w', but a press looks up keynames'
  'W', so the new binding does nothing until the next run reloads it from the ini. KEYBINDING w
  returns empty. A User.ini line such as W=moveforward finds no alias and falls through to 'Unknown
  command'. The original's key and alias names are FNames, compared without case.
- **Evidence:** Engine.h:322 declares std::map<std::string,std::string> keybindings, inputAliases.
  :2661 is keybindings[propertyName.ToString()] = value. :3259 is keybindings[keynames[key]]. :3264
  is inputAliases.find(command) on the raw text.
- **Verified:** Engine.h:322 declares std::map<std::string,std::string> keybindings and
  inputAliases, which compare case-sensitively. SET stores keybindings[propertyName.ToString()] as
  typed (:2661), but a press reads keybindings[keynames[key]] (:3259) with the canonical case, and
  KEYBINDING (:2445) also uses the raw text. Aliases are found with inputAliases.find(command)
  (:3261) on the exact text. The original's keys and aliases are FNames, which compare without case.
  The game's own menus send KEYNAME's canonical names, so only text a user types or edits is
  affected.
- **Proof:** 'set input w Jump' stores keybindings["w"]. Pressing IK_W reads keybindings["W"], which
  still holds the old line from LoadKeybindings (:2835), so no Jump runs until the next start
  reloads it from the ini.

### ENG.6 ~Engine can throw while writing DXE-Log-LastRun.txt, and a throw from a destructor calls
std::terminate

- **State:** confirmed, low (error-handling)
- **Where:** `DXEngine/DXEngine/Engine.cpp:189`, `DXEngine/DXEngine/Engine.cpp:194`
- **What:** SaveLogAsPlaintext calls File::write_all_text, which throws when the file cannot be
  created or the write or close fails (ENOSPC, EIO, a read-only folder). Destructors are implicitly
  noexcept, so the process aborts instead of exiting cleanly. If the destructor runs while an
  exception from Run is already unwinding, the abort happens before the error window appears.
- **Evidence:** Engine.cpp:194 calls Logger::SaveLogAsPlaintext, which calls File::write_all_text,
  which reaches create_always/write/close, all of which Exception::Throw on failure (File.cpp). No
  try/catch in ~Engine.
- **Verified:** ~Engine (Engine.cpp:189-197) calls Logger::SaveLogAsPlaintext (Logger.cpp:141),
  which calls File::write_all_text (File.cpp:257-261): create_always, write and close all throw on
  failure. ~Engine is not noexcept(false), so it is implicitly noexcept and a throw calls
  std::terminate. When an exception from Run is already unwinding, the same terminate comes before
  GameApp's catch can show the error window. The folder normally exists: CheckCrashReporter creates
  engineUserDir/CrashReports at startup, ErrorWindow.cpp:52-53. So the trigger needs a full disk, an
  I/O error or a folder made read-only later.
- **Proof:** Path traced: Engine.h:75 declares ~Engine() with no noexcept(false), and ~Engine has no
  try/catch.

## WIN: the window system: DXWidgets and the game window

### WIN.1 The X11 RunLoop busy-spins with no timer: GetTimerTimeout answers 0, a poll, not -1

- **State:** confirmed, medium (wrong-result)
- **Where:** `DXEngine/DXWidgets/src/window/x11/x11_connection.cpp:271`,
  `DXEngine/DXWidgets/src/window/x11/x11_connection.cpp:128`,
  `DXEngine/DXWidgets/src/window/x11/x11_connection.cpp:156`,
  `DXEngine/DXEngine/UI/ErrorWindow/ErrorWindow.cpp:70`
- **What:** When no timer is running, GetTimerTimeout returns 0. WaitForEvents then passes select a
  zeroed timeval, so the call returns at once and RunLoop spins at 100% of a core. This hits the
  error window (ErrorWindow::ExecModal runs RunLoop, and its scrollbar timer only starts on a press)
  under the X11 backend. X11 is the default on linux-x86_64 when the engine is run without the
  launcher, and it is also the harness's backend.
- **Evidence:** x11_connection.cpp:271-272: `if (timers.empty()) return 0;`. WaitForEvents:156
  passes `timeout >= 0 ? &tv : nullptr`, so 0 becomes a zero timeval (a poll). Wayland's twin had
  this fix (bugs E16b.30): wayland_display_backend.cpp:690-693 returns -1 with the comment '0
  returned at once, spinning'. X11 has no such fix. Timer::Timer (core/timer.cpp:6) starts nothing,
  so the error window has no timer.
- **Verified:** x11_connection.cpp:271-272 returns 0 when timers is empty. WaitForEvents:146-156
  leaves tv at {0,0} for timeout 0 and passes &tv because timeout>=0, so select polls and returns at
  once. RunLoop:125-129 then loops ProcessEvents (XPending, which does not block) and WaitForEvents
  with no wait. ErrorWindow::ExecModal:70 runs DisplayWindow::RunLoop. Wayland's GetTimerTimeout
  (wayland_display_backend.cpp:690-693) returns -1 for this case (E16b.30); X11 got no such fix. X11
  is the backend picked when there is no Wayland display, and it is the harness's.
- **Proof:** With no timer: GetTimerTimeout()=0 -> tv={0,0} -> select(fd+1,&rfds,0,0,&tv) returns 0
  at once -> the while loop repeats with no wait, spinning a core while the error window is open.

### WIN.2 No window backend leaves DisplayBackend null, and the game window's creation then
dereferences it: a segfault, not the reported error

- **State:** confirmed, medium (crash)
- **Where:** `DXEngine/DXWidgets/src/window/window.cpp:128`,
  `DXEngine/DXEngine/UI/WidgetResourceData.cpp:98`, `DXEngine/DXWidgets/src/window/window.cpp:14`,
  `DXEngine/DXEngine/GameApp.cpp:85`
- **What:** With no display (no DISPLAY or WAYLAND_DISPLAY, e.g. over ssh), every Try* returns
  nullptr, and InitWidgetResources stores null without throwing. A client run then reaches
  Engine::OpenWindow, then GameWindow, then the Widget constructor, then DisplayWindow::Create,
  which calls DisplayBackend::Get()->Create on null and segfaults. There is no 'DXEngine error:'
  line and no error window (the error window would crash the same way). GameApp's comment says such
  a failure (SDL's) is reported like any other.
- **Evidence:** window.cpp:111-128: when every backend fails it returns nullptr.
  WidgetResourceData.cpp:98-99 calls Set(TryCreateBackend()) with no check. window.cpp:14 does
  `return DisplayBackend::Get()->Create(...)`. widget.cpp:15 calls DisplayWindow::Create for every
  Window widget. Engine.cpp:2954-2955 also calls backend->IsSDL3() unchecked. Only the dedicated
  server skips UpdateInput (Engine.cpp:369), so only it survives.
- **Verified:** Each Try* backend catches its constructor's throw and returns nullptr
  (window.cpp:155-247; X11 throws 'Could not open X11 display' at x11_connection.cpp:19-20).
  TryCreateBackend can therefore return null (window.cpp:111-128). InitWidgetResources
  (WidgetResourceData.cpp:96-99) Sets that null without a check, and GameApp.cpp:90 continues. Later
  DisplayBackend::Get() is null, and Engine.cpp:2954-2955 (backend->IsSDL3()), DisplayWindow::Create
  (window.cpp:14) and ErrorWindow (GetScreenSize, RunLoop) dereference it. The result is a segfault
  with no 'DXEngine error:' line, which contradicts GameApp.cpp:85-89's comment.
- **Proof:** No DISPLAY or WAYLAND_DISPLAY and no SDL video driver -> every Try* returns nullptr ->
  Set(nullptr) -> Engine::OpenWindow calls DisplayBackend::Get()->IsSDL3() on null (or Widget ->
  DisplayWindow::Create calls Get()->Create) -> SIGSEGV.

### WIN.3 The windowed game is placed with its client area at the screen's (0,0), so on Windows its
title bar sits above the screen

- **State:** confirmed, medium (wrong-result)
- **Where:** `DXEngine/DXEngine/Engine.cpp:2964`, `DXEngine/DXEngine/GameWindow.cpp:42`,
  `DXEngine/DXWidgets/src/window/sdl3/sdl3_display_window.cpp:168`,
  `DXEngine/DXWidgets/src/window/win32/win32_display_window.cpp:281`
- **What:** Deterministic on Windows (SDL3 and Win32 backends). On X11 and Wayland the result
  depends on the window manager (Wayland ignores the position). The RE does not document where the
  original places its window.
- **Evidence:** Engine.cpp:2964 `SetFrameGeometry(Rect::xywh(0.0, 0.0, width / dpi, height / dpi))`.
  sdl3_display_window.cpp:168 calls SDL_SetWindowPosition(x,y). SDL3's WIN_AdjustWindowRectWithStyle
  (SDL_windowswindow.c:209-241) starts from rect top=0, runs AdjustWindowRectEx, then does `*y +=
  rect.top`, which is negative for a captioned window. win32_display_window.cpp:273-283 does the
  same through DelayLoadAdjustWindowRectExForDpi. GameWindow.cpp:42-46 reuses GetFrameGeometry's x
  and y.
- **Verified:** Engine.cpp:2964 sets the frame to xywh(0,0,w,h). Win32's SetClientFrame
  (win32_display_window.cpp:270-283) takes the rect as the client area, runs
  AdjustWindowRectExForDpi (which makes rect.top negative for a captioned style) and calls
  SetWindowPos with that rect, so the caption lands above y=0. SDL3's SetClientFrame
  (sdl3_display_window.cpp:156-169) calls SDL_SetWindowPosition(0,0). SDL3's
  WIN_AdjustWindowRectWithStyle does *y += rect.top after AdjustWindowRectEx, and the game window is
  not borderless (flags at :23-29, only a Popup is). GameWindow.cpp:42-46 reuses that x,y when
  leaving full screen.
- **Proof:** Client y=0 -> AdjustWindowRectEx on WS_OVERLAPPEDWINDOW gives rect.top of about -31 ->
  SetWindowPos(hwnd, x<0, y<0, ...) -> the title bar is off the top of the screen.

### WIN.4 Alt+Enter does not toggle full screen as the original's window does; it reaches the game
as Enter

- **State:** confirmed, medium (differs-from-original)
- **Where:** `DXEngine/DXWidgets/src/window/sdl3/sdl3_display_window.cpp:713`,
  `DXEngine/DXWidgets/src/window/sdl2/sdl2_display_window.cpp:714`,
  `DXEngine/DXEngine/Engine.cpp:3124`
- **What:** In the original, Enter with Alt held toggles full screen instead of pressing Enter.
  DXEngine has no Alt+Enter handling in any window backend or in the engine, so the press goes to
  the input as IK_Enter with Alt down, and the window keeps its mode. NATIVES.md lists no such
  difference.
- **Evidence:** dx-reverse-info/windrv-dll.md, the keys: 'Enter with Alt held toggles fullscreen
  instead (0x1110a28e)'. grep finds no Alt+Enter or IK_Enter handling in DXEngine/*.cpp or
  DXWidgets/src. The only way to toggle is the console's togglefullscreen (Engine.cpp:2741).
  SDL3/SDL2 OnKeyDown just forward ScancodeToInputKey.
- **Verified:** windrv-dll.md:62-63: 'Enter with Alt held toggles fullscreen instead (0x1110a28e)'.
  No backend checks for Alt+Enter: SDL3 and SDL2 OnKeyDown just forward the scancode, and Win32's
  WM_SYSKEYDOWN (win32_display_window.cpp:910-918) forwards the key. GameWindow::OnKeyDown
  (GameWindow.cpp:130-133) and Engine::OnWindowKeyDown (Engine.cpp:3124-3135) pass it to InputEvent.
  The only toggle is the console's togglefullscreen (Engine.cpp:2741). NATIVES.md's 'A key's name'
  Differs (1616-1634) and the rest of NATIVES.md do not mention Alt+Enter.
- **Proof:** grep for alt+enter, IK_Enter or Enter handling in DXEngine/*.cpp and DXWidgets/src
  finds only key-name mapping tables, and nothing in any docs.

### WIN.5 Under the SDL3 window on Windows, Alt+F4 closes the game, where the original's window
takes it as keys and closes nothing

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXWidgets/src/window/sdl3/sdl3_display_window.cpp:669`,
  `DXEngine/DXWidgets/src/window/sdl3/sdl3_display_window.cpp:19`,
  `DXEngine/DXEngine/Engine.cpp:3154`
- **What:** SDL3 generates a window-close event for Alt+F4 by default
  (SDL_HINT_WINDOWS_CLOSE_ON_ALT_F4 defaults to true), and DXEngine never sets that hint. On
  windows-x86_64, whose engine uses SDL3's window, Alt+F4 therefore raises
  SDL_EVENT_WINDOW_CLOSE_REQUESTED, and OnWindowClose sets quit. The Win32 backend was fixed to
  match the original (bugs E16a.18); the shipped SDL3 path was not.
- **Evidence:** windrv-dll.md: WM_SYSKEYDOWN is handled and returned, 'DefWindowProc never seeing
  them ... Alt+F4 closes nothing'. In the vendored SDL3, SDL_hints.h:4507-4521 gives the default as
  1, and SDL_windowsevents.c:405-407 and 1589 act on it. grep shows no SDL_SetHint in DXEngine or
  DXWidgets.
- **Verified:** deps/prefix/windows-x86_64/include/SDL3/SDL_hints.h:4507-4521 gives
  SDL_HINT_WINDOWS_CLOSE_ON_ALT_F4 a default of 1, meaning a window-close event. Neither DXEngine
  nor DXWidgets calls SDL_SetHint, and the windows-x86_64 launcher sets only ALLOW_SCREENSAVER and
  NO_SIGNAL_HANDLERS, for its own window. SDL3 is the first backend on Windows (window.cpp:115-118).
  sdl3_display_window.cpp:669-670 maps CLOSE_REQUESTED to OnWindowClose, and Engine.cpp:3154-3156
  sets quit. windrv-dll.md:63-64: in the original, 'Alt+F4 closes nothing'. NATIVES.md has no
  Differs entry for this.
- **Proof:** Alt+F4 on windows-x86_64 -> SDL raises SDL_EVENT_WINDOW_CLOSE_REQUESTED ->
  WindowHost->OnWindowClose() -> Engine quit=true.

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

### LCORE.2 An install is accepted with System in any case, but the engine only finds the exact name
System

- **State:** confirmed, low (contract)
- **Where:** `deusex-launcher/linux-x86_64/src/core/install.c:28-33`,
  `deusex-launcher/linux-x86_64/src/core/finder.c:77-85`, `DXEngine/DXEngine/UE1GameDatabase.cpp:14`
- **What:** dxl_install_check and dxl_finder_has_game look up the System folder with
  dxl_path_resolve_ci, so a copy whose folder is `system` (an innoextract --lowercase install, for
  one) is found, marked ok and played. The engine passes the root to FindUE1GameInPath, which opens
  root/"System"/DeusEx.exe exactly with fopen. On a case-sensitive filesystem (ext4) the engine
  reports "No UE1 game found", even though the launcher showed the install as playable. install.h
  says the check is how the engine finds it.
- **Evidence:** install.c:28 `char *sys = dxl_path_resolve_ci(out->root, "System");`, then the exe
  is joined to it. finder.c:76 has the comment "in a System of any case". UE1GameDatabase.cpp:14 has
  `fs::path(root) / "System"`, and SHA1Sum::of_file uses File::open_existing, which is plain fopen
  (Utils/File.cpp:230-236). install.h:4-5 says "System/DeusEx.exe is there by that exact name, which
  is how the engine finds it". run.c:276 passes install.root as the engine's game folder.
- **Verified:** install.c:28 resolves System with dxl_path_resolve_ci (paths.c:95-104 matches any
  case when the exact name is missing), and it sets ok from that. run.c:276 passes install.root to
  the engine. GameFolder.cpp:87 calls FindUE1GameInPath, which opens root/"System"/DeusEx.exe by
  that exact name (UE1GameDatabase.cpp:14-20). On ext4, a root/system tree is accepted by the
  launcher and not found by the engine. install.h:4-5 and LAUNCHER.md:69 say the exact name is how
  the engine finds it, so the code contradicts its own docs.
- **Proof:** Code path: dxl_install_check -> resolve_ci('system') -> exe found -> ok=1. Then
  engine_argv .game=root -> FindUE1GameInPath(root) -> SHA1Sum::of_file(root/System/DeusEx.exe) is
  empty -> UE1_GAME_NOT_FOUND.

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
  overlay, so this is the launcher's own behaviour falling short of what LAUNCHER.md:112 describes.
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
  Recovery, which quits at once on *stop, then calls end_without_game. LAUNCHER.md:331 says a run
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
  So 20 quick wakes reach DXL_PLAY_KILL_AFTER_MS long before 5 s, against LAUNCHER.md:672-674
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
  activates. shell.c:820-822 still draws a clickable 'Cancel' hint, and LAUNCHER.md:413-414 says 'a
  hint clicked does what its key does'. A click there reaches dxl_key_slot as DXL_SLOT_CAPTURED and
  captured() maps it to dxl_key_from_mouse(1). LAUNCHER.md:524 says a slot takes the next mouse
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
  Done 'Play' (547) and sets result=1 (564-566). LAUNCHER.md:360-361 and 865 give Play only to the
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

## MAIN: the recreation: deusex-launcher's main

### MAIN.1 Renderer page's detection run gets a mangled command line: wrong DescFlags section, a log
named Detected.log"

- **State:** confirmed, medium (differs-from-original)
- **Where:** `deusex-launcher/main/src/wizard/wizard.c:27`,
  `deusex-launcher/main/src/wizard/wizard.c:305`, `deusex-launcher/main/src/core/argv.c:5`,
  `deusex-launcher/main/src/core/cmdline.c:28`, `deusex-launcher/main/src/core/policy.c:47`,
  `deusex-launcher/main/src/launch/launch.c:89`, `deusex-launcher/main/src/core/renderdev.c:182`
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
  test_launch.c:290 passes two argv words, so the tests miss this.
- **Verified:** wizard.c:305 passes DETECT_FLAGS to dxl_child_start. posix/process.c:71
  dxl_argv_line makes it argv[1], one word. The child's launch.c:309 dxl_cmdline_join runs it
  through quoted_word (cmdline.c:28): the word holds a space, no quote and no leading blank, so it
  is quoted after its '='. policy.c:47 then takes the device as the whole quoted text,
  renderdev.c:182 writes DescFlags=2 under that junk section, and log_path_of gets
  LOG=`Detected.log"`. test_launch.c:290 passes two argv words, so the tests never take this path.
- **Proof:** A scratch build of cmdline.c with argv {DeusEx, DETECT_FLAGS} printed: joined
  [testrendev="D3DDrv.D3DRenderDevice log=Detected.log"], testrendev [D3DDrv.D3DRenderDevice
  log=Detected.log], LOG [Detected.log"].

### MAIN.2 CD check rebuilds System/ by literal name: a lowercase 'system' folder fails the check
forever

- **State:** confirmed, low (wrong-result)
- **Where:** `deusex-launcher/main/src/core/install.c:14`,
  `deusex-launcher/main/src/launch/launch.c:190`
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
  PackageManager.cpp:1137-1146 and 1162-1168 test with fs::exists, which is exact on Linux. When the
  file is missing the engine seeds it from Default.ini or DefUser.ini and writes it at that
  exact-case name. So on an install holding only deusex.ini or user.ini, the engine plays a fresh
  file and never sees the wizard's choices.
- **Proof:** Read path: dxl_config_path gives .../deusex.ini, while run-game.sh passes
  .../DeusEx.ini. The engine's fs::exists(.../DeusEx.ini) is false, so it takes the Default.ini
  branch (PackageManager.cpp:1143-1146).

### MAIN.4 'Another instance' is probed then released before the lock is taken; a failed acquire is
ignored

- **State:** confirmed, low (race)
- **Where:** `deusex-launcher/main/src/launch/launch.c:335`,
  `deusex-launcher/main/src/launch/launch.c:369`, `deusex-launcher/main/src/launch/launch.c:376`
- **What:** other is taken from a flock that is tried and released at line 335. The lock itself is
  taken only at line 369, after the config, the strings and the splash (SDL start-up) are done. Two
  launches started close together can both see other=0. Both then empty the log (dxl_log_open_new),
  and with a stale Running.ini both open RecoveryMode. The second's dxl_instance_acquire returns
  NULL, but that result is never folded into other_instance. The original's CreateMutex check is
  atomic (ERROR_ALREADY_EXISTS).
- **Evidence:** launch.c:335 int other = dxl_instance_other_running(r.id); instance.c:150-152 takes
  and releases the lock. Line 369 r.inst = ... acquire, and its NULL result is not used for other.
  Line 376 .other_instance = other. launch-flow.md §5.3: the mutex is made, and finding it there
  already records another live instance.
- **Verified:** launch.c:335 sets other from dxl_instance_other_running, which takes the flock and
  releases it at once (instance.c:150-155). The lock is taken only at launch.c:369, after the config
  load and the splash. A NULL result from dxl_instance_acquire is never folded into other, and line
  376 passes other unchanged. In the original, CreateMutex at InitEngine step 3 is atomic: the
  second process sees ERROR_ALREADY_EXISTS (launch-flow.md §5.3). Two launches inside that window
  both get other=0. Both then empty the log, and with a stale Running.ini both take the crash path.
  The window is narrow, so low.
- **Proof:** launch.c: nothing after line 369 reads !r.inst to set other or in.other_instance. other
  is assigned only at line 335.

### MAIN.5 Single-instance lock and handoff are per install; the original's are global

- **State:** confirmed, low (differs-from-original)
- **Where:** `deusex-launcher/main/src/core/instance.c:23`,
  `deusex-launcher/main/src/core/instance.c:62`, `deusex-launcher/main/src/core/instance.c:72`,
  `deusex-launcher/main/src/launch/launch.c:321`
- **What:** The lock and the socket are named by a hash of the install's real path. A second launch
  of another install, a copy or a mod folder, does not forward to the running game, and its stale
  Running.ini opens RecoveryMode even while a game runs. In the original, the named mutex
  DeusExIsRunning and the IsBrowser WLog window are found whatever the install. README's 'Where it
  differs from the original' does not list this.
- **Evidence:** instance.c:23-31 id_hash(id) is used in both the socket path (line 62) and the lock
  path (line 72), with id = realpath(base) (launch.c:313-314). launch-flow.md §1: it enumerates
  every top-level WLog window with IsBrowser. §5.3 and platform seam 3: one named mutex. The
  README's differences list has no entry for this.
- **Verified:** instance.c:62 and :72 name the socket and the lock by id_hash(id), and
  launch.c:313-314 sets id to realpath(base). The original's mutex DeusExIsRunning and its
  WLog/IsBrowser lookup carry no path (launch-flow.md §1, §5 step 3, seam 2-3). So a second install
  neither forwards to a running game nor sees it as another instance. instance.c's comment shows the
  per-install naming is deliberate (commit 5ded611), but README's 'Where it differs from the
  original' does not list it, so it is an undocumented difference from main's 1:1 recreation.
- **Proof:** instance.c:62 "%s/deusex-launcher.%08lx.sock" and :72 "%s/deusex-launcher.%08lx.lock"
  both use id_hash(id). README.md:104-145 has no entry on instance scope.

## X: across the repositories: the launcher-engine contract, the scripts, CI, the harness

### X.1 publish's last_release picks the lexically largest tag of a day, not the newest, so a later
run can re-create an existing tag and fail

- **State:** confirmed, medium (wrong-result)
- **Where:** `port-ex-machina/scripts/ci.sh:282-285`, `port-ex-machina/scripts/ci.sh:263-275`
- **What:** Two same-day releases are enough: if the second one's sha8 sorts below the first's, the
  next push that day re-creates the second tag.
- **Evidence:** last_release: `awk ... { print $2, $1 }' | sort | tail -n 1` sorts on the tag name.
  The tag is "$date-${eng:0:8}" (line 266), date = `date -u +%Y.%m.%d`. ENGINE-PIN.txt history shows
  several engine pins on one day (2026-10-22: 36f783c, 9d8a70a, 381b310). Example: tags
  2026.10.22-3749626x, -cc0f268x, -a628827x all exist. A 4th push that day keeps the engine at
  a628827 but changes a launcher: last=cc0f268, so code_between(cc0f268,a628827) is true and the tag
  2026.10.22-a628827x is made again.
- **Verified:** port-ex-machina/scripts/ci.sh:282-285: last_release prints '<tag> <sha>' and sorts
  the tag string, so tags from one day are ordered by sha8, not by when they were made. cmd_publish
  (263-275) then runs code_between(last, pin). That diff is non-empty when last is an older same-day
  commit, so release() calls gh release create for '$date-${eng:0:8}'. That tag already exists, gh
  fails, and rc=1. The release notes are also built from the wrong base. The same happens for each
  port's '$p-$date-${pin:0:8}' tag.
- **Proof:** I fed the awk|sort|tail pipeline three ls-remote lines for 2026.10.22-3749626a,
  -cc0f268a and -a628827a, listed in the order they were made. It printed cc0f268aaaa, not
  a628827aaaa, the newest.

### X.2 Launcher accepts a game whose System folder is in another case; the engine looks only for
exact "System" and refuses it

- **State:** confirmed, low (contract)
- **Where:** `deusex-launcher/linux-x86_64/src/core/install.c:28-33`,
  `DXEngine/DXEngine/UE1GameDatabase.cpp:15-19`,
  `deusex-launcher/linux-x86_64/src/core/install.h:4-6`
- **What:** dxl_install_check finds the System folder case-insensitively ("system", "SYSTEM") and
  accepts the install once DeusEx.exe's SHA1 matches. The engine builds root/"System"/"DeusEx.exe"
  exactly. On a case-sensitive Linux file system it finds no game, throws "No UE1 game found in the
  given folder" and exits 1. So the launcher offers a game the engine will not start. install.h says
  the opposite: System/DeusEx.exe is there "by that exact name, which is how the engine finds it".
- **Evidence:** install.c:28 `char *sys = dxl_path_resolve_ci(out->root, "System");` and :32
  `dxl_path_join(out->system_dir, "DeusEx.exe")`. paths.c:95-104 matches folder entries with
  dxl_stricmp. UE1GameDatabase.cpp:15 `fs::path(root) / "System"`, then SHA1Sum::of_file, which is
  empty when the file is missing, so not found. GameApp.cpp throws when Games is empty. The same
  code is on every port branch.
- **Verified:** deusex-launcher/linux-x86_64/src/core/install.c:28-32 finds the System folder with
  dxl_path_resolve_ci, in any case, then joins DeusEx.exe to it. So 'system/DeusEx.exe' is found and
  its SHA1 is checked. DXEngine/DXEngine/UE1GameDatabase.cpp:15 builds root/"System" exactly. On a
  case-sensitive file system SHA1Sum::of_file returns empty and the result is UE1_GAME_NOT_FOUND.
  ExamineFolder (GameFolder.cpp:87) then gives no game. The launcher accepts an install the engine
  refuses. This contradicts install.h:4-5 ('by that exact name, which is how the engine finds it')
  and the comment at install.c:31. LAUNCHER.md's Known defects do not list it.
- **Proof:** Path: install.c:28 resolve_ci(root,"System") -> system_dir=root/system -> exists ->
  found. Engine: fs::path(root)/"System"/"DeusEx.exe" -> missing -> continue -> NOT_FOUND.

### X.3 The original's -lanplay given through --cmdline is ignored: a dedicated server ticks at
NetServerMaxTickRate

- **State:** confirmed, low (differs-from-original)
- **Where:** `DXEngine/DXEngine/GameFolder.cpp:57-77`, `DXEngine/DXEngine/Engine.cpp:2246`
- **What:** The original's dedicated server runs at LanServerMaxTickRate when its command line has
  -lanplay (network.md). DXEngine sets lanPlay only from its own --lanplay option. GetLaunchInfo
  maps -server, INI=, USERINI= and the start URL from the original's line, but not -lanplay. A
  server started by a launcher with "map -server -lanplay" in the player's words therefore ticks at
  20 (Default.ini NetServerMaxTickRate) instead of 35. NATIVES.md's command-line section names no
  Differs for it.
- **Evidence:** dx-reverse-info/network.md:404-407: dedicated server at `NetServerMaxTickRate`
  (`LanServerMaxTickRate` with `-lanplay`), UGameEngine::GetMaxTickRate 0x1038fc60.
  GameFolder.cpp:57 `info.lanPlay = commandline->HasArg("", "--lanplay")`. Lines 62-77 read only
  original.Param("server"), INI and USERINI. grep finds no other lanplay use.
  gamefiles/System/DeusEx.ini:130-131 has 20 and 35.
- **Verified:** dx-reverse-info/network.md:404-407: the original's dedicated server ticks at
  LanServerMaxTickRate with -lanplay. DXEngine/DXEngine/GameFolder.cpp:62-76 maps only the start
  URL, -server, INI= and USERINI= from OriginalCommandLine. lanPlay comes only from --lanplay
  (GameFolder.cpp:57). Engine.cpp:2246 therefore uses NetServerMaxTickRate. NATIVES.md 'The command
  line' (1657-1711) says flags are found as the original finds them and lists what DXEngine does
  with each flag. -lanplay is not in that list, not under 'Nothing to do', and there is no Differs
  paragraph for it. ENGINE.md:129-132 also leaves it out of what --cmdline carries.
- **Proof:** grep -ri lanplay DXEngine/DXEngine finds only the --lanplay option (GameApp.cpp:98,163;
  GameFolder.cpp:57; GameFolder.h:13; Engine.cpp:2239,2246). Nothing reads it from
  OriginalCommandLine.

### X.4 dx_fetch_debs and dx_fetch_arch_pkg keep a partly downloaded file as if complete, and the
next runs fail on it

- **State:** confirmed, low (error-handling)
- **Where:** `port-ex-machina/scripts/lib/common.sh:327-339`,
  `port-ex-machina/scripts/lib/common.sh:360-365`
- **What:** Both helpers download straight into the cache file and treat any non-empty file (-s) as
  cached. If curl fails mid-transfer, the partial .deb or .pkg.tar.zst stays. Every later deps run
  skips the download and fails in ar/tar until someone deletes the file by hand. A truncated
  Packages.xz (curl | xz -d > Packages) leaves a partial index, which then gives a misleading "no
  package X". dx_fetch_cached and dx_fetch_src in the same file avoid this with a .part file and a
  checksum.
- **Evidence:** common.sh:336-338 `if [ ! -s "$deb" ]; then curl -fsSL --retry 3 -o "$deb" ...`.
  :327-329 `[ ! -s "$cache/Packages" ]` then `curl ... | xz -d > "$cache/Packages"`. :360-363 the
  same -s check and direct -o for the Arch package. Compare :139-140 (`-o "$tar.part"` then mv) and
  :391-396.
- **Verified:** port-ex-machina/scripts/lib/common.sh:336-338 and 360-364 download with curl -o
  straight into the cache file. curl keeps a partial file when a transfer fails, unless
  --remove-on-error is given. The next run sees a non-empty file (-s), skips the download, and fails
  in ar x or tar. At 327-329, 'curl | xz -d > Packages' leaves a truncated index after a failure. A
  later run then reports a misleading 'no package X'. dx_fetch_cached (139-141) downloads to .part,
  moves it into place and checks its sha256, so it does not have this problem.
- **Proof:** Code path read: if [ ! -s "$deb" ]; then curl -fsSL --retry 3 -o "$deb" ...; fi, then
  (cd "$tmp" && ar x "$deb"). A truncated $deb is never fetched again.

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
