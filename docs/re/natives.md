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
  `Current`. The original game's saves load too (2026-09-25, seen: the
  reference Liberty Island save loads and plays): their saved event manager
  is recognized and skipped, its listeners lost
  ([hearing](#hearing-the-ai-event-system)); what its NPCs still do is to
  check by hand.
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

The fork reads its own saves back (2026-09-24), and the original game's
load past their saved event manager now (2026-09-25), the manager skipped
and its listeners lost until its exact bytes are read
([hearing](#hearing-the-ai-event-system)).

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
the game's script does the rest. The fork's differences here were closed on
2026-09-25; what each was, and what remains to check by hand:

- **Comment events are kept.** The fork deleted them from a mission's
  conversations at level load; the script passes over one, but a comment can
  carry a label, and 11 jumps in 10 conversations go to a label only a
  comment has, which logged "Label ... NOT FOUND" and ended the conversation,
  marked played. The two large ones, each playing once: Maggie Chow's meeting
  (`MeetMaggie`, Hong Kong), for a player who has not heard of the Dragon's
  Tooth, ended after "A nanotech blade." -- losing the goal to search the Wan
  Chai police station, the notes of its vault code, the `KnowsAboutNanoSword`
  flag and the `MaggieWanders` trigger -- and Max Chen's meeting
  (`MeetMaxChen`), for a player who brings the evidence without having heard
  of the sword, ended before its goals and before `MaxChenConvinced`, the
  flag that starts the MJ12 raid on the Lucky Money (`Mission06`) and that
  only this conversation sets. Smaller: Jughead's deal at the Brooklyn Bridge
  station (`M03MeetJugHead`), and lines of Harley Filben, a sick bum, three
  goths, Carmela and the mission 4 troopers. To check by hand: both meetings
  play past those lines.
- **An actor's conversations bind as the original's** (seen: a Liberty
  Island run with a temporary hook printed the lists, 2026-09-25). A bark
  (`_Bark` in its name) is owned by the actor's `BarkBindName`, or its
  `BindName` when it has none; any other conversation by its `BindName`,
  never its bark name
  ([an actor's conversations](consys-dll.md#an-actors-conversations)). The
  fork gave an actor every conversation its bark name owns, so the 490
  actors with a bark name not their own -- the named troopers of missions 1
  to 4 under `UNATCOTroop`, two Paris policemen under `MetroCop` -- started
  the generic conversations instead of their own, Kaplan's `MeetKaplan`
  among them. The list is now the one the level's `ConversationPackage`
  names, as a mod's own conversations need, and each binding counts into
  `ownerRefCount`, so the script rebinds a shared conversation's invoker
  (`BindActorEvents`, matching by bark name only for the invoker). To check
  by hand: Kaplan's own greeting on Liberty Island.
- **Lines that cycle once keep the last.** 169 of the game's random-label
  events give their lines in turn and then hold the last, 164 of them the
  chatter of NPCs walked up to or frobbed; the fork's `GetRandomLabel`
  started them over ([random labels](consys-dll.md#random-labels)).
- **A destroyed actor ends its conversation.** `BindEvents` empties and
  fills the script's ten bound-actor slots with each actor an event took,
  which is how `ActorDestroyed` knows the conversation's actors; the fork
  never filled them. A pawn killed is destroyed. Each event binds by name --
  or by bark name for the invoker (speech) or in a first-person conversation
  (transfer, trade, animation) -- and of several actors with one name, the
  invoker is the one bound. A transfer's and a check's item class loads as
  the event binds; `ClearBindEvents` does nothing, as the original's, so an
  event keeps the last actor bound to it. To check by hand: a conversation
  partner killed mid-line.
- **Speech loads one sound per line** (seen: the intro's lines played at
  their own lengths, 2026-09-25). `GetSpeechAudio` loads
  `ConAudio<name>_<id>` by name from `<package>Audio<name>.u`, `<package>`
  the conversation's own package less a final `Text`
  ([speech audio](consys-dll.md#speech-audio)). The fork went through the
  package's audio list, which loaded every sound in the package the first
  time any line from it played -- 12.4 MB for mission 1, 14.8 MB for the
  NPCs' barks, 31.7 MB for Hong Kong's -- and named the package
  `DeusExConAudio<name>` outright, where the original's prefix follows the
  conversation's package, as a mod's would need. `GetSpeechLength` answers
  0 for -1 or no sound, as the original.

## What the player reads

Books, datacubes, newspapers, emails, bulletins and the credits are the
game's 492 tagged texts, which `DeusExTextParser` breaks into tokens for the
script ([the original](deusextext-dll.md)). The fork's parser now reads as
the original's -- its tokens, its tag table, its reading to an end tag
(2026-09-25; a run with a temporary hook put five texts through it against
their SDK sources). Its own tokenizer changed most texts:

- **No computer listed an email** (an `EMAIL` tag read as a file: all 66
  accounts, 136 emails, and the passwords and codes they hold) and **no
  bulletin opened** (the `=` of a `FILE` tag kept in the name). The fields
  now split at commas and trim, a missing one empty, and the last row of an
  account or board -- a tag at the text's very end, which the fork missed
  with 10 others' closing `</B>`/`</I>` -- is kept.
- **The designers' comments showed** in 166 texts (where a datacube lies,
  whose inbox an email is); `NOTE`, `GOAL` and `COMMENT` now hide what they
  hold, to their end tag. One of the original's quirks is not kept: at a
  comment with no end tag it reads past the text's end
  (`09_EmailMenu_ShipOps`, whose listing there depends on the memory after
  it); the fork stops at the end, hiding the rest.
- **Words ran together** ("From:Anon", 395 places in 135 texts) and **blank
  lines vanished** (242 texts): a token's text now keeps its spaces, nothing
  trimmed, each CR and LF a space, so a blank paragraph is the original's
  line of two spaces.
- **Raw tags showed**: at a tag it did not know the fork gave the rest of
  the text raw, where a tag is now the first of the 30 names its content
  starts with (`<LOG ERROR>` an `L`) and an unknown one is `TT_None`, which
  the script passes over.
- **The player's first name was empty** (9 texts): `SetPlayerName` now keeps
  the part before the first space. `DC` and `C` read their colours (the
  fork's integer reader took its target by value: every colour was black),
  and the first `<P>` is swallowed, so an email no longer starts with an
  empty line and a datacube's note runs its first two paragraphs together,
  as the original's does. `JC`/`JL`/`JR` come through as tokens; what the
  screens do with them is the script's.

To check by hand: a computer's emails and bulletins, a datacube and its
note, a book's centred title, the credits' section breaks
([open decision 1](../../agent.md#open-decisions)).

## Every NPC

### The native tick: `AScriptedPawn::Tick`

Every `ScriptedPawn` has a C++ `Tick` in `DeusEx.dll`
([what it does](deusex-dll.md#the-native-tick)), and the fork runs it now
(2026-09-25), before the actor tick, in the original's order. What was
missing without it, each in place:

- **Agitation and fear decay**: the tick calls `UpdateAgitation` and
  `UpdateFear` -- the script's own versions, which nothing called and which
  mirror the DLL's.
- **The sixteen AI timers count** -- twelve down to zero; `ReloadTimer` and
  the counting-up `WeaponTimer` only with a weapon; `PotentialEnemyTimer`
  clearing `PotentialEnemyAlliance` as it runs out; `DistressTimer` counting
  up until it passes `FearSustainTime` and becomes -1. Seen: a set
  `AlarmTimer` of 5 read 2.0 three seconds on and 0 later, the
  `DistressTimer` counting toward its 25.
- **Cloaking**: with `bHasCloak`, the script's
  `EnableCloak(Health <= CloakThreshold)` every tick.
- **Burning out**: past `BurnPeriod`, the script's `ExtinguishFire`.
- **Bleeding**: `SpurtBlood` as `DropCounter` passes the wound's period,
  faster the harder it bleeds and the faster it moves, clotting away over
  `ClotPeriod` -- and, with `bTickVisibleOnly`, only within 1,200 units of
  the player (seen gated off beyond it).
- **A `bDisappear` NPC** in stasis or unseen for 5 s is destroyed
  ([render time and stasis](#out-of-sight)).
- **The pivot eases** to `DesiredPrePivot` as `PrePivotTime` runs out, under
  the script's own `PlayAnimPivot` values (seen).
- **The advanced-tactics manoeuvre ends** once the pawn stops accelerating,
  leaves walking or has no turn direction.

To check by hand: a cloaked commando, a burning NPC going out, a rat
disappearing once out of sight
([open decision 1](../../agent.md#open-decisions)).

### Hearing: the AI event system

NPCs learn of gunfire, footsteps, noises, alarms, bodies and distress through
events that actors raise and NPCs listen for
([the original](engine-dll.md#the-ai-event-system)). The fork has the
original's manager now (2026-09-25): `UEventManager`, one per level in
`LevelInfo.EventManager`, ticked by the level after the actors and saved
with it, with the raising and listening natives (`AISetEventCallback` 710,
`AIClearEventCallback` 711, `AISendEvent` 713, `AIStartEvent` 714,
`AIEndEvent` 715, `AIClearEvent` 716, `LevelInfo.InitEventManager` 650 --
all stubs before, reached in every map run, so no NPC heard anything),
`AICanHear` 706 as the original's, and `AICanSmell` 707 answering 0 as the
original's does. Its class is the DLL's own with no script, made at startup
into the Engine package. Seen on Liberty Island: terrorists took
`HandleDistress` by sight of a distressed civilian, a security bot heard
footstep pulses fade with distance, a raised `WeaponFire` reached
`HandleShot` and the terrorists' answering gunfire became senders in turn,
and a quick save carried 10 event types and 325 listeners through a load.
To check by hand: a shot fired around a corner turning guards, a thrown
body found ([open decision 1](../../agent.md#open-decisions)).

The fork saves the manager in a layout of its own. The original game's
saved manager -- its exact bytes unread -- is recognized and skipped on
load with a message, its listeners lost: what an original save's NPCs
still hear is to be checked when those bytes are read.

### Moving: wandering and tactical movement

All the original's now (2026-09-25; the originals:
[moving](engine-dll.md#moving)); they were stubs, so a wandering NPC never
picked where to go, one in a fight or a search never tested a direction,
and a seeking NPC got no overshoot destination:

- **`AIPickRandomDestination` 709** (`Wandering.PickDestination`, reached
  in every map but the menu): up to its tries of biased random directions,
  each tested through `AIDirectionReachable`, the multiplier stopping the
  pawn short of what it can reach.
- **`AIDirectionReachable` 708** (17 call sites: `PickDestination`,
  `TryLocation`, `GetNextLocation`, `FindBackupPoint`, `CleanerBot`,
  animals): the pawn itself walks, swims or flies the direction in steps of
  its collision radius and is put back; walls, ledges, steps, the void,
  pain zones and water stop it. Seen: a probe from a dock pawn found a spot
  280 units along its facing inside the asked range.
- **`ReachablePathnodes` 1004** (`GetOvershootDestination`,
  `ComputeAwayVector`) iterates up to 32 nodes nearest first, from the
  start node's usable reach specs, or the nearest reachable nodes within
  1,000 units; `ComputePathnodeDistances` 1020 floods `visitedWeight` over
  the network from the same list. Seen: 13 nodes nearest first by the dock,
  and the flood reaching 876 of Liberty Island's 1,198 navpoints.

To check by hand: NPCs wandering their bit of Liberty Island, and a
searching NSF stepping around corners in a fight
([open decision 1](../../agent.md#open-decisions)).

## Out of sight

The original's renderer records when each actor and each zone was last drawn
([render time](render-dll.md#render-time)), and the engine and the scripts
skip work for what the player has not seen lately
([stasis and render time](engine-dll.md#stasis-and-render-time)). The fork
keeps both now (2026-09-25), beside Distant AI's own `LastVisibleFrame`
([its patches](../ENGINE.md#settings-the-launcher-exposes)):

- **Render time.** `LastRenderTime` is stamped where the renderer's own
  visibility test passes, a spawned actor starts 10 s undrawn, and a zone's
  is the frame's own zone and both zones a visible portal borders (this
  renderer draws the BSP whole behind a span clipper, not zone by zone as
  the original's `OccludeBsp`). `LastRendered()` answers the time since,
  never below 0 -- it returned 0 for every actor, so everything counted as
  just drawn and the scripts spared nothing: `ParticleGenerator`s unseen
  for 2 s, `bTickVisibleOnly` NPCs' enemy and body checks, light-beam
  checks, and NPC shadows laid each tick all ran, on the handheld too. A
  decal's own `LastRenderedTime` is stamped when drawn and never read, as
  the original's: a `Shadow` never counts as drawn.
- **Stasis.** `InStasis()` is the original's -- `bStasis`; `bForceStasis`,
  or physics none or rotating; not drawn for 5 s; its zone not drawn for
  5 s, or more than 1,200 units from the player -- where the fork's old one
  answered whether stasis was *allowed*. The tick of an actor in stasis
  does nothing -- no script tick, physics, animation or timers -- and
  destroys a `bTransient` one. Seen on Liberty Island: of ~2,600 actors,
  ~220 allow stasis, and the count in it grew from 6 to 30 over 40 s as
  unseen trees and lamps aged past 5 s. **[perf]** To re-measure on the
  Smart Pro when M3's AI work lands with it.
- **The event manager** reads both: a listener drawn in the last 5 s or
  within 1,200 units, and not in stasis, weighs every sender; any other
  only those within 400 ([hearing](#hearing-the-ai-event-system)).

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

The fork keeps the whole mechanism now (2026-09-25): the interface made and
dropped as the original's renderer does; Init, First, IsDone, CurrentItem
and Next each scene frame; each item drawn as the proxy stood when listed --
place, turn, scale and glow -- a sprite at its captured place, a mesh with
the proxy put back for the draw and restored after; and the proxy's
`LastRenderTime` stamped per listed item, which the generators' freeze logic
reads. `UpdateParticles` 3017 ages, drifts, rises or sinks, grows, fades and
moves the 64 particles; `LaserIterator` lists an item every 16 units of each
beam, every 15 with `bRandomBeam`, whose segment ends jitter by a random
unit vector and chain -- the electricity -- and one extra item at a segment
chosen at random, where the proxy is left. Differences, read from both
codes:

- **Occlusion.** The original occludes each item's sprite through its span
  buffer; the fork clips items to the view (and a portal's spans) before
  its BSP walk, and the depth buffer hides what a wall covers -- the cost
  of a hidden item is paid, the look is the same.
- **The particle curves.** The documented shapes are kept -- the drift
  offset -3 to +2 a frame, growth from 0.01 to 3 times the draw scale over
  the life, the fade with the remaining life, the rise as acceleration at
  the rise rate -- but the original's exact curves are unread: whether
  smoke reads the same is the by-hand check's judgement.

To check by hand: steam from a Hell's Kitchen street grate against the
original -- drift, growth, fade; a laser tripwire's beam seen on Liberty
Island (its trace fix is
[implemented, not as the original](#implemented-not-as-the-original)'s);
electricity arcing on a damaged panel; a weapon's laser sight in play.

### Coronas

A light with `bCorona` and a `Skin` texture shows a glow over it on screen
([the original](render-dll.md#coronas)). The fork keeps the original's now
(2026-09-25): the lights shining into the player's own leaf of the BSP at
any distance -- the leaf's permeating list, and the dynamic corona lights
standing in it -- hidden by the world, movers, pawns and other actors but
the player's own pawn; each fading in and out over about a third of a
second on real time, up to 32 kept from frame to frame; drawn in the
colour of the light's hue and saturation times the fade, at the same
screen size as before. Other games keep the fork's old take.

To check by hand: coronas near and far and behind an NPC (already in the
list below), and a lamp's glow coming up and going over about a third of
a second as a corner hides and shows it.

### Mesh detail

The fork works the original's vertex budget out each draw now (2026-09-25;
[the formula and its numbers](render-dll.md#mesh-detail)): faces whose
`FaceLevel` is past the clamped budget go, each kept corner walks down its
collapse list into it, and the top `LODMorph` fraction of the raw budget
slides toward what it collapses to, texture coordinates with it, so detail
fades rather than pops. A temporary log matched the doc's own numbers on
Liberty Island (a trooper's 244 vertices at 7,865 deep at 1,920 pixels wide
is the doc's ~200 at 5,000 at 853, scaled by the resolution term). The
per-vertex work falls with the faces, since the fork animates and lights a
vertex once a draw, the first time a kept face uses it. **[perf]** To
re-measure on the Smart Pro: the per-vertex work of ~40 meshes was ~8 ms of
the render
([where a frame goes](../../ports/trimui-smartpro/README.md#where-a-frame-goes)).
Still different: the original's exact morph curve is unread (the fork
slides linearly over the zone), and the original lights only the vertices
of faces turned to the eye, which is [lighting](#lighting)'s to take up.

To check by hand: an NPC walking away on Liberty Island -- detail fades
with no pop or seam as it recedes, and reads whole again as it comes back.

### Lighting

Read from both codes ([the original's](render-dll.md#lighting)):

- **Light maps.** The fork keeps the original's three kinds now
  (2026-09-25): a surface's still lights are its static map, built once
  and kept until one changes; its animating lights -- pulse, flicker, an
  animating effect -- are added over the loaded static colors every
  frame, each through its own shadow bits; the moving lights stay the
  shadowless per-frame pass; and a mover's maps are rebuilt when the
  mover moved or turned since, not every frame. Still the fork's own:
  the maps are floats, converted for the Smart Pro's GPU on the CPU
  (engine patches 0002 and 0024), where the original's are bytes -- and
  the fork clamps each texel at 1.0 on upload, where the original's byte
  reaches twice unit brightness (64 unit, 127 double:
  [the driver](d3ddrv-dll.md#the-light-maps-brightness)), so its
  brightest lights top out at half the original's overbright; a
  changed map goes to the GPU whole in both; the lookup is a `std::map`
  where the original's cache hashes and first checks the item it found
  last; and a still-shaped animated light is re-run rather than kept as
  its shadowed light and rescaled. To check by hand: a flickering
  sconce's wall (the 'Ton's entrance), a pulsing light throbbing, and a
  triggered light going dark, each with its shadows still there.
- **`NoDynamicLights`**: works now (2026-09-25) -- animated lights count
  as still and bake into the static map, and moving ones are left out;
  by hand with the ini setting on.
- **`LE_CloudCast`**: the fork builds it once, and its effect's shape is
  a placeholder (upstream's to-do), as are the torch and fire wavers and
  the watery shimmer; in the original the cloud shape changes over time
  and is run every frame, and the wavers dim each texel by up to 5% and
  20% as a map is merged. The formulas are unread.
- **Meshes.** The fork keeps the original's now (2026-09-25): the
  candidates from the actor's leaf of the BSP plus the moving lights near
  it and last frame's, the strongest picked first -- statics until 8, none
  below an eighth of the strongest, `bCorona` lights counting -- shadows
  checked through the BSP every 16 frames, each light fading in and out
  over about a third of a second and lighting as it fades, and the
  original's per-vertex formula: the (cos + 1)^2 - 1.5 diffuse, the
  6 cos^2 highlight toward the eye, linear falloff, 1.4 x `ScaleGlow`,
  ambient added, channels clamped. One knowing difference: the moving
  lights come from the fork's light tree near the actor, not the leaf's
  own list. Whether the brightness pairs with the fork's light maps as the
  original's does is judged against the original's display driver, read
  in [`d3ddrv-dll.md`](d3ddrv-dll.md) (the light maps' brightness, the
  blends); a by-hand look first: an NPC
  under a street lamp, one walking from light into shadow (the fade), and
  a fire's glow on a face.

### Head turns and lip sync: blend animations

The fork keeps the original's now (2026-09-25;
[blend animations](engine-dll.md#blend-animations)): `PlayBlendAnim` with
the original's defaults, `TweenBlendAnim` as TweenAnim for a slot from its
kept last pose, and the slots' tick moving only while the main animation
plays or tweens, up to three times their rate -- what the game's head
turns and lip sync were made with -- a slot that ends leaving the rest
only the time over. `Pawn.PlayTurnHead` (NPCs turning to look),
`Pawn.LipSynch` (mouths in conversations) and the player's
`ViewModelBlendPlay` drive it. The per-call logs a handheld paid for are
gone. The mesh side was already upstream's: each slot's pose added as its
difference from the mesh's first frame.

To check by hand: a conversation partner's mouth moving with the speech
(Tech Sergeant Kaplan is in the conversations list already), an NPC's head
turning to follow the player and easing back, and blinking.

### Lists

The list window is behind the load and save screens, emails, the logs,
images, the conversation history, the key bindings, the colour themes and a
new game's skills ([the original](extension-dll.md#lists)). Its differences
were closed on 2026-09-25 (an in-engine self-test drove the sorting, the
number reading, the moves and the focus); what each was, and the by-hand
checks:

- **Fields read back** (2026-09-25; the test was the wrong way round, and a
  field past the row's last was read out of bounds). The screens that keep
  what a row stands for in a hidden column -- the load and save screens'
  slots, the colour editor, the images screen -- get their data; a float
  field keeps the number its text reads as, which `GetFieldValue` answers,
  and shows it through the column's format
  ([the original](extension-dll.md#rows-and-fields)).
- **Rows activate.** A double click or Enter sends `ListRowActivated` to
  the list's parents with the activate sound; the fork counted every click
  as one and never sent it, so the game's Customize Keys screen -- which
  starts rebinding a key only that way -- could rebind nothing. To check by
  hand: rebinding a key there.
- **Keys move.** `MoveRow` moves the focus row up, down, a page, first or
  last -- clamped, selecting or extending from the anchor, the move sound
  played, the row scrolled into view -- and the list's script sends it the
  arrow keys, Page Up and Down, Home and End, so a pad whose d-pad maps to
  the arrows moves through a list too. To check by hand with a pad.
- **Sorting is the original's.** The keys are an ordered list with reverse
  and case flags per column (`SetSortColumn`, `AddSortColumn`,
  `ResetSortColumns`, `Sort`, and auto sort keeping new and changed rows in
  place); a float or time column compares its numbers, a string column its
  text, stable ([the original](extension-dll.md#sorting)). The load game
  list now sorts by its hidden date column, and emails by sender or subject
  from their headers' clicks; to check by hand.
- **Columns.** A new column is 26 wide (20 plus both margins), the window's
  text colour and font, and a sort key; auto-expanding columns, on by
  default, widen a column to each field put in it; hidden columns take no
  space and do not draw. A click below the last row selects the last row.

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
differed when read from both codes; a landed item carries its date and its
by-hand check, the rest are still the fork's own:

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
- **Loudness.** Landed (2026-09-25): the fork plays the script's volume
  as the original does -- no rescale toward 1, no halving -- with
  fall-off linear from the sound to its radius and silent there, and the
  product capped at full, the Sound slider its ceiling
  ([volume](galaxy-dll.md#volume)). Not carried: the original's 1/256
  volume floor, an integer artifact. Other games keep the fork's old
  loudness. To check by hand: a humming light or a generator fading
  steadily on the walk away and silent right at its radius, not gone
  early; effects sitting louder against the music than before.
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
- **The roadmap's M3 traces-and-moves item is the original's now**
  (2026-09-25; the originals: [traces](engine-dll.md#traces),
  [moving](engine-dll.md#moving),
  [events and probes](core-dll.md#events-and-probes),
  [the natives](core-dll.md#the-natives)). What each was:
  - **`Object.Enable` 117 and `Disable` 118**: one probe mask per object,
    set afresh at every `GotoState` even into the same state, saved as the
    original's `FStateFrame` keeps it. The fork kept per-state sets of
    disabled names that no state change cleared and that every scripted
    call looked its name up in -- `Wandering.Bump`'s disabled `AnimEnd`
    stayed off, where the state's `Wander` label re-enables it now. To
    check by hand with the rest of M3's AI.
  - **Conversions**: a bool prints `True`/`False` and reads back as one; a
    vector or rotator reads what is there, a missing part 0; a rotator
    prints its parts unwrapped; an object prints its path name.
  - **`Object.VRand` 252** (70 call sites): the points kept are those
    inside the unit sphere, which lean nowhere.
  - **`Actor.RandomBiasedRotation` 717**: the offsets spread over the
    range at last -- an NPC sprinting aside in a fight no longer always
    goes square to its enemy.
  - **The trace iterators `TraceTexture` 1000 and `TraceVisibleActors`
    1003** run over the original's `MultiLineCheck`: nothing beyond the
    first wall, the wall itself listed as the `LevelInfo`, a level hit
    giving the surface's texture and `PolyFlags` and an actor hit no
    texture. A laser beam stops at the player and NPCs now (they have no
    `Skin`, which the fork required) and at walls; an NPC seeking a spot
    no longer sees it through a wall. To check by hand: Liberty Island's
    laser tripwires.
  - **`Actor.ParabolicTrace` 722**: the original's defaults, gravity the
    right way up, the zone's velocity, terminal velocity and water,
    per-step tracing, bounces, and failure to the start. NPCs judge a
    grenade's landing again.
  - **`Actor.GetBoundingBox` 724** with a test place or rotation puts the
    actor there for the moment: the HUD's highlight on a door and a
    `DeusExMover`'s area sit right. To check by hand: a door's highlight.
  - **`Actor.SetPhysics` 3970** takes the floor it is given as the base
    (its `SupportActor` event): a grenade or pool ball coming to rest
    moves with what it landed on.
  - **`Pawn.StrafeTo` 504 and `StrafeFacing` 506** take Deus Ex's speed:
    an NPC strafing in a fight runs at its full `MaxDesiredSpeed`.
- **`Object.Mid` 127** with a negative start: the original returns an empty
  string, the fork counts from 0.
- **`Actor.LastRendered` 723 and `Actor.InStasis` 721:**
  [out of sight](#out-of-sight).
- **`Actor.PlaySound` 264** with no radius, from an actor with no
  `TransientSoundRadius`: 800 units in the original, 1,500 in the fork.

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
