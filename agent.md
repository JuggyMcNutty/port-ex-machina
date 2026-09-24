# Port Ex Machina -- where things stand

The session handoff: state, decisions, what is open and what is next. It holds
no facts of its own beyond those; each lives in one doc, and the
[README's table](README.md#documentation) says which. Before working, read
[`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md).

## State (2026-09-24)

- **The repository** is on `main`, public at
  https://github.com/JuggyMcNutty/port-ex-machina. Its history was rewritten
  before publishing (2026-09-22) to drop the game's files and a personal email
  address.
- **The launcher** runs on linux-x86_64 and the Smart Pro. It is deliberately
  verbose for development (open decision 2).
- **The engine** is pinned at `engine-patches/UPSTREAM-BASE.txt` plus the
  patches in `engine-patches/`: upstream's latest when last upgraded
  (2026-09-23). Whether and when to take in newer upstream commits
  (`scripts/engine.sh status`, then `upgrade`) is the owner's call.
- **linux-x86_64**, the base: launcher and engine build natively; the staged
  app ran the engine into the intro level on the development PC.
- **trimui-smartpro**: the game runs; the performance work is in progress
  ([its Performance](ports/trimui-smartpro/README.md#performance)). The device
  has every patch up to 0034 in a build with the profiling hooks, Overclock;
  its owner's settings are Distant AI (characters out of sight think less
  often) on and 853×480 (2026-09-23), which the fight's native rows switch to
  native for the run. It has no battery (its battery
  warnings are off: [its Gotchas](ports/trimui-smartpro/README.md#gotchas)).
- **The game's DLLs** are being reverse-engineered (from 2026-09-24):
  [`docs/re/`](docs/re/README.md) covers every binary, IDA databases get the
  game's class layouts from [`tools/ida/ue1_types.py`](tools/ida/ue1_types.py),
  and `DeusEx.dll`'s is typed. What Surreal lacks of them is
  [`docs/re/natives.md`](docs/re/natives.md), from
  [`tools/natives_audit.py`](tools/natives_audit.py) and five map runs.
- **linux-aarch64**: the launcher cross-builds; never run on a device.
- **android**: planned; [its README](ports/android/README.md) is the plan.
- **x360**: planned; nothing about it is worked out yet.

## Decided

1. **What the project is** (owner, 2026-09-23). A modern, cross-platform
   launcher for Deus Ex (UE1) of our own: the original `DeusEx.exe` was
   reverse-engineered as a starting point, not as a contract to stay faithful
   to. The engine is Surreal Engine as a vendored dependency: pinned, not
   following upstream, and upgraded to a newer upstream only when the owner
   chooses ([`docs/ENGINE.md`](docs/ENGINE.md#how-it-is-kept)). **linux-x86_64 is
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
     `ExpressionValue` per argument.
   - **Per-actor work** around the scripts, `IsEventEnabled` among it.
   - **The audio update's scan** of every actor for an ambient sound.
   - **Actor meshes** (0018–0019 so far): the per-vertex work itself.
   - **Visibility** (0020–0021 so far): still the largest render item.
   - **Lightmap uploads**: re-uploading only the rows a light changed, and each
     surface's lightmap lookup.

   At native resolution the game tick does not move the frame until the GPU's
   time comes down (open decision 3); at 853×480 it does.
3. **Renderers on aarch64** (owner, 2026-09-22): the goal is Vulkan, OpenGL ES
   and software rendering all selectable. Not now: Vulkan is the only one the
   engine has. GLES means porting Surreal's desktop OpenGL 3.2 renderer (the
   Smart Pro's `renderers.ini` then needs only `EngineType=GLES`); Surreal has
   no software renderer at all.

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
     into a game, a pad in game; opening Save Game (`GetConfig`,
     [natives.md](docs/re/natives.md#stops-the-game)). The desktop defaults (4x MSAA, VSync on) are
     chosen by reasoning.
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
5. **What Surreal lacks of the original**: potential work, in no order, for
   the owner to take up: [`docs/re/natives.md`](docs/re/natives.md). Two
   items stop the game. Since patch 0034's fights, an NPC searching in Battery
   Park reaches `ReachablePathnodes`, an iterator the fork lacks: seen in a
   run. The Save Game screen calls the unregistered `GetConfig`: read from the
   code, not yet seen. Next on it: `DeusEx.dll` in full, then `Engine.dll`
   (AI events, movement, render iterators) and the rest
   ([the binaries](docs/re/README.md#the-binaries)).
