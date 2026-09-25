# What Surreal Engine lacks

Deus Ex's natives and native C++ against the engine fork's: what is missing or
wrong, what the player sees of it, and the work that would fill it. What each
original function does is in its DLL's doc ([the binaries](README.md#the-binaries));
what each patch changed is in [what the fork changes](../ENGINE.md#what-the-fork-changes).
Which item is taken up, and when, is the owner's call; the decided order is
[`ROADMAP.md`](../ROADMAP.md).

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
  ([below](#saving-loading-and-travel)). One more, of Liberty Island, logged
  the conversations the fork gives each NPC ([conversations](#conversations)).
  One more put eight of the game's texts through the fork's text parser
  ([what the player reads](#what-the-player-reads)).
- **The DLLs.** C++ that is not a native -- a class's own `Tick`, what the
  renderer does with an actor, the audio -- leaves no stub behind: only
  reading the original shows it is missing.
- **Reading both.** Only reading the original shows whether an implemented
  native does what it does. Found so far: `IsValidEnemy` (fixed by patch
  0034), and the ones under [not as the original](#implemented-not-as-the-original).
- **The data.** Where a difference depends on content, the game's
  conversation, text and mesh packages, its classes' defaults and its maps
  were read for what it reaches (a throwaway reader of the package format).
  Code compiled out with `#if 0` also leaves no stub; the audit reports it as
  partial.

## Stops the game

Nothing known does. The fork still ends the game on an error it does not
catch -- a script error, an unknown native, a failed save: the engine exits
with 1, and the launcher shows its crash banner -- but each trigger found is
fixed: `Pawn.ReachablePathnodes` makes an iterator, which yields nothing
([moving](#moving-wandering-and-tactical-movement)); the save's
`DeusExSaveInfo` lives in package DeusEx, which its save once refused
([saving, loading and travel](#saving-loading-and-travel)); `GetConfig` is
registered, as the original's ([`GetConfig`](core-dll.md#getconfig));
`GetPawnAllianceType(None)` answers Neutral; integer division by zero gives
0; and string `>` is native 116. An 80 s Battery Park run, whose opening
fight once ended the game about 20 s in ("Iterator statement without an
iterator in Terrorist11.GetOvershootDestination"), and a quick save in
UNATCO HQ, which once died on "Object does not belong to this package",
both ran out their clocks (2026-09-24).

## Saving, loading and travel

Deus Ex keeps a mission's maps as the player left them, and a save is those
maps plus the one being played. `DDeusExGameEngine`, its C++ game engine,
does both ([travel and saving](deusex-dll.md#the-game-engine-travel-and-saving)).
The fork's `Engine` has a save of its own for Deus Ex and none of the rest,
and the game's own screens and keys cannot save or load with it. Two runs
with a temporary hook (2026-09-24, UNATCO HQ) typed the game's console
commands and showed three of these. The rest is read from the code:

- **Saving** follows the original (seen with a console hook, 2026-09-24):
  slot 0 takes the highest `SaveNNNN` plus one, the quick save writes
  `QuickSave`, the slot is emptied and `Current` copied in with the level
  saved on top, and the `SaveInfo` -- `MyDeusExSaveInfo`, as the original
  names it -- carries the description (the level's `Title` without one),
  the place, the map, the slot, the player's save count, play time and
  cheats flag, and the full-date time stamp. Still missing: the picture
  ([the UI](#the-ui)), and nothing fills `Current` until travel is ported,
  so a slot copies only what an original run left there.
- **The Save Game screen** reads `GetConfig("Engine.Engine",
  "GameRenderDevice")` every time it opens (registered, answered from the
  system ini) and then asks for the save's picture, which the fork does not
  make ([the UI](#the-ui)). To check by hand: open Save Game.
- **The Load Game screen**'s save infos now load: the listing and the kept
  infos fill, and `GetSaveInfo(-1)` knows the quick save. What its rows
  show hangs on the list window's fields ([lists](#lists)); to check by
  hand.
- **Loading works for the fork's own saves** (seen: a slot and the quick
  save round-trip, the saved pawn possessed, 2026-09-24). `?loadgame=N`
  does what the original's `Browse` does: the slot's `SaveInfo` names the
  map, `Current` is emptied, the slot copied in, and the map loads from
  `Current`. The original game's saves do not load yet: they carry its
  saved event manager, a C++-only class the fork lacks until the AI event
  system is ported ([hearing](#hearing-the-ai-event-system)).
- **Deleting** works: the screens' `DeleteGame N` console command removes
  the slot, and `DeleteSaveInfo` lets go of a kept info without touching
  the disk, as the original's does. To check by hand.
- **Maps remember** (seen: a travel out and back, the revisit loaded from
  `Current` with the saved pawn found and reused by the game's own login,
  2026-09-25). Within a mission the departing level is pruned and saved
  into `Current`; a new mission, a player starting a new game, or
  `?restart` empties it. The pruning destroys the augmentations and skills
  with their managers, as the original's does; the fork keeps no offset
  for a carried decoration, the original's other prune, and saves the
  destroyed actors the original drops. To check by hand: a hub map's
  doors and bodies staying as left.
- **The player's history, log and notes** are made in the level
  (`CreateHistoryObject` and its kin, 2026-09-25), as the original makes
  them, so a save keeps them; to check by hand with the screens.

**The fix:** the original's travel and save logic, all of it read and, but
for one piece, in the fork (2026-09-25). Left:

- the save's picture ([the UI](#the-ui)): the fork's frame read-back tears
  down the frame its renderer overlaps, wherever it is called, so the
  picture needs a capture point built into the renderer's own end of
  frame. The original itself saves none for its OpenGL driver, and the
  screens take a missing one.

The fork reads its own saves back (2026-09-24). The original game's saves
stop at its saved event manager; to be retried once the event manager is
ported.

## Flags

The flag base holds what missions and conversations set and test: every
conversation played sets `<name>_Played` (the game has 1,955 conversations),
and the mission scripts set their events' flags
([the original](extension-dll.md#flags)).

- **As the original's now** (2026-09-25, checked by an in-engine
  self-test): 64 buckets by a CRC of the flag's name in upper case, each a
  chain in order of hash then type, with no limit; every set stamps the
  expiration -- the one given, or the base's default for -1 -- an
  expiration of 0 never expires, `DeleteExpiredFlags` deletes up to its
  criteria, and `GetExpiration` answers -1 for a flag that is not there
  and reads the flag's own type. A typed flag is found again: the fork
  wrote every flag's type as bool, so an int or float flag could be set
  but never read. The original's exact CRC polynomial is unread; it
  matters only for reading the original game's own saved chains.
- **Kept and carried** (2026-09-25, seen: 21 flags set across the buckets,
  travelled and loaded back). The flag base and its flags live in the
  level package, so a save keeps them, and they cross a travel in the
  pawn's travel graph -- the base is the pawn's travel property, and the
  fork's travel now walks every element of a fixed-array object property,
  which the base's 64 buckets are. The pre-travel prune deletes them from
  the departing level, as the original's does, so they are not saved
  twice.

## Conversations

What the original binds and plays is ConSys's ([`ConSys.dll`](consys-dll.md));
the game's script does the rest.

### Jumps to a comment's label end the conversation

When a level loads, the fork deletes the comment events from its mission's
conversations. The original keeps them, and the script passes over one: its
event switch has no case for it. But a comment can carry a label, and 11 jumps
in 10 conversations go to a label only a comment has. In the fork that label
is gone, so the conversation logs "Label ... NOT FOUND" and ends there, marked
played. Read from the code and the data; to check by hand.

- **Maggie Chow's meeting** (`MeetMaggie`, Hong Kong), for a player who has
  not heard of the Dragon's Tooth, ends after "A nanotech blade.". Lost: the
  goal to search the Wan Chai police station, the notes of its vault code, the
  Luminous Path and Max Chen, the `KnowsAboutNanoSword` flag and the
  `MaggieWanders` trigger. It plays once.
- **Max Chen's meeting** (`MeetMaxChen`), for a player who brings the evidence
  without having heard of the sword, ends before its two goals and before
  `MaxChenConvinced`, the flag that starts the MJ12 raid on the Lucky Money
  (`Mission06`) and that only this conversation sets. It plays once. The
  player hears of the sword from Gordon Quick, the market's waiter or
  newswoman, or Maggie past her break.
- Smaller: Jughead's deal at the Brooklyn Bridge station (`M03MeetJugHead`),
  and lines of Harley Filben, a sick bum, three goths, Carmela and the
  mission 4 troopers.

**The fix:** keep the comment events.

### Named troopers get the generic trooper's conversations (seen)

The fork gives an actor every conversation its `BarkBindName` owns, not only
the barks, and puts them first in its list; the original gives it only barks
by that name ([an actor's conversations](consys-dll.md#an-actors-conversations)).
490 of the maps' actors have a bark name other than their own name, and two
of those names own other conversations:

- `UNATCOTroop`: the named troopers of missions 1 to 4 -- Corporal Collins,
  Tech Sergeant Kaplan, Private Lloyd, the HQ's guards, the Battery Park,
  clinic and hotel guards and more.
- `MetroCop`: two Paris policemen.

The script starts the first conversation in the list that qualifies. So
walking up to one of these, or frobbing them, starts the generic trooper's
instead of their own, such as Kaplan's `MeetKaplan`: on Liberty Island
`UNATCOTroopInitialBarks`, then `UNATCOTroopSecondBarks` once the statue
mission is complete. Its lines are bound by name to an actor called
`UNATCOTroop`: on Liberty Island the one generic trooper, wherever it stands.
A Liberty Island run with a temporary hook showed the lists: Collins, Kaplan,
Lloyd, the custody trooper and the five post-mission troopers each start with
the generic two. What then plays is read from the script; to check by hand.

**The fix:** the original's binding. The fork also finds the list by the
mission's number in `DeusExConText`; the original loads the one the level's
`ConversationPackage` names, so a mod's own conversations bind only in the
original.

### Smaller

- **Lines that cycle once loop.** 169 of the game's random-label events give
  their lines in turn and then keep the last, 164 of them in the chatter of
  NPCs. The fork's `GetRandomLabel` starts them over
  ([random labels](consys-dll.md#random-labels)).
- **An actor destroyed mid-conversation** does not end it. The fork's
  `BindEvents` never fills the script's list of the conversation's actors,
  which is how `ActorDestroyed` knows them. A pawn killed is destroyed.
- **Speech loads a whole package.** The fork finds a line through the audio
  package's list, and loading the list loads every sound in the package the
  first time any line from it plays: 12.4 MB for mission 1, 14.8 MB for the
  NPCs' barks, 31.7 MB for Hong Kong's. The original loads the one sound by
  name, the same sound ([speech audio](consys-dll.md#speech-audio)). The fork
  also names the package `DeusExConAudio<name>`, where the original takes it
  from the conversation's own package, as a mod's would need.
- **Bindings are cleared.** The fork's `ClearBindEvents` empties every
  event's actors, and its `BindEvents` empties a name no actor has; the
  original never empties one. No difference in play is known.
- **`ownerRefCount`** is never counted, so the script never calls
  `BindActorEvents`, and the fork never binds the invoker by its bark name as
  the original does. No conversation's lines name its owner's bark name, so
  the game shows no difference.

## What the player reads

Books, datacubes, newspapers, emails, bulletins and the credits are the
game's 492 tagged texts, which `DeusExTextParser` breaks into tokens for the
script ([the original](deusextext-dll.md)). The fork's parser is its own, and
most texts come out differently. Read from the code and the texts; a run with
a temporary hook put eight texts through the fork's parser and gave the
tokens these follow from. What the screens then do is read from the script:
to check by hand.

- **No computer lists an email.** The fork reads an `EMAIL` tag as a file, so
  every account's list is empty and its screen says there is no email: all
  66 accounts, 136 emails. The passwords and codes they hold cannot be read.
- **No bulletin opens.** The fork keeps the `=` of a `FILE` tag in the name
  (`=01_Bulletin01`), so a board lists its bulletins' titles, and picking one
  shows nothing.
- **The designers' comments show.** The original hides a comment; the fork
  shows its text, in 166 texts. Most are datacubes, which then open with
  where they lie ("Datacube in Alex's office", "MJ12 lab"), and the note a
  datacube adds keeps it too.
- **Words run together.** The fork trims the spaces at both ends of each run
  of text, so a space beside a tag goes: "From:Anon" for "From: Anon" in every
  email's header, "Arms:Combat StrengthorMicrofibral Muscle" in a book. 395
  places in 135 texts.
- **Blank lines vanish.** The original's blank paragraph is a line of two
  spaces. The fork's is an empty text window, which the fork makes no line
  tall, so the paragraphs of 242 books and datacubes run together.
- **Nothing is centred.** `JC` and `JR` do nothing in the fork: the titles of
  90 books, newspapers and datacubes sit on the left.
- **Raw tags.** At a tag it does not know, the fork gives the rest of the
  text as it is, tags and all, where the original passes over the tag: `JL`
  in a newspaper and a book, `<LOG ERROR>` in a Paris bulletin, and
  `<"...">` in an email and a datacube. The fork also misses a tag that is a
  text's last character: `</B>` or `</I>` shows at the end of 11 texts, and
  the last row of 52 accounts and 8 boards is dropped.
- **The player's first name** is empty (9 texts): "Hey, didn't have a chance"
  for "Hey JC, didn't have a chance".
- **The credits and the quotes** lose the blank lines between their sections:
  the original prints a line for each line break between two tags, which the
  fork skips.
- **Smaller.** An email or bulletin starts with an empty line, where the
  original swallows a text's first `<P>`; for the same reason the original's
  datacube note runs the first two paragraphs together, and the fork's does
  not. `DC` reads as black, but lands on an empty window the fork makes
  before the first paragraph, so nothing shows it. `GotoLabel` is a stub, as
  good as the original's, which does nothing; no script calls it.

**The fix:** the original's parser, read in full: its tokens, its tag table
and its reading to an end tag.

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
- **`ReachablePathnodes` 1004** is an iterator: the script walks it with
  `foreach`, from `GetOvershootDestination` -- an NPC in its Seeking state
  guessing where a lost target went -- and `ComputeAwayVector`. The fork's
  yields nothing, so a seeking NPC gets no overshoot destination.
  `ComputePathnodeDistances` 1020 serves the same code.

All are stubs; the originals are read: [moving](engine-dll.md#moving).

## Out of sight

The original's renderer records when each actor and each zone was last drawn
([render time](render-dll.md#render-time)), and the engine and the scripts
skip work for what the player has not seen lately
([stasis and render time](engine-dll.md#stasis-and-render-time)). The fork's
renderer marks only whether an actor was drawn in the last frame
(`LastVisibleFrame`), for Distant AI, which has pawns neither seen nor near
think every third or sixth frame
([its patches](../ENGINE.md#settings-the-launcher-exposes)); the rest it runs.

- **Render time.** The fork never sets `LastRenderTime`, and its
  `LastRendered()` returns 0 for every actor, and for a decal the decal's
  `LastRenderedTime`, which it never sets either: everything counts as just
  drawn. So what the script spares actors out of sight, it never does: a
  `ParticleGenerator` unseen for 2 s goes on; an NPC with `bTickVisibleOnly`
  more than 600 units away and unseen for 5 s still looks for enemies other
  than the player, and beyond 1,200 still looks for bodies; any NPC out of
  sight still checks for light beams; and an NPC's shadow is traced down and
  laid again each tick the NPC moves, where the original does it only while
  the NPC was drawn in the last second. On the handheld all of that runs.
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
draws each item it lists ([render iterators](render-dll.md#render-iterators)).
Deus Ex uses this for two classes:

- **`ParticleGenerator`**: smoke, steam, water, sparks. It is in 32 maps, and
  fires, rockets, faucets, damaged robots and fragments spawn one.
- **`LaserEmitter`**: the beams of `LaserTrigger` (18 maps) and `BeamTrigger`
  (11), a weapon's laser sight, and `ElectricityEmitter` (21 maps).

The fork never makes a `RenderInterface` (its accessor is commented out), and
`ParticleIterator.UpdateParticles` 3017 is a stub. So the script's
`ParticleIterator(RenderInterface)` is always `None`: no particle is made, and
no beam is drawn. The originals, `UParticleIterator` and `ULaserIterator`, are
in `DeusEx.dll`, `URenderIterator` in `Engine.dll`, and the loop that makes,
runs and draws them in `Render.dll`. Besides the loop, the fork's renderer
would need each item's glow, scale, place and turn kept with it as the item
is listed: the iterators move one proxy actor from item to item.

### Coronas

A light with `bCorona` and a `Skin` texture shows a glow over it on screen
([the original](render-dll.md#coronas)). The fork's are its own, and differ
(read from both codes; to check by hand):

- **Which lights:** the fork takes those in the parts of the level it draws,
  within 2,000 units in Deus Ex; the original, those shining into the
  player's own leaf of the BSP, at any distance.
- **Hidden by:** in the fork, the world; in the original, movers, pawns and
  other actors too, but the player's own pawn.
- **Coming and going:** the fork shows one or not; the original fades each
  in and out over about a third of a second.
- **Brightness:** the fork's are 2.5 times the light's colour; the
  original's, the colour times the fade. The size is the same.

### Mesh detail

The fork draws every LOD mesh whole, at any distance: it loads the tables for
dropping detail and never uses them (read from the code). The original works
out a vertex budget from the depth and the view each draw
([mesh detail](render-dll.md#mesh-detail)): a trooper keeps its 370 vertices to
about 2,700 units deep at 853 pixels wide, and has about 200 at 5,000 and 100
at 10,000. Porting it is a filter on the faces (`FaceLevel`) and a walk down
each corner's collapse list; the fork already works out each vertex once, the
first time a face uses it, so its per-vertex work would fall with the faces.
Morphing, for the look, is apart. On the Smart Pro the per-vertex work of
~40 meshes is ~8 ms of the render
([where a frame goes](../../ports/trimui-smartpro/README.md#where-a-frame-goes)).

### Lighting

Read from both codes ([the original's](render-dll.md#lighting)):

- **Light maps.** The fork sorts a surface's lights in two: the level's list
  for the surface, and the lights near it that are neither `bStatic` nor
  `bNoDelete`. It keeps the first group's light and adds the second's again
  when one of them changes, as the original does with its static and moving
  lights. But when a light of the surface's list pulses, flickers or has an
  animated effect, the fork builds the whole list again every frame, ambient,
  shadows and all; the original keeps the steady ones in its static map and
  adds only the animated one, from its light on the surface kept in the cache
  when its shape holds still. The fork's maps are floats, converted for the
  Smart Pro's GPU on the CPU (engine patches 0002 and 0024); the original's
  are bytes. Both send a changed map to the GPU whole. The fork looks each
  surface's map up in a `std::map`; the original's cache hashes, and first
  checks the item it found last.
- **`NoDynamicLights`**: the fork reads and saves the setting, and nothing
  uses it. In the original it stills animated lights and leaves moving ones
  out.
- **`LE_CloudCast`**: the fork builds it once; in the original its shape
  changes over time, and it is run every frame.
- **Meshes.** The fork lights a mesh with the first 8 lights in reach that
  its light tree lists, not the strongest, and keeps the weak ones the
  original drops. It traces from each light in reach to the actor whenever
  the actor has moved -- every frame for one walking -- where the original
  checks each of its lights every 16 frames. It has no fading, and leaves out
  lights with `bCorona`, which light meshes in the original. Its formula per
  vertex is its own: a smooth falloff and plain diffuse, two square roots a
  light, no highlight, and ambient and light scaled otherwise.

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

- **Fields read back** (2026-09-25; the test was the wrong way round, and a
  field past the row's last was read out of bounds). The screens that keep
  what a row stands for in a hidden column -- the load and save screens'
  slots, the colour editor, the images screen -- get their data now; what
  each then does is to check by hand. A computer's emails still hang on
  the parser listing any ([what the player reads](#what-the-player-reads)).
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
- **Save pictures**: none, where the original's are grey 160 × 120 images
  ([save pictures](extension-dll.md#save-pictures)). `SetSnapshotSize` keeps
  its sizes now, and `GenerateSnapshot` returns nothing on purpose until
  the renderer gets a capture point its frame overlap allows
  ([the fix](#saving-loading-and-travel)).
- **Keys.** `MoveTabGroupNext` and `Prev` (Tab and Shift+Tab between
  controls), `EditWindow.Undo` and `Redo` (Ctrl+Z and Ctrl+Y in an edit
  field), and `RootWindow.LockMouse` (the pointer held while a key is being
  bound) are stubs.

## Sound

The fork's audio is its own, over OpenAL; the original's is `Galaxy.dll`
([`galaxy-dll.md`](galaxy-dll.md)). Both scan every actor for ambient sounds
each frame, keep one record a channel and choose which sound wins alike. They
differ (read from both codes; to check by hand):

- **Sounds behind walls are not muffled.** In the original a sound fades over
  half a second to a third of its volume while the level's BSP stands between
  the player's eyes and its actor, speech excepted
  ([sounds behind walls](galaxy-dll.md#sounds-behind-walls)); the fork plays
  it as in the open. Porting it is the same line test for each playing sound
  each frame -- up to 16 -- and a gain kept on each channel.
- **No reverb.** The fork has none (its EFX is to do). The original gives a
  zone with `bReverbZone` its own reverb, from its `MasterGain`, `CutoffHz`
  and six echoes ([reverb](galaxy-dll.md#reverb)): 21 zones in 16 maps, from
  Battery Park to the endgame (the data). OldUnreal's `ALAudio.dll` emulates
  it with OpenAL's EFX ([the binaries](README.md#the-binaries)).
- **Ambient sounds on lights.** 402 actors in 46 maps have both an ambient
  sound and a light: lights, spotlights and cage lights (the data). The
  original scales such a sound by `LightBrightness` ÷ 255 -- a quarter or
  less for 235 of them -- and makes it follow the light's pulse or flicker
  (42 of them) ([each frame](galaxy-dll.md#each-frame)); the fork plays each
  steady, as if unlit.
- **Music** ([the original's](galaxy-dll.md#music)). The fork switches at
  once, where the original fades out over 1 s, over 5 s after a fight and
  over 1/3 s into one. It never writes the order playing back into
  `SongSection`, so after a fight or a conversation the ambient music starts
  its section again, where the original goes on where it was. Section 255,
  silence in the original, plays the song's first section.
- **The Speech slider does nothing.** The fork has no speech volume: speech
  follows the Sound slider, `Actor.SetInstantSpeechVolume` 269 is a stub, and
  `SpeechVolume` is no setting of its audio device. The original plays speech
  at the Speech slider and the rest at the Sound slider
  ([volume](galaxy-dll.md#volume)).
- **Loudness.** The fork plays every sound at half the Sound slider and
  speech at twice its volume, and rescales the rest but ambient sounds --
  (volume − 1) × 0.25 + 1, and 0.8 from 8 up, so a volume of 0 plays at
  0.75; the original plays the script's volume, up to full. The
  original's fall-off is linear from the sound to its radius; the fork's
  (OpenAL's clamped linear model) is full within a tenth of the radius and
  silent from about nine-tenths.
- **Doppler.** The original shifts only an ambient sound's pitch, by its
  actor's own speed, at `DopplerSpeed` 6,500 units a second; the fork shifts
  every sound by the player's speed (its sources have none), at about 14,800.
- **Smaller.** A sound beyond its radius takes a free channel in the fork,
  silent (its priority is kept at 0 or more); the original drops it. The
  fork's mouth shapes follow the original's bands but for `M`, which it gives
  from 100 to 250 Hz and the original never does ([lip sync](galaxy-dll.md#lip-sync)),
  and it sets `bIsSpeaking` itself whenever a pawn's speech plays, where the
  original moves a mouth only while the script has set it.

## Implemented, not as the original

- **The list window, the flag base, conversations, the text parser and
  coronas:** [lists](#lists), [flags](#flags), [conversations](#conversations),
  [what the player reads](#what-the-player-reads) and [coronas](#coronas).
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
- **`Object.VRand` 252** (70 call sites). The fork keeps the points it draws
  *outside* the unit sphere, so its directions lean toward the cube's
  diagonals; the original keeps those inside, which lean nowhere.
- **`Actor.LastRendered` 723 and `Actor.InStasis` 721:**
  [out of sight](#out-of-sight).
- **The trace iterators, `TraceTexture` 1000 and `TraceVisibleActors`
  1003** (read from both codes; [the originals](engine-dll.md#traces)). The
  fork's line goes on past walls, and its hit on the level has no actor, where
  the original's is the `LevelInfo`. Its `TraceTexture` gives an actor's
  `Skin` as its texture and passes over an actor without one, and gives the
  texture's flags, not the surface's; its `TraceVisibleActors` lists only
  actors of the class that are not hidden, so never the level. In play, by the
  code:
  - **Laser tripwires.** A `LaserTrigger`'s beam stops at the first actor
    `TraceTexture` gives. The player and the NPCs wear `MultiSkins` and have no
    `Skin`, so the beam passes through them and trips nothing, while an actor
    with a `Skin` beyond a wall trips it. A beam reflects only off a texture
    flagged as a mirror, where the original's reflects off a mirrored surface.
    Liberty Island has laser tripwires.
  - **An NPC seeking a spot** within its seek distance takes it as seen
    through a wall, and goes no closer.
- **`Actor.ParabolicTrace` 722** (read from both codes;
  [the original](engine-dll.md#traces)). The fork has no default for an
  argument the script leaves out and reads an empty value -- the NPCs' check
  of a falling grenade gives no step --; adds the zone's gravity with the wrong
  sign, so a thrown thing falls upward; leaves out the zone's velocity, its
  terminal velocity and water; traces each step from the start; and never
  fails. NPCs misjudge where a grenade will land, and whether their own throw
  is safe.
- **`Actor.GetBoundingBox` 724** with a test place or rotation, as every script
  call gives: the fork moves the actor's box, already in the world, by them,
  so it is off by the actor's own place ([the original](engine-dll.md#traces)).
  By the code, the HUD's highlight on a door is drawn in the wrong place, and
  a `DeusExMover`'s area is wrong: which NPCs step out of its way, and which
  pawns it tells when it stops.
- **`Actor.PlaySound` 264** with no radius, from an actor with no
  `TransientSoundRadius`: 800 units in the original, 1,500 in the fork.
- **`Actor.SetPhysics` 3970** ignores the floor it is given
  ([the original](engine-dll.md#moving)): a grenade, pool ball, basketball or
  fragment coming to rest does not take what it landed on as its base, and
  stays put when that moves.
- **`Pawn.StrafeTo` 504 and `StrafeFacing` 506** ignore the speed
  ([the originals](engine-dll.md#moving)). The fork's `StrafeTo` gives an NPC
  0.8 of its `MaxDesiredSpeed`, where the original gives all of it (the scripts
  give no speed), and its `StrafeFacing` keeps the speed the NPC had and its
  `bReducedSpeed`. NPCs strafe as they run and fire in a fight.

## Housekeeping, not seen directly

- **`Object.CriticalDelete` 751** (20 call sites): the original frees the
  object at once, whatever still refers to it
  ([`CriticalDelete`](core-dll.md#criticaldelete)), and the game's callers
  delete objects of their own and drop their reference. The fork's stub
  leaves them to its garbage collector, which frees them later: no other
  difference, now that `CreateGameDirectoryObject` makes a new object each
  call as the original does (2026-09-25).

## Small

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

## Multiplayer

The fork has none of it (read from the code). Every level runs standalone
(`LevelInfo.NetMode` 0): there is no net driver -- `IpDrv.dll`'s
`TcpNetDriver` -- and none of `Engine.dll`'s connections, channels or
replication. The scripts' sockets, which the multiplayer menus use, are
partial: a `TcpLink` never opens, a `UdpLink` sends but never receives, and
`InternetLink`'s `ParseURL` and `Validate` are stubs. So neither Join screen
finds a server: Join Internet's `DeusExGSpyLink` asks the master server over
TCP, Join LAN's `DeusExLocalLink` listens over UDP, and `DeusExServerPing`
queries each server over UDP. The master server is `MasterServerAddress`
under `[DeusEx.MenuScreenJoinGame]` in `DeusEx.ini`; the GOG build's names
GameSpy's, which closed in 2014. The original's protocol is
[the network](network.md), its sockets [`IpDrv.dll`](ipdrv-dll.md); its
`Validate` answers a master server with a key of six spaces for Deus Ex, where
the fork's gives nothing.

## Not needed for single player

`DumpLocation` (21 stubs: Ion Storm's bug-location tool, though
`DeusExGameInfo.Login` calls `HasLocationBeenSaved` on every map),
`StatLog`/`StatLogFile`, `DebugInfo` (compiled out in the
original too: [`DebugInfo`](core-dll.md#debuginfo)), `SaveTimeDemo`,
`Commandlet.Main`, and `Object`'s `clock`, `unclock` and `CyclesToSeconds`
([timing by hand](core-dll.md#clock-unclock-and-cyclestoseconds)). The
network's -- `InternetLink`, network numbers and addresses -- are
[multiplayer](#multiplayer)'s. `ComputerWindow` has 22 stubs, but no script calls them; the InfoLink's text
window, its only user, calls only implemented ones. No script calls
`ClipWindow`'s unit sizes, `GC`'s `PushGC`, `PopGC`, `CopyGC` and
`Intersect`, or 16 more of the windows' stubs.
