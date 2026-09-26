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
- [x] Groundwork, runs of the original game: a reference set of its saves (a
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
      has it. The acceptance captures are scripted runs of both engines
      (2026-09-26, [scripted runs](DEVELOPMENT.md#scripted-runs-of-both-engines));
      each result is under its feature
      ([how it is known](re/natives.md#how-it-is-known)).

- [x] Found while proving M4's render iterators (2026-09-25), judged and
      fixed (2026-09-26): both are in the levels, so play reached them
      however the player arrived and whatever the audio --
      `09_NYC_ShipBelow`'s mover with no brush, and `14_OceanLab_Lab`'s
      massless pawns falling into water
      ([stops the game](re/natives.md#stops-the-game)).

## M1 -- a playthrough survives: saving, loading, travel

All read; the inventory is
[saving, loading and travel](re/natives.md#saving-loading-and-travel).

- [x] The engine's travel and saves the original's way: `Browse`, `SaveGame`,
      `SaveCurrentLevel`, `PruneTravelActors`, `CopySaveGameFiles`,
      `DeleteSaveGameFiles`, `DeleteGame` and the mission numbers; a mission's
      maps keep their state
      ([the original](re/deusex-dll.md#the-game-engine-travel-and-saving)).
- [x] `GameDirectory`: the listing, the save info, the slot numbering and
      the per-call object; `CriticalDelete` stays the garbage collector's,
      with no difference left
      ([housekeeping](re/natives.md#housekeeping-not-seen-directly)).
- [x] `UpdateTimeStamp`, and the player's history, log and notes made in the
      level, so a save keeps them.
- [x] Flags: chains past 64, expiry, kept by saves in the level package,
      and carried across travel in the pawn's travel graph
      ([flags](re/natives.md#flags)).
- [x] The save screens: the list window's `GetField` (2026-09-25); the
      picture, the menus' background, the lists drawn and the screens
      saving (2026-09-26; [saving](re/natives.md#saving-loading-and-travel),
      [lists](re/natives.md#lists), [the UI](re/natives.md#the-ui)).
- [x] Acceptance: our own saves round-trip (a slot and the quick save,
      2026-09-24); the original's reference saves load and play
      (2026-09-25), their saved event manager skipped
      ([hearing](re/natives.md#hearing-the-ai-event-system)).

## M2 -- the story stays intact

- [x] Conversations: comment events kept, the original's bark-name binding,
      lines that cycle once, an actor destroyed mid-conversation, one sound
      loaded per line (2026-09-25;
      [conversations](re/natives.md#conversations) has what each was and the
      by-hand checks left, in
      [open decision 1](../agent.md#open-decisions)).
- [x] The text parser the original's way: its tokens, its tag table, its
      reading to an end tag (2026-09-25;
      [what the player reads](re/natives.md#what-the-player-reads) has what
      changed and the by-hand checks left).
- [x] Lists: rows activate (key rebinding hangs on it), keys move, sorting,
      columns (2026-09-25; [lists](re/natives.md#lists) has what each was
      and the by-hand checks left).

## M3 -- the world behaves

- [x] First, render time and stasis: the native tick and the event manager
      both read them (2026-09-25;
      [out of sight](re/natives.md#out-of-sight)). **[perf]**
- [x] The AI event system, `UEventManager` and `AICanHear`: NPCs hear
      gunfire, footsteps, alarms, distress and bodies (2026-09-25;
      [hearing](re/natives.md#hearing-the-ai-event-system) has what was
      seen and the by-hand checks; the original game's saves load past
      their saved manager now).
- [x] `ScriptedPawn`'s native tick: agitation and fear, the sixteen timers,
      cloaking, bleeding, burning out, disappearing (2026-09-25;
      [its section](re/natives.md#the-native-tick-ascriptedpawntick)).
- [x] Moving: `AIPickRandomDestination`, `AIDirectionReachable`,
      `ReachablePathnodes` in full, `ComputePathnodeDistances` (2026-09-25,
      with `RandomBiasedRotation` from the traces item;
      [moving](re/natives.md#moving-wandering-and-tactical-movement)).
- [x] Traces and moves as the original: `ParabolicTrace`, `TraceTexture` and
      `TraceVisibleActors`, `StrafeTo` and `StrafeFacing`,
      `RandomBiasedRotation`, `SetPhysics`, `GetBoundingBox`, `Enable` and
      `Disable`, `VRand`, the conversions (2026-09-25;
      [implemented, not as the original](re/natives.md#implemented-not-as-the-original)
      has what each was and the by-hand checks).

## M4 -- the look

- [x] Render iterators: particles and beams (2026-09-25;
      [its section](re/natives.md#particles-and-lasers-render-iterators)
      has what changed and the by-hand checks left).
- [x] Mesh detail (2026-09-25; [its section](re/natives.md#mesh-detail)
      has what changed and the by-hand check). **[perf]** re-measure on
      the device.
- [x] Lighting (2026-09-25; [its section](re/natives.md#lighting) has
      what changed, what stays the fork's own -- float maps, the lookup,
      the unread effect shapes -- and the by-hand checks). **[perf]**
      re-measure on the device.
- [x] Coronas (2026-09-25; [coronas](re/natives.md#coronas) has what
      changed and the by-hand checks).
- [x] Blend animations: head turns and lip sync (2026-09-25;
      [its section](re/natives.md#head-turns-and-lip-sync-blend-animations)
      has what changed and the by-hand checks).
- [x] `D3DDrv.dll` read (2026-09-25; [decided 4](../agent.md#decided)):
      whether a reimplemented look matches is judged through the original's
      display driver -- gamma, the light maps' brightness, fog, detail
      textures and each pass's blending, in
      [`re/d3ddrv-dll.md`](re/d3ddrv-dll.md).

## M5 -- the sound

[Sound](re/natives.md#sound), all of it; each item's inventory and by-hand
check live there.

- [x] Loudness: the script's volume, fall-off linear to the radius, the
      cap at full (2026-09-25).
- [x] The Speech slider: `SpeechVolume` a real setting, speech gained by
      it, the instant-volume natives set the sliders (2026-09-25).
- [x] Sounds behind walls: the fade to a third over half a second,
      BSP only, speech excepted (2026-09-25).
- [x] Ambient sounds on lights: brightness and the light's animation
      scale the hum (2026-09-25).
- [x] Doppler: the ambient sound's alone, its actor's speed, a real
      `DopplerSpeed` (2026-09-25).
- [x] Music: the fades, its place kept in `SongSection`, section 255 as
      silence (2026-09-25).
- [x] Zone reverb over EFX, the mapping the fork's own; the owner's
      copied `ALAudio.dll` stays the read-if-needed reference for a
      closer take ([the binaries](re/README.md#the-binaries))
      (2026-09-25).
- [x] The smaller notes: a sound beyond its radius dropped, the mouth
      shapes' `M` band gone, `bIsSpeaking` the script's alone
      (2026-09-25).

## M6 -- polish

[The UI](re/natives.md#the-ui) and [small](re/natives.md#small); each
item's inventory and by-hand check live there.

- [x] Keys released under a menu; a taken mouse button clears only fire
      (2026-09-25).
- [x] Showing and hiding: `Show`/`Hide` ask the parent,
      `SetChildVisibility` whole, the HUD laying out again (2026-09-25).
- [x] The vision augmentation's heat sources (`GC.DrawActor`)
      (2026-09-25).
- [x] Borders tiled at one texel a pixel (`GC.DrawBorders`)
      (2026-09-25).
- [x] The key stubs: `MoveTabGroupNext`/`Prev`, `EditWindow` undo and
      redo, `RootWindow.LockMouse` (2026-09-25).
- [x] What remains of [small](re/natives.md#small): the stairs tilt,
      bindings re-read, the AI light level, `SET InputExt`, positional
      window sounds (2026-09-25).

## M7 -- multiplayer

Joining Deus Ex's live PvP servers, and hosting for the original's clients
([multiplayer](re/natives.md#multiplayer), [the network](re/network.md),
[`IpDrv.dll`](re/ipdrv-dll.md); [decided 5](../agent.md#decided)). Each
step is checked against the original, run as the other side where it can
be.

- [x] The script's sockets: `InternetLink`, `TcpLink`, `UdpLink` -- the
      Join Internet screen lists the live servers (2026-09-26).
- [x] A server's address in a URL: protocol, host and port, as the
      original's (2026-09-26).
- [x] The net driver and a connection: UDP packets, acks, bunches, the
      reliable ordering, and the control channel's handshake to `WELCOME`
      and `JOIN`, with the original as the server (2026-09-26).
- [x] The package map and actor channels: objects by package and index,
      actors spawned and their properties replicated -- a live server's
      world seen from the fork (2026-09-26; the original's listen server's,
      the live servers' still to see).
- [ ] Remote functions and the player's moves: playing on a live server.
- [ ] The server: accepting, relevancy and priority, replication out --
      the original joining the fork.
- [ ] The uplink: a fork server on the master servers' lists.
