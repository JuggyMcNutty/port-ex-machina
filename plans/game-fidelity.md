# Game-fidelity pass: five play-reported differences from the original

Status: **done** — five play-reported differences taken up 2026-10-01:
four fixed and proven (speech skip, belt text, choice focus, direct-walk
pathing), the fifth's search ported 2026-10-02; the death path and the
mission scripts compared clean. All pushed and pinned (2026-10-02, the
owner's go-ahead). The search's port is being redone: the RE reading in
step 10 misreads the binary (agent.md's State, the review).

## Context

Five symptoms from the owner's play session, all "the original does it, the
fork does not (or not right)":

1. **Bot and animal AI/pathing** off — they don't follow the original's
   movement logic.
2. **Conversations**: speech audio does not stop when the dialog is skipped;
   the blue choice selector is missing.
3. **Object belt**: items show no name/text; the game seems to lock up,
   refusing item usage and swapping.
4. **Dead NPCs do not lay flat on the ground.**
5. **Mission scripting** must be verified functional.

Owner's standing decisions apply: the engine is the work (decided 8), RE is
behaviour in our own words with IDA as needed (decided 4), and each change
is proven by scripted runs of both engines (dxcap) before it is called done.

## Findings (from the exploration)

### A. Conversation speech never stops on skip — root cause found

- `ConPlay.PlaySpeech` plays the line with **`speaker.PlaySound`** and keeps
  the returned ID; skipping a line calls `PlayNextEvent` → `StopSpeech` →
  **`player.StopSound(playingSoundID)`** — the *player's* StopSound with the
  *speaker's* ID.
- The original: `Actor.StopSound(Id)` (Engine.dll `0x103e27d0`) hands the ID
  to the audio subsystem, which stops the channel **by ID alone** (Galaxy
  `StopSoundId` `0x10607ef0`; engine-dll.md#small, galaxy-dll.md#playing-a-sound).
  The ID itself encodes actor (object index × 16) + slot.
- The fork: `USurrealAudioDevice::StopSound(UActor*, int)` requires **both**
  actor and ID to match (`SurrealEngine/Packages/Engine/Subsystems/USurrealAudioDevice.cpp`).
  Player ≠ speaker, so nothing matches and the line plays on. Its only
  caller is `NActor::StopSound_Deus`, so matching by ID alone is safe.
- Side difference: the fork builds the ID from the actor **pointer's** low
  24 bits (`NActor::PlaySound_Deus`), the original from the **object
  index**. Self-consistent today; align while there or note it.

### B. Blue choice selector

- Choices are `ConChoiceWindow` (a ButtonWindow) children of
  `ConWindowActive`; the focused choice's *text colour* is the blue
  (`colConTextFocus`). Up/Down are deliberately unhandled by
  `ConWindowActive.VirtualKeyPressed` and fall through to Extension's
  `RootWindow.VirtualKeyPressed` → `MoveFocusUp` and `MoveFocusDown` (natives 1442–1445,
  registered in the fork). The fork has all the machinery, so the failure
  is behavioural (focus never lands on a choice, focus moves not finding
  the buttons, or focus colours not applied on draw) — needs a
  side-by-side capture, then a code read; IDA on Extension.dll's
  `XButtonWindow` only if the read doesn't settle it.

### C. Belt text + lockup

- Belt text is pure script: `HUDObjectSlot.DrawWindow` →
  `gc.DrawText(..., item.BeltDescription)` plus the count text from
  `UpdateItemText`; font `Font'FontTiny'`; the whole block is gated on
  `item.Icon != None`. No native on this path is missing (audit).
- Item use/swap: `DeusExPlayer.PutInHand` → `UpdateInHand` waits each tick
  for the in-hand item's script-state transitions (`SkilledTool.PutDown`/
  `Idle2`, weapon `Idle`/`Reload`/`DownWeapon`) — a stuck latent
  (`FinishAnim`) or state transition would refuse everything
  (`bInHandTransition`). Mouse swaps drag through the inventory screen
  (`winInv`), assigned only when that screen opens.
- Owner's correction (plan review, 2026-10-01): it is **the belt itself
  that locks up**, not the game — items already there stop being drawn,
  and nothing can be swapped or picked up into it. That points at the
  belt window's item list (`HUDObjectBelt`'s slots, `AddObjectToBelt`/
  `RemoveObjectFromBelt`/`UpdateInHand` on the window side) rather than
  the player's in-hand state machine.
- Needs a live repro with the log open before fixing.

### D. Dead NPCs do not lay flat

- Death flow (script): `PlayDying` → `PlayAnimPivot('DeathFront'/'DeathBack')`
  by hit direction → state `Dying`: `WaitForLanding`, `MoveFallingBody`
  (uses `AnimRate`/`AnimLast`), `FinishAnim()`, `SpawnCarcass()` →
  `DeusExCarcass.InitFor` (mesh variant by death side, Z adjustment), then
  the pawn hides and destroys itself.
- Fork candidates: death anim not playing / `FinishAnim` never completing
  (pawn stuck, no carcass), `PrePivot` not applied to the drawn mesh,
  carcass mesh or defaults import, the Z adjustment. Needs a repro run and
  capture comparison; IDA on Engine.dll's anim handling only if implicated.

### E. Bot/animal pathing

- Every RE-documented movement native is in (NATIVES.md#moving): animals
  wander/flee/eat through `AIPickRandomDestination`/`AIDirectionReachable`;
  bots patrol through `findPathToward`.
- **The path search itself is still the fork's own** (`UPawn_Path.cpp`,
  a Dijkstra-like search). The RE notes cover only findPathToward's
  "next node" rule, not the search. Diagnose with AIConsole diffs first;
  RE only what the diff implicates.

### F. Mission scripting

- `MissionScript`/`MissionNN` are pure script over the flags system (already
  the original's, NATIVES.md#flags) + `DeusExLevelInfo.MissionNumber` +
  timers; nothing on the path is missing. Verification = a per-map smoke
  sweep in both engines.

## Approach

Six workstreams in this order (owner's choice: quick wins first). Each one:
reproduce with a dxcap console (both engines where a reference helps),
diagnose, fix in the engine, re-run the console to prove it, update the
docs the fact belongs in. New consoles live in `vibe/tools/dxcap` with the
others; temporary debug hooks follow the `TEMPORARY DEBUG TOOL` rule and
are reverted before committing.

1. **Speech skip** — fix `USurrealAudioDevice::StopSound` to match by ID
   alone, as the original's `StopSoundId`. Decide on the ID-scheme alignment
   (object index vs pointer) while there.
2. **Belt** — new `BeltConsole`: give the player a spread of items
   (weapon + ammo, pickup with copies, a skilled tool), log each slot's
   `BeltDescription`/`itemText`/font, shot the HUD, then drive belt
   activation and a swap, logging `bInHandTransition` and the item's state
   over time. Run both engines, diff, fix what it shows.
3. **Death poses** — new `DeathConsole`: on Liberty Island kill an NPC
   (front and back hits), log the Dying state's progress (anim sequence/
   frame/rate, `FinishAnim` completion, carcass class/mesh/place), shots as
   it settles. Both engines, diff, fix; IDA (Engine.dll anim channel) only
   if the semantics stay unclear.
4. **Choice selector** — new `ChoiceConsole`: start a conversation with
   choices (one with speech audio), log the choice buttons and the focus
   window, send Up/Down/Enter, shots at each step. Both engines, diff, fix
   the implicated focus/draw behaviour in the Extension windows.
5. **AI/pathing** — run the existing `AIConsole` on both engines, lay the
   logs side by side, and extend it (or add a focused console) to watch the
   animals and the UNATCO bots over a longer window (routes, MoveTarget,
   path choices). RE with IDA exactly what the diff implicates — the path
   search (`APawn::findPathToward` `0x103db3f0` and below) is the likely
   candidate — document it in engine-dll.md#moving, port it, re-run.
6. **Mission smoke sweep** — new `MissionConsole`: load a map (named by the
   run's ini, as the other consoles do), wait ~10 s, log the mission script
   actor found, the `InitStateMachine` line, the `M<n>MissionStart` /
   startup flags actually set, and any script errors; exit. A small driver
   loops it over every mission map (00_Intro, 00_Training*, 01_* … 15_*),
   fork first, the original re-run for any map that fails or differs, and
   produces a results table.

Cross-cutting: every fix's finding goes to the doc that owns the fact
(NATIVES.md section, or a dx-reverse-info .md for new RE), in the same
commit as the change; commits as JuggyMcNutty in VibeEngine; pins updated
in port-ex-machina after pushes; agent.md's State updated at the end.

## Files to modify

**VibeEngine (branch `deusex`)** — the fixes:
- `SurrealEngine/Packages/Engine/Subsystems/USurrealAudioDevice.cpp` —
  `StopSound(UActor*, int)` matches by ID alone (workstream 1); possibly
  `SurrealEngine/Native/NActor.cpp` for the ID scheme.
- Belt / death / choice / pathing fixes: wherever each diagnosis lands —
  candidates seen: `SurrealEngine/Packages/Extension/Windows/**` (focus and
  button draw), the anim/latent-action code the death diagnosis names, and
  `SurrealEngine/Packages/Engine/Actors/Pawn/UPawn_Path.cpp` (path search,
  workstream 5).
- `vibe/tools/dxcap/**` — new console classes `BeltConsole`,
  `DeathConsole`, `ChoiceConsole`, `MissionConsole` (+ any `AIConsole`
  extension), UnrealScript following the existing consoles; recompiled with
  the SDK's `UCC.exe` via `dxcap.sh compile`.
- `vibe/docs/NATIVES.md` — the section each fix belongs to (conversations,
  the UI, small, moving).

**dx-reverse-info** — only if workstreams 3–5 turn up undocumented original
behaviour: the owning .md (engine-dll.md#moving for the path search;
extension-dll.md for window focus/draw behaviour).

**port-ex-machina** — `ENGINE-PIN.txt` after each pushed engine change
(`scripts/engine.sh pin`), and this plan's status; `agent.md` State at the
end.

## Reuse

- `vibe/tools/dxcap.sh` + the console classes in `vibe/tools/dxcap/` — the
  side-by-side harness; `ProveConsole`/`CaptureConsole`/`AIConsole` are the
  patterns for the new consoles; `DXCAP_RECORD=1` records each run's audio
  (the speech-skip proof), `DXCAP_HIDDEN=1` keeps the desktop clean.
- `vibe/tools/natives_audit.py` — re-run after fixes; `--runs` with engine
  logs to confirm no new stub fires on the touched paths.
- The RE docs (engine-dll.md, extension-dll.md, galaxy-dll.md,
  consys-dll.md) — the original's behaviour, already read for the root
  causes above.
- IDA MCP (`dx-reverse-info/tools/ida/idalib-mcp.sh`, currently
  disconnected — start it when a workstream needs it; databases are beside
  the DLLs in `gamefiles/System/`) with `ue1_types.py`/`ue1_names.py`/
  `utf16_strings.py` for any fresh database.
- The game's script source, read from the packages (`tools/ida/ue1_types.py`
  reads them; its literal blanking is by design — read raw package text
  locally for behaviour, never committing game content).
- `scripts/dx.sh build/stage/run/test linux-x86_64` — the build-and-run
  loop; `scripts/engine.sh pin` for pins.

## Steps

- [x] 1. **Speech skip**: change `USurrealAudioDevice::StopSound(UActor*, int)`
      to match by ID alone — done 2026-10-01; the ID scheme stays the
      fork's own (the fork's `UObject` has no global object index to build
      the original's from, and the ID is opaque to scripts, only fed back
      to `StopSound`). Builds clean; all 12 unit tests pass.
- [x] 2. **Speech skip proof** — done 2026-10-01: new `SkipConsole`
      (`vibe/tools/dxcap`) plays Kaplan's MeetKaplan on Liberty Island and
      skips every line mid-line (13 lines, adopting the conversation that
      starts on standing by him); `skip.py` (beside `sound.py`) reads the
      recorded runs, aligned by the start/end beep pair. Fixed fork vs
      unfixed fork: all 7 NPC tails +1.1..+2.9 dB louder unfixed (played
      on under the next lines and past the conversation's end); all 4
      player tails level — the player's lines stopped even unfixed, as the
      player is both the StopSound caller and the sound's actor, which is
      what the old match required. Fixed fork vs original: every tail
      window at or below the fixed fork's level. Runs:
      `fork-SkipConsole-172614` (fixed), `fork-SkipConsole-173254`
      (unfixed, built from a stashed fix), `original-SkipConsole-172657`.
- [x] 3. **Belt repro** — done 2026-10-01: `BeltConsole` (vibe/tools/dxcap)
      gives the player a weapon (with ammo, as the game's pickups do), a
      2-copy multitool, a 3-copy lockpick and later a biocell; logs every
      slot's item, BeltDescription, Icon and itemText, the player's
      inHand/transitioning state through a use, a swap, a new pickup and
      NextBeltItem, with marked shots. **The logs are identical between
      the engines** — slots, descriptions, counts, use, swap and pickup
      all work in the fork; the "lockup" did not reproduce. What differs
      is the pixels: the belt's descriptions, counts and slot numbers
      never drew — the belt's items looked unnamed and half-there, which
      reads exactly as the owner described.
- [x] 4. **Belt fix** — done 2026-10-01: `UGC::DrawText` centered and
      right-aligned each line within the *wrap* width, which the no-wrap
      path had widened to 100,000 — so every centered or right-aligned
      text with word wrap off drew ~50,000 px off-screen. The original's
      `XGC::DrawText` (Extension.dll `0x10028180`, decompiled with IDA)
      hands the wrap width only to line breaking and aligns within the
      width as passed. The fork now keeps an `alignWidth` (the width as
      passed) beside `wrapWidth`. Proof: BeltConsole's shots now show the
      belt's text (structure matching the original's — descriptions,
      counts, numbers); the menu regression (GetConsole) is 99.8 %
      identical to before; the proving run is clean; all 12 unit tests
      pass. Runs: `fork-BeltConsole-180926`, `original-BeltConsole-175020`.
      Noted on the way: the fork's own `shot` reads the framebuffer
      before the present pass's gamma, so its shots are darker than the
      screen shows — capture comparisons must expect it (the belt's
      apparent dimness beside the original's display grabs was this, not
      a rendering difference).
- [x] 5. **Death repro** — done 2026-10-01: `DeathConsole` (vibe/tools/dxcap)
      kills two isolated human NPCs on Liberty Island — one from behind
      (DeathFront, the carcass's Mesh2), one from the front (DeathBack,
      Mesh) — with a robot pass and an alive-shot added on the way, logging
      the pawn's state, physics, animation (sequence, frame, rate, last),
      acceleration, velocity, rotation, hidden and health through Dying,
      and the carcass's class, mesh, place, base, scale, pivot and physics
      until it settles, with marked shots. **The fork matches the original
      in every logged respect**: the same death animation at the same rate
      completing in the same ~0.5 s, the same death lurch (13–14 units),
      the same hide timing, the same carcass class and mesh by hit side,
      the same placement to within a unit — and the robots' odd
      freeze-and-hide in Dying forever, which turns out to be the
      *original's own* behaviour (Deus Ex's robots do exactly this; the
      fork matches it). The two residual differences: the fork's carcass
      spawns falling and lands ~1 unit lower, unbased, where the
      original's rests at its spawn spot based on the level — the known
      "what falls comes to rest where its last step's trace stops" area
      (NATIVES.md), whose original side the RE notes call unread — and the
      fork's own `shot` command reads the framebuffer before the present
      pass's gamma, which defeated pixel comparison of the drawn pose (the
      same capture artifact noted in the belt workstream).
