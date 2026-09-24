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
  which stubs a map reaches, not how often.
- **The DLLs.** C++ that is not a native -- a class's own `Tick`, what the
  renderer does with an actor -- leaves no stub behind: only reading the
  original shows it is missing.
- **Reading both.** Only reading the original shows whether an implemented
  native does what it does. Found so far: `IsValidEnemy`, fixed by patch 0034.

## Stops the game

Surreal ends the game on a script error: the engine exits and the launcher
shows its crash banner.

### An NPC searching in Battery Park: `ReachablePathnodes`

`Pawn.ReachablePathnodes` (native 1004) is an iterator: the script walks it
with `foreach`. The fork registers it as a plain function that only logs, so
the `foreach` has no iterator and the VM stops the game. `ScriptedPawn` reaches it
from `GetOvershootDestination`, when an NPC in its Seeking state guesses where
a lost target went, and from `ComputeAwayVector`. In the Battery Park run an
NSF terrorist got there about 20 s in: "Iterator statement without an
iterator in Terrorist11.GetOvershootDestination", exit 1. Before patch 0034
no NPC saw anyone, so likely none searched. Now Battery Park's opening fight
ends the game.

- **The least that stops the crash:** an empty iterator.
- **The fix:** the original's, in `Engine.dll`. Not yet read.

### The Save Game screen: `GetConfig` (read from the code, not yet seen)

`Object.GetConfig(section, key)` is a Deus Ex native with no number, called by
name, and the fork registers nothing under that name. A call to it throws
"Unknown native function", the same way the iterator's error does.
`MenuScreenSaveGame` makes that call every time it opens: its `InitWindow`
asks for a snapshot, and its `Tick` then reads
`GetConfig("Engine.Engine", "GameRenderDevice")` before taking one. To check
by hand: open Save Game.

- **The fix:** read one key from the ini, a few lines.

## Every level: maps that remember

