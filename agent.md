# Port Ex Machina -- where things stand

The session handoff: state, decisions, what is open and what is next. It holds
no facts of its own beyond those; each lives in one doc, and the
[README's table](README.md#documentation) says which. Before working, read
[`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md).

## State (2026-10-05)

- **The audit** (2026-10-04/05, the owner's ask: every doc and the code read,
  and made to line up). Every repository's docs were read against the code,
  git history, the device logs and each other, then corrected where they had
  drifted -- in the workspace, the RE, the engine's `vibe/docs/` and the
  launcher's `main`, `linux-x86_64` and `trimui-smartpro` (the device branches
  take `linux-x86_64`'s by merge). **Pushed and pinned 2026-10-05** (the
  owner's go-ahead): each repository and branch committed, `linux-x86_64`
  merged into the four device branches, and the engine and every port
  branch pinned at what was pushed; `main`'s commit waits, with its icons
  (State, below), for the merge into `linux-x86_64`, the owner's call. The
  machine changed under the workspace (a new container, its
  folders moved, set up for the project on 2026-10-05 -- every port builds
  and tests, the harness and the recreation's check run clean: [this
  machine](docs/DEVELOPMENT.md#this-machine)); that left
  the launcher's worktrees unreadable to git, which `check-docs.sh` passed
  over in silence -- they are linked by relative paths now, and the guards
  fail on an unreadable checkout. The code defects it found are
  [known defects](#known-defects), not fixed.
- **The handoff** (2026-10-04, the owner's ask: every origin pushed and the
  project ready for another developer to take over). All four repositories
  are pushed -- every branch level with GitHub, nothing uncommitted or
  stashed -- and the pins are at what is pushed (`dx.sh check`). A clone of
  this repository from GitHub into an empty folder, run as the
  [quick start](README.md#quick-start) has a newcomer run it, fetched the
  others over anonymous HTTPS, passed `dx.sh check` (nothing missing; what
  it cannot verify is the game install's and `reference/`'s), and built and
  tested linux-x86_64 (`dx.sh test` passing, `test_gamefiles` skipped
  without the game; `dx.sh build`, launcher and engine, warning-free). The
  GLES renderer's milestone record and the two probes it cites, until then
  only in the parent folder, are in this repository's `plans/`.
  **Only on this machine**, so handed over apart from the repositories or
  remade: `gamefiles/` -- the game install, which a developer supplies for
  themselves (1112fm), and beside its binaries the IDA databases
  (`System/*.i64`, the RE's working state; they hold the game's code, so
  are never committed, and a fresh one gets its types, strings and names
  from dx-reverse-info's scripts:
  [working on the binaries](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries));
  `reference/` -- the 1112f SDK, the DeusExe launcher source, the
  databases' and the inis' backups, the original's saves and its wizard's
  captures; and the machine's set-up, the distrobox and IDA under Proton's
  wine ([this machine](docs/DEVELOPMENT.md#this-machine): a new container
  since, with IDA for Linux in their place, the audit above). `deps/` and
  `build/` the scripts remake. The fetches set the owner's commit identity,
  JuggyMcNutty, in every fresh clone ([commits](docs/DEVELOPMENT.md#commits));
  whether another developer commits as it or as themselves is the owner's
  call.
- **The review of the last session** (2026-10-02, the owner's ask: the open
  to-dos, the recent commits checked for inaccuracies and bugs, then the
  to-dos taken up; the owner's scope: the bugs, the path search, and four
  engine to-dos). Done, each proven in both engines, pushed, and the engine
  pinned with it (last at `9861c61`; Pushed and pinned has each commit):
  - **The bugs found**: DrawBorders drew no edges -- the margins' commit
    (`dfdc499`) gave every edge a negative length (`BorderConsole`, new) --
    and its layout is `XGC::DrawBorders`' now (`ColorsConsole`, new); a held
    item is drawn at the weapon triangle as Render.dll draws it, where the
    previous port's misreading drew a multitool the size of a building
    (`HeldConsole`, new); sound IDs are each object's own number, not its
    address's low 24 bits; `Mid` clamps its end as unsigned (`MidConsole`,
    new); the 800-unit sound radius is Deus Ex's alone.
  - **The path search, redone** from the binary -- the RE writeup the first
    port followed misread it three ways (the next bullet) -- and **the
    reachability tests** the original's: `pointReachable`, `actorReachable`,
    their walks and `AIDirectionReachable`'s steps (`ReachConsole`, new).
    51 of Liberty Island's 52 pawns move within tolerance of the original's,
    none stalling where it does not, and 134 of `ReachConsole`'s 144
    answers match to the unit (the other ten: two patrolling bots, asked
    from other spots).
  - **Where a falling actor rests**: every move the original makes is
    `ULevel::MoveActor`'s, which stops 2 units short of what it hits; the
    fork's went up to the hit -- the one difference behind both the falling
    rest and a walk's steps ending low (`RestConsole`, new). Deus Ex's falls
    and the reach tests' moves hold off now, landing goes through
    `setPhysics` with the hit actor (the floor's `SupportActor`), and Deus
    Ex's test moves pass decorations as they pass pawns: crates rest within
    0.04 of the original's, a carcass where the original's does, both based
    on the level.
  - **The look items**, against `D3DDrv`: no `darkClamp` -- `D3DDrv` draws
    32-bit textures on any display of 24 bits or more (`Use32BitTextures`
    is no option of its) -- the pier from 4.5-5.9% under to 1.7-3%, the corridor
    within 0.9%; a mesh's faces turned away culled unless two-sided, as
    Render.dll culls them -- the laser tripwires' beams within 0.5% of the
    original's red (65-70% over), with the iterator's own segment
    placement, and Liberty Island's trees, 1.9 times the original's,
    matching it (`LaserConsole`, `CoronaConsole`); the unlit level and the
    wavers' and shimmer's random tables the original's; the cloud cast is
    the plain shape in both.
  - **The docs** that carried the misreadings are corrected:
    dx-reverse-info, VibeEngine's NATIVES.md, the fidelity plan.

  **Left open from it**, each in its doc: the pier's last 1.7-3% (unread);
  `processLanded`'s other branches -- a decoration nudged off a ledge it
  overhangs, a carcass's bounce off a slope, a bounce zone's re-throw, a pawn
  fitted off a ledge -- not ported; the other light effects' shapes (the
  waves and the rest) the fork's, unread; Terrorist10 moving farther than the
  original's (1010 units to 652, as before the review); by hand, jumping onto
  an NPC's head (its stomp and bounce come through `SupportActor` now). And,
  recorded only here: the Vulkan device's scene shader without `darkClamp`
  is built but has not run -- the comparisons ran the GL device, which the
  per-user `Settings.json` named.
- **The path search is the original's** (2026-10-02, redone in the review),
  the game-fidelity pass's last diagnosed item: `FindPathToward` and
  `FindPathTo` as Deus Ex's Engine.dll has them end to end -- the lists
  around the pawn and the goal, the anchor and the end points
  (`definePathsFor`), the search best first over its own open list
  (`breadthPathFrom`), the reach flags, the step after it, the second way,
  `HandleSpecial`, a falling goal's landing, and no `RouteCache`, which the
  original never fills (VibeEngine's
  [moving](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#moving-wandering-and-tactical-movement);
  [the search](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#the-search),
  rewritten from the binary). `MoveConsole` against the original's own run,
  laid side by side by the new `move.py`: 48 of Liberty Island's 52 pawns'
  distance moved within tolerance, where the first port gave 43 and the
  fork's own search 50; UNATCOTroop1 walks its whole patrol, and
  Terrorist35, which the first port stalled, walks as the original's.
  Terrorist34, which the redone search still stalled on the fork's own
  `pointReachable`, walks as the original's since the reachability tests
  are the original's too (the review): 51 of 52.
- **Three small originals** (2026-10-01), each RE-backed and built: a pawn
  holding no weapon draws its `SelectedItem` (`Render.dll`, a pawn's
  attachments) -- redone in the review above, the first port having drawn it
  "where the item is", a misreading; a script
  sound with no radius is heard 800 units out, the original's figure, where the
  fork passed 1,500 through and computed its priority on it; `Object.Mid` with a
  negative start gives an empty string. A fourth found **not landable**:
  `Object.CriticalDelete` frees an object at once, and the fork has no object
  lifecycle at all -- nothing in the engine ever deletes a `UObject` (its
  collector is never run, the one call commented out), so there is no
  collector to be early for, and a bare `delete` would leave the package's
  object table pointing at freed memory. All seven commits are pushed and
  the pins moved with them.
- **The game-fidelity pass** (2026-10-01, from the owner's play report;
  the record is [plans/game-fidelity.md](plans/game-fidelity.md)): five
  play-reported differences taken up, all five now fixed (the fifth by the
  search above). **Fixed:** skipped
  conversation speech now stops (the device's StopSound matched the caller
  as well as the ID; the original stops by ID alone -- SkipConsole and
  skip.py prove it from the recordings); the object belt's text now draws
  (centred/right-aligned text with wrap off aligned within the wrap width,
  100,000 px wide, so the belt's descriptions, counts and slot numbers
  drew off-screen -- BeltConsole proves it); keyboard focus now moves
  between windows (MoveFocus was a stub and buttons were not selectable:
  a conversation's choices had no selector and never answered Up/Down --
  ChoiceConsole proves the cycle and the blue); and findPathToward walks
  straight to a directly reachable target (bots detoured through path
  nodes to patrol points in plain sight -- MoveConsole proves the routes
  match). **The fifth (the path search) is fixed too**, 2026-10-02, by the
  first bullet of this State. **Compared clean:** the
  death path matches the original's throughout (the robots' freeze in
  Dying forever is the original's own behaviour), and the mission sweep
  (MissionConsole) finds nothing of the fork's on the 83 maps: 78 OK, three
  not mission maps, and two odd ones the original's own behaviour too.
- **The repositories**, four since the split (2026-09-27,
  [decided 7](#decided)): this one, the workspace, on `main`, public at
  https://github.com/JuggyMcNutty/port-ex-machina (its history was rewritten
  before publishing, 2026-09-22, to drop the game's files and a personal
  email address); the launcher,
  [deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher); the
  engine, [VibeEngine](https://github.com/JuggyMcNutty/VibeEngine); the RE,
  [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info). Each is
  a folder of its own beside the others ([the layout](README.md#layout)), and
  each is pushed with the owner's go-ahead.
- **Pushed and pinned** (2026-10-05, the audit, above: the engine at
  `8835557`). Before it (2026-10-02, the owner's go-ahead): everything in
  the four repositories; the engine pinned at `9861c61`, the review's last.
  The review's engine commits, newest first, each pushed with its
  dx-reverse-info commit and this file: `9861c61` the random tables and the
  cloud cast, `8f94d46` mesh faces culled and the laser iterator, `bf12e65`
  no `darkClamp`, `eb7a50e` moves held off and landing, `6829f0e` the
  reachability tests, `f804c77` DrawBorders' layout, `9f45b2b` the path
  search redone, `ac0c28e` the bugs (with dx-reverse-info's correction of a
  pawn's attachments, and its `acc4cf1`, LevelInfo's clock, left unpushed by
  the previous session). Before the review, `cfda48d`: the game-fidelity
  pass's five commits (the skip, the belt text, the focus movement, the
  direct-walk pre-check, the mission sweep) and that session's two (the
  first path-search port; a pawn's held item, `PlaySound`'s radius, `Mid`'s
  negative start), with the RE's own commit in
  [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info). The
  workspace pins the engine and every port branch (`ENGINE-PIN.txt`,
  `LAUNCHER-PIN.txt`); no port branch changed in the review. Before that (2026-10-01, the owner's
  go-ahead): the engine's
  GLES renderer -- the GL device on desktop GL and OpenGL ES
  3.2 from one code path, `Type=GLES`, with the CPU staging streams, the RGBA8
  scene buffers when Hdr is off, the render scale, the CPU decoders for what
  the GE8300 cannot sample or filter, the ReadPixels implementation and its
  orientation, the GE8300's sampler set and fullscreen -- the workspace's
  `plans/gles-renderer.md` is the milestone record) and the launcher's
  trimui-smartpro (`renderers.ini`'s `[GLES]` row selectable, the GLES numbers
  in the README's table) -- the engine and every port branch pinned at what
  was pushed. The previous push (2026-09-29, the owner's go-ahead) took the
  three repositories with work -- port-ex-machina (decided 9, the level
  start's renaming, the [perf] re-measurements), VibeEngine (the profiling
  hooks' re-based patch, the level start's renaming, the re-measured [perf]
  items) and the launcher's trimui-smartpro (the 853×480 default, the level
  start's renaming, the M3–M7 numbers). `main`'s windows' icons (2026-09-28,
  Next) are not merged into the port branches -- whether they go there too is
  the owner's call.
- **The launcher** the ports run -- `linux-x86_64`'s branch, and the devices'
  from it -- runs on linux-x86_64 and the Smart Pro. It is deliberately
  verbose for development. `main`, the original recreated
  almost 1:1, is whole: the `DeusEx` program -- the original's launch
  sequence, wizard, splash and message boxes -- starts VibeEngine through its
  `run-game.sh` and stays with it; `scripts/recreation.sh` builds, installs,
  checks and runs it (Next has what it turned up).
- **The engine** is our own fork repository (VibeEngine, branch `deusex`;
  renamed from SurrealEngine by the owner, 2026-09-26, the old address
  redirecting), pinned by `ENGINE-PIN.txt`; the patch stack was retired into
  it (2026-09-24), and its docs and tools are its own `vibe/` (2026-09-27). It
  holds upstream's latest when last merged (2026-09-24); upstream has moved
  on since (`scripts/engine.sh status` says how far), and whether and when to
  merge it (the fork's `vibe/tools/upgrade.sh`) is the owner's call. A merge
  needs care in the script VM: upstream has since written an interpreter of
  its own (`Frame::RunExpr`, its default from 2026-10-04) in place of the
  `ExpressionEvaluator` that the fork's VM work reworked ([decided 2](#decided)). Its GL
  device now runs on desktop GL and OpenGL ES 3.2 from one code path -- the
  GLES renderer (2026-10-01, [What the fork changes](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#rendering),
  the workspace's `plans/gles-renderer.md` the milestone record); pinned at
  `8835557` since the audit (State). The profiling hooks were re-based onto the
  head of 2026-09-29 and are applied only
  for a device profile (`perf.sh on/off`; the device's current build is clean
  of them).
- **linux-x86_64**, the base: launcher and engine build natively; the staged
  app ran the engine into the intro level on the development PC, and its
  launcher straight into Liberty Island (`DXL_NO_HOME=1`, 2026-09-27);
  unattended runs drove saves, loads and hub travel through in play
  (2026-09-25).
- **trimui-smartpro**: the game runs; the performance work is on hold with
  the ports ([decided 8](#decided);
  [its Performance](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#performance)). The device
  has the fork's GLES renderer (2026-10-01) in a clean build: the launcher's
  Video tab lists OpenGL ES as selectable, and the level start under
  `Type=GLES` runs at 8.5–8.8 FPS native and 10.0 at 853×480. The Vulkan
  device's same-build run is 11.4 at 853×480 -- the gap there is the GL
  driver's per-draw-call cost -- and its native figure M3–M7's 8.2, so at
  native the two are level (the run once given as Vulkan's native 11.4 was
  at 853×480: its log's `RenderScale` is 0.666667, 2026-10-04; the numbers
  and their story are in [the port
  README](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#where-a-frame-goes)).
  The Vulkan renderer's numbers stand (M3–M7: 8.2 FPS native, 11.5 at
  853×480). Its owner's settings are Distant AI
  (characters out of sight think less often) on and 853×480 (2026-09-23),
  which the level start's native rows switch to native for the run. It has no battery (its battery
  warnings are off: [its Gotchas](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#gotchas)).
- **The game's DLLs**: seven passes are read (the last 2026-09-28;
  [decided 4](#decided)), and what the game-fidelity pass and the review
  needed since (2026-10-01/02). [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info)
  covers each binary read, and an IDA database gets three scripts from its
  `tools/ida/` (types, strings, names; `Render.dll` its types from a fourth,
  `render_types.py`).
  - **`DeusEx.dll`**, **`Engine.dll`** (its network code in
    [`network.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/network.md)), **`Core.dll`**, **`Extension.dll`**,
    **`ConSys.dll`**, **`DeusExText.dll`**, **`Render.dll`** (where a
    feature's drawing lives there, its mesh detail and lighting),
    **`IpDrv.dll`**, **`Galaxy.dll`**, **`D3DDrv.dll`**, **`Fire.dll`** and
    **`WinDrv.dll`** (its command-line flags)
    ([`deusex-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/deusex-dll.md),
    [`engine-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md),
    [`core-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/core-dll.md),
    [`extension-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/extension-dll.md),
    [`consys-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/consys-dll.md),
    [`deusextext-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/deusextext-dll.md),
    [`render-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/render-dll.md),
    [`ipdrv-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/ipdrv-dll.md),
    [`galaxy-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/galaxy-dll.md),
    [`d3ddrv-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/d3ddrv-dll.md),
    [`fire-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/fire-dll.md),
    [`windrv-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/windrv-dll.md)); their databases are typed,
    named and backed up. Engine.dll's lost its root node, IDA's record of
    the input file, in an idle worker's save (2026-09-27; what removed it
    is not found) and has it back from the backup, nothing else lost.
    Render's and Engine's took the light maps' names and comments on
    2026-09-28, Galaxy's the mixer's and the reverb's and Render's the
    drawn test's, and are backed up again. Engine's was saved again early on
    2026-10-02 (by whom unrecorded), and it and D3DDrv's were re-saved by
    idle workers later that day after read-only sessions (decompiler
    caches). On 2026-10-04 the six newer than their backups -- Core's,
    DeusEx.dll's and IpDrv's (saved 2026-09-27), Extension's (2026-10-01),
    D3DDrv's (2026-10-02) and Engine's (2026-10-03) -- were compared with
    them item by item, copies of both opened in IDA: each holds all its
    backup does (the input file's record, names, comments, local types,
    functions, string definitions, the same hand-set prototypes), and more
    only of the decompiler's own guesses, prototypes and data types set on
    what was decompiled since -- a few in place of the ones the mangled
    names gave. They are the backups now, all thirteen byte for byte the
    databases; the six backups they replaced are kept in
    `reference/idb-backup/superseded-2026-10-04/`. No working files are
    left beside them.
  - **What Surreal lacks** of them is VibeEngine's
    [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md),
    from [`natives_audit.py`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/tools/natives_audit.py),
    map runs, and runs with temporary hooks (each removed before committing).
- **linux-aarch64**: the launcher cross-builds; never run on a device.
- **android**: planned; [its README](https://github.com/JuggyMcNutty/deusex-launcher/blob/android/ports/android/README.md) is the plan.
- **x360**: planned; nothing about it is worked out yet.
- **Next** ([decided 8](#decided)): the engine's stability and features, M7
  finished (2026-09-27); the ports and the open decisions wait.
  **M7, multiplayer** (the owner's ask, 2026-09-26;
  [decided 5](#decided)): the fork joins a server and plays on it -- the
  original's listen server, and live servers since the owner's go-ahead
  (2026-09-27): it downloads what it lacks, loads their mods and stays in
  --; as a server it takes the original's join, replicates the level to it,
  calls it, serves it downloads, and answers LAN and GameSpy queries and a
  master server as the original does, travels with its clients following
  (as they follow the original's), and runs dedicated (`--server`). M7's
  items are all in (2026-09-27,
  [`ROADMAP.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ROADMAP.md#m7----multiplayer)); its by-hand check
  waits with the open decisions ([open decision 1](#open-decisions)). What
  it left is finished too (owner, 2026-09-27: "finish all of M7"; the last
  of it the same day: a walk on a live server, by the harness's live mode --
  VibeEngine's
  [`DEVELOPMENT.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#scripted-runs-of-both-engines)).

  The reimplementation's milestones are done -- M0 through M6 all in
  (most of their code 2026-09-25, M0's acceptance captures 2026-09-26, the
  last item Galaxy's mixer 2026-09-28;
  [`ROADMAP.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ROADMAP.md) tracks each item, and
  [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md) says what changed and what stays the
  fork's own). The look is compared against the original's own renderer,
  `D3DDrv`, since 2026-09-28 (`DXCAP_RENDERER=D3D`; its frames match
  `OpenGLDrv`'s, and a run the game moves to `SoftDrv` stops:
  [scripted runs](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/DEVELOPMENT.md#scripted-runs-of-both-engines)).
  **Potential work the comparisons left**, in no order, each under its
  feature: Liberty Island's pier floor 1.7 to 3% darker than `D3DDrv`'s in
  the frames' terms ([brightness](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#brightness)); the shapes of
  the light effects whose original is unread, still the fork's
  ([lighting](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#lighting)); at the level's start 9 NPCs counted
  as drawn where the original has 3, far off over the seawall, 1 to 9
  pixels of theirs showing at the edges -- a pixel's difference between the
  engines' rasterizing, the proxies' rectangles the original's and per-zone
  span buffers no help (tried; `AIConsole`,
  [out of sight](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#out-of-sight)); and the review's open items
  (State). **Matched since**, each in its section: the light maps and
  meshes (2026-09-28, [lighting](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#lighting)); where a trace
  stops, walking's float over the floor, `SetLocation`'s and a spawn's
  fitting in with the encroachment check, the visible-actor iterators, and
  `LineOfSightTo`, `CanSee` and `PlayerCanSeeMe`
  ([implemented, not as the original](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#implemented-not-as-the-original));
  the weapon in hand's field of view and lag ([small](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#small));
  the zone reverb and the pan ([sound](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#sound)); the saved
  level's out-of-world troops, sitters, a terrorist fighting a security bot,
  the event manager's listeners, `TraceTexture`, traces from inside a pawn,
  and Terrorist15's patrol (2026-09-27 and 28;
  [starting up](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#starting-up),
  [hearing](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#hearing-the-ai-event-system),
  [moving](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#moving-wandering-and-tactical-movement),
  [implemented, not as the original](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#implemented-not-as-the-original)); the path
  search, the reachability tests, where a falling actor rests and the look
  items (2026-10-02, State).
  What remains of the milestones is by hand, waiting with the open
  decisions: the checks in [open decision 1](#open-decisions), the sound's
  heard with real audio, which this container reaches too, as the previous
  one did ([this machine](docs/DEVELOPMENT.md#this-machine)). Waiting with the ports
  ([decided 8](#decided)): the Smart Pro's performance work ([decided
  2](#decided)) and the next ports ([open decision 2](#open-decisions)); the
  **[perf]** items that landed with M3 and M4 were re-measured on the device
  2026-09-29 (decided 2's list carries each, and [where a frame
  goes](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#where-a-frame-goes)
  the breakdown).

  **The launcher's `main`** ([decided 7](#decided)) is whole (2026-09-27):
  the original `DeusEx.exe` recreated from dx-reverse-info's
  [`launch-flow.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/launch-flow.md)
  and [`wizard.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/wizard.md)
  in the shape the owner set that day -- the pages laid out from
  `Window.dll`'s templates, 14 of the 15 screens captured from the original
  under wine matching to the pixel (the splash's picture the other); the
  launcher resident for the game's
  run; the engine's side in VibeEngine (the original's command line, safe
  mode's flags, `EXEC=`, the line that hands the running game a forwarded
  URL); `DeusEx` installed in the game's `System/` by
  `scripts/recreation.sh`, which also runs it end to end on a private
  display (`check`, 35 checks). Merged into the port branches; pushed, and
  the ports and the engine pinned at it (2026-09-27; its icons since are
  `main`'s alone, State). Potential work it turned up, in no order:
  installed beside `DeusEx.exe`, `DeusEx` stops the original game there
  under Wine or Proton at its start, taken for the `DeusEx` package
  ([a package's file](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/core-dll.md#packages-and-linkers); the harness runs the
  original from a view without it); the profiling hooks' patch was
  re-based onto the fork's head (2026-09-29, `perf.sh on`, resolve,
  `save`) and took the device's profiles (the Performance section's
  M3–M7 row); and upstream's new commits (above). Its windows' icons are
  done since 2026-09-28: the wizard shows
  the game's, read out of the install's `DeusEx.exe`, and the error box
  wine's own, to the pixel.

## Known defects

Found by the audit (2026-10-04/05), not fixed: each wants a build, which this
container can do once [this machine](docs/DEVELOPMENT.md#this-machine) is set
up, and a run; some want a decision. Each is recorded where its code is
documented.

1. **No pad in game on the desktop build.** With SDL3 installed,
   SurrealWidgets builds its SDL3 window backend, and the fork's pad support
   is the SDL2 backend's alone; `run-game.sh` asks for SDL2 and falls back to
   Wayland or X11 without a word
   ([linux-x86_64's gotcha](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/ports/linux-x86_64/README.md#no-pad-in-game);
   open decision 1's "a pad in game"). Likely fix: `ENABLE_SDL3` off in its
   `engine.cmake`.
2. **`main` drops the command line's quotes** on the way to the engine
   (`INI="My Mod.ini"` arrives as `INI=My`), and **its `ParseParam` is
   stricter than the original's** (`-safemode` is `-safe` to the original)
   ([main's known defects](https://github.com/JuggyMcNutty/deusex-launcher/blob/main/README.md#known-defects)).
3. **The ports' launcher**: the game gets the launcher's words as the
   engine's own options, so a map, `-server` or `INI=` is lost; the
   single-instance lock is let go at the hand-over, so a launch during a game
   reads as a crash; the Video tab's Resolution row is Vulkan's only, though
   the GL device honours `RenderScale`; and `test_target_<port>` passes a CPU
   mode named anywhere in `port-hooks.sh`
   ([LAUNCHER.md's known defects](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/docs/LAUNCHER.md#known-defects)).
4. **The engine**: anchored, the path search returns a navigation-point goal
   where the original returns the anchor
   ([moving](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#moving-wandering-and-tactical-movement)); the coronas take
   every dynamic corona light, not the viewer's leaf's (no Deus Ex map has
   one: [coronas](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#coronas)); and `GC.DrawActor`, the vision
   augmentation's, stamps no render time ([out of sight](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#out-of-sight)).

Smaller, recorded only here: `dxcap.sh help` stops without `gamefiles/`;
`natives_audit.py --runs` names each run's map after its log file, which is
`engine.log` for every harness run; `skip.py` prints nothing without
arguments; the recreated launcher's `policy.c` keeps bypass and splash rules
of its own that only its tests and nothing live use; dx-reverse-info's
`types/launch.h` needs `<stddef.h>` to compile on its own (IDA parses it as
is); and a clean Smart Pro build shows one warning, the third-party
`Thirdparty/resample/pffft.cpp`'s own `#warning` that it builds without SIMD
on aarch64 (it tests `__arm__`, not `__aarch64__`) -- compiled, but unused:
the resampler is built without it (`R8B_PFFFT` 0). The builds that were
called warning-free were incremental ones, which never recompiled it.

## Decided

1. **What the project is** (owner, 2026-09-23). A modern, cross-platform
   launcher for Deus Ex (UE1) of our own: the original `DeusEx.exe` was
   reverse-engineered as a starting point, not as a contract to stay faithful
   to -- the ports' launcher is that; since the split, the launcher's `main`
   is the original recreated almost 1:1 beside it (decided 7). The engine is
   Surreal Engine as a vendored dependency: our own fork
   repository, pinned, not following upstream, and upgraded to a newer
   upstream only when the owner chooses
   ([`ENGINE.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#how-it-is-kept)). **linux-x86_64 is
   the base**: the project is developed there and every port starts from it
   ([`docs/PORTING.md`](docs/PORTING.md)).
2. **Smart Pro performance** (owner, 2026-09-22): the target is **~20 FPS in
   Liberty Island's level start** (~50 ms a frame), and every trade-off made
   for it is accepted. It needs the script VM several times faster, so the deep
   VM work is in scope. Where it stands and where a frame goes:
   [the Smart Pro's Performance](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#performance);
   what each patch did: [`ENGINE.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ENGINE.md#what-the-fork-changes).

   The work, in no order (owner, 2026-09-23): what a profile turns up is added
   here as potential work, to take up or come back to. What is left in each is
   in [where a frame goes](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#where-a-frame-goes).
   Re-measure after each change, and profile on the device (`SAMPLE=1`): the
   desktop's proportions are not the device's.
   - **Collision traces** (patches 0025–0027, 0030–0032). Re-measured
     2026-09-29: the traces' own time is ~2 ms of the tick
     (`TraceAABBModel::Trace` ~1.8), where ~8 a frame went through the traces
     in all.
   - **The script interpreter** (patches 0012–0017, 0028–0029). Re-measured
     2026-09-29: ~15 ms under the VM (~12 by the frame-time hooks' own
     count), ~6 of it the interpreter's own, over ~2,000 VM calls a frame
     (was ~10,000). What is left of its own time is mostly the Cortex-A53
     waiting on memory for each expression node: only a denser, compiled form
     of each function's code would change that -- a rewrite of the
     evaluator's core. Even with no cost of its own, the VM's time would only
     fall by a little less than half: the natives the scripts call are the
     rest. Smaller: calls without an `ExpressionValue` per argument. How the
     original does both -- its bytecode run in place, each argument evaluated
     straight into the callee's frame: [the script interpreter](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/core-dll.md#the-script-interpreter).
   - **Per-actor work** around the scripts. `IsEventEnabled` answers from
     one mask on the object now, and checks no call to any other function,
     as the original's (landed with M3's probe-mask work, 2026-09-25;
     [events and probes](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/core-dll.md#events-and-probes));
     re-measured 2026-09-29, its own time is ~0.1 of the tick (was ~1.4), and
     the work around the scripts is ~9 ms of it. The rest of the per-actor
     work remains.
   - **The audio update's scan** of every actor for an ambient sound. The
     original does the same scan every frame
     ([each frame](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/galaxy-dll.md#each-frame)): nothing of it to port.
   - **Actor meshes** (0018–0019 so far): the per-vertex work itself. A
     distant mesh is drawn with fewer vertices now (landed with M4's mesh
     detail, 2026-09-25;
     [mesh detail](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#mesh-detail)) --
     re-measured 2026-09-29, the meshes are ~7.5 ms of the render for ~40 in
     view (was ~11). A vertex is lit by the strongest lights only, their shadows checked
     every 16 frames, as the original's (landed with M4, 2026-09-25;
     [lighting](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#lighting));
     re-measured 2026-09-29, the lightmaps are ~3.7 ms and the vertex
     lighting ~1.1.
   - **Visibility** (0020–0021 so far): still the largest render item (~20 ms
     of it, re-measured 2026-09-29).
   - **Lightmap uploads**: the original's rebuild shape landed with M4
     (2026-09-25) -- a map rebuilt only for its animated and moving
     lights over its kept static part, `NoDynamicLights` stopping that
     altogether, movers only when moved. Left: re-uploading only the rows a
     light changed, byte maps (the fork's floats convert on the CPU; Deus
     Ex's hold the original's byte values since 2026-09-28, as floats), and
     each surface's lightmap lookup
     ([lighting](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#lighting)) --
     re-measured 2026-09-29: lightmaps ~3.7 ms (6 rebuilt a frame, the
     BarrelFire's), uploads ~2.
   - **What is out of sight**: landed with M3 (2026-09-25) -- the fork
     keeps render time and stasis now, so actors in stasis do not tick and
     the scripts spare what was not drawn lately, an NPC's shadow among it
     ([out of sight](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#out-of-sight)); since 2026-09-27 only
     what shows past the world in front of it counts as drawn, as the
     original, by occlusion proxies filtered down the BSP on the render
     CPU. Measured 2026-09-29: the frame went from ~96 to ~87 ms at 853×480;
     at native the render grew ~3 with the proxies' walk, and its GPU wait
     from ~24 to ~30.

   At native resolution the game tick does not move the frame until the GPU's
   time comes down (decided 9); at 853×480 it does.
3. **Renderers on aarch64** (owner, 2026-09-22): the goal is Vulkan, OpenGL ES
   and software rendering all selectable. **Vulkan and OpenGL ES are both real
   now** (2026-10-01): the GLES renderer is the fork's GL device on an ES 3.2
   context, selectable in the Smart Pro's Video tab (`renderers.ini`
   `EngineType=GLES`), measured at 10 fps at 853×480 against the Vulkan's
   11.4 on the same build, and at native level with it (8.5–8.8 against 8.2;
   the milestone record is the workspace's `plans/gles-renderer.md`); what
   remains to the ~20 FPS target is the GL driver's per-draw-call cost.
   Surreal has no software renderer at all.
4. **Reverse-engineering the game's DLLs** (owner, 2026-09-24), documented
   in [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info) as
   behaviour in our own words, never decompiled code. Desktop engine runs as needed to see what fires in play.
   IDA runs headless, any database opened by its path, through Hex-Rays'
   own IDA MCP server and IDA for Linux since 2026-10-05 -- before, Windows
   IDA under Proton ([how](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#working-on-the-binaries)).
   A session rewrites a database's `.i64` when it closes, so the databases
   drift from their backups as they are worked on.
   - **The first pass** (done): DeusEx, Engine, Core, Extension, ConSys,
     DeusExText, and Render.dll where a feature's drawing lives there.
   - **The second** (done): Render.dll's
     [mesh detail](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/render-dll.md#mesh-detail) and
     [lighting](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/render-dll.md#lighting); Engine.dll's last natives
     ([traces](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#traces), [moving](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#moving),
     [small](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#small)) and its
     [network code](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/network.md); [`IpDrv.dll`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/ipdrv-dll.md);
     [`Galaxy.dll`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/galaxy-dll.md), the audio.
   - **The third** (done, 2026-09-25, with M4): `D3DDrv.dll` -- how the
     original looks: gamma, the light maps' brightness on screen, fog,
     detail textures, each pass's blending
     ([`d3ddrv-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/d3ddrv-dll.md)); the reimplemented look is
     judged against it.
   - **The fourth** (done, 2026-09-27): `Fire.dll` -- the fire, water and
     ice textures, once the Dragon's Tooth's blade showed them wrong
     ([`fire-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/fire-dll.md)); checked against its own
     routines run in an emulator.
   - **The fifth** (done, 2026-09-27, for the launcher's `main`): what the
     recreation still needed of `DeusEx.exe`, with the SDK's source of its
     pages checked against it
     ([`wizard.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/wizard.md),
     [`launch-flow.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/launch-flow.md));
     `Window.dll`'s dialog templates, as data (owner, 2026-09-27;
     [page layouts](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/wizard.md#page-layouts));
     and where the engine reads each flag safe mode emits -- `Engine.dll`,
     `Galaxy.dll`, `Core.dll` and `WinDrv.dll`
     ([`cli-flags.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/cli-flags.md#flags-the-launcher-emits-safe-mode)).
   - **The sixth** (done, 2026-09-28): the light maps to the byte --
     `Render.dll`'s texel (shadows, shapes, the light's table, the merge),
     `GlobalLighting`'s types and `Engine.dll`'s `FGetHSV`
     ([light maps](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/render-dll.md#light-maps)) -- and `D3DDrv.dll`'s
     light-map upload and passes again, which corrected the third pass: in
     the one-pass path a byte is worth 1/128, not 1/64
     ([the light maps' brightness](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/d3ddrv-dll.md#the-light-maps-brightness)).
   - **The seventh** (done, 2026-09-28): `Galaxy.dll`'s mixer and reverb
     to the sample -- the pan's formula and law, the sliders squared, the
     SSE routine's linear resampling, the loops, and the reverb's three
     allpass stages, checked by running them over the original's own
     recording ([the mixer](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/galaxy-dll.md#the-mixer),
     [reverb](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/galaxy-dll.md#reverb)).
   - **Since, as a fix needed it** (2026-10-01/02, the game-fidelity pass and
     the review): `Engine.dll`'s path search, reachability tests, `MoveActor`'s
     hold-off and landing, and `LevelInfo`'s clock
     ([the search](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#the-search),
     [reaching](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#reaching), [small](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/engine-dll.md#small));
     `Render.dll`'s attachments, faces drawn, laser iterator, random tables
     and cloud cast ([`render-dll.md`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/render-dll.md)); `D3DDrv.dll`'s
     textures ([textures](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/d3ddrv-dll.md#textures)); `Extension.dll`'s
     borders ([borders](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/extension-dll.md#borders)) -- its text alignment
     and focus movement, read for the fidelity pass, are only in VibeEngine's
     [the UI](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-ui) so far.
   - **Only if a need comes up**: `SoftDrv.dll` (a
     software renderer, decided 3), `WinDrv.dll` beyond its flags (mouse and keyboard), and the owner's copied
     `ALAudio.dll` for what the original lacks (an EFX take on the reverb,
     HRTF; [what it is](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/README.md#the-binaries)). **Not at all**:
     `Editor.dll`, `Window.dll`'s code, the Glide, Metal and SGL drivers,
     `Setup.exe`, the GOG DLL and `RGalaxy.dll` (Galaxy.dll renamed, with one
     change).
5. **Multiplayer** (owner, 2026-09-24): Deus Ex's PvP servers are still up on
   a master server, and co-op is to be added one day. PvP is under way
   (owner's ask, 2026-09-26): M7 in [`ROADMAP.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ROADMAP.md),
   the fork's state in [multiplayer](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#multiplayer); the
   original's protocol is Unreal Tournament's
   ([the network](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/network.md), [`IpDrv.dll`](https://github.com/JuggyMcNutty/dx-reverse-info/blob/main/ipdrv-dll.md)).
6. **The order of reimplementation** (owner, 2026-09-24): VibeEngine's
   [`ROADMAP.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/ROADMAP.md) -- crashes, then saving and travel,
   then the story, the world's behaviour, the look, the sound, polish;
   multiplayer last. The milestones' status is tracked there. The engine
   moved to a full fork repository for this work (decided 1).
7. **Four repositories** (owner, 2026-09-27). The RE is
   [dx-reverse-info](https://github.com/JuggyMcNutty/dx-reverse-info): the
   original binaries only. The launcher is
   [deusex-launcher](https://github.com/JuggyMcNutty/deusex-launcher),
   started anew: its default branch, `main`, is an almost 1:1 recreation of
   the original launcher on desktop Linux with SDL2 (the "almost" only where
   Win32 has no Linux counterpart), and each port is a branch holding that
   port's configs and additions -- `linux-x86_64` is `main` plus the launcher
   as it grew, and the devices' branches start from it
   ([`PORTING.md`](docs/PORTING.md)). The engine and everything about it --
   its docs, roadmap, what it lacks, its tools -- is VibeEngine's, in its
   own `vibe/`. This repository is the workspace that builds the ports. Each
   of the four is a folder of its own in one parent folder, none inside
   another (owner, 2026-09-27;
   [the repositories](docs/DEVELOPMENT.md#the-repositories)).
8. **The engine first** (owner, 2026-09-27). The porting work waits until
   the engine is more stable and has more of the game's features: the
   Smart Pro's performance (decided 2) with its device re-measures and the
   profiling hooks' re-basing, and the next ports (open decision 2). The
   open decisions wait until the owner takes them up -- likely in a large
   play-testing session -- unless one stops the work, when the owner is
   asked. M7 is finished first, with everything it left.
9. **The Smart Pro's default resolution** (owner, 2026-09-28): 853×480 --
   `Performance.RenderScale` 0.6666667 in the port's packaged
   `engine-settings.json.default`, committed on the launcher's
   `trimui-smartpro` and pushed with it (2026-09-29). There the frame
   is the CPU's work, so [decided 2](#decided)'s speed-ups buy frames
   directly and the ~20 FPS target needs no GPU renderer work; at the
   panel's 1280×720 the GPU's ~63 ms holds the Vulkan device's frame, and
   the GLES renderer (decided 3) runs level with it there, its time the GL
   driver's draw calls on the CPU
   ([where a frame goes](https://github.com/JuggyMcNutty/deusex-launcher/blob/trimui-smartpro/ports/trimui-smartpro/README.md#where-a-frame-goes)).
   The device already ran 853×480 as its owner's setting (2026-09-23); the
   Video tab still offers 960×540 and the panel's 1280×720 per session. A
   640×360 option was weighed and left out: 480 lines is the least Deus
   Ex's menus fit in (its own resolution menu refuses under 640×480).

## Open decisions

Each waits until the owner takes it up ([decided 8](#decided)).

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
   - On a desktop, the recreation (`scripts/recreation.sh run`): its wizard,
     splash and message boxes under the desktop's own window manager, and a
     second launch with a map bringing the game's window to the front --
     what `recreation.sh check`'s private display, which has no window
     manager, cannot show.
   - On a desktop: `scripts/dx.sh run linux-x86_64`, the home screen driven
     into a game, a pad in game; the Save and Load Game screens listing the
     saves with description and date, saving into a new slot, loading one
     and deleting one, each save's picture there (the original game's
     saves' too), and a hub map keeping its state over a travel out
     and back, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#saving-loading-and-travel); the UI
     background option's three settings under the main menu and the
     credits over black, by [the UI](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-ui); rebinding a
     key in the game's Customize Keys screen (a double click or Enter starts
     it), the Load Game list sorted by date and re-sorted from its headers,
     moving through a list with the keys and a pad's d-pad, and a movement
     key held into a menu and let go there moving nothing when it
     closes, a click on the HUD mid-fight stopping fire but not the
     walk, an InfoLink message pushing the HUD's other parts into
     place as it appears and goes, a selection border in the
     inventory crisp with its pattern repeating rather than smearing,
     Ctrl+Z and Ctrl+Y in a save name, Tab between a screen's control
     groups, the pointer hidden and pinned while binding a key, Look Up Stairs
     tilting the view down UNATCO's stairs and easing level, (with
     positional sound on) a click at a screen's left edge sounding from
     the left, and the vision augmentation at level 1 showing a warm
     NPC through its grid, by [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#lists),
     [the UI](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-ui) and
     [small](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#small);
     walking up to Tech Sergeant Kaplan on Liberty Island, an NPC's
     one-time chatter holding its last line, a conversation partner killed
     mid-line ending it, and -- deep in a Hong Kong game -- Maggie Chow's
     and Max Chen's meetings playing past their comment-label jumps, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#conversations);
     logging in to a computer and its emails, a public computer's
     bulletins, reading a datacube and its kept note, a book's centred
     title and the credits' section breaks, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#what-the-player-reads);
     coronas near and far and behind an NPC, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#coronas); walking through a laser
     tripwire on Liberty Island, and a door's highlight, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#implemented-not-as-the-original); a
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
     it left off (not from the top), Battery Park's underground
     echoing against the open park with the echo gone on stepping back
     out, and (with headphones) a sound off to one side heard in both
     ears, the far one softer, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#sound) (with the
     desktop's audio: [linux-x86_64's](https://github.com/JuggyMcNutty/deusex-launcher/blob/linux-x86_64/ports/linux-x86_64/README.md#audio));
     a shot fired around a corner turning the guards, and a body found
     raising the alarm, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#hearing-the-ai-event-system);
     an NPC walking around a crate or another NPC in its way rather than
     backing off, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#moving-wandering-and-tactical-movement);
     a robot exploding at its death sparing what stands behind a wall, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#implemented-not-as-the-original); one of
     the original game's reference saves played on for a while
     ([`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#saving-loading-and-travel)); a
     cloaked commando, a burning NPC going out and a rat disappearing
     once out of sight, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#the-native-tick-ascriptedpawntick);
     NPCs wandering their bit of Liberty Island and a searching NSF
     stepping around corners, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#moving-wandering-and-tactical-movement);
     steam from a Hell's Kitchen street grate against the original,
     electricity arcing and a weapon's laser sight, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#particles-and-lasers-render-iterators);
     an NPC walking away keeping its shape as detail fades, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#mesh-detail); a lamp's corona coming
     up and going over about a third of a second as a corner hides and
     shows it, by [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#coronas); a
     conversation partner's mouth moving with the speech, an NPC's head
     turning to follow the player and easing back, and blinking, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#head-turns-and-lip-sync-blend-animations);
     an NPC walking from light into shadow, a fire's glow on a face, a
     flickering sconce's wall, a pulsing light throbbing and a triggered
     light going dark with their shadows still there, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#lighting);
     the Dragon's Tooth in hand and on the ground, the flamethrower's
     flame, the riot prod's arcs, the EMP grenade's blast, tear and poison
     gas, the drunk effect, a burning NPC, a laser sight's spot and water,
     by [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#fire-water-and-ice-textures).
     Multiplayer: a game hosted from the Host screen and joined from the
     original on another machine -- its Join LAN screen listing the server
     --, by [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#multiplayer) (the game's own ini
     announces it nowhere: its uplinks lack `DoUplink`).
     The desktop defaults (4x MSAA, VSync on) are chosen by reasoning.
   - On a desktop, the review's (2026-10-02): jumping onto an NPC's head --
     bounced off and the NPC stomped, through its `SupportActor` --, and a
     crate dropped on the floor resting just over it, by
     [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#implemented-not-as-the-original); the laser
     tripwires, Liberty Island's trees and the Dragon's Tooth's glow beside
     the original's, by [`NATIVES.md`](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#lighting).
   - linux-aarch64 on any real device.
2. **Next ports**: a cross-built engine for linux-aarch64 (a sysroot with the
   engine's libraries, as the Smart Pro has); Android (its README lists the
   work, starting with an in-process hand-over).
3. **A fork server on the master servers' lists** (deferred by the owner to
   future network work, 2026-09-27). Joining live servers the owner allowed
   ("feel free to connect to public servers", 2026-09-27), and runs have
   joined empty ones since. A listing is another step: neither engine
   announces a server unless its uplink's `DoUplink` is set, which the
   game's own `DeusEx.ini` does not set -- so the Host screen's game is
   never listed --, and a master then asks the server's query port, which
   this machine's NAT keeps from the internet. The fork's uplink is the
   original's, checked against a master on this machine
   ([multiplayer](https://github.com/JuggyMcNutty/VibeEngine/blob/deusex/vibe/docs/NATIVES.md#multiplayer)).
