# What Surreal Engine lacks

Deus Ex's natives and native C++ against the engine fork's: what is missing or
wrong, what the player sees of it, and the work that would fill it. What each
original function does is in its DLL's doc ([the binaries](README.md#the-binaries));
what each patch changed is in [what the fork changes](../ENGINE.md#what-the-fork-changes).
Which item is taken up, and when, is the owner's call.

## How it is known

- **The audit.** [`tools/natives_audit.py`](../../tools/natives_audit.py)
  lists every native the Deus Ex packages declare against the fork function
  registered for it, with Surreal's per-game conditions evaluated as Deus Ex.
  It tells whether that function is a stub (it, or the package method it hands
  over to, logs `Unimplemented`), an iterator that makes no iterator, or
  missing. It counts each native's call sites in the game's scripts, and with
  `--runs` it reads engine logs for which stubs fired where. Run it for the
  current state; this doc does not copy its listing.
- **Play.** Unaltered desktop runs of five maps, 80 s each from the start with
  the player untouched (2026-09-24, the fork at patch 0034): the menu map
  `DX.dx`, `01_NYC_UNATCOIsland.dx`, `01_NYC_UNATCOHQ.dx`,
  `02_NYC_BatteryPark.dx` and `06_HongKong_WanChai_Market.dx`. A stub logs
  once per session, with the script function that called it, so a run shows
  which stubs a map reaches, not how often. Two more runs, with a temporary
  hook that typed console commands, tried saving and loading
  ([below](#saving-loading-and-travel)).
- **The DLLs.** C++ that is not a native -- a class's own `Tick`, what the
  renderer does with an actor -- leaves no stub behind: only reading the
  original shows it is missing.
- **Reading both.** Only reading the original shows whether an implemented
  native does what it does. Found so far: `IsValidEnemy` (fixed by patch
  0034), and the ones under [not as the original](#implemented-not-as-the-original).
  Code compiled out with `#if 0` also leaves no stub; the audit reports it as
  partial.

## Stops the game

The fork ends the game on an error it does not catch -- a script error, an
unknown native, a failed save: the engine exits with 1, and the launcher shows
its crash banner.

### An NPC searching in Battery Park: `ReachablePathnodes`

`Pawn.ReachablePathnodes` (native 1004) is an iterator: the script walks it
with `foreach`. The fork registers it as a plain function that only logs, so
the `foreach` has no iterator and the VM stops the game. `ScriptedPawn`
reaches it from `GetOvershootDestination`, when an NPC in its Seeking state
guesses where a lost target went, and from `ComputeAwayVector`. In the Battery Park run an
NSF terrorist got there about 20 s in: "Iterator statement without an
iterator in Terrorist11.GetOvershootDestination", exit 1. Before patch 0034
no NPC saw anyone, so likely none searched. Now Battery Park's opening fight
ends the game.

- **The least that stops the crash:** an empty iterator.
- **The fix:** the original's, read:
  [`ReachablePathnodes`](engine-dll.md#moving).

### Saving: any save (seen)

The fork saves a Deus Ex game's `DeusExSaveInfo` into package `DeusEx`, but it
made that object in the transient package, and the save refuses it: "Object
does not belong to this package", exit 1. A run with a temporary hook typed
`QuickSave` into the console in UNATCO HQ. The fork wrote the level, as
`01_NYC_UNATCOHQ.dxs` in a directory `Save00-1`, and stopped the game before
writing the save's info. A save from the Save Game screen takes the same path,
when that screen gets that far
([saving, loading and travel](#saving-loading-and-travel)).

### The Save Game screen: `GetConfig` (read from the code, not yet seen)

`Object.GetConfig(section, key)` is a Deus Ex native with no number, called by
name, and the fork registers nothing under that name. A call to it throws
"Unknown native function", the same way the iterator's error does.
`MenuScreenSaveGame` makes that call every time it opens: its `InitWindow`
asks for a snapshot, and its `Tick` then reads
`GetConfig("Engine.Engine", "GameRenderDevice")` before taking one. To check
by hand: open Save Game.

- **The fix:** the original's, read: a key's value in the system ini (the
  fork's `SE-DeusEx.ini`), or an empty string
  ([`GetConfig`](core-dll.md#getconfig)). Past it, the screen asks for the
  save's picture, which the fork does not make ([the UI](#the-ui)).

## Saving, loading and travel

Deus Ex keeps a mission's maps as the player left them, and a save is those
maps plus the one being played. `DDeusExGameEngine`, its C++ game engine,
does both ([travel and saving](deusex-dll.md#the-game-engine-travel-and-saving)).
The fork's `Engine` has a save of its own for Deus Ex and none of the rest,
and the game's own screens and keys cannot save or load with it. Two runs
with a temporary hook (2026-09-24, UNATCO HQ) typed the game's console
commands and showed three of these. The rest is read from the code:

- **Saving.**
  - Any save stops the game ([above](#saving-any-save-seen)), after writing
    the level. Seen with `QuickSave`.
  - A new save always writes `Save0000`: the Save Game screen passes slot 0,
    and the original takes the next free slot. The quick save writes
    `Save00-1`, where the original writes `QuickSave` (seen).
  - Either holds only the level being played. There is no `Current`, so none
    of the mission's other maps.
  - The `SaveInfo` has no picture, play time, save count or cheats flag.
  - Its date is wrong. `UpdateTimeStamp` counts the year from 1900 and the
    month from 0, so a load list would show year 126 and, sorting by date,
    put the fork's saves before the original's.
- **The Save Game screen** stops the game, by the code
  ([`GetConfig`](#stops-the-game)).
- **The Load Game screen** lists no save.
  - `GetSaveInfoFromDirectoryIndex` searches a list the fork never fills (the
    code that fills it is `#if 0`).
  - `GetSaveInfo(-1)` does not know the quick save.
- **Loading does nothing** (seen: `LoadGame -1` and `LoadGame 3` ran, and
  nothing happened). The game asks for `?loadgame=N`, and the fork looks only
  for an option named `load`. It also looks for a file
  `Save<N>.<ext>`, not the directory it saved to.
- **Deleting** a save from either screen leaves it on disk. The screens send
  the console command `DeleteGame N`, which the fork lacks (seen: "Unknown
  command: DeleteGame 1").
- **Maps forget.** `LoadMap` loads every map fresh from `Maps/` ("To do:
  handle level hubs"), and `DeleteSaveGameFiles` 3012 is a stub. A map
  revisited within a mission is back as it started: its enemies alive, its
  items back, its doors locked. New York's and Hong Kong's hub maps are
  revisited throughout.
- **The player's history, log and notes** are made transient
  (`CreateHistoryObject` and its kin). The original makes them in the level,
  which a save keeps.

**The fix:** the original's travel and save logic in the fork's `Engine` and
its `GameDirectory` natives, all of it now read:

- the engine's `Browse`, `SaveGame`, `SaveCurrentLevel`,
  `PruneTravelActors`, `CopySaveGameFiles`, `DeleteSaveGameFiles` and
  `DeleteGame`, with the mission numbers;
- `GameDirectory`'s listing, save info and new-slot numbering;
- `UpdateTimeStamp` and `CreateHistoryObject` and its kin;
- the `DeleteGame` console command.

Done the original's way, the fork might also read the original game's saves;
to be checked.

## Flags

The flag base holds what missions and conversations set and test: every
conversation played sets `<name>_Played` (the game has 1,955 conversations),
and the mission scripts set their events' flags
([the original](extension-dll.md#flags)).

- **At most 64.** The fork keeps one flag in each of the script's 64 slots,
  where the original chains any number from each. Past 64, a new flag is not
  made ("Could not create flag ...: no room in FlagBase.HashTable") and
  `SetBool` returns false: a conversation not marked played plays again, and
  a mission event not marked is not remembered. Read from the code; a 70 s
  unattended run of Liberty Island set too few to see it.
- **None expire.** `DeleteExpiredFlags` is a stub. In the original a flag set
  with no expiration of its own, as the conversations' are, is deleted when
  the next mission's first level is reached by travel. In the fork every flag
  stays, so a later mission can read an earlier mission's flags, and the 64
  fill sooner.
- **Smaller.** A new flag gets its expiration only when set again.
  `GetExpiration` looks a flag up as a bool whatever its type, and gives 0 for
  a flag that is not there, where the original gives -1. The fork's flags are
  transient objects in the transient package; the original's are inside the
  flag base, which a save keeps.

**The fix:** the original's chains and expiry, read in full.

## Every NPC

### The native tick: `AScriptedPawn::Tick`

Every `ScriptedPawn` has a C++ `Tick` in `DeusEx.dll`
([what it does](deusex-dll.md#the-native-tick)), and the fork has nothing of
it: no field it updates is written anywhere in the fork. Without it:

- **Agitation and fear never decay.** Only this tick calls `UpdateAgitation`
  and `UpdateFear`. The script has versions of both that nothing calls. An NPC
  the player bumped or scared stays that way.
- **Sixteen AI timers never count down:** `AlarmTimer`, `FireTimer`,
  `SpecialTimer`, `ReloadTimer`, `AvoidWallTimer`, `AvoidBumpTimer`,
  `ObstacleTimer`, `CloakEMPTimer`, `TakeHitTimer`, `CarcassCheckTimer`,
  `PotentialEnemyTimer`, `BeamCheckTimer`, `FutzTimer`,
  `PlayerAgitationTimer`, and the counting-up `WeaponTimer` and
  `DistressTimer`. The script sets these and waits for them to reach zero.
- **Cloaking NPCs never cloak** (`EnableCloak`).
- **A burning NPC never goes out** (`ExtinguishFire`).
- **A wounded NPC never bleeds** (`SpurtBlood`).
- **A `bDisappear` NPC is never removed** once out of sight.
- **An NPC's pivot never eases** to `DesiredPrePivot` (sitting, standing).

The fix is the original, read in full. The fork needs a place for a class's
own C++ tick, where Surreal's actor tick would call it; its disappearing also
needs [render time and stasis](#out-of-sight).

### Hearing: the AI event system

NPCs learn of gunfire, footsteps, noises, alarms, bodies and distress through
events that actors raise and NPCs listen for
([the original](engine-dll.md#the-ai-event-system)):

- `AISetEventCallback` 710, `AIClearEventCallback` 711, `AISendEvent` 713,
  `AIStartEvent` 714, `AIEndEvent` 715 and `AIClearEvent` 716, with about 100
  script call sites between them;
- `LevelInfo.InitEventManager` 650.

The fork has them all as stubs, and every map run reached some of them. So no NPC
hears anything: a shot, a thrown object or a body found raises nothing.

- **The fix:** the original's manager, a C++ class (`UEventManager`) that the
  level ticks and saves, and what it calls: `AICanHear` 706, a stub in the
  fork, and `AICanSee`, ported. `AICanSmell` 707 returns 0 in the original
  too. The manager also reads [render time](#out-of-sight), which the fork
  does not keep.

### Moving: wandering and tactical movement

- **`AIPickRandomDestination` 709** is how a wandering NPC picks where to go
  (`ScriptedPawn.Wandering.PickDestination`); reached in every map but the
  menu.
- **`AIDirectionReachable` 708** has 17 call sites: `PickDestination`,
  `TryLocation`, `GetNextLocation`, `FindBackupPoint`, `CleanerBot` and
  animals. It is how an NPC tests a direction before moving there in a fight or
  a search; reached in UNATCO HQ and Battery Park.
- **`ReachablePathnodes`**, above, and `ComputePathnodeDistances` 1020 serve
  the same code.

All are stubs; the originals are read: [moving](engine-dll.md#moving).

## Out of sight

The original records when each actor was last drawn (`LastRenderTime`), and
the engine and the scripts skip work for what the player has not seen lately
([stasis and render time](engine-dll.md#stasis-and-render-time)). The fork's
renderer marks only whether an actor was drawn in the last frame
(`LastVisibleFrame`), for Distant AI, which has pawns neither seen nor near
think every third or sixth frame
([its patches](../ENGINE.md#settings-the-launcher-exposes)); the rest it runs.

- **Render time.** The fork never sets `LastRenderTime`, and its
  `LastRendered()` returns 0 for every actor but a decal: everything counts as
  just drawn. So what the script spares actors out of sight, it never does: a
  `ParticleGenerator` unseen for 2 s goes on; an NPC with `bTickVisibleOnly`
  more than 600 units away and unseen for 5 s still looks for enemies other
  than the player, and beyond 1,200 still looks for bodies; any NPC out of
  sight still checks for light beams. On the handheld all of that runs.
- **Stasis.** The original skips the whole tick of an actor in stasis --
  `bStasis`, not drawn for 5 s, not moving, and far from the player or in a
  zone not drawn -- and destroys a `bTransient` one. The fork ticks every
  actor, and its `InStasis()` returns `bStasis || bForceStasis`: whether stasis
  is allowed, not whether the actor is in it. Its script users are `Shadow`,
  which leaves the shadow of an owner in stasis where it is -- in the fork, of
  any owner with `bStasis`, moving or not -- and a debug window. Which actors
  the game's maps set `bStasis` on is to be checked.
- **The event manager**, once there, reads both: a listener drawn in the last
  5 s weighs senders at any distance, one unseen and more than 1,200 units
  from the player only those within 400.

## On screen

### Particles and lasers: render iterators

An actor with a `RenderIteratorClass` is drawn as the many things its
iterator lists: the renderer makes the iterator (`Actor.RenderInterface`) and
draws each item it lists ([render iterators](engine-dll.md#render-iterators)).
Deus Ex uses this for two classes:

- **`ParticleGenerator`**: smoke, steam, water, sparks. It is in 32 maps, and
  fires, rockets, faucets, damaged robots and fragments spawn one.
- **`LaserEmitter`**: the beams of `LaserTrigger` (18 maps) and `BeamTrigger`
  (11), a weapon's laser sight, and `ElectricityEmitter` (21 maps).

The fork never makes a `RenderInterface` (its accessor is commented out), and
`ParticleIterator.UpdateParticles` 3017 is a stub. So the script's
`ParticleIterator(RenderInterface)` is always `None`: no particle is made, and
no beam is drawn. The originals, `UParticleIterator` and `ULaserIterator`, are
in `DeusEx.dll`, and `URenderIterator` in `Engine.dll`; the loop that makes and
draws them is in `Render.dll`, not yet read.

### Head turns and lip sync: blend animations

`Actor.PlayBlendAnim` 1010 is partial in the fork. `Pawn.PlayTurnHead`
(NPCs turning to look), `Pawn.LipSynch` (mouths in conversations) and the
player's `ViewModelBlendPlay` call it; every map run did. Upstream's blend
code is partly written and logs every call (`TweenBlendAnim: seq=...`,
`DrawLodMeshDX blend[...]`): a line per mouth shape in a conversation, which a
handheld pays for. How far the blending gets on screen is to be checked. The
original is all in `Engine.dll`, and read
([blend animations](engine-dll.md#blend-animations)): the natives, the slots'
tick and the blending of the mesh's vertices. Its tick moves the slots only
while the main animation plays, and up to three times their rate: what the
game's head turns and lip sync were made with.

### Lists

The list window is behind the load and save screens, emails, the logs,
images, the conversation history, the key bindings, the colour themes and a
new game's skills ([the original](extension-dll.md#lists)). The fork's own
list differs in more than its stubs:

- **Every field reads as empty.** The fork's `GetField` tests the column the
  wrong way round: a field that is there comes back empty, and one past the
  row's last is read out of bounds. `GetFieldValue` reads through it and is
  always 0. The game's screens keep what a row stands for in a hidden column
  and read it back, so:
  - on a computer, every email the player picks shows the first one
    (`ComputerScreenEmail.ListSelectionChanged` reads the email's number from
    column 2);
  - loading a colour theme does nothing, and the colour editor cannot tell
    which colour it is editing;
  - the images screen never marks an image viewed or unloads its textures;
  - the load and save screens take every save for slot 0, once they list any.
- **No row is activated.** The fork counts every click as one and never sends
  `ListRowActivated`, for a double click or for Enter. The game's Customize
  Keys screen starts rebinding a key only that way, so no key can be rebound
  there (read from the code, to check by hand). The load and new game
  screens and a hacked computer's accounts have buttons for what a double
  click does, and the save screen does it on a single click.
- **No key moves in a list.** `MoveRow` is a stub, and the list's script sends
  it the arrow keys, Page Up and Down, Home and End. A pad whose d-pad is
  mapped to the arrows cannot move through a list either.
- **Nothing is sorted.** `Sort`, `SetSortColumn`, `AddSortColumn` and
  `ResetSortColumns` are stubs, and `EnableAutoSort` sets its flag. The load
  game list, emails, the conversation history, images and logs stay in the
  order they were filled: once the load list lists saves, by directory, not
  by date.
- **Columns.** `EnableAutoExpandColumns` sets its flag and widens nothing;
  the original widens a column to each field put in it, and does so by
  default. The fork's new columns are 0 wide, the original's 26. The fork
  draws hidden columns too, after the others.
- **Small.** A float column keeps its text as given, where the original shows
  the number through the column's format. A click below the last row selects
  nothing, where the original selects the last row.

### The UI

- **Keys held under a menu.** The original releases every key held when a
  menu takes the input ([the input](extension-dll.md#the-engine-and-the-input)).
  The fork gives an open menu every key, releases included, and releases
  nothing, so a movement key held as a menu opens and let go inside it is
  still held when the menu closes (read from the code, to check by hand).
- **Showing and hiding.** The fork's `Show` and `Hide` set the window's flag
  themselves. They do not ask the parent (`ChildRequestedVisibilityChange` is
  never sent; `SetChildVisibility` is a stub), and do not move focus from a
  window being hidden or tell its children. So the HUD does not lay itself
  out again as the InfoLink and the log come and go
  ([showing and hiding](extension-dll.md#showing-and-hiding)).
- **The vision augmentation** (`GC.DrawActor`, a stub): no heat source is
  drawn, only the tint ([actors in a window](extension-dll.md#actors-in-a-window)).
- **Borders** (`GC.DrawBorders`, partial): the fork stretches each edge and
  the centre over its length, where the original tiles them at one texel a
  pixel ([borders](extension-dll.md#borders)). The game passes no margins,
  which is all the fork handles.
- **Save pictures** (`RootWindow.GenerateSnapshot` and `SetSnapshotSize`,
  stubs): none, where the original's are grey 160 × 120 images
  ([save pictures](extension-dll.md#save-pictures)). `SaveGame` takes them too
  ([saving](#saving-loading-and-travel)).
- **Keys.** `MoveTabGroupNext` and `Prev` (Tab and Shift+Tab between
  controls), `EditWindow.Undo` and `Redo` (Ctrl+Z and Ctrl+Y in an edit
  field), and `RootWindow.LockMouse` (the pointer held while a key is being
  bound) are stubs.

## Implemented, not as the original

- **The list window and the flag base:** [lists](#lists) and [flags](#flags).
- **`ScriptedPawn.GetPawnAllianceType(None)`.** The fork reads through the
  null pawn and crashes; the original answers Neutral. A distress call's
  sender or `GetPlayerPawn()` during a level change could be `None`.
- **`ConBindEvents`.** The fork binds conversations from `DeusExConText`'s
  mission list, found by mission number. The original loads the list the
  level's `ConversationPackage` names, so a mod's own conversations bind only
  in the original. What ConSys's `BindConversations` adds is not yet read.
- **`GameDirectory.GetNewSaveFileIndex`.** The fork takes the first free
  number; the original takes the highest plus one and never refills a gap.
- **`DeusExPlayer.CreateGameDirectoryObject`.** The fork keeps one object;
  the original makes a new one each call. The scripts `CriticalDelete` it
  after use: harmless while that is a stub, but once it deletes, the fork's
  kept object would go with it.
- **`DeusExPlayer.GetDeusExVersion`.** The fork's own string, by choice; the
  original's is "Mon Mar 19 12:06:14 2001 v1.112fm".
- **`LevelInfo`'s clock.** The fork's main loop fills `Year` counted from 1900
  and `Month` from 0, as in its save dates; the original's are the full year
  and 1 to 12. In Deus Ex only `StatLog` reads them.
- **`Actor.RandomBiasedRotation` 717.** The fork never scales its random
  offsets to rotator units, so it returns the central yaw and pitch give or
  take one unit; the original spreads them over up to half a turn of yaw and a
  quarter of pitch ([moving](engine-dll.md#moving)). An NPC sprinting aside in
  a fight always goes square to its enemy, and a `PawnGenerator`'s pawns all
  face its way.
- **`Object.Enable` 117 and `Disable` 118.** The original keeps a bit per
  probe on the state frame and sets them all afresh at every `GotoState`, even
  into the state the object is in; a name that is not a probe it only logs
  ([events and probes](core-dll.md#events-and-probes)). The fork keeps a set
  of disabled names per state name, which no state change clears: a probe
  disabled in a state is off whenever the object is back in that state, until
  enabled, and any name can be disabled -- so every script call to an object
  that has disabled anything, as most NPCs have `AnimEnd`, looks its name up
  in the set. The game disables only probes, and the difference shows where
  it disables one and then goes to the state it is in: `ScriptedPawn`'s
  `Wandering.Bump` disables `AnimEnd` and goes to its `Wander` label, which
  enables it again in the original and not in the fork. What that changes in
  play is to be checked.
- **Conversions**, the VM's tokens. The fork makes a bool `1` or `0`, where
  the original makes it `True` or `False`; it reads a string as a bool only as
  a number, so `True` is false, though only the server browser converts one;
  it needs both commas to read a vector or rotator and makes it all 0 without
  them, where the original reads what is there; it wraps a rotator's parts to
  0–65535 when printing it; and it prints an object as its package and name,
  not its path ([the originals](core-dll.md#the-natives)).
- **`Object.Mid` 127** with a negative start: the original returns an empty
  string, the fork counts from 0.
- **Integer division by 0** (`/` 145, and `/=` 134 on a byte): 0 in the
  original, which leaves the byte as it was. The fork divides, which on
  x86-64 kills the engine; the Smart Pro's CPU gives 0.
- **`Object.VRand` 252** (70 call sites). The fork keeps the points it draws
  *outside* the unit sphere, so its directions lean toward the cube's
  diagonals; the original keeps those inside, which lean nowhere.
- **`Actor.LastRendered` 723 and `Actor.InStasis` 721:**
  [out of sight](#out-of-sight).

## Housekeeping, not seen directly

- **`Object.CriticalDelete` 751** (20 call sites): the original frees the
  object at once, whatever still refers to it
  ([`CriticalDelete`](core-dll.md#criticaldelete)), and the game's callers
  delete objects of their own and drop their reference. The fork's stub
  leaves them to its garbage collector, which frees them later: no other
  difference. Freeing at once would take the fork's kept `GameDirectory`
  with it ([above](#implemented-not-as-the-original)).

## Small

- **`Actor.SetInstantSpeechVolume` 269:** the speech volume slider's change
  is not applied as it moves. The original hands it to the audio subsystem at
  once, as the fork does for sound and music.
- **`Pawn.FindStairRotation` 524:** with Look Up Stairs on, the player's view
  does not tilt on stairs. The original is UE1's
  ([`Engine.dll`](engine-dll.md#small)).
- **`PlayerPawn.ResetKeyboard` 544** (every level change): the original's
  `ResetConfig` of the input's class copies nothing in Deus Ex and has the
  input read the player's bindings from `User.ini` again
  ([configuration](core-dll.md#configuration)); the fork keeps its own
  bindings. No difference in play.
- **`Actor.AIGetLightLevel` 700:** returns 1. The original is the light
  patch 0034 computes for `AIVisibility` (`AILightAt`); no script calls it.
- **`InputExt`:** the fork has no class of that name
  ([the original](extension-dll.md#the-engine-and-the-input)), so the
  multiplayer key bindings' `SET InputExt ...` commands fail on every map.
- **Window sounds:** the fork plays a UI sound at the player, and takes a
  position given as world X and Y. The original plays it a unit away, turned
  by the window's place on screen when positional sound is on
  ([window sounds](extension-dll.md#window-sounds)).
- **`Object.>` for strings (native 116):** unregistered. The fork registers it
  as 1186, a typo: the original has 116. No script is known to use it; a use
  would stop the game.

## Not needed for single player

`DumpLocation` (21 stubs: Ion Storm's bug-location tool, though
`DeusExGameInfo.Login` calls `HasLocationBeenSaved` on every map),
`StatLog`/`StatLogFile`, `InternetLink`, `DebugInfo` (compiled out in the
original too: [`DebugInfo`](core-dll.md#debuginfo)), network numbers and
addresses, `SaveTimeDemo`, `Commandlet.Main`, and `Object`'s `clock`,
`unclock` and `CyclesToSeconds`
([timing by hand](core-dll.md#clock-unclock-and-cyclestoseconds)).
`ComputerWindow` has 22 stubs, but no script calls them; the InfoLink's text
window, its only user, calls only implemented ones. No script calls
`ClipWindow`'s unit sizes, `GC`'s `PushGC`, `PopGC`, `CopyGC` and
`Intersect`, or 16 more of the windows' stubs.