- [x] 6. **Death fix** — nothing to fix from this repro: the death logic
      is not implicated. The owner's "NPCs that die do not lay flat" did
      not reproduce as a state or data difference — the fall, the lurch,
      the carcass and its pose all match. What remains is recorded as
      follow-up: the carcass's ~1-unit-lower unbased resting (the known
      falling-rest area, needing the original's per-tick physics logged to
      read what stops it), and a by-hand check of the drawn pose in play
      for the owner, since the capture path could not compare it. Runs:
      `fork-DeathConsole-182846`, `original-DeathConsole-182912`
      (humans), `fork-DeathConsole-182636`/`original-DeathConsole-182702`
      (robots).
- [x] 7. **Choice repro** — done 2026-10-01: `ChoiceConsole`
      (vibe/tools/dxcap) plays MeetKaplan to its choices, logs each
      choice button's selectability/sensitivity/visibility and the focus
      window, drives Down/Down/Up through the root window's own
      VirtualKeyPressed (the keys' path), a marked shot after each, then
      picks through the focused button's object. **Reproduced**: in the
      original the focus cycles the buttons (seeded on the first, Down to
      the second, wrap); in the fork the focus stayed None through every
      key — no selector, ever. Two causes found: the fork's
      `MoveFocusDown`, `MoveFocusUp`, `MoveFocusLeft` and `MoveFocusRight` were stubs returning nullptr, and the
      choice buttons were `bIsSelectable=False` where the original's are
      True (its noexport class defaults; the fork's NewChild never set
      it).
- [x] 8. **Choice fix** — done 2026-10-01, RE'd with IDA on
      Extension.dll and ported: `XWindow::MoveFocus` (0x1004ef30) walks
      the focus's group's row/column-major window lists (position-sorted,
      wrapping, skipping `IsTraversable` failures — 0x1004c460:
      selectable, the parent chain visible and sensitive, and under the
      topmost modal), and with no focus seeds it via the topmost modal's
      group (`XModalWindow::Init` adds a modal to its own table);
      `XRootWindow::Tick` (0x1003a540) does that seeding every tick while
      nothing has focus. The fork now: `UWindow::MoveFocus` with the
      group lists built on demand (same membership and order as the
      original's maintained tables), `IsTraversable`, the type-based
      `GetTabGroupWindow` (any non-generic ancestor — the conversation
      window itself is the choices' group), `windowType` set at creation
      (root 3 / modal 2 / tab group 1), buttons selectable by default,
      the root's tick seeding, and `UButtonWindow::DrawWindow` drawing by
      state (focused/pressed/insensitive per `ChangeButtonAppearance`
      0x10008380) — the fork always drew the Normal color. Proof: the
      focus now cycles exactly as the original's (seeded on choice 0,
      Down 0→1, Down 1→wrap→0, Up 0→wrap→1) and the blue focus colour
      moves with it (the blue-dominant pixels' peak rows track the
      focused line across the shots). Regressions: all 12 unit tests
      pass, the menu shot 99.9 % identical, the belt's text still drawn,
      the proving run clean. Run: `fork-ChoiceConsole-190328` vs
      `original-ChoiceConsole-185257`.
- [x] 9. **AI/pathing diagnose** — done 2026-10-01: `MoveConsole`
      (vibe/tools/dxcap) logs every in-world ScriptedPawn's state,
      orders, move target, destination, velocity and distance moved every
      2 s for 120 s on Liberty Island, both engines. 52 pawns each; most
      match (several apparent stalls — Terrorist2, 6, 12 — turn out
      identical in both engines). The real divergences: **UNATCOTroop1**
      frozen mid-MoveToward against geometry for 100+ s (moved 399 vs
      the original's 6849 — it re-picks the same broken route through
      PathNode406 where the original routes through PathNode405),
      **Terrorist10** similarly stuck at PathNode684, and **SecurityBot1**
      taking node detours (PathNode918) where the original walks straight
      to its next patrol point. No animals were in-world on the island's
      start (the generators untriggered); the animals' own movement
      natives are already the original's, and the stuck bug hits them
      through the same MoveToward.
- [x] 10. **AI/pathing fix** — the original's `findPathToward`
      (Engine.dll `0x103db3f0`, decompiled) first asks whether the target
      itself can be walked to straight — `CanMoveTo` for a navigation
      point, `pointReachable` for a spot, then a pass over the candidate
      nodes for one already reachable — and returns the target as the
      route when so; only then does it search
      (`breadthPathFrom` `0x103dcd60`: a best-first walk over the level's
      reach-spec table, cost the accumulated spec distance plus per-node
      penalties, a sorted frontier list, giving up after 1000 nodes;
      the route is reversed and the already-documented next-node rule
      applied). **Landed now**: the direct-reach pre-check in
      `UPawn::FindPathToward` — proven with MoveConsole:
      SecurityBot1's route now matches the original's exactly
      (PatrolPoint14→1→2→3→13, no detours, 10714 moved vs 10821).
      **Follow-up, taken up 2026-10-02**: the search itself. A first port
      followed an RE writeup that misread the binary -- the walk taken for
      the level's navigation list, not the search's own sorted open list,
      and the end points left the fork's own -- and its verdict on
      `definePathsFor` was measured with that walk. Redone in the review
      the same day from the binary read in full
      (`dx-reverse-info/engine-dll.md#the-search`, rewritten): 48 of 52
      pawns' distance moved within tolerance by `move.py`, UNATCOTroop1
      walking its whole patrol, Terrorist35 no longer stalling; Terrorist34
      stalls where the original's does not, the fork's own
      `pointReachable` letting a farther node replace the original's end
      point -- the reachability tests are what is left. Runs:
      `fork-MoveConsole-154921` (the redone search),
      `fork-MoveConsole-225552` (the first port) vs
      `original-MoveConsole-191311`, `fork-MoveConsole-192456`
      (the pre-check), `fork-MoveConsole-191106` (pre-fix).
- [x] 11. **Mission sweep** — done 2026-10-01: `MissionConsole`
      (vibe/tools/dxcap) checks each map's DeusExLevelInfo, its
      MissionScript actor and the state machine's initialization from the
      player's flag base (`DXMISSION:` lines; the fork's runs land on the
      map by their URL, the original's via `DXCAP_MISSION_MAP` through
      the run's ini, since the game starts at the menu map). **Fork: 78
      of 83 OK**, and the five non-OK are not the fork's: `DX`, `DXOnly`
      and `Entry` are not mission maps (the menu and entry levels), and
      `12_Vandenberg_Tunnels` (no MissionScript actor) and
      `99_Endgame4` (no DeusExLevelInfo) behave identically in the
      original — the maps themselves have no script to run. The mission
      scripting is functional everywhere. Results table:
      `build/dxcap/mission-sweep/fork.txt`.
- [x] 12. **Wrap-up** — done 2026-10-01: NATIVES.md gained the four
      sections (conversations: the skip; the UI: the belt text and the
      focus movement; moving: the direct-walk pre-check, the
      breadthPathFrom follow-up and the death comparison);
      DEVELOPMENT.md the six new consoles (Skip, Belt, Death, Choice,
      Move, Mission) and `DXCAP_MISSION_MAP`; five commits in VibeEngine
      (one per fix + the harness/docs), as JuggyMcNutty, pushed with the
      owner's go-ahead on 2026-10-02 and the pin moved with them;
      agent.md's State carries the session.

## Verification

Every workstream's proof is a dxcap side-by-side run of the same console
class in both engines (the fork on linux-x86_64, the original under
Proton's wine), comparing logs and shots — plus, for the speech fix, the
recorded `audio.wav` around the skip. A run goes long enough to see the
behaviour settle (60 s or more where the harness's own guidance says so),
and the staged binary is rebuilt and restaged before each run (the "build
does not restage" gotcha). `natives_audit.py --runs` over the new logs
confirms no stub fires on the touched paths. The mission sweep's table is
the workstream-6 proof. Nothing is called done with a failing or missing
comparison, and every temporary hook is reverted before its commit.
