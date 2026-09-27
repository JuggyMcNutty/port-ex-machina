# Port Ex Machina -- where things stand

The session handoff: state, decisions, what is open and what is next. It holds
no facts of its own beyond those; each lives in one doc, and the
[README's table](README.md#documentation) says which. Before working, read
[`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md).

## State (2026-09-26)

- **The repository** is on `main`, public at
  https://github.com/JuggyMcNutty/port-ex-machina. Its history was rewritten
  before publishing (2026-09-22) to drop the game's files and a personal email
  address.
- **The launcher** runs on linux-x86_64 and the Smart Pro. It is deliberately
  verbose for development (open decision 2).
- **The engine** is our own fork repository
  (https://github.com/JuggyMcNutty/SurrealEngine, branch `deusex`), pinned by
  `ENGINE-PIN.txt`; the patch stack was retired into it (2026-09-24). It holds
  upstream's latest when last merged (2026-09-24); whether and when to merge
  newer upstream commits (`scripts/engine.sh status`, then `upgrade`) is the
  owner's call. The profiling hooks are off, and the desktop build is at the
  pin (2026-09-26).
- **linux-x86_64**, the base: launcher and engine build natively; the staged
  app ran the engine into the intro level on the development PC, and
  unattended runs drove saves, loads and hub travel through in play
  (2026-09-25).
- **trimui-smartpro**: the game runs; the performance work is in progress
  ([its Performance](ports/trimui-smartpro/README.md#performance)). The device
  has every patch up to 0034 in a build with the profiling hooks, Overclock;
  its owner's settings are Distant AI (characters out of sight think less
  often) on and 853×480 (2026-09-23), which the fight's native rows switch to
  native for the run. It has no battery (its battery
  warnings are off: [its Gotchas](ports/trimui-smartpro/README.md#gotchas)).
- **The game's DLLs**: both passes are read (2026-09-24;
  [decided 4](#decided)). [`docs/re/`](docs/re/README.md) covers
  each binary read, and an IDA database gets three scripts from `tools/ida/`
  (types, strings, names).
  - **`DeusEx.dll`**, **`Engine.dll`** (its network code in
    [`network.md`](docs/re/network.md)), **`Core.dll`**, **`Extension.dll`**,
    **`ConSys.dll`**, **`DeusExText.dll`**, **`Render.dll`** (where a
    feature's drawing lives there, its mesh detail and lighting),
    **`IpDrv.dll`** and **`Galaxy.dll`**
    ([`deusex-dll.md`](docs/re/deusex-dll.md),
    [`engine-dll.md`](docs/re/engine-dll.md),
    [`core-dll.md`](docs/re/core-dll.md),
    [`extension-dll.md`](docs/re/extension-dll.md),
    [`consys-dll.md`](docs/re/consys-dll.md),
    [`deusextext-dll.md`](docs/re/deusextext-dll.md),
    [`render-dll.md`](docs/re/render-dll.md),
    [`ipdrv-dll.md`](docs/re/ipdrv-dll.md),
    [`galaxy-dll.md`](docs/re/galaxy-dll.md)); their databases are typed,
    named and backed up.
  - **What Surreal lacks** of them is [`docs/re/natives.md`](docs/re/natives.md),
    from [`tools/natives_audit.py`](tools/natives_audit.py), map runs, and
    runs with temporary hooks (each removed before committing).
- **linux-aarch64**: the launcher cross-builds; never run on a device.
- **android**: planned; [its README](ports/android/README.md) is the plan.
- **x360**: planned; nothing about it is worked out yet.
- **Next**: multiplayer, M7 (the owner's ask, 2026-09-26;
  [decided 5](#decided)): the fork joins a server and plays on it -- the
  original's listen server, so far; a live server is yet to try --; as a
  server it takes the original's join, replicates the level to it, calls
  it, and answers LAN and GameSpy queries as the original does. Next: the
  uplink to the master servers and a live server -- both outward-facing, so
  the owner's go-ahead first --, then server travel and downloads.
  Potential work it left, in no order: the fork's player standing 3.75
  units lower than the original's server has it, at rest (its X and Y
  exact); the fork's ini reader taking an empty value (`ServerName=`) for a
  missing one, so a fork server's name is the class default; comparing the
  original's native replication lists with the script statements the fork
  evaluates for them (both in [multiplayer](docs/re/natives.md#multiplayer));
  and the fork's `vec4`
  inequality (`engine/SurrealEngine/SurrealEngine/Math/vec.h`), which tests
  the fourth component for equality -- so the render devices' screen-flash
  test always passes.
  The reimplementation's milestones are done -- M0 through M6 all in
  (2026-09-26, M0's acceptance captures last;
  [`docs/ROADMAP.md`](docs/ROADMAP.md) tracks each item, and
  [natives.md](docs/re/natives.md) says what changed and what stays the
  fork's own). The captures left potential work, in no order, each under its
  feature: Liberty Island's laser tripwires drawn not at all
  ([lasers](docs/re/natives.md#particles-and-lasers-render-iterators)),
  coronas smaller and dimmer than the original's and one at the frame's edge
  missing ([coronas](docs/re/natives.md#coronas)), the zone reverb ringing a
  third as long and the pan hard where the original's is soft
  ([sound](docs/re/natives.md#sound)), and `SetLocation`'s fitting in
  ([implemented, not as the original](docs/re/natives.md#implemented-not-as-the-original)).
  What remains of the milestones is by hand: the checks in
  [open decision 1](#open-decisions), the sound's heard with real audio,
  which the distrobox reaches now
  ([this machine](docs/DEVELOPMENT.md#this-machine)). The **[perf]** items
  that landed with M3 and M4 are to be re-measured on the device (decided
  2's list carries each). Beside that: the Smart Pro's performance work
  ([decided 2](#decided)) and the next ports
  ([open decision 4](#open-decisions)), at the owner's pick.

## Decided

1. **What the project is** (owner, 2026-09-23). A modern, cross-platform
   launcher for Deus Ex (UE1) of our own: the original `DeusEx.exe` was
   reverse-engineered as a starting point, not as a contract to stay faithful
   to. The engine is Surreal Engine as a vendored dependency: our own fork
   repository, pinned, not following upstream, and upgraded to a newer
   upstream only when the owner chooses
   ([`docs/ENGINE.md`](docs/ENGINE.md#how-it-is-kept)). **linux-x86_64 is
   the base**: the project is developed there and every port starts from it
   ([`docs/PORTING.md`](docs/PORTING.md)).
2. **Smart Pro performance** (owner, 2026-09-22): the target is **~20 FPS in
   Liberty Island's opening fight** (~50 ms a frame), and every trade-off made
   for it is accepted. It needs the script VM several times faster, so the deep
   VM work is in scope. Where it stands and where a frame goes:
   [the Smart Pro's Performance](ports/trimui-smartpro/README.md#performance);
   what each patch did: [`docs/ENGINE.md`](docs/ENGINE.md#what-the-fork-changes).

   The work, in no order (owner, 2026-09-23): what a profile turns up is added
   here as potential work, to take up or come back to. What is left in each is
   in [where a frame goes](ports/trimui-smartpro/README.md#where-a-frame-goes).
   Re-measure after each change, and profile on the device (`SAMPLE=1`): the
   desktop's proportions are not the device's.
   - **Collision traces** (in progress: patches 0025–0027, 0030–0032).
   - **The script interpreter** (patches 0012–0017, 0028–0029). What is left of
     its own time is mostly the Cortex-A53 waiting on memory for each
     expression node: only a denser, compiled form of each function's code
     would change that -- a rewrite of the evaluator's core. Even with no cost
     of its own, script time would only fall by a little over half: the
     natives the scripts call are the rest. Smaller: calls without an
     `ExpressionValue` per argument. How the original does both -- its
     bytecode run in place, each argument evaluated straight into the
     callee's frame: [the script interpreter](docs/re/core-dll.md#the-script-interpreter).
   - **Per-actor work** around the scripts. `IsEventEnabled` answers from
     one mask on the object now, and checks no call to any other function,
     as the original's (landed with M3's probe-mask work, 2026-09-25;
     [events and probes](docs/re/core-dll.md#events-and-probes)); its share
     of the tick is to be re-measured on the device. The rest of the
     per-actor work remains.
   - **The audio update's scan** of every actor for an ambient sound. The
     original does the same scan every frame
     ([each frame](docs/re/galaxy-dll.md#each-frame)): nothing of it to port.
   - **Actor meshes** (0018–0019 so far): the per-vertex work itself. A
     distant mesh is drawn with fewer vertices now (landed with M4's mesh
     detail, 2026-09-25; **[perf]** re-measure on the device:
     [mesh detail](docs/re/natives.md#mesh-detail)). The original also
     lights a vertex with the strongest lights only, checking their
     shadows every 16 frames ([lighting](docs/re/natives.md#lighting)):
     still to port.
   - **Visibility** (0020–0021 so far): still the largest render item.
   - **Lightmap uploads**: the original's rebuild shape landed with M4
     (2026-09-25) -- a map rebuilt only for its animated and moving
     lights over its kept static part, `NoDynamicLights` stopping that
     altogether, movers only when moved; **[perf]** re-measure on the
     device. Left: re-uploading only the rows a light changed, byte maps
     (the fork's floats convert on the CPU), and each surface's lightmap
     lookup ([lighting](docs/re/natives.md#lighting)).
   - **What is out of sight**: landed with M3 (2026-09-25) -- the fork
     keeps render time and stasis now, so actors in stasis do not tick and
     the scripts spare what was not drawn lately, an NPC's shadow among it
     ([out of sight](docs/re/natives.md#out-of-sight)). What it buys the
     device is to be measured there (**[perf]** re-measure).

   At native resolution the game tick does not move the frame until the GPU's
   time comes down (open decision 3); at 853×480 it does.
3. **Renderers on aarch64** (owner, 2026-09-22): the goal is Vulkan, OpenGL ES
   and software rendering all selectable; Vulkan is the only one the engine
   has. A GLES renderer is to be made at some point (owner, 2026-09-24), not
   yet scheduled: it means porting Surreal's desktop OpenGL 3.2 renderer (the
   Smart Pro's `renderers.ini` then needs only `EngineType=GLES`). Surreal has
   no software renderer at all.
4. **Reverse-engineering the game's DLLs** (owner, 2026-09-24), documented
   in [`docs/re/`](docs/re/README.md) as behaviour in our own words, never
   decompiled code. Desktop engine runs as needed to see what fires in play.
   IDA runs headless in the distrobox, any database opened by name
   ([how](docs/re/README.md#working-on-the-binaries)).
   - **The first pass** (done): DeusEx, Engine, Core, Extension, ConSys,
     DeusExText, and Render.dll where a feature's drawing lives there.
   - **The second** (done): Render.dll's
     [mesh detail](docs/re/render-dll.md#mesh-detail) and
     [lighting](docs/re/render-dll.md#lighting); Engine.dll's last natives
     ([traces](docs/re/engine-dll.md#traces), [moving](docs/re/engine-dll.md#moving),
     [small](docs/re/engine-dll.md#small)) and its
     [network code](docs/re/network.md); [`IpDrv.dll`](docs/re/ipdrv-dll.md);
     [`Galaxy.dll`](docs/re/galaxy-dll.md), the audio.
   - **The third** (done, 2026-09-25, with M4): `D3DDrv.dll` -- how the
     original looks: gamma, the light maps' brightness on screen, fog,
     detail textures, each pass's blending
     ([`d3ddrv-dll.md`](docs/re/d3ddrv-dll.md)); the reimplemented look is
     judged against it.
   - **Only if a need comes up**: `SoftDrv.dll` (a
     software renderer, decided 3), `Fire.dll` (fire, water and ice
     textures), `WinDrv.dll` (mouse and keyboard), and the owner's copied
     `ALAudio.dll` for what the original lacks (an EFX take on the reverb,
     HRTF; [what it is](docs/re/README.md#the-binaries)). **Not at all**:
     `Editor.dll`, `Window.dll`, the Glide, Metal and SGL drivers,
     `Setup.exe`, the GOG DLL and `RGalaxy.dll` (Galaxy.dll renamed, with one
     change).
5. **Multiplayer** (owner, 2026-09-24): Deus Ex's PvP servers are still up on
   a master server, and co-op is to be added one day. PvP is under way
   (owner's ask, 2026-09-26): M7 in [`docs/ROADMAP.md`](docs/ROADMAP.md),
   the fork's state in [multiplayer](docs/re/natives.md#multiplayer); the
   original's protocol is Unreal Tournament's
   ([the network](docs/re/network.md), [`IpDrv.dll`](docs/re/ipdrv-dll.md)).
6. **The order of reimplementation** (owner, 2026-09-24):
   [`docs/ROADMAP.md`](docs/ROADMAP.md) -- crashes, then saving and travel,
   then the story, the world's behaviour, the look, the sound, polish;
   multiplayer last. The milestones' status is tracked there. The engine
   moved to a full fork repository for this work (decided 1).

## Open decisions

1. **Verify by hand** (owner):
   - On the Smart Pro: that enemies notice the player and fight (patch
     0034); that NPCs out of sight still behave (Distant AI); the
     Video tab's Resolution at 960×540 and 853×480 -- the look, and the menu
     pointer's speed; START opens the pause menu on the first press after
     skipping the intro; SELECT opens it too; B/Y/SELECT/START close menus; the
     Customize buttons screen; the retired-layout upgrade being written on
     Play/Quit; CPU mode chosen from the Video tab; stick speeds -- look
     (`Speed=3.75`/`2.25`) and pointer speed are calibrated by reasoning, not by
     feel.
   - On a desktop: `scripts/dx.sh run linux-x86_64`, the home screen driven
     into a game, a pad in game; the Save and Load Game screens listing the
     saves with description and date, saving into a new slot, loading one
     and deleting one, each save's picture there (the original game's
     saves' too), and a hub map keeping its state over a travel out
     and back, by
     [natives.md](docs/re/natives.md#saving-loading-and-travel); the UI
     background option's three settings under the main menu and the
     credits over black, by [the UI](docs/re/natives.md#the-ui); rebinding a
     key in the game's Customize Keys screen (a double click or Enter starts
     it), the Load Game list sorted by date and re-sorted from its headers,
     moving through a list with the keys and a pad's d-pad, and a movement
     key held into a menu and let go there moving nothing when it
     closes, a click on the HUD mid-fight stopping fire but not the
     walk, an InfoLink message pushing the HUD's other parts into
     place as it appears and goes, a selection border in the
     inventory crisp with its pattern repeating rather than smearing,
     Ctrl+Z and Ctrl+Y in a save name, Tab between a screen's control
     groups, the pointer pinned while binding a key, Look Up Stairs
     tilting the view down UNATCO's stairs and easing level, (with
     positional sound on) a click at a screen's left edge sounding from
     the left, and the vision augmentation at level 1 showing a warm
     NPC through its grid, by [natives.md](docs/re/natives.md#lists),
     [the UI](docs/re/natives.md#the-ui) and
     [small](docs/re/natives.md#small);
     walking up to Tech Sergeant Kaplan on Liberty Island, an NPC's
     one-time chatter holding its last line, a conversation partner killed
     mid-line ending it, and -- deep in a Hong Kong game -- Maggie Chow's
     and Max Chen's meetings playing past their comment-label jumps, by
     [natives.md](docs/re/natives.md#conversations);
     logging in to a computer and its emails, a public computer's
     bulletins, reading a datacube and its kept note, a book's centred
     title and the credits' section breaks, by
     [natives.md](docs/re/natives.md#what-the-player-reads);
     coronas near and far and behind an NPC, by
     [natives.md](docs/re/natives.md#coronas); walking through a laser
     tripwire on Liberty Island, and a door's highlight, by
     [natives.md](docs/re/natives.md#implemented-not-as-the-original); a
     sound behind a wall, a light's hum, the music after a fight and the
     Speech slider, and a humming light or generator fading steadily on
     the walk away and silent right at its radius, effects louder against
     the music than before, its pitch steady on the walk toward it (only
     a moving ambient carrier bends), an NPC's bark moving no mouth
     outside a conversation and a partner's mouth closing on its own at
     a line's end, a generator dulling through a wall and opening back
     up in a doorway (never a conversation line), a security camera's
     hum quieter than an unlit machine's and a flickering light's hum
     wavering with it, combat music in fast and the ambient back where
     it left off (not from the top), and Battery Park's underground
     echoing against the open park with the echo gone on stepping back
     out, by
     [natives.md](docs/re/natives.md#sound) (with the
     desktop's audio: [linux-x86_64's](ports/linux-x86_64/README.md#audio));
     a shot fired around a corner turning the guards, and a body found
     raising the alarm, by
     [natives.md](docs/re/natives.md#hearing-the-ai-event-system); one of
     the original game's reference saves played on for a while
     ([natives.md](docs/re/natives.md#saving-loading-and-travel)); a
     cloaked commando, a burning NPC going out and a rat disappearing
     once out of sight, by
     [natives.md](docs/re/natives.md#the-native-tick-ascriptedpawntick);
     NPCs wandering their bit of Liberty Island and a searching NSF
     stepping around corners, by
     [natives.md](docs/re/natives.md#moving-wandering-and-tactical-movement);
     steam from a Hell's Kitchen street grate against the original,
     electricity arcing and a weapon's laser sight, by
     [natives.md](docs/re/natives.md#particles-and-lasers-render-iterators);
     an NPC walking away keeping its shape as detail fades, by
     [natives.md](docs/re/natives.md#mesh-detail); a lamp's corona coming
     up and going over about a third of a second as a corner hides and
     shows it, by [natives.md](docs/re/natives.md#coronas); a
     conversation partner's mouth moving with the speech, an NPC's head
     turning to follow the player and easing back, and blinking, by
     [natives.md](docs/re/natives.md#head-turns-and-lip-sync-blend-animations);
     an NPC under a street lamp, one walking from light into shadow, and
     a fire's glow on a face, and a flickering sconce's wall, a pulsing
     light throbbing and a triggered light going dark with their shadows
     still there, by [natives.md](docs/re/natives.md#lighting).
     The desktop defaults (4x MSAA, VSync on) are chosen by reasoning.
   - linux-aarch64 on any real device.
2. **Release polish** (owner's request, deferred): the home screen is
   deliberately verbose for development; a final build needs a declutter pass,
   and Surreal Engine's always-on Deus Ex stats overlay (FPS/actors/surfaces,
   `RenderCanvas.cpp` `DrawTimedemoStats`) hidden behind an option.
3. **The Smart Pro's Resolution default**: at native resolution, 20 FPS also
   needs the GPU's time per frame well below what it is now
   ([where a frame goes](ports/trimui-smartpro/README.md#where-a-frame-goes)),
   which a lower default Resolution would give -- for the owner to weigh.
4. **Next ports**: a cross-built engine for linux-aarch64 (a sysroot with the
   engine's libraries, as the Smart Pro has); Android (its README lists the
   work, starting with an in-process hand-over).
