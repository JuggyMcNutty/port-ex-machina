# Port Ex Machina -- where things stand

The session handoff: state, decisions, what is open and what is next. It holds
no facts of its own beyond those; each lives in one doc, and the
[README's table](README.md#documentation) says which. Before working, read
[`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md).

## State (2026-09-23)

- **The repository** is on `main`, public at
  https://github.com/JuggyMcNutty/port-ex-machina. Its history was rewritten
  before publishing (2026-09-22) to drop the game's files and a personal email
  address.
- **The launcher** runs on linux-x86_64 and the Smart Pro. It is deliberately
  verbose for development (open decision 2).
- **The engine** is pinned at `engine-patches/UPSTREAM-BASE.txt` (upstream as of
  2026-09-20) plus the patches in `engine-patches/`. Upstream has moved on since;
  whether and when to take it in is the owner's call.
- **linux-x86_64**, the base: launcher and engine build natively; the staged
  app ran the engine into the intro level on the development PC.
- **trimui-smartpro**: the game runs; the performance work is in progress
  ([its Performance](ports/trimui-smartpro/README.md#performance)). The device
  has the current build: every patch, Overclock, Distant AI on, native
  resolution.
- **linux-aarch64**: the launcher cross-builds; never run on a device.
- **android**: planned; [its README](ports/android/README.md) is the plan.

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

   Next, in order, re-measuring after each; what is left in each is in
   [where a frame goes](ports/trimui-smartpro/README.md#where-a-frame-goes).
   Profile on the device (`SAMPLE=1`): the desktop's proportions are not the
   device's.
   - **The script interpreter** (in progress, patches 0012–0017). Further gains
     need its structure changed.
   - **Collision traces** (owner, 2026-09-23: next after the list above; in
     progress, patches 0025–0027).
   - Found 2026-09-23, not yet placed in this order by the owner: the
     **per-actor work** around the scripts, and the audio update's scan of
     every actor for an ambient sound.
   - **Actor meshes** (done so far, 0018–0019): the per-vertex work itself.
   - **Visibility** (in progress, 0020–0021): still the largest render item.
   - **Lightmap uploads**: re-uploading only the rows a light changed, and each
     surface's lightmap lookup.
3. **Renderers on aarch64** (owner, 2026-09-22): the goal is Vulkan, OpenGL ES
   and software rendering all selectable. Not now: Vulkan is the only one the
   engine has. GLES means porting Surreal's desktop OpenGL 3.2 renderer (the
   Smart Pro's `renderers.ini` then needs only `EngineType=GLES`); Surreal has
   no software renderer at all.

## Open decisions

1. **Verify by hand** (owner):
   - On the Smart Pro: that NPCs out of sight still behave (Distant AI); the
     Video tab's Resolution at 960×540 and 853×480 -- the look, and the menu
     pointer's speed; START opens the pause menu on the first press after
     skipping the intro; SELECT opens it too; B/Y/SELECT/START close menus; the
     Customize buttons screen; the retired-layout upgrade being written on
     Play/Quit; CPU mode chosen from the Video tab; stick speeds -- look
     (`Speed=3.75`/`2.25`) and pointer speed are calibrated by reasoning, not by
     feel.
   - On a desktop: `scripts/dx.sh run linux-x86_64`, the home screen driven
     into a game, a pad in game. The desktop defaults (4x MSAA, VSync on) are
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