Deus Ex keeps a mission's maps as the player left them. `DDeusExGameEngine`,
its C++ game engine (the ini's `GameEngine`), does this in `Browse`
(`0x10010b90`, [read in part](deusex-dll.md#classes)):

- **Leaving a map for another of the same mission** saves the map left to
  `Save\Current\<map>` (`SaveCurrentLevel`, after `PruneTravelActors`).
- **Arriving** loads the destination from there when it has been visited.
- **A new mission** empties `Save\Current`, as `DeleteSaveGameFiles` 3012
  does with no argument. `StartNewGame` and `StartTrainingMission` call that
  too.
- **A save** copies `Save\Current` into its slot along with the current
  level and a picture, and loading one copies the slot back. This is read
  from what `SaveGame` and `Browse` call: `CopySaveGameFiles`,
  `SaveCurrentLevel`, `XRootWindow::GenerateSnapshot`.

The fork has none of it. Its `LoadMap` loads every map fresh from `Maps/`
("To do: handle level hubs"), `DeleteSaveGameFiles` is a stub, and a save
holds only the current map. So a map revisited within a mission is back as it
started, with its enemies alive, items back and doors locked. A loaded save
forgets every map but the one it was made in. New York's and Hong Kong's hub
maps are revisited throughout.

- **The fix:** the original's travel and save logic, in the fork's `Engine`:
  `Browse`, `SaveCurrentLevel`, `SaveGame`, `CopySaveGameFiles`,
  `PruneTravelActors`, and the mission numbers they compare. It needs reading
  in full first.

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
own C++ tick, where Surreal's actor tick would call it.

### Hearing: the AI event system

NPCs learn of gunfire, footsteps, noises, alarms, bodies and distress through
events that actors raise and NPCs subscribe to, all in `Engine.dll`:

- `AISetEventCallback` 710, `AIClearEventCallback` 711, `AISendEvent` 713,
  `AIStartEvent` 714, `AIEndEvent` 715 and `AIClearEvent` 716, with about 100
  script call sites between them;
- `LevelInfo.InitEventManager` 650;
- the checks the manager makes before it calls an NPC back: `AICanHear` 706,
  `AICanSmell` 707, `AIGetLightLevel` 700 (partial).

The fork has them all as stubs, and every map run reached some of them. So no NPC
hears anything: a shot, a thrown object or a body found raises nothing. The
manager itself is a C++ class, `UEventManager`
(`Engine/Inc/UnEventManager.h` in the SDK).

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

All are stubs.

## Seen on screen

### Particles and lasers: render iterators

An actor with a `RenderIteratorClass` is drawn as the many things its
iterator lists. In UE1 the engine makes the iterator (`Actor.RenderInterface`)
and draws each item it lists; where the original does this is still to be read. Deus Ex uses this for two classes:

- **`ParticleGenerator`**: smoke, steam, water, sparks. It is in 32 maps, and
  fires, rockets, faucets, damaged robots and fragments spawn one.
- **`LaserEmitter`**: the beams of `LaserTrigger` (18 maps) and `BeamTrigger`
  (11), a weapon's laser sight, and `ElectricityEmitter` (21 maps).

The fork never makes a `RenderInterface` (its accessor is commented out), and
`ParticleIterator.UpdateParticles` 3017 is a stub. So the script's
`ParticleIterator(RenderInterface)` is always `None`: no particle is made, and
no beam is drawn. The originals, `UParticleIterator` and `ULaserIterator`, are
in `DeusEx.dll`; `URenderIterator` is in `Engine.dll`; the drawing loop is
likely in `Render.dll`.

### Head turns and lip sync: blend animations

`Actor.PlayBlendAnim` 1010 is partial in the fork. `Pawn.PlayTurnHead`
(NPCs turning to look), `Pawn.LipSynch` (mouths in conversations) and the
player's `ViewModelBlendPlay` call it; every map run did. Upstream's blend
code is partly written and logs every call (`TweenBlendAnim: seq=...`,
`DrawLodMeshDX blend[...]`): a line per mouth shape in a conversation, which a
handheld pays for. How far the blending gets on screen is to be checked; the original is in `Engine.dll`, and the drawing of
blended meshes is in `Render.dll`.

### The UI

In `Extension.dll`, all stubs or partial in the fork:

- **List sorting.** `Sort` 1784, `SetSortColumn` 1780, `AddSortColumn`,
  `ResetSortColumns`, `MoveRow`, and `EnableAutoSort`, which sets its flag and
  never sorts: the load game list, emails, the conversation history, images
  and logs stay in the order they were filled.
- **Column sizes.** `EnableAutoExpandColumns` (partial, 16 call sites).
- **`GC.DrawBorders`** (partial, 13 call sites): the HUD's and inventory's
  window borders.
- **`GC.DrawActor`** (partial): the vision augmentation's view of actors.
- **Save-game pictures.** `RootWindow.GenerateSnapshot` and `SetSnapshotSize`
  (no picture, even once `GetConfig` exists).
- **The HUD.** `Window.SetChildVisibility`: how the HUD shows and hides its
  parts.
- **Keyboard navigation.** `MoveTabGroupNext`/`Prev` (tab between controls),
  `EditWindow.Undo`/`Redo`, `RootWindow.LockMouse` (while a key is being
  bound).

## Housekeeping, not seen directly

- **`FlagBase.DeleteExpiredFlags` 1124.** Each mission script calls it at
  start (`MissionScript.InitStateMachine`) to drop flags set to expire. Flags
  never expire, so a later mission can read an earlier mission's flags. What
  that changes in play is not known yet.
- **`Object.CriticalDelete` 751** (20 call sites): by its name and callers,
  deletes an object at once (logs, notes, info windows, the credits' text).
  The objects stay until collected. Its original is in `Core.dll`.

## Small

- **`Actor.SetInstantSpeechVolume` 269:** the speech volume slider's change
  is not applied as it moves.
- **`Pawn.FindStairRotation` 524:** with Look Up Stairs on, the player's view
  does not tilt on stairs.
- **`PlayerPawn.ResetKeyboard` 544** (every level change): the fork keeps its
  own bindings.
- **`InputExt`:** `Extension.dll`'s input class. The multiplayer key
  bindings' `SET InputExt ...` commands fail on every map.
- **`Object.>` for strings (native 116):** unregistered. The fork registers it
  as 1186, likely a typo. No script is known to use it; a use would stop the
  game.

## Not needed for single player

`DumpLocation` (21 stubs: Ion Storm's bug-location tool, though
`DeusExGameInfo.Login` calls `HasLocationBeenSaved` on every map),
`StatLog`/`StatLogFile`, `InternetLink`, `DebugInfo`, network numbers and
addresses, `SaveTimeDemo`, `Commandlet.Main`, and `Object`'s `clock`,
`unclock` and `CyclesToSeconds`. `ComputerWindow` has 22 stubs, but no script
calls them; the InfoLink's text window, its only user, calls only implemented
ones. `ClipWindow`'s unit sizes are stubs with no callers.
