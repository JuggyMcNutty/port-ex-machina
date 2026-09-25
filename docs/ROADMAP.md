# The reimplementation roadmap

The order the original's behaviour goes into the engine fork, and where each
milestone stands (owner, 2026-09-24). Only order and status live here. What
each item is, and what the player sees of it, is
[`re/natives.md`](re/natives.md)'s; how the original does it, each DLL doc's
([the binaries](re/README.md#the-binaries)); what landed, each fork commit's
message and [what the fork changes](ENGINE.md#what-the-fork-changes).

The order: crashes, then a playthrough survives, then the story stays intact,
then the world behaves, then the look, the sound, polish; multiplayer last
([decided 5](../agent.md#decided)). **[perf]** marks work shared with the
Smart Pro's list ([decided 2](../agent.md#decided)): re-measure on the device
when it lands.

A milestone is done when `tools/natives_audit.py --runs` reaches no stub of
its area in the maps that did, the matching by-hand checks pass
([open decision 1](../agent.md#open-decisions)), and
[`re/natives.md`](re/natives.md) is updated to stay true.

## M0 -- nothing stops the game

- [x] `ReachablePathnodes` yields as an iterator -- empty for now, the real
      walk is M3's ([stops the game](re/natives.md#stops-the-game)).
- [x] The save's `DeusExSaveInfo` made in package DeusEx, not transient --
      the crash only; the save itself is M1's.
- [x] `GetConfig` registered ([the original's](re/core-dll.md#getconfig)).
- [x] The crash-shaped strays: `GetPawnAllianceType(None)` answers Neutral,
      integer division by zero gives 0, string `>` registered as 116
      ([stops the game](re/natives.md#stops-the-game)).
- [ ] Groundwork, runs of the original game: a reference set of its saves (a
      quick save, numbered slots, a mid-mission save past a hub map), and
      watching what was read but never seen -- a comment-jump conversation, a
      sound behind a wall, a zone's reverb, coronas, a laser tripwire -- as
      each fix's acceptance reference. The save set is banked (2026-09-24,
      `reference/original-saves/`, this machine only -- saves hold the
      game's data and are never committed): Liberty Island's start
      (Save0001, no cheats), and a quick save, a hub save (Save0002,
      `bCheatsEnabled`) and the `Current` directory from a travel to
      UNATCO HQ and back, all as
      [the original's layout](re/deusex-dll.md#the-game-engine-travel-and-saving)
      has it. Still wanted: the acceptance captures.

## M1 -- a playthrough survives: saving, loading, travel

All read; the inventory is
[saving, loading and travel](re/natives.md#saving-loading-and-travel).

- [x] The engine's travel and saves the original's way: `Browse`, `SaveGame`,
      `SaveCurrentLevel`, `PruneTravelActors`, `CopySaveGameFiles`,
      `DeleteSaveGameFiles`, `DeleteGame` and the mission numbers; a mission's
      maps keep their state
      ([the original](re/deusex-dll.md#the-game-engine-travel-and-saving)).
- [ ] `GameDirectory`: the listing, the save info and the slot numbering are
      in (2026-09-24); left, the per-call object freed by `CriticalDelete`
      ([housekeeping](re/natives.md#housekeeping-not-seen-directly)).
- [x] `UpdateTimeStamp`, and the player's history, log and notes made in the
      level, so a save keeps them.
- [ ] Flags: the chains past 64 and the expiry are in (2026-09-25); left,
      living in the flag base a save keeps, and crossing a travel
      ([flags](re/natives.md#flags)).
- [ ] The save screens: the list window's `GetField` is in (2026-09-25);
      left, the picture -- a capture point the renderer's frame overlap
      allows -- and the by-hand checks
      ([lists](re/natives.md#lists), [the UI](re/natives.md#the-ui)).
- [x] Acceptance: our own saves round-trip (a slot and the quick save,
      2026-09-24); the original's reference saves were tried and stop at
      the original's saved event manager -- retry after M3.

## M2 -- the story stays intact

- [ ] Conversations: comment events kept, the original's bark-name binding,
      lines that cycle once, an actor destroyed mid-conversation, one sound
      loaded per line ([conversations](re/natives.md#conversations)).
- [ ] The text parser the original's way: its tokens, its tag table, its
      reading to an end tag
      ([what the player reads](re/natives.md#what-the-player-reads)).
- [ ] Lists: rows activate (key rebinding hangs on it), keys move, sorting,
      columns ([lists](re/natives.md#lists)).

## M3 -- the world behaves

- [ ] First, render time and stasis: the native tick and the event manager
      both read them ([out of sight](re/natives.md#out-of-sight)). **[perf]**
- [ ] The AI event system, `UEventManager` and `AICanHear`: NPCs hear
      gunfire, footsteps, alarms, distress and bodies
      ([hearing](re/natives.md#hearing-the-ai-event-system)).
- [ ] `ScriptedPawn`'s native tick: agitation and fear, the sixteen timers,
      cloaking, bleeding, burning out, disappearing
      ([its section](re/natives.md#the-native-tick-ascriptedpawntick)).
- [ ] Moving: `AIPickRandomDestination`, `AIDirectionReachable`,
      `ReachablePathnodes` in full, `ComputePathnodeDistances`
      ([moving](re/natives.md#moving-wandering-and-tactical-movement)).
- [ ] Traces and moves as the original: `ParabolicTrace`, `TraceTexture` and
      `TraceVisibleActors`, `StrafeTo` and `StrafeFacing`,
      `RandomBiasedRotation`, `SetPhysics`, `GetBoundingBox`, `Enable` and
      `Disable`, `VRand`, the conversions
      ([implemented, not as the original](re/natives.md#implemented-not-as-the-original)).

## M4 -- the look

- [ ] Render iterators: particles and beams
      ([its section](re/natives.md#particles-and-lasers-render-iterators)).
- [ ] Mesh detail ([its section](re/natives.md#mesh-detail)). **[perf]**
- [ ] Lighting ([its section](re/natives.md#lighting)). **[perf]**
- [ ] Coronas ([coronas](re/natives.md#coronas)).
- [ ] Blend animations: head turns and lip sync
      ([its section](re/natives.md#head-turns-and-lip-sync-blend-animations)).
- [ ] `D3DDrv.dll` is read here ([decided 4](../agent.md#decided)): whether a
      reimplemented look matches is judged through the original's display
      driver.

## M5 -- the sound

[Sound](re/natives.md#sound), all of it: sounds behind walls, zone reverb
(EFX; the owner's copied `ALAudio.dll` as a reference,
[the binaries](re/README.md#the-binaries)), ambient sounds on lights, the
music's fades and its place kept, the Speech slider, loudness and fall-off
and Doppler, and the smaller notes.

## M6 -- polish

- [ ] [The UI](re/natives.md#the-ui): keys released under a menu, showing
      and hiding, the vision augmentation, borders, the remaining stubs.
- [ ] What remains of [small](re/natives.md#small).

## Later -- multiplayer

The net driver, connections, channels and replication
([multiplayer](re/natives.md#multiplayer), [the network](re/network.md),
[`IpDrv.dll`](re/ipdrv-dll.md)). To be added one day, unscheduled
([decided 5](../agent.md#decided)).
