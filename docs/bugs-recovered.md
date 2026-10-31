# bugs.md and quality.md, as the commits cite them

Recovered 2026-10-28 from the commit messages of every repository and branch: the lists
themselves were never committed and were lost with the drive. An item listed here was
cited by a commit, so it was handled there (fixed, or settled by the owner's decision the
commit names). A number in **not cited** was open, settled without a commit, or handled by
a commit that did not cite it: that is what has to be rediscovered. A section's last
items may be missing altogether when no commit cites them. Sub-items: `.S` and `.N` as
the commits use them (`E12.S1`, `E11b.N2`), `X2.L` the launcher's side of `X2`.

## Left open by their commits

| Item | What the commits say | Where it stands |
|---|---|---|
| `E10b.15` | left open by DXEngine `244c8aaf` (`FogConsole`, new: meshes in a fog zone shot in both engines) | closed by DXEngine `8b5804a9`: a mesh takes no vertex fog, as Render.dll's |
| `E6.14` | its HitWall half fixed by DXEngine `35a73209`: "its 2-unit hold-off stays open"; `c1d0cab2` (the projectile's frame) cites it again | closed by `c1d0cab2`: the projectile's frame is one held-off move (`TickProjectile`'s `TryMoveHeldOff`) |
| `X2.15` | its engine half fixed by DXEngine `f1c670ad`; no launcher commit cites it | closed: no branch of the launcher asks for a GL error (`git grep glGetError` on each) |
| `E15b.16` | its engine half fixed by DXEngine `c7951ab4`, which leaves `PORT=` not taken; no other commit cites it | closed by DXEngine `51baf70c`: `PORT=`, `MULTIHOME=` and `-PRIMARYNET` taken as the original's net driver takes them |
| `L7.5` | the launcher's half by deusex-launcher `f2c4a588` (main): "the engine's half, which opens INI= as given, is an engine item" | closed by DXEngine `d6f13c4f`: `INI=` and `USERINI=` found in any case, from `System` |

`E15b.N2`, marked open by `e75caa7d`, was closed by `c5392c00` 37 minutes later.

## Kept in the docs

Items moved out of the lists before the loss live in the docs that own their code, which git
kept. They carry no IDs, so they cannot be matched to the numbers below:

- each launcher branch's `docs/LAUNCHER.md`, Known defects: linux-x86_64's, linux-aarch64's
  and trimui-smartpro's, the finder reading no Lutris database, which deusex-launcher
  `67fa2f8` fixed and its picks carried; windows-x86_64's, none;
- DXEngine's `docs/NATIVES.md`, each section's "Differs", each now the owner's choice or an
  item of bugs.md;
- port-ex-machina's `AGENTS.md`, To do.

## bugs.md (574 items cited)

### E1

In DXEngine; its fixes touched `DXEngine/VM`, `tools/dxcap`, `DXEngine/Native`, `DXEngine/Packages`.

**Not cited:** E1.5, E1.7, E1.10, E1.11, E1.15, E1.16

| Item | Cited by | What the commit says |
|---|---|---|
| `E1.1` | DXEngine `a4c8ca40` 2026-10-25 | The VM's typed float-to-int and float-to-byte wrap as the generic path's do, by ftol |
| `E1.2` | DXEngine `2bc4c2d9` 2026-10-25 | A None context's value is its type's zero, as the original's execContext zero-fills the result, and an array element through None is nothing, as its execArrayElement copies nothing |
| `E1.3` | DXEngine `2bc4c2d9` 2026-10-25 | A None context's value is its type's zero, as the original's execContext zero-fills the result, and an array element through None is nothing, as its execArrayElement copies nothing |
| `E1.4` | DXEngine `2bc4c2d9` 2026-10-25 | A None context's value is its type's zero, as the original's execContext zero-fills the result, and an array element through None is nothing, as its execArrayElement copies nothing |
| `E1.6` | DXEngine `301251d4` 2026-10-25 | new's flags outside the script's own are logged and left out, as the original's execNew does |
| `E1.8` | DXEngine `a851fe08` 2026-10-25 | A code reference that did not load is a script error, not a crash |
| `E1.9` | DXEngine `2bc4c2d9` 2026-10-25 | A None context's value is its type's zero, as the original's execContext zero-fills the result, and an array element through None is nothing, as its execArrayElement copies nothing |
| `E1.12` | DXEngine `514fe5f4` 2026-10-25 | A state frame's run ends as the original's ProcessState (Engine.dll 0x103e5b60) ends it: before a statement once the actor is destroyed, and at the fifth state change of the tick |
| `E1.13` | DXEngine `e2acaeef` 2026-10-25 | A state's labels and functions are found through the state it extends, as the original finds them |
| `E1.14` | DXEngine `80bbd996` 2026-10-25 | A switch on a string matches its cases without regard to case, and one on a float by its bits, as the original's execSwitch compares |
| `E1.17` | DXEngine `99b414b3` 2026-10-25 | Every actor iterator takes Actor for a None class and walks the caller's own level, as the original's do |
| `E1.18` | DXEngine `05b0f364` 2026-10-25 | The actor iterators walk the level's list at each step, as the original's do |
| `E1.S3` | DXEngine `99b414b3` 2026-10-25 | Every actor iterator takes Actor for a None class and walks the caller's own level, as the original's do |

### E2

In DXEngine; its fixes touched `DXEngine/Package`, `DXEngine/Packages`, `tools/dxcap`,
`DXEngine/UnrealURL.cpp`.

**Not cited:** E2.3, E2.4, E2.5, E2.8, E2.14, E2.15, E2.16, E2.17, E2.19, E2.20, E2.23, E2.24,
E2.27, E2.28

| Item | Cited by | What the commit says |
|---|---|---|
| `E2.1` | DXEngine `f7252104` 2026-10-25 | An import whose outers never end is refused |
| `E2.2` | DXEngine `63580112` 2026-10-25 | An export whose class, outer or base rings on itself is refused |
| `E2.6` | DXEngine `c0cb95a7` 2026-10-25 | A unique name counts past the package's live objects too, as the original's MakeUniqueObjectName counts past any object of the outer (Core.dll 0x101575b0) |
| `E2.7` | DXEngine `0ac421ee` 2026-10-25 | A name's length that is no name is refused |
| `E2.9` | DXEngine `152fa970` 2026-10-25 | A save slot whose SaveInfo is no package or is cut short is passed over, with a line in the log |
| `E2.10` | DXEngine `674cff13` 2026-10-25 | A map folder the paths name but the disk lacks is passed over |
| `E2.11` | DXEngine `1ccd1c56` 2026-10-25 | A path's map pattern is taken in any case |
| `E2.12` | DXEngine `22a1ee45` 2026-10-25 | A path's folder in another case gives its text too |
| `E2.13` | DXEngine `44e1ec28` 2026-10-25 | The package streams close before a save folder's files are deleted, as the original resets its loaders before it deletes |
| `E2.18` | DXEngine `cb99407e` 2026-10-25 | A class DXEngine saves has its super class for its base |
| `E2.21` | DXEngine `57908cac` 2026-10-25 | An ini's key above its first section header is dropped, as the original's config reader drops it, and a section given twice is written back as one |
| `E2.22` | DXEngine `57908cac` 2026-10-25 | An ini's key above its first section header is dropped, as the original's config reader drops it, and a section given twice is written back as one |
| `E2.25` | DXEngine `06574911` 2026-10-25 | An ini that is not there yet is empty, as the original's config cache takes a missing file, and written whole at its first save |
| `E2.26` | DXEngine `b69bed69` 2026-10-25 | An OutputRate that is no number ending in Hz keeps its default |
| `E2.29` | DXEngine `9b1a7b22` 2026-10-25 | A URL's '/' is a character of a map's name or an option's value |
| `E2.30` | DXEngine `0c83b011` 2026-10-25 | A relative travel that names a server's address goes there |
| `E2.S1` | DXEngine `156c390a` 2026-10-25 | A travel's items are taken whether or not Login spawned a pawn, and found by the name's first 20 characters, as the original's SpawnPlayActor takes them |
| `E2.S2` | DXEngine `156c390a` 2026-10-25 | A travel's items are taken whether or not Login spawned a pawn, and found by the name's first 20 characters, as the original's SpawnPlayActor takes them |
| `E2.S3` | DXEngine `f7dbe220` 2026-10-25 | A travelling actor is spawned at the player's place whatever stands there |
| `E2.S5` | DXEngine `3108c293` 2026-10-25 | The compact index's negation is defined at INT_MIN |
| `E2.S6` | DXEngine `ea26b990` 2026-10-25 | A string's, an array's, a sound's and a song's sizes are held before anything is allocated |
| `E2.S7` | DXEngine `ea9619aa` 2026-10-25 | A dynamic array property's element is read where it is |
| `E2.S8` | DXEngine `6f6027e7` 2026-10-25 | A URL cleared is empty whole |
| `E2.S10` | DXEngine `fc9ebe0d` 2026-10-25 | The package remaps go |
| `E2.S11` | DXEngine `9b1a7b22` 2026-10-25 | A URL's '/' is a character of a map's name or an option's value |

### E3E7

In DXEngine; its fixes touched `DXEngine/Collision`, `DXEngine/Native`, `DXEngine/Packages`,
`tools/dxcap`.

**Not cited:** E3E7.2, E3E7.3, E3E7.4, E3E7.5, E3E7.8, E3E7.12, E3E7.13, E3E7.14, E3E7.15, E3E7.20,
E3E7.22, E3E7.24, E3E7.25

| Item | Cited by | What the commit says |
|---|---|---|
| `E3E7.1` | DXEngine `f7c81b40` 2026-10-26 | DXE_GC_VERIFY checks a pointer against the live set before reading its header |
| `E3E7.6` | DXEngine `dd9d89e3` 2026-10-26 | A CriticalDelete of an actor destroys it and no more, one whose Destroy refuses stays, and the engine's levels, infos, games, save info and root window are spared |
| `E3E7.7` | DXEngine `dd9d89e3` 2026-10-26 | A CriticalDelete of an actor destroys it and no more, one whose Destroy refuses stays, and the engine's levels, infos, games, save info and root window are spared |
| `E3E7.9` | DXEngine `ea91859f` 2026-10-26 | A filed mover polygon's texture is kept for the collector while the polygon is filed |
| `E3E7.10` | DXEngine `d56de643` 2026-10-26 | The collision cells' coordinates are held to the cells an int can name |
| `E3E7.11` | DXEngine `c25bd7dd` 2026-10-25 | The collision queries' stamps start again before their counters overflow |
| `E3E7.16` | DXEngine `5568f929` 2026-10-25 | A line check of the level stops at a filed mover's invisible and portal polygons as at the level's nodes that do not block visibility |
| `E3E7.17` | DXEngine `02eb9916` 2026-10-25 | The TouchingActors iterator walks the actor's Touching slots, as the original's does |
| `E3E7.18` | DXEngine `e0fad36b` 2026-10-26 | No fused multiply-adds anywhere in the engine |
| `E3E7.19` | DXEngine `b91973cf` 2026-10-25 | A map's actors at rest find their base at its start as the original's LoadMap pass (Engine.dll 0x1038c1f0) finds it, by FindBase |
| `E3E7.21` | DXEngine `51f1f271` 2026-10-25 | A mover's encroachment test takes its scale by its size on each axis |
| `E3E7.23` | DXEngine `8f9e049d` 2026-10-25 | ENGINE.md: the collision's claim of moves and traces without casts and heap allocations is dropped |
| `E3E7.26` | DXEngine `ae649269` 2026-10-25 | The box tests take no visibility flag, as the original's box check takes none |
| `E3E7.27` | DXEngine `d3294549` 2026-10-25 | The mixer's walls are the solid-space walk the original's FastLineCheck makes |
| `E3E7.N1` | DXEngine `9e31d40a` 2026-10-26 (+1) | ParabolicTrace moves as the original's MoveActor moves, 2 units short of what it meets |
| `E3E7.S1` | DXEngine `dd9d89e3` 2026-10-26 | A CriticalDelete of an actor destroys it and no more, one whose Destroy refuses stays, and the engine's levels, infos, games, save info and root window are spared |

### E4a

In DXEngine; its fixes touched `DXEngine/Native`, `tools/dxcap`, `DXEngine/Packages`,
`DXEngine/Math`.

**Not cited:** E4a.6, E4a.7, E4a.8, E4a.12, E4a.13, E4a.25

| Item | Cited by | What the commit says |
|---|---|---|
| `E4a.1` | DXEngine `5ea9d1dd` 2026-10-25 | An int divided or multiplied by a float in place converts as the original's _ftol does |
| `E4a.2` | DXEngine `6cd1bee2` 2026-10-25 | INT_MIN divided by -1 is INT_MIN on every port |
| `E4a.3` | DXEngine `eaa6171c` 2026-10-25 | The shifts take their count's low 5 bits |
| `E4a.4` | DXEngine `acf0358f` 2026-10-25 | Object.Caps takes each byte as an unsigned char |
| `E4a.5` | DXEngine `6ce2127b` 2026-10-25 | OrthoRotation is implemented, the original's |
| `E4a.9` | DXEngine `1517142d` 2026-10-25 (+1) | ConfigConsole's object save is the console's own |
| `E4a.10` | DXEngine `9e31d40a` 2026-10-26 (+1) | ParabolicTrace moves as the original's MoveActor moves, 2 units short of what it meets |
| `E4a.11` | DXEngine `9e31d40a` 2026-10-26 (+1) | ParabolicTrace moves as the original's MoveActor moves, 2 units short of what it meets |
| `E4a.14` | DXEngine `99b414b3` 2026-10-25 | Every actor iterator takes Actor for a None class and walks the caller's own level, as the original's do |
| `E4a.15` | DXEngine `540b3cf7` 2026-10-25 | GetPlayerPawn is the viewport's actor or None, as the original's |
| `E4a.16` | DXEngine `8de00232` 2026-10-25 | Actor.GetMeshTexture answers as the original's does (Engine.dll 0x103e17c0): the actor's skin of that number, else the mesh's texture of a number above 0, else the actor's Skin, else the mesh's texture of that number |
| `E4a.17` | DXEngine `ffbfe656` 2026-10-25 | PlayBlendAnim takes its slot as the int the script declares, and its messages are the original's |
| `E4a.18` | DXEngine `507b0508` 2026-10-25 | GetNextSkin walks the .int files' Texture objects as the original's does |
| `E4a.19` | DXEngine `bd65d67f` 2026-10-25 | RemovePawn leaves the removed pawn's own nextPawn, as the original's does |
| `E4a.20` | DXEngine `e30394ac` 2026-10-25 | A travel's PreTravel goes to every actor of the level that has the function, as the original's UGameEngine::SetClientTravel (Engine.dll 0x1038fa10) sends it |
| `E4a.21` | DXEngine `7389ec60` 2026-10-25 | The canvas's StrLen measures under any style, and its text natives with no font log and do nothing, as the original's |
| `E4a.22` | DXEngine `7389ec60` 2026-10-25 | The canvas's StrLen measures under any style, and its text natives with no font log and do nothing, as the original's |
| `E4a.23` | DXEngine `7eb0d870` 2026-10-25 | A dedicated server attaches no decals, as the original's |
| `E4a.24` | DXEngine `0ac3a85b` 2026-10-26 | The TimeDemo, WebRequest and WebResponse natives are gone |
| `E4a.26` | DXEngine `1100489e` 2026-10-25 | A UDP link's SendText and SendBinary answer False when nothing went |

### E4b

In DXEngine; its fixes touched `DXEngine/Packages`, `DXEngine/Render`, `tools/dxcap`.

**Not cited:** E4b.2, E4b.3

| Item | Cited by | What the commit says |
|---|---|---|
| `E4b.1` | DXEngine `0a599ca1` 2026-10-25 | A root window per pawn, the engine's the viewport's own player's, as the original's InitRootWindow (Extension.dll 0x10037e0b) makes it |
| `E4b.4` | DXEngine `09853538` 2026-10-25 | A script's GetGC gives it its own window's state, as the original's does |

### E5a

In DXEngine; its fixes touched `DXEngine/Packages`, `tools/dxcap`, `DXEngine/Package`,
`DXEngine/VM`.

**Not cited:** E5a.2, E5a.5, E5a.6, E5a.11, E5a.21, E5a.26, E5a.29, E5a.30

| Item | Cited by | What the commit says |
|---|---|---|
| `E5a.1` | DXEngine `98092370` 2026-10-25 | A property tag the class cannot take is logged and skipped, as the original's SerializeTaggedProperties skips it |
| `E5a.3` | DXEngine `90c43513` 2026-10-25 | A GotoState from within EndState or BeginState preempts the one under way, as the original's RF_InEndState and RF_StateChanged have it |
| `E5a.4` | DXEngine `e2acaeef` 2026-10-25 | A state's labels and functions are found through the state it extends, as the original finds them |
| `E5a.7` | DXEngine `98092370` 2026-10-25 | A property tag the class cannot take is logged and skipped, as the original's SerializeTaggedProperties skips it |
| `E5a.8` | DXEngine `3ae0a12e` 2026-10-25 | A string stored as UTF-16 loads, read as the engine's Latin-1 |
| `E5a.9` | DXEngine `11a9cd4b` 2026-10-25 | A struct's binary members are every element of a static array, the native and transient ones passed over, as the original's SerializeBin has them |
| `E5a.10` | DXEngine `5b7f567f` 2026-10-25 | A dynamic array's elements are saved as they are loaded |
| `E5a.12` | DXEngine `2c9e9f68` 2026-10-25 | A script's Unicode string constant is written back in 16-bit units |
| `E5a.13` | DXEngine `534002c9` 2026-10-27 (+1) | The VM's bytecode reader takes an expression 1,024 deep, as the loader does |
| `E5a.14` | DXEngine `baaacaba` 2026-10-25 | A class's script text stays unloaded |
| `E5a.15` | DXEngine `9487f437` 2026-10-25 | A class's config values are read and written through the properties' own text, as the original's LoadConfig and SaveConfig do |
| `E5a.16` | DXEngine `9487f437` 2026-10-25 | A class's config values are read and written through the properties' own text, as the original's LoadConfig and SaveConfig do |
| `E5a.17` | DXEngine `9487f437` 2026-10-25 | A class's config values are read and written through the properties' own text, as the original's LoadConfig and SaveConfig do |
| `E5a.18` | DXEngine `9487f437` 2026-10-25 | A class's config values are read and written through the properties' own text, as the original's LoadConfig and SaveConfig do |
| `E5a.19` | DXEngine `9487f437` 2026-10-25 | A class's config values are read and written through the properties' own text, as the original's LoadConfig and SaveConfig do |
| `E5a.20` | DXEngine `9487f437` 2026-10-25 | A class's config values are read and written through the properties' own text, as the original's LoadConfig and SaveConfig do |
| `E5a.22` | DXEngine `5ab115c4` 2026-10-26 | A class's defaults are read through a defaults object of its native class |
| `E5a.23` | DXEngine `ff391c44` 2026-10-25 | A struct's text is read and written as the original's ImportText and ExportTextItem have it |
| `E5a.24` | DXEngine `ff391c44` 2026-10-25 | A struct's text is read and written as the original's ImportText and ExportTextItem have it |
| `E5a.25` | DXEngine `ff391c44` 2026-10-25 | A struct's text is read and written as the original's ImportText and ExportTextItem have it |
| `E5a.27` | DXEngine `12a04e8d` 2026-10-25 | A dynamic array's text is "(a,b,...)", as the original's |
| `E5a.28` | DXEngine `e37ba1fb` 2026-10-25 | An object's text is found as the original's ImportText finds it |
| `E5a.31` | DXEngine `4c71c610` 2026-10-25 | The audio device's Latency is held to 10 to 250 ms, as Galaxy holds it |
| `E5a.32` | DXEngine `77cdf181` 2026-10-26 (+1) | NATIVES.md says the render device's Translucency is read and changes nothing |
| `E5a.33` | DXEngine `aac20a93` 2026-10-26 | The mix queue is Latency deep and UseStereo off mixes mono |
| `E5a.N1` | DXEngine `68b13a12` 2026-10-25 (+1) | A name set from text is the text's first token, as the original's import reads it |

### E5b

In DXEngine; its fixes touched `DXEngine/Packages`, `tools/dxcap`, `DXEngine/Render`,
`DXEngine/Native`.

**Not cited:** E5b.1, E5b.13, E5b.16, E5b.17, E5b.20, E5b.21, E5b.22, E5b.23, E5b.26

| Item | Cited by | What the commit says |
|---|---|---|
| `E5b.2` | DXEngine `408a3fb5` 2026-10-25 | An actor's region takes the walked model's zone actors and the LevelInfo given, as the original's PointRegion does |
| `E5b.3` | DXEngine `afd78000` 2026-10-25 | A point on a node's plane is in front of it for FindRegion as for FindLeafAt, as the original's PointRegion takes it (Engine.dll 0x103f5db0: the front when dot − W is 0 or more, the walk ending at a side with no child) |
| `E5b.4` | DXEngine `f2fb3969` 2026-10-25 | A model's indexes are checked as it loads |
| `E5b.5` | DXEngine `8afdf720` 2026-10-25 | A sequence name the mesh lacks is none, as the original's GetAnimSeq answers |
| `E5b.6` | DXEngine `246314ad` 2026-10-25 | A mesh's smoothed normals stay within its vertices |
| `E5b.7` | DXEngine `259f72c0` 2026-10-25 | A mesh is saved in the packing Load reads |
| `E5b.8` | DXEngine `9859ad62` 2026-10-26 | Unreal Tournament's skeletal meshes are gone |
| `E5b.9` | DXEngine `514402b4` 2026-10-25 | An animation's Save adds no moves |
| `E5b.10` | DXEngine `a56dc4ec` 2026-10-25 | ScriptedTexture.ReplaceTexture is implemented, the original's |
| `E5b.11` | DXEngine `47ab1728` 2026-10-25 | A scripted texture draws within its source and takes a script's floats as they come |
| `E5b.12` | DXEngine `0bc09462` 2026-10-25 | A font's glyph is found by the character's byte, whatever char's sign |
| `E5b.14` | DXEngine `a4c591f6` 2026-10-25 | A palette has 256 colours whatever the package says |
| `E5b.15` | DXEngine `15ee04d2` 2026-10-25 | PackageFormat.md: a palette's colour bytes are red, green, blue, alpha, red the low byte |
| `E5b.18` | DXEngine `103181d6` 2026-10-25; the water texture's drops by `f0c13a16` | A fire texture's sparks and drops stay within it |
| `E5b.19` | DXEngine `aea1187d` 2026-10-25 | A mip's data is held to its size |
| `E5b.24` | DXEngine `59a2120f` 2026-10-25 | The AI event ring resumes at the right receiver after a deletion, and a new receiver's first turn comes after every other's |
| `E5b.25` | DXEngine `4d006597` 2026-10-25 | The event manager walks a copy of the senders while scores run |
| `E5b.27` | DXEngine `944f8d65` 2026-10-25 | An inventory item counts as its owner for a pawn's senses alone, as the original's |
| `E5b.28` | DXEngine `6315c769` 2026-10-25 | A texture steps by the time since its own last update, however often it is drawn, as the original's UTexture::Update passes it (fire-dll.md, stepping) |
| `E5b.29` | DXEngine `17065b07` 2026-10-25 | SET takes the client's StartupFullscreen |
| `E5b.N1` | DXEngine `cddb174d` 2026-10-25 (+1) | The first frame is a microsecond, not nothing |
| `E5b.N2` | DXEngine `ba199c53` 2026-10-26 | ScoutConsole counts the level's rolls for flies |
| `E5b.S1` | DXEngine `3aa58123` 2026-10-25 | An animation chain's walk stops at 2,048 textures |

### E6

In DXEngine; its fixes touched `DXEngine/Packages`, `tools/dxcap`, `DXEngine/VM`, `DXEngine/Native`.

**Not cited:** E6.8, E6.9, E6.10, E6.13, E6.15, E6.16, E6.17, E6.18, E6.26, E6.28, E6.30

| Item | Cited by | What the commit says |
|---|---|---|
| `E6.1` | DXEngine `5a67f98c` 2026-10-25 | UActor::Destroy sets what stands on the dying actor free in one pass, as the original's |
| `E6.2` | DXEngine `43e14ad0` 2026-10-25 | A destroyed actor leaves the collision hash and its BSP node once more after bDeleteMe is set |
| `E6.3` | DXEngine `d75cb674` 2026-10-25 | The script's Move is the original's: held off 2 units short of what blocks it, and true when the actor moved at all |
| `E6.4` | DXEngine `80c3ec34` 2026-10-25 | MoveSmooth is the original's moveSmooth |
| `E6.5` | DXEngine `b91973cf` 2026-10-25 | A map's actors at rest find their base at its start as the original's LoadMap pass (Engine.dll 0x1038c1f0) finds it, by FindBase |
| `E6.6` | DXEngine `35a73209` 2026-10-25 | A mover stops what collides with the world, as the original's IsBlockedBy |
| `E6.7` | DXEngine `232eb993` 2026-10-25 | SetLocation's touches are made in its encroachment check, before what stands on the actor is unbased and before it moves, as the original's FarMoveActor makes them (Engine.dll 0x10398ba0, CheckEncroachment 0x1039a350 with its touch flag, wh… |
| `E6.11` | DXEngine `a3863297` 2026-10-25 | A move the level stops sends BumpWall, as the original's MoveActor does |
| `E6.12` | DXEngine `e75caa7d` 2026-10-25 | A fall's step is split where its Z velocity turns over, as the original splits it |
| `E6.14` | DXEngine `c1d0cab2` 2026-10-25 (+1) | A projectile's frame is the original's physProjectile |
| `E6.19` | DXEngine `2d3a3ec9` 2026-10-25 | FindRandomDest passes over a reach spec with no end, as GetPathnodeList and BreadthPathFrom pass theirs |
| `E6.20` | DXEngine `2c202c95` 2026-10-25 | The dead search behind PathSpecialHandling is gone |
| `E6.21` | DXEngine `ff985541` 2026-10-25 | FindRandomDest answers None, as the original's does |
| `E6.22` | DXEngine `328b4319` 2026-10-25 | A jump's landing puts the pawn's feet and head in the zones they land in, as the original's test moves do |
| `E6.23` | DXEngine `e06f779d` 2026-10-25 | Auto-aim's PickTarget and PickAnyTarget are the original's |
| `E6.24` | DXEngine `e06f779d` 2026-10-25 | Auto-aim's PickTarget and PickAnyTarget are the original's |
| `E6.25` | DXEngine `323607f8` 2026-10-25 | ComputePathnodeDistances clears the paths as the original's does |
| `E6.27` | DXEngine `5ce0106f` 2026-10-25 | A pawn's move timer runs as the original's does: set at the speed a strafe sets, at 200 units a second for a pawn with no physics, and counting only while its physics run |
| `E6.29` | DXEngine `48e5553e` 2026-10-25 | A warp zone's Warp and UnWarp turn the rotation as they turn the velocity, roll kept |
| `E6.31` | DXEngine `4784c671` 2026-10-25 | An actor spawned into water keeps its physics, the spawn setting its region alone, as the original's SpawnActor does (Engine.dll 0x10394f60: SetActorZone as a test, with forceRefresh, between BeginPlay and PostBeginPlay, no event, nothing e… |
| `E6.32` | DXEngine `19ac2d46` 2026-10-25 | A touch links each side on its own, as the original's AActor::BeginTouch does (Engine.dll 0x10369570, each side 0x10369630) |
| `E6.33` | DXEngine `ee4122e3` 2026-10-25 | A mesh's sequences are held to its frames as they load, each at least a frame long |
| `E6.34` | DXEngine `3aecf7a0` 2026-10-25 | An animation's tick runs four passes, each as the original's AActor::Tick runs it (Engine.dll 0x103a1aa0) |
| `E6.35` | DXEngine `f426398d` 2026-10-25 | FinishAnim waits as the original's execFinishAnim does (Engine.dll 0x103dff60): not at all with no sequence, with nothing playing or tweening, or with the frame at AnimLast or past, else until the animation finishes, as its poll (0x103e00a0… |
| `E6.36` | DXEngine `975e4c5f` 2026-10-25 | A tick's chain of falls and swims shares the fall's eight steps, as the original's hand-overs carry the count |
| `E6.37` | DXEngine `886c95a2` 2026-10-25 (+1) | Every physics runs once a frame as the original's performPhysics runs it, and the path, the turn and the trailer are the original's |
| `E6.N1` | DXEngine `4c6b8a7b` 2026-10-25 | PlayAnim, LoopAnim and TweenAnim start an animation as the original's do (Engine.dll execPlayAnim 0x103e0160, execLoopAnim 0x103e05e0, execTweenAnim 0x103e0bd0) |

### E8

In DXEngine; its fixes touched `DXEngine/Network`, `DXEngine/Packages`, `tools/dxcap`,
`DXEngine/Engine.cpp`.

**Not cited:** E8.1, E8.5, E8.7, E8.11, E8.18

| Item | Cited by | What the commit says |
|---|---|---|
| `E8.2` | DXEngine `e3d9765c` 2026-10-26 | A first actor bunch that opens nothing closes the connection, and a spawn that fails closes its channel |
| `E8.3` | DXEngine `83b11738` 2026-10-26 (+1) | The download's progress line takes the 64-bit size cast to what its format takes |
| `E8.4` | DXEngine `648a3340` 2026-10-26 | A destroyed channel leaves no record for the next bunch to merge into |
| `E8.6` | DXEngine `215e40e8` 2026-10-26 | A player pawn with a live net connection has the connection closed instead of being destroyed, as the original's DestroyActor has it |
| `E8.8` | DXEngine `1d3d714c` 2026-10-26 | A client opens four downloads at once at most |
| `E8.9` | DXEngine `c7951ab4` 2026-10-26 | A server binds its URL's port or the next free of the 20 from it, and the socket is set up as the original's |
| `E8.10` | DXEngine `0134d8cc` 2026-10-26 | A join's host name is looked up on a thread of its own |
| `E8.12` | DXEngine `71b4460c` 2026-10-26 | A client's reference to a map actor destroyed and freed is refused |
| `E8.13` | DXEngine `fb9551da` 2026-10-26 | A replicated enum's bits are held to its byte |
| `E8.14` | DXEngine `2520d994` 2026-10-26 | A replicated coordinate that is no number, or past an int, is held |
| `E8.15` | DXEngine `127e4421` 2026-10-25 | RstConsole: a TCP link whose peer resets the connection |
| `E8.16` | DXEngine `6cee1942` 2026-10-26 | A TcpLink's failed connect waits on Linux as on Windows, nothing is read while connecting, and a listening link's close lets its port go |
| `E8.17` | DXEngine `6cee1942` 2026-10-26 | A TcpLink's failed connect waits on Linux as on Windows, nothing is read while connecting, and a listening link's close lets its port go |
| `E8.19` | DXEngine `8a601779` 2026-10-25 | A link's sockets close with its actor, as the original's AInternetLink::Destroy closes them |
| `E8.20` | DXEngine `1ac933d7` 2026-10-26 | NATIVES.md: InternetLink.GetLocalIP differs |

### E9a

In DXEngine; its fixes touched `DXEngine/Packages`, `tools/dxcap`, `DXEngine/Render`,
`DXEngine/Engine.cpp`.

**Not cited:** E9a.5, E9a.11, E9a.14

| Item | Cited by | What the commit says |
|---|---|---|
| `E9a.1` | DXEngine `07e5eb1c` 2026-10-25 | Window.IsVisible and IsSensitive recurse by default, up through the parents, as the original's execIsVisible and execIsSensitive (Extension.dll 0x10051a10, 0x10051bd0) answer |
| `E9a.2` | DXEngine `54c2c398` 2026-10-25 | A window made insensitive loses the focus and a grab, and SensitivityChanged reaches its sensitive descendants, as the original's SetSensitivity has it (Extension.dll 0x1004c890, InvokeSensitivityChange 0x10050ef0) |
| `E9a.3` | DXEngine `c149e0f1` 2026-10-25 | A window's size set lays it out again as the original's does |
| `E9a.4` | DXEngine `802c7dc3` 2026-10-25 | NewChild answers None for no class and for a window that destroyed itself in its InitWindow, as the original's CreateNewWindow does (Extension.dll 0x1004df70) |
| `E9a.6` | DXEngine `d9fe7754` 2026-10-25 | A style or visibility change reaches every child as it was before the event |
| `E9a.7` | DXEngine `417340e6` 2026-10-25 | DestroyAllChildren takes each child in turn, as the original's KillAllChildren does (Extension.dll 0x100505d0) |
| `E9a.8` | DXEngine `e39fbf03` 2026-10-25 | Window.CarriageReturn is "\n", as the original's |
| `E9a.9` | DXEngine `d200ca1f` 2026-10-25 | On a screen narrower than 4:3 the root window is the screen's width, as the original's root is the viewport |
| `E9a.10` | DXEngine `c37a0a41` 2026-10-25 | A window asking for a layout asks its parent with itself, and a parent that does not take it asks its own parent in its own name, as the original's AskParentForReconfigure does (Extension.dll 0x1004e060) |
| `E9a.12` | DXEngine `2e4b2ab0` 2026-10-25 | GC.DrawBox draws four strips of its pattern that do not overlap, as the original's XGC::DrawBox does (Extension.dll 0x10028c50) |
| `E9a.13` | DXEngine `e255378f` 2026-10-25 | A BorderWindow hands its nine textures to DrawBorders in the drawer's order |
| `E9a.15` | DXEngine `e255378f` 2026-10-25 | A BorderWindow hands its nine textures to DrawBorders in the drawer's order |
| `E9a.16` | DXEngine `b8daa13f` 2026-10-25 | The pointer is the one CursorRequested gives, with its shadow, hot spot and colour, as the original's QueryCursor (Extension.dll 0x1004f910) chooses it |
| `E9a.17` | DXEngine `b20b17a1` 2026-10-26 | The UI's pointer moves as the original's root moves it |
| `E9a.N1` | DXEngine `baa22afc` 2026-10-26 (+1) | A window that does not show is only counted as placed |

### E9b

In DXEngine, dx-reverse-info; its fixes touched `DXEngine/Packages`, `tools/dxcap`.

**Not cited:** E9b.5, E9b.7, E9b.9, E9b.11, E9b.12

| Item | Cited by | What the commit says |
|---|---|---|
| `E9b.1` | DXEngine `07e711cf` 2026-10-25 | A list window draws as the original's XListWindow::Draw does (Extension.dll 0x10033350) |
| `E9b.2` | DXEngine `07e711cf` 2026-10-25 | A list window draws as the original's XListWindow::Draw does (Extension.dll 0x10033350) |
| `E9b.3` | DXEngine `07e711cf` 2026-10-25 | A list window draws as the original's XListWindow::Draw does (Extension.dll 0x10033350) |
| `E9b.4` | DXEngine `07e711cf` 2026-10-25 | A list window draws as the original's XListWindow::Draw does (Extension.dll 0x10033350) |
| `E9b.6` | DXEngine `e7918a94` 2026-10-26 | A list window draws its columns' fonts without one of its own and sends the script's DrawWindow |
| `E9b.8` | DXEngine `51288b1f` 2026-10-26 (+1) | A list's selected text colour is the script's inverseColor, its colours the original's defaults |
| `E9b.10` | DXEngine `972afc8a` 2026-10-26 (+1) | An edit field's copy and cut take the selection alone, and a paste reads the clipboard as Latin-1 |
| `E9b.13` | DXEngine `ea8dffc8` 2026-10-26 | TextWindow.GetTextPart sums its end in 64 bits |
| `E9b.14` | DXEngine `2633e213` 2026-10-26 | A large text window sets the script's lineHeight |
| `E9b.15` | DXEngine `15983d00` 2026-10-26 (+1) | A text log window starts left- and top-aligned with a 5 s timeout |
| `E9b.16` | DXEngine `8fcb89cc` 2026-10-26 | A button window writes the script's view of its look |
| `E9b.17` | DXEngine `6bc92f63` 2026-10-26 | A computer window's fading character holds its fade to 0..1 |
| `E9b.18` | DXEngine `b905484d` 2026-10-26 | A flag iterator keeps the flag last returned |

### E9c

In DXEngine; its fixes touched `DXEngine/Packages`, `tools/dxcap`, `DXEngine/Package`,
`DXEngine/Native`.

**Not cited:** E9c.2, E9c.3, E9c.4, E9c.5, E9c.6, E9c.8, E9c.9, E9c.12, E9c.14

| Item | Cited by | What the commit says |
|---|---|---|
| `E9c.1` | DXEngine `0fc8c36f` 2026-10-25 | A conversation line's length is read from the sound's header, as the original's DConversation::GetSoundLength (ConSys.dll 0x10010460) reads it |
| `E9c.7` | DXEngine `df47f9d5` 2026-10-26 | DeusExTextParser.OpenText with a None name opens no text |
| `E9c.10` | DXEngine `28113326` 2026-10-26 | The player's log, history, note and location objects are each named apart |
| `E9c.11` | DXEngine `6b702666` 2026-10-26 | DeusExPlayer.UnloadTexture's difference is said |
| `E9c.13` | DXEngine `1781fb93` 2026-10-26 | The Load Map window lists the map files of every map folder |
| `E9c.15` | DXEngine `e0242aa4` 2026-10-26 | GameDirectory's save file names are the original's |
| `E9c.16` | DXEngine `208dc1bb` 2026-10-26 | GameDirectory.DeleteSaveInfo takes None |

### E10a

In DXEngine; its fixes touched `DXEngine/RenderDevice`, `DXEngine/Render`, `DXEngine/Packages`,
`tools/dxcap`.

**Not cited:** E10a.1, E10a.2, E10a.5, E10a.8, E10a.10, E10a.11, E10a.19, E10a.20, E10a.21, E10a.23,
E10a.27, E10a.40, E10a.41, E10a.42

| Item | Cited by | What the commit says |
|---|---|---|
| `E10a.3` | DXEngine `bfbe959c` 2026-10-26 | A map whose nodes point outside their surfaces, bounds, vertices or points, or whose collision hulls run outside their list or name a node past the nodes, is refused at the load |
| `E10a.4` | DXEngine `e2a71b21` 2026-10-26 | A warp zone with no other side opens no frame, its Generate called first |
| `E10a.6` | DXEngine `1860acf9` 2026-10-26 | The translucent sort has an order with a NaN key |
| `E10a.7` | DXEngine `9eba7e78` 2026-10-25 | A sky's and a warp zone's frames open three deep at most, as a mirror's, as the original's OccludeBsp opens them (Render.dll 0x10b173d0, Recursion < 3) |
| `E10a.9` | DXEngine `050b93ba` 2026-10-25 | The occlusion grid follows the image up to a 4K frame |
| `E10a.12` | DXEngine `ca0fd86a` 2026-10-25 | A detail level's texture takes a cache ID no address or tag can make |
| `E10a.13` | DXEngine `7627ddf7` 2026-10-25 | A realtime texture's change reaches every draw of it in the frame |
| `E10a.14` | DXEngine `4fef5639` 2026-10-26 | A mesh animation's weight between frames is taken before the frame is wrapped into the sequence, and a sequence of no frames draws its first |
| `E10a.15` | DXEngine `4fef5639` 2026-10-26 | A mesh animation's weight between frames is taken before the frame is wrapped into the sequence, and a sequence of no frames draws its first |
| `E10a.16` | DXEngine `35389be9` 2026-10-26 | The vertex budget is held to the mesh's vertices as a float before its conversions |
| `E10a.17` | DXEngine `a9ba422e` 2026-10-26 | A LOD mesh's own indexes are held to its arrays in the draw |
| `E10a.18` | DXEngine `fcb65300` 2026-10-26 | A blend's vertex reads are held to the mesh's vertices in the plain mesh draw too |
| `E10a.22` | DXEngine `9859ad62` 2026-10-26 | Unreal Tournament's skeletal meshes are gone |
| `E10a.24` | DXEngine `9fef1ee6` 2026-10-26 | Canvas.DrawActor and Canvas.DrawClippedActor leave their actor hidden, as the original's do; DrawClippedActor draws from an eye of its own; the unread WireFrame goes |
| `E10a.25` | DXEngine `9fef1ee6` 2026-10-26 | Canvas.DrawActor and Canvas.DrawClippedActor leave their actor hidden, as the original's do; DrawClippedActor draws from an eye of its own; the unread WireFrame goes |
| `E10a.26` | DXEngine `9fef1ee6` 2026-10-26 | Canvas.DrawActor and Canvas.DrawClippedActor leave their actor hidden, as the original's do; DrawClippedActor draws from an eye of its own; the unread WireFrame goes |
| `E10a.28` | DXEngine `0bc09462` 2026-10-25 | A font's glyph is found by the character's byte, whatever char's sign |
| `E10a.29` | DXEngine `ce4f3c73` 2026-10-26 | A clipped text's glyphs are drawn from their own pages, under their letters when centred, and a missing one draws nothing |
| `E10a.30` | DXEngine `ce4f3c73` 2026-10-26 | A clipped text's glyphs are drawn from their own pages, under their letters when centred, and a missing one draws nothing |
| `E10a.31` | DXEngine `ce4f3c73` 2026-10-26 | A clipped text's glyphs are drawn from their own pages, under their letters when centred, and a missing one draws nothing |
| `E10a.32` | DXEngine `6a898d73` 2026-10-26 | The collision debug that drew nothing goes |
| `E10a.33` | DXEngine `f489c7c7` 2026-10-26 | A corona's skin is sent to the device as every texture is |
| `E10a.34` | DXEngine `01f4f0d9` 2026-10-26 | A viewport window of no size draws nothing, and a frame is a pixel at the least and 16,384 at the most |
| `E10a.35` | DXEngine `ef7d609c` 2026-10-26 | A bHighDetail actor is passed over while the render device's HighDetailActors is off, as the original's SetupDynamics passes it |
| `E10a.36` | DXEngine `62a0c6da` 2026-10-26 | A mover polygon's macro texture is read from the texture that stands in |
| `E10a.37` | DXEngine `aaaaeb2b` 2026-10-26 | A mover whose brush has no polygons is passed over by the draw |
| `E10a.38` | DXEngine `993817a3` 2026-10-25 | A sprite is drawn as the viewer's Sprite texture when it has one, the Matrix cheat's, at the actor's texture's size, as the original's DrawActorSprite draws it |
| `E10a.39` | DXEngine `1c3fa947` 2026-10-26 | A sprite animated once holds its last frame, as the original's does |
| `E10a.43` | DXEngine `53c82f09` 2026-10-26 (+2) | Contrast, saturation, the gamma's offsets and the textures' LOD bias are Settings.json's RenderDevice keys |
| `E10a.44` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |
| `E10a.N1` | DXEngine `f2c60a56` 2026-10-25 | The corona pass runs only with the render device's Coronas on, as the original's DrawFrame tests it before its corona pass (Render.dll 0x10b1a2d0), and SET and GET reach a device by its bare class name |
| `E10a.N2` | DXEngine `af356b43` 2026-10-25 (+1) | An Array of an over-aligned type takes storage at its alignment |
| `E10a.S1` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |

### E10b

In DXEngine; its fixes touched `DXEngine/Light`, `tools/dxcap`, `DXEngine/Packages`.

**Not cited:** E10b.2, E10b.5, E10b.8, E10b.9, E10b.12, E10b.13, E10b.18, E10b.19, E10b.20, E10b.23,
E10b.24, E10b.25, E10b.26, E10b.27

| Item | Cited by | What the commit says |
|---|---|---|
| `E10b.1` | DXEngine `8cf5da9c` 2026-10-26 | The light maps' effects table is Deus Ex's shapes alone |
| `E10b.3` | DXEngine `b83b26f8` 2026-10-26 | The light maps' SSE2 lanes divide exactly |
| `E10b.4` | DXEngine `c75839a5` 2026-10-25 | A texel at a light's own point faces it full on |
| `E10b.6` | DXEngine `d06d5415` 2026-10-26 | A mesh's vertices are lit by the original's formula alone |
| `E10b.7` | DXEngine `496fd50d` 2026-10-25 | The light tree's nodes go with its lights when a tree light dies |
| `E10b.10` | DXEngine `ae912643` 2026-10-25 | A light that leaves a surface's moving lights, and a spotlight that turns, light their surfaces anew, as the original's dynamic map, built every frame from the lights then over the surface, has them (Render.dll DrawFrame's dynamic map; rend… |
| `E10b.11` | DXEngine `ae912643` 2026-10-25 | A light that leaves a surface's moving lights, and a spotlight that turns, light their surfaces anew, as the original's dynamic map, built every frame from the lights then over the surface, has them (Render.dll DrawFrame's dynamic map; rend… |
| `E10b.14` | DXEngine `2fe209a1` 2026-10-25 | The light map check counter starts again before it overflows |
| `E10b.15` | DXEngine `244c8aaf` 2026-10-25 | FogConsole, new: meshes in a fog zone shot in both engines |
| `E10b.16` | DXEngine `54a1a8b8` 2026-10-26 | A level model's listed lights are kept, destroyed or not |
| `E10b.17` | DXEngine `84ae5127` 2026-10-26 | A mover's raytrace key is held to the 8 |
| `E10b.21` | DXEngine `b60beaa7` 2026-10-26 | The palette loop's phase says why it changes nothing: the original's |
| `E10b.22` | DXEngine `59fb4c00` 2026-10-26 | A light map's scale is held and its product taken in 64 bits |
| `E10b.28` | DXEngine `3f9fa273` 2026-10-26 | Shadow bits past the stored ones cast no shadow |

### E11a

In DXEngine; its fixes touched `DXEngine/RenderDevice`, `DXGPU/src`, `DXEngine/Render`,
`DXGPU/include`.

**Not cited:** E11a.7, E11a.8, E11a.9, E11a.10, E11a.12, E11a.13, E11a.14, E11a.15, E11a.16,
E11a.17, E11a.18, E11a.19, E11a.20, E11a.22, E11a.24, E11a.26, E11a.27, E11a.30

| Item | Cited by | What the commit says |
|---|---|---|
| `E11a.1` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |
| `E11a.2` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |
| `E11a.3` | DXEngine `f9b33e1d` 2026-10-26 | The Vulkan scene's push constants fit 128 bytes |
| `E11a.4` | DXEngine `53c82f09` 2026-10-26 | Contrast, saturation, the gamma's offsets and the textures' LOD bias are Settings.json's RenderDevice keys |
| `E11a.5` | DXEngine `c960e2ca` 2026-10-26 | The present has one colour correction and eight variants |
| `E11a.6` | DXEngine `b923e759` 2026-10-26 | The editor's orthographic drawing is gone from every device |
| `E11a.11` | DXEngine `5cea0425` 2026-10-26 | The Vulkan present is laid out for the swapchain's extent |
| `E11a.21` | DXEngine `0229ce32` 2026-10-26 | A Vulkan surface lost is made again |
| `E11a.23` | DXEngine `22ec436f` 2026-10-26 | D3D11's exclusive fullscreen is gone, with the Vulkan swap chain's exclusive structs |
| `E11a.25` | DXEngine `681ec9a5` 2026-10-25 | The Vulkan instance's layer count starts at 0 and a failed enumeration gives no layers |
| `E11a.28` | DXEngine `3e095840` 2026-10-26 | The Vulkan device filter asks for the features the engine uses |
| `E11a.29` | DXEngine `9a05003d` 2026-10-26 | A Vulkan device without shaderClipDistance clips a mirror's plane by a discard |
| `E11a.31` | DXEngine `9705a7fa` 2026-10-26 | A Vulkan device of type OTHER ranks before the CPU driver |

### E11b

In DXEngine; its fixes touched `DXEngine/RenderDevice`, `DXGPU/src`, `DXEngine/Render`,
`tools/dxcap`.

**Not cited:** E11b.3, E11b.4, E11b.6, E11b.8, E11b.9, E11b.10, E11b.11, E11b.12, E11b.13, E11b.14,
E11b.17, E11b.18, E11b.20, E11b.23, E11b.24, E11b.25

| Item | Cited by | What the commit says |
|---|---|---|
| `E11b.1` | DXEngine `3e2ddf2b` 2026-10-26 | A Vulkan texture's levels are those that fit, zeros where a mip does not, each band aligned to its texel, and a top mip without bytes the white texel |
| `E11b.2` | DXEngine `3e2ddf2b` 2026-10-26 | A Vulkan texture's levels are those that fit, zeros where a mip does not, each band aligned to its texel, and a top mip without bytes the white texel |
| `E11b.5` | DXEngine `3e2ddf2b` 2026-10-26 | A Vulkan texture's levels are those that fit, zeros where a mip does not, each band aligned to its texel, and a top mip without bytes the white texel |
| `E11b.7` | DXEngine `5d77968e` 2026-10-26; D3D11's by `629a32d3` | A P8 texture without a palette draws as its grey levels on the Vulkan and GL devices |
| `E11b.15` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |
| `E11b.16` | DXEngine `05ea3049` 2026-10-26 (+1) | The Vulkan scene buffers are ten bits a channel, A2B10G10R10, as the GL device's RGB10_A2 |
| `E11b.19` | DXEngine `9a05003d` 2026-10-26 | A Vulkan device without shaderClipDistance clips a mirror's plane by a discard |
| `E11b.21` | DXEngine `2de6da23` 2026-10-26 | Bloom works in SDR, from the world alone |
| `E11b.22` | DXEngine `cf9bf114` 2026-10-26 | The SPIR-V target and VMA's version follow the device's Vulkan version, not the instance's alone |
| `E11b.26` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |
| `E11b.N1` | DXEngine `95ce0897` 2026-10-26 | Every Vulkan staging copy starts at a multiple of its texel's size and of 16 |
| `E11b.N2` | DXEngine `e17b614f` 2026-10-26 | A Vulkan texture's rect copies wait for its whole one in the same batch |

### E12

In DXEngine; its fixes touched `DXEngine/RenderDevice`, `DXEngine/Render`, `DXGPU/src`,
`DXWidgets/src`.

**Not cited:** E12.7, E12.11, E12.12, E12.13, E12.14, E12.15, E12.17, E12.18, E12.19, E12.20,
E12.21, E12.25

| Item | Cited by | What the commit says |
|---|---|---|
| `E12.1` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |
| `E12.2` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |
| `E12.3` | DXEngine `29cc2e7f` 2026-10-26 | A frame whose scene buffers' resize failed is skipped, not drawn into the window |
| `E12.4` | DXEngine `49e662ee` 2026-10-26 | The GL scene buffers' set counts as made only when all of it is |
| `E12.5` | DXEngine `23f42818` 2026-10-26 | Every GL framebuffer is checked complete as it is made |
| `E12.6` | DXEngine `4125c055` 2026-10-25 | The SDL windows answer SetGLSwapInterval with SDL's own result |
| `E12.8` | DXEngine `66768570` 2026-10-25 | The GL device takes no HDR output |
| `E12.9` | DXEngine `2de6da23` 2026-10-26 | Bloom works in SDR, from the world alone |
| `E12.10` | DXEngine `acb84235` 2026-10-25 | A gamma-corrected shot's levels are read back by rounding |
| `E12.16` | DXEngine `9ae8175d` 2026-10-26 | The desktop GL driver is asked for S3TC and anisotropic filtering too |
| `E12.22` | DXEngine `f1c670ad` 2026-10-26 | GL errors are asked for once a texture made, not once a level or an upload |
| `E12.23` | DXEngine `3bc847c6` 2026-10-26 | A GL texture level with no fitting mip is zeros, on the plain path as on the compressed |
| `E12.24` | DXEngine `ce48f333` 2026-10-26 | An RGB8 rectangle's source starts at its column times 3 |
| `E12.26` | DXEngine `c85a6e98` 2026-10-26 | The GL uploads' sizes are counted in size_t |
| `E12.S1` | DXEngine `53c82f09` 2026-10-26 (+2) | Contrast, saturation, the gamma's offsets and the textures' LOD bias are Settings.json's RenderDevice keys |

### E13

In DXEngine; its fixes touched `DXEngine/RenderDevice`.

**Not cited:** E13.2, E13.3, E13.8, E13.10

| Item | Cited by | What the commit says |
|---|---|---|
| `E13.1` | DXEngine `4893176e` 2026-10-25 | A texture coordinate that is no number samples texel 0 in the software device, as its float reference does |
| `E13.4` | DXEngine `5018e71d` 2026-10-26 | The software device's frame is kept within its guard band |
| `E13.5` | DXEngine `3317bd57` 2026-10-25 | A line that starts on a pixel centre keeps that pixel |
| `E13.6` | DXEngine `e6e97784` 2026-10-25 | A light map rectangle that is no number names the whole texture |
| `E13.7` | DXEngine `fd05277f` 2026-10-26 | The software device's reference check compares once a frame, at its present |
| `E13.9` | DXEngine `0fc0db43` 2026-10-25 | The software device's pool ends at a thread the system refuses and runs on the threads it got |
| `E13.11` | DXEngine `a03e3c52` 2026-10-25 | A realtime texture's change reaches every copy of it |

### E14

In DXEngine; its fixes touched `DXEngine/RenderDevice`, `DXEngine/Render`, `DXGPU/src`,
`DXGPU/include`.

**Not cited:** E14.5, E14.7, E14.8, E14.14, E14.15, E14.16, E14.17, E14.18, E14.19, E14.20, E14.21

| Item | Cited by | What the commit says |
|---|---|---|
| `E14.1` | DXEngine `d0f88746` 2026-10-26 | A removed D3D11 device is named |
| `E14.2` | DXEngine `4307a6f2` 2026-10-26 | The D3D11 device's last frame is the scene's size |
| `E14.3` | DXEngine `2de6da23` 2026-10-26 | Bloom works in SDR, from the world alone |
| `E14.4` | DXEngine `ed888b20` 2026-10-26 | D3D11's detail fade takes the view depth as SV_Position.w gives it |
| `E14.6` | DXEngine `e8b74da4` 2026-10-26 | D3D11's HDR swap chain asks for scRGB, the linear light its present writes, and the read-back takes the SDR present's gamma |
| `E14.9` | DXEngine `ad06f4df` 2026-10-26 | The D3D11 device takes a shader compiler failure without messages |
| `E14.10` | DXEngine `f56369bc` 2026-10-26 | The D3D11 device asked for the debug layer makes do without it |
| `E14.11` | DXEngine `35c35e51` 2026-10-26 | The D3D11 device checks its scene buffers again after a flush |
| `E14.12` | DXEngine `4e575aeb` 2026-10-26 | The editor's hit test is gone from every device |
| `E14.13` | DXEngine `22ec436f` 2026-10-26 | D3D11's exclusive fullscreen is gone, with the Vulkan swap chain's exclusive structs |
| `E14.22` | DXEngine `9605740f` 2026-10-26 | D3D11's rectangle upload is right for every format |

### E15a

In DXEngine; its fixes touched `DXEngine/Audio`, `DXEngine/Utils`, Thirdparty/resample (gone from the tree),
`DXEngine/Packages`.

**Not cited:** E15a.8, E15a.10, E15a.11, E15a.12, E15a.18, E15a.19, E15a.20, E15a.26, E15a.27,
E15a.29, E15a.30, E15a.32, E15a.37, E15a.41

| Item | Cited by | What the commit says |
|---|---|---|
| `E15a.1` | DXEngine `4afce6d2` 2026-10-25 | The music thread's checks see its own calls' errors, and a stop unqueues what is queued |
| `E15a.2` | DXEngine `4afce6d2` 2026-10-25 | The music thread's checks see its own calls' errors, and a stop unqueues what is queued |
| `E15a.3` | DXEngine `2134e729` 2026-10-25; its fill's halving of the music undone by `e63437c2` | A music source that ends or reads short leaves silence, not the chunk's stale tail |
| `E15a.4` | DXEngine `b6c5f465` 2026-10-25 | A pitch or volume that is no number plays plain, as Galaxy plays on |
| `E15a.5` | DXEngine `4eb94747` 2026-10-25 | The music's order is the jump's at once, and a new song's never the old song's |
| `E15a.6` | DXEngine `a56e753b` 2026-10-25 | The OpenAL context takes the attributes built for it |
| `E15a.7` | DXEngine `069ee8a8` 2026-10-26 | The audio device mixes as Galaxy alone |
| `E15a.9` | DXEngine `aac20a93` 2026-10-26 | The mix queue is Latency deep and UseStereo off mixes mono |
| `E15a.13` | DXEngine `8a8f8c6a` 2026-10-25 | Data with no MP3 frame is refused |
| `E15a.14` | DXEngine `0ba80473` 2026-10-25 | A WAV's sample count is held to its data, and a sound too large refused |
| `E15a.15` | DXEngine `ddcb4668` 2026-10-25 | A WAV's first loop is played whatever its count, as Galaxy plays it |
| `E15a.16` | DXEngine `09af7cd3` 2026-10-26 | ResampleAudioSource is gone, with r8brain, the resampler it alone used |
| `E15a.17` | DXEngine `60505620` 2026-10-26 | The music source's comment states what is read of Galaxy's song start, not a TODO |
| `E15a.21` | DXEngine `250ec3e9` 2026-10-25 | A loop wraps as often as a step longer than it needs |
| `E15a.22` | DXEngine `bae534e0` 2026-10-25 | The mixing rate is held to 8,000 to 48,000 Hz, as Galaxy holds it |
| `E15a.23` | DXEngine `0c463ed0` 2026-10-25 | A reverb whose cutoff is above the output rate keeps the reverb as it is, as the original plays on |
| `E15a.24` | DXEngine `7870d4f6` 2026-10-26 | The reverb's guards for a delay and a gain of 0 say why they never fire |
| `E15a.25` | DXEngine `afbaa010` 2026-10-25 | Array's insert makes the slots past the old size, not assigns into them |
| `E15a.28` | DXEngine `e72fade6` 2026-10-25 | The JSON reader refuses what is no JSON and a failed Settings.json is said |
| `E15a.31` | DXEngine `10b17628` 2026-10-25 | A file's last flush is checked at its close |
| `E15a.33` | DXEngine `cb4e1ba1` 2026-10-25 | A folder made from a path with a trailing separator is made once |
| `E15a.34` | DXEngine `cc1a45c5` 2026-10-25 | The executable's path leaves readlink a byte for the terminator |
| `E15a.35` | DXEngine `62d60f9f` 2026-10-26 | File.cpp's font search and the never-compiled __progname path are gone |
| `E15a.36` | DXEngine `838de93f` 2026-10-25 | The log is written under a lock, the VM's call stack read on the main thread alone |
| `E15a.38` | DXEngine `cec207bb` 2026-10-26 | The Windows stack walk's line lookup is sized, checked and under the lock |
| `E15a.39` | DXEngine `6c537f4d` 2026-10-26 | An engine exception names its native frames on Linux too |
| `E15a.40` | DXEngine `e1a05df7` 2026-10-26 | StrTools' replace and int_to_string are gone |
| `E15a.42` | DXEngine `0ed46246` 2026-10-25 | Text that is no value of a property leaves an int, a byte or a bool as it was and makes a float 0, as the original's import does (core-dll.md, configuration) |
| `E15a.43` | DXEngine `a8c4450f` 2026-10-26 | MemoryStreamWriter is gone |

### E15b

In DXEngine, dx-reverse-info; its fixes touched `DXEngine/Engine.cpp`, `tools/dxcap`,
`DXEngine/Network`, `DXEngine/Engine.h`.

**Not cited:** E15b.13, E15b.15, E15b.18, E15b.21, E15b.24, E15b.25, E15b.26, E15b.29, E15b.31,
E15b.32, E15b.33

| Item | Cited by | What the commit says |
|---|---|---|
| `E15b.1` | DXEngine `e026e128` 2026-10-25 | A key let go with no player writes nowhere |
| `E15b.2` | DXEngine `f410cc0d` 2026-10-25 | A map saved into Current is found again whatever case the URL spells it in, with or without its .dx |
| `E15b.3` | DXEngine `6504f4d6` 2026-10-25 | No PreLogin for the local player, as the original's SpawnPlayActor (Engine.dll 0x10396bd0) runs none |
| `E15b.4` | DXEngine `c82fc45b` 2026-10-25 | An Axis binding's speed= is taken in any case, and 1 without one, as the original's UInput::Exec (Engine.dll 0x10393e10) parses SPEED= |
| `E15b.5` | DXEngine `08008518` 2026-10-25 | An exec function's string parameter takes the rest of the line, as the original gives it |
| `E15b.6` | DXEngine `c9842195` 2026-10-25 | A dedicated server logs no player in on a restart or a travel |
| `E15b.7` | DXEngine `6f5a4b18` 2026-10-25 | The window's and the renderer's commands do nothing on a dedicated server |
| `E15b.8` | DXEngine `4d5e06d3` 2026-10-25 | switchlevel on a client reads no GameInfo |
| `E15b.9` | DXEngine `e61fc031` 2026-10-25 | servertravel takes its URL from after the command word |
| `E15b.10` | DXEngine `c6d546ba` 2026-10-25 | SaveGame takes any slot number, as the original's does (DeusEx.dll 0x1000ef10): -1 the quick save, 0 a new slot, any other Save%04d |
| `E15b.11` | DXEngine `7cee5f24` 2026-10-25 | DELETEGAME removes a numbered slot alone, as the original's DeleteGame (DeusEx.dll 0x10010710) removes Save%04d |
| `E15b.12` | DXEngine `23d4cff7` 2026-10-25 | open travels partial on the last URL, as the original's OPEN does, and a partial travel keeps the player's name |
| `E15b.14` | DXEngine `23d4cff7` 2026-10-25 | open travels partial on the last URL, as the original's OPEN does, and a partial travel keeps the player's name |
| `E15b.16` | DXEngine `c7951ab4` 2026-10-26 | A server binds its URL's port or the next free of the 20 from it, and the socket is set up as the original's |
| `E15b.17` | DXEngine `d4bc3be2` 2026-10-26 | The WindowSystem ini key is gone |
| `E15b.19` | DXEngine `b46d4999` 2026-10-25 | A Button or Axis binding naming a property the pawn lacks is "Bad Button command" or "Bad Axis command", logged and left alone, as the original's UInput::Exec answers it (Engine.dll 0x10393e10), and a Button's input byte is written as a byt… |
| `E15b.20` | DXEngine `17cf7b4b` 2026-10-25 | A key's line reaches the exec functions on a press alone, as the original's input sends it |
| `E15b.22` | DXEngine `aaad9205` 2026-10-25 | EXEC reads a relative file from the game's System folder, as the original does |
| `E15b.23` | DXEngine `524f922a` 2026-10-25 | GETRES and GETCURRENTRES give the window's own size, not the render size |
| `E15b.27` | DXEngine `f7504dc7` 2026-10-26 | A live run exits only when truly back in the menu |
| `E15b.28` | DXEngine `b4ab5db1` 2026-10-25 | No game found is a failure, reported as every other |
| `E15b.30` | DXEngine `a108a76a` 2026-10-25 | Settings.json's absent RenderDevice keys keep the header's defaults |
| `E15b.34` | DXEngine `5de3e4d1` 2026-10-25 | The error window's times pad their milliseconds on the left |
| `E15b.35` | DXEngine `ff2fcb2f` 2026-10-25 | DXEngine.pk3's home folders are tried only where HOME or XDG_DATA_HOME is set |
| `E15b.N1` | DXEngine `f2c60a56` 2026-10-25 | The corona pass runs only with the render device's Coronas on, as the original's DrawFrame tests it before its corona pass (Render.dll 0x10b1a2d0), and SET and GET reach a device by its bare class name |
| `E15b.N2` | DXEngine `c5392c00` 2026-10-25 (+1) | The console ticks as each frame's draw begins, after the level's tick, as the original's does |
| `E15b.S8` | DXEngine `6dd72c81` 2026-10-26 (+1) | GETPING and GETLOSS answer the server connection's stats |
| `E15b.S9` | DXEngine `3637b7a2` 2026-10-26 | The console's showlog says what it does not do |

### E16a

In DXEngine, dx-reverse-info; its fixes touched `DXWidgets/src`, `DXEngine/GameWindow.cpp`,
`tools/dxcap`, `DXWidgets/include`.

**Not cited:** E16a.2, E16a.3, E16a.4, E16a.6, E16a.11, E16a.12, E16a.14, E16a.15, E16a.16, E16a.17,
E16a.20, E16a.21, E16a.22, E16a.24, E16a.25, E16a.26

| Item | Cited by | What the commit says |
|---|---|---|
| `E16a.1` | DXEngine `b50255f1` 2026-10-26 | A failed SDL_Init tries the next window backend |
| `E16a.5` | DXEngine `d42408ca` 2026-10-25 | The SDL windows fail at a GL context they cannot make, as the docs say |
| `E16a.7` | DXEngine `2fa928ca` 2026-10-26 | The window's activation reaches the engine |
| `E16a.8` | DXEngine `d6c9f0ca` 2026-10-26 | The SDL3 window converts its units to DIPs by the display scale over the pixel density |
| `E16a.9` | dx-reverse-info `f4088918` 2026-10-26 | WinDrv: the wheel is one press and release per poll by the movement's sign, whatever its size |
| `E16a.10` | DXEngine `dea1e996` 2026-10-26 | The SDL3 window keeps the relative motion's fractions |
| `E16a.13` | DXEngine `49dc5193` 2026-10-26 | A timeline's key counts in the SDL windows' key state |
| `E16a.18` | DXEngine `2bb98b5e` 2026-10-26 (+1) | The Win32 window takes the system key messages |
| `E16a.19` | DXEngine `a779580c` 2026-10-26 | The Win32 window's icon is drawn the right way up |
| `E16a.23` | DXEngine `df8233b8` 2026-10-26 | An absolute raw mouse moves the view by its difference |
| `E16a.27` | DXEngine `570ac33f` 2026-10-26 | The ini's window size is pixels, scaled to the frame's DIPs |
| `E16a.28` | DXEngine `a72fd124` 2026-10-26 | The Display menu's screen size is pixels, the backends' answer DIPs |
| `E16a.N1` | DXEngine `64a5048e` 2026-10-26 | The full-screen modes are the screen's size in pixels under the X11, Wayland and Win32 windows |
| `E16a.N2` | DXEngine `681c28c6` 2026-10-26 | SDL's quit event closes every window |

### E16b

In DXEngine; its fixes touched `DXWidgets/src`, `tools/dxcap`.

**Not cited:** E16b.4, E16b.9, E16b.16, E16b.17, E16b.18, E16b.34, E16b.35, E16b.36

| Item | Cited by | What the commit says |
|---|---|---|
| `E16b.1` | DXEngine `458a1d44` 2026-10-25 | The X11 and Wayland windows let their EGL context go before destroying it |
| `E16b.2` | DXEngine `20026577` 2026-10-25 | The X11 and Wayland windows refuse a negative swap interval, so the GL device's vsync falls back to 1 |
| `E16b.3` | DXEngine `64ea899a` 2026-10-25 | The X11 window frees its icon's buffers |
| `E16b.5` | DXEngine `509ece57` 2026-10-25 | The X11 window asks a mapped window's full screen of the window manager |
| `E16b.6` | DXEngine `4400cc83` 2026-10-25 | The X11 window asks for no frame of no size |
| `E16b.7` | DXEngine `48c1a0ac` 2026-10-25 | The X11 and Wayland windows turn typed text off in play |
| `E16b.8` | DXEngine `84aa1ab4` 2026-10-26 | The X11 window's pointer capture grabs the pointer, shown |
| `E16b.10` | DXEngine `df31a9f8` 2026-10-25 | The X11 window's client frame is placed from the root |
| `E16b.11` | DXEngine `8372d17b` 2026-10-25 | A paste on the X11 window gives the clipboard's text |
| `E16b.12` | DXEngine `75d80134` 2026-10-25 | The X11 window serves its selection as ICCCM has it |
| `E16b.13` | DXEngine `6e8d87df` 2026-10-25 | The X11 window takes the keypad's digits without Num Lock |
| `E16b.14` | DXEngine `0bfaafc5` 2026-10-25 | The X11 window lets go of every key held as its focus leaves |
| `E16b.15` | DXEngine `10ccaba2` 2026-10-25 | The X11 window destroys its input context |
| `E16b.19` | DXEngine `cf359c33` 2026-10-25 | The X11 window's backbuffer refuses a visual that is not 8-bit BGRA |
| `E16b.20` | DXEngine `8372d17b` 2026-10-25 | A paste on the X11 window gives the clipboard's text |
| `E16b.21` | DXEngine `e313571a` 2026-10-25 | The X11 window's keys go through the input method first |
| `E16b.22` | DXEngine `5ce9b813` 2026-10-25 | The X11 screen's size scales by Xft.dpi, as its windows do |
| `E16b.23` | DXEngine `db93c968` 2026-10-25 | The X11 backend opens the display as it is made, so a missing server falls through to the next backend |
| `E16b.24` | DXEngine `3d3e7570` 2026-10-26 | The Wayland window binds each global at the lower of the advertised version and the one it knows |
| `E16b.25` | DXEngine `cfa58cf3` 2026-10-26 | The Wayland window's wheel branches read their own field |
| `E16b.26` | DXEngine `56cff823` 2026-10-26 | The Wayland window releases the keys held as the keyboard leaves |
| `E16b.27` | DXEngine `b2442606` 2026-10-26 | The Wayland window's key press and repeat take no focus window |
| `E16b.28` | DXEngine `054dfc7d` 2026-10-26 | The Wayland window takes a repeat rate of 0 |
| `E16b.29` | DXEngine `e3e1bc20` 2026-10-26 | The Wayland window's repeat timers start null and its rate and delay with X's defaults |
| `E16b.30` | DXEngine `f8a60839` 2026-10-26 | The Wayland window's loop waits with no timer |
| `E16b.31` | DXEngine `cb815c47` 2026-10-26 | The Wayland window reads the selection when pasted, its own from the text set |
| `E16b.32` | DXEngine `007dc471` 2026-10-26 | The Wayland window binds one output and takes its current mode |
| `E16b.33` | DXEngine `359ae66e` 2026-10-26 | The Wayland window names a key by its first level, the console's key by its place |
| `E16b.37` | DXEngine `bd7d6e63` 2026-10-26 | The Wayland window's frame is in its logical units |
| `E16b.38` | DXEngine `5b799786` 2026-10-26 | The Wayland window's members are destroyed in the protocol's order |
| `E16b.39` | DXEngine `a507e652` 2026-10-26 | The Wayland window's activation waits for its token |
| `E16b.40` | DXEngine `fd79ffbb` 2026-10-26 | The dbus save dialog is the portal's SaveFile |

### E16c

In DXEngine; its fixes touched `DXWidgets/src`, `DXEngine/GameWindow.cpp`, `DXEngine/GameWindow.h`,
`DXEngine/UI`.

**Not cited:** E16c.3, E16c.4, E16c.7, E16c.8, E16c.9, E16c.10, E16c.11, E16c.12, E16c.19, E16c.20,
E16c.21, E16c.24, E16c.25, E16c.26, E16c.27, E16c.30, E16c.32, E16c.34, E16c.36, E16c.37, E16c.38,
E16c.39, E16c.40, E16c.41

| Item | Cited by | What the commit says |
|---|---|---|
| `E16c.1` | DXEngine `db9a8c88` 2026-10-25 | A paint event draws nothing in the game window |
| `E16c.2` | DXEngine `75508976` 2026-10-26 | A widget's parent is read before its handler |
| `E16c.5` | DXEngine `b9ec47e5` 2026-10-26 | A font that cannot be read is left out of its group |
| `E16c.6` | DXEngine `32bd09f5` 2026-10-26 | A character with no glyph and no space glyph is passed over |
| `E16c.13` | DXEngine `c45ccdbc` 2026-10-26 | A plot outside the canvas does nothing |
| `E16c.14` | DXEngine `b2c371fd` 2026-10-26 | A texture of no size draws nothing |
| `E16c.15` | DXEngine `41dfce48` 2026-10-26 | A composite glyph's transform starts as the identity |
| `E16c.16` | DXEngine `98da71ab` 2026-10-26 | A format-4 cmap segment ending at 0xFFFF ends |
| `E16c.17` | DXEngine `cc7f53f3` 2026-10-26 | A TrueType font's counts and indexes are checked |
| `E16c.18` | DXEngine `cd040005` 2026-10-26 | The CFF path's subroutines and operators are checked |
| `E16c.22` | DXEngine `0f20fe0d` 2026-10-25 | The error window shows and copies the engine's text as UTF-8 |
| `E16c.23` | DXEngine `9bbc1ecc` 2026-10-25 | A word wider than its line is broken into pieces that fit |
| `E16c.28` | DXEngine `4e2c6249` 2026-10-26 | A CSS value ending at the '}' with no ';' ends its ruleset |
| `E16c.29` | DXEngine `ffff7fa6` 2026-10-26 | A theme property of another type answers as unset |
| `E16c.31` | DXEngine `df070ad5` 2026-10-26 | A bad url() in the theme leaves its property unset |
| `E16c.33` | DXEngine `9600b4f3` 2026-10-26 | The stylesheet's escapes are read as written |
| `E16c.35` | DXEngine `08fb1e79` 2026-10-26 | An image's size is held to what its allocation holds |
| `E16c.42` | DXEngine `fd79ffbb` 2026-10-26 | The dbus save dialog is the portal's SaveFile |
| `E16c.43` | DXEngine `5dc5131d` 2026-10-26 | The Win32 save dialog keeps its defaults and counts its filters from one |
| `E16c.44` | DXEngine `5dc5131d` 2026-10-26 | The Win32 save dialog keeps its defaults and counts its filters from one |

### E17

In DXEngine; its fixes touched `tools/dxcap.sh`, `tools/dxcap`, `tools/natives_audit.py`,
`CMakeLists.txt`.

**Not cited:** E17.3, E17.4, E17.9, E17.10, E17.11, E17.14, E17.15, E17.18, E17.20, E17.21, E17.23,
E17.24, E17.25, E17.26, E17.27, E17.28, E17.29, E17.30, E17.31, E17.32, E17.33

| Item | Cited by | What the commit says |
|---|---|---|
| `E17.1` | DXEngine `52dd0ed6` 2026-10-26 | The harness's rec_stop kills under errexit |
| `E17.2` | DXEngine `05d1bb53` 2026-10-26 | The harness checks DXCAP_FPS's MangoHud before the run |
| `E17.5` | DXEngine `2929d039` 2026-10-26 | A dxe run refuses stray shots and collects a DXCAP_USERDIR's |
| `E17.6` | DXEngine `2929d039` 2026-10-26 | A dxe run refuses stray shots and collects a DXCAP_USERDIR's |
| `E17.7` | DXEngine `f7e2c660` 2026-10-26 | The harness closes the original at its time limit |
| `E17.8` | DXEngine `cb5439e5` 2026-10-26 | A harness interrupt ends what the script started |
| `E17.12` | DXEngine `72b8ffee` 2026-10-26 | The harness takes DXCAP_ENGINE and DXCAP_PREFIX relative |
| `E17.13` | DXEngine `42917107` 2026-10-26 | The harness's errexit holds inside a command substitution |
| `E17.16` | DXEngine `cc07c29b` 2026-10-26 | A harness run's folder is named with its date |
| `E17.17` | DXEngine `6efd4bcd` 2026-10-26 | A Windows build's engine is waited for as long as the run and a miss is said |
| `E17.19` | DXEngine `3ee9bc62` 2026-10-26 | netrelay.py learns the channel types from both directions' opening bunches |
| `E17.22` | DXEngine `f872b17d` 2026-10-26 | The natives audit counts a body that only throws as nothing |
| `E17.34` | DXEngine `4c482736` 2026-10-26 | The resource zip's file list is an input of its own |
| `E17.S4` | DXEngine `227583fd` 2026-10-26 | grab.py grabs the newest window of the name |

### L1

In DXEngine, deusex-launcher, dx-reverse-info; its fixes touched `src/core/ini.c`,
`tests/test_ini.c`, `src/core/ini.h`, `src/core/common.c`.

**Not cited:** L1.5, L1.6, L1.7, L1.9, L1.11, L1.12, L1.13, L1.14, L1.16, L1.19, L1.20

| Item | Cited by | What the commit says |
|---|---|---|
| `L1.1` | deusex-launcher `8e0b53ec` 2026-10-26 (+3) (linux-x86_64) | A key appended to a section whose last line is the file's unterminated last line goes on a line of its own |
| `L1.2` | deusex-launcher `d61d78ce` 2026-10-26 (+3) (linux-x86_64) | A key given twice reads and is set at its last value, and a section written twice is one section, its blocks read in order |
| `L1.3` | deusex-launcher `c461aeea` 2026-10-26 (+3) (linux-x86_64) | A UTF-8 byte-order mark an editor left at an ini's start is read past, and gone at the next save, which the file is marked for |
| `L1.4` | dx-reverse-info `c4c74ff4` 2026-10-26 (+4) | Core: appLoadFileToString's encodings |
| `L1.8` | DXEngine `82199805` 2026-10-25 | Settings.json's reader passes over a UTF-8 byte order mark, as DXLauncher's does |
| `L1.10` | deusex-launcher `43ccc800` 2026-10-26 (+3) (linux-x86_64) | A Settings.json member given twice is the last given, as the engine's reader (a std::map) keeps it: read there, set there, written once |
| `L1.15` | deusex-launcher `ae7647fa` 2026-10-26 (+3) (linux-x86_64) | A save's text cut at the reader's 511 bytes is cut at a whole character before it is judged UTF-8 (dxl_text_drop_cut) |
| `L1.17` | deusex-launcher `f7e65cb6` 2026-10-26 (+3) (linux-x86_64) | peicon.c and localize.c are gone from the port branch |
| `L1.18` | deusex-launcher `b0fe90c0` 2026-10-26 (+3) (linux-x86_64) | A newer answer's packet supersedes an earlier ask's late answer being gathered |
| `L1.21` | deusex-launcher `1d383163` 2026-10-26 (+3) (linux-x86_64) | A server's name, map, map title, game type, rules and players' names are shown as UTF-8 through dxl_text_utf8, the packet's Latin-1 (a DXEngine server sends its 8-bit text as it is) made UTF-8 and a long name cut at a whole character |
| `L1.22` | deusex-launcher `f7e65cb6` 2026-10-26 (+3) (linux-x86_64) | peicon.c and localize.c are gone from the port branch |
| `L1.23` | deusex-launcher `91d839eb` 2026-10-26 (+3) (linux-x86_64) | Lead bytes F5-F7 are no UTF-8 |

### L2

In deusex-launcher; its fixes touched `src/core/settings.h`, `tests/test_settings.c`,
`src/app/run.c`, `src/core/common.c`.

**Not cited:** L2.3, L2.4, L2.7, L2.8, L2.9, L2.11, L2.12, L2.14, L2.15, L2.17, L2.18, L2.19, L2.21,
L2.22

| Item | Cited by | What the commit says |
|---|---|---|
| `L2.1` | deusex-launcher `7eab860f` 2026-10-26 (+3) (linux-x86_64) | A line's INI= or USERINI=, the original's words, names the ini the launcher reads and seeds too |
| `L2.2` | deusex-launcher `749b708c` 2026-10-26 (+2) (linux-x86_64) | An imported shot keeps the time the game wrote it |
| `L2.5` | deusex-launcher `ec52cc9d` 2026-10-26 (+3) (linux-x86_64) | A bool reads as the engine reads one |
| `L2.6` | deusex-launcher `55419c1f` 2026-10-26 (+3) (linux-x86_64) | settings.h and LAUNCHER.md say what the engine does with a value its converters cannot read |
| `L2.10` | deusex-launcher `11b74664` 2026-10-26 (+3) (linux-x86_64) | Settings.json's Gamepad.Layout is gone from the launcher's table |
| `L2.13` | deusex-launcher `ec52cc9d` 2026-10-26 (+3) (linux-x86_64) | A bool reads as the engine reads one |
| `L2.16` | deusex-launcher `458e8521` 2026-10-26 (+3) (linux-x86_64) | A dry run or a printed command asks whether another launcher holds the lock |
| `L2.20` | deusex-launcher `5b92ec10` 2026-10-26 (+3) (linux-x86_64) | The last-run log's tail checks fseek and ftell |
| `L2.23` | deusex-launcher `befdefd5` 2026-10-26 (+3) (linux-x86_64) | A linked folder is not descended by the directory copy, nor taken for a save folder by the import |
| `L2.24` | deusex-launcher `7fe9ff44` 2026-10-26 (+1) (linux-x86_64) | The PowerVR anti-aliasing lock reads the probe's GL ES renderer too |

### L3

In deusex-launcher; its fixes touched `src/platform/posix`, `src/app/run.c`, `tests/stub_engine.c`,
`tests/test_process.c`.

**Not cited:** L3.1, L3.2, L3.4, L3.5, L3.6, L3.7, L3.10, L3.12, L3.13, L3.14, L3.15, L3.16, L3.18,
L3.19, L3.20, L3.21, L3.24, L3.25, L3.26, L3.29, L3.33, L3.35, L3.37, L3.38

| Item | Cited by | What the commit says |
|---|---|---|
| `L3.3` | deusex-launcher `de69f60b` 2026-10-26 (+3) (linux-x86_64) | The engine's path is made absolute from where the launcher started |
| `L3.8` | deusex-launcher `7ae82b4d` 2026-10-26 (+3) (linux-x86_64) | The log's command line masks the whole of a server's password, to its word's end |
| `L3.9` | deusex-launcher `e4d96f82` 2026-10-26 (+3) (linux-x86_64) | dxl-cli --import whose launcher.ini save fails exits so |
| `L3.11` | deusex-launcher `0c7b2683` 2026-10-26 (+3) (linux-x86_64) | A start beside a launcher that runs appends to launcher.log |
| `L3.17` | deusex-launcher `39adb51b` 2026-10-26 (+3) (linux-x86_64) | The game's RELAUNCH line plays at once, as the original's relaunched DeusEx.exe does |
| `L3.22` | deusex-launcher `3583c9d7` 2026-10-26 (+3) (linux-x86_64) | A line from the engine arrives whole, of any length, malloc'd to the caller |
| `L3.23` | deusex-launcher `9b60ee0f` 2026-10-26 (+2) (linux-x86_64) | The game's environment is made before the fork and handed to execve |
| `L3.27` | deusex-launcher `c85a0296` 2026-10-26 (+1) (linux-x86_64) | A missing xdg-open is reported |
| `L3.28` | deusex-launcher `2e9d9eb4` 2026-10-26 (+2) (linux-x86_64) | A helper named bare is found on PATH before the fork, as a shell finds it, and one that is nowhere is said so |
| `L3.30` | deusex-launcher `fe1ac7ab` 2026-10-26 (+1) (linux-x86_64) | The GPU probe reads llvmpipe's answer after the GPU drivers', in the time left |
| `L3.31` | deusex-launcher `a572db6f` 2026-10-26 (+2) (linux-x86_64) | The lock file is opened with O_NOFOLLOW |
| `L3.32` | deusex-launcher `41cfc048` 2026-10-26 (+2) (linux-x86_64) | The hand-over channel is a socket file in the user's runtime folder (XDG_RUNTIME_DIR, else one made 0700 under /tmp and checked to be the user's), removed at release |
| `L3.34` | deusex-launcher `a9cf9b09` 2026-10-26 (+2) (linux-x86_64) | A linked file is written through, its target replaced and the link kept |
| `L3.36` | deusex-launcher `d7b2be4e` 2026-10-26 (+1) (linux-x86_64) | A link is trashed, not its target |
| `L3.39` | deusex-launcher `08f9e435` 2026-10-26 (+3) (linux-x86_64) | The stub engine can leave a child holding the line, and takes SIGHUP as the engine does |
| `L3.S9` | deusex-launcher `d5459f1b` 2026-10-26 (+3) (linux-x86_64) | The profile's EngineArgs and DXL_ENGINE_ARGS are joined in a growing buffer |

### L4a

In deusex-launcher; its fixes touched `src/ui/text.c`, `tests/test_ui_text.c`, `src/ui/kit.c`,
`src/ui/widgets.c`.

**Not cited:** L4a.6, L4a.7, L4a.9, L4a.10, L4a.11, L4a.12

| Item | Cited by | What the commit says |
|---|---|---|
| `L4a.1` | deusex-launcher `701ba926` 2026-10-26 (+3) (linux-x86_64) | The typewriter draws nothing where its reveal rect misses the clip |
| `L4a.2` | deusex-launcher `4096ddbb` 2026-10-26 (+3) (linux-x86_64) | The text cache's copy is dxl_xstrdup's |
| `L4a.3` | deusex-launcher `939de143` 2026-10-26 (+3) (linux-x86_64) | dxl_text_fit and dxl_text_fit_tail measure the whole string and cut into the buffer |
| `L4a.4` | deusex-launcher `c8b8e6f4` 2026-10-26 (+3) (linux-x86_64) | A font missing after the first leaves none open and the kit saying so |
| `L4a.5` | deusex-launcher `78783a83` 2026-10-26 (+3) (linux-x86_64) | A stick held into a capture repeats no further |
| `L4a.8` | deusex-launcher `d61089ce` 2026-10-26 (+3) (linux-x86_64) | A whole-row list keeps a wheel's part of a row for the next frames |
| `L4a.13` | deusex-launcher `fbf1e516` 2026-10-26 (+3) (linux-x86_64) | A UI script's find lets a frame draw the control focused before the next line |

### L4b

In deusex-launcher; its fixes touched `src/screens/shell.h`, `src/screens/firstrun.c`,
`src/screens/settings.c`, `src/screens/shell.c`.

**Not cited:** L4b.1, L4b.2, L4b.4, L4b.5, L4b.6, L4b.8, L4b.9, L4b.10, L4b.11, L4b.12, L4b.13,
L4b.14, L4b.17, L4b.18, L4b.19, L4b.21, L4b.24, L4b.25, L4b.26, L4b.27, L4b.28, L4b.29, L4b.30,
L4b.31, L4b.32

| Item | Cited by | What the commit says |
|---|---|---|
| `L4b.3` | deusex-launcher `d86820f1` 2026-10-26 (+3) (linux-x86_64) | The sounds', themes' and art's cache keys are kept whole |
| `L4b.7` | deusex-launcher `a92504fc` 2026-10-26 (+3) (linux-x86_64) | A second start brings the window back only when it is minimized |
| `L4b.15` | deusex-launcher `7732c888` 2026-10-26 (+3) (linux-x86_64) | Make the profile, pressed before the probe has answered, makes it once it has |
| `L4b.16` | deusex-launcher `d68dbda1` 2026-10-26 (+3) (linux-x86_64) | Your profile's Graphics and Controller rows and the Controller page's Layout and controller rows open their list on activation |
| `L4b.20` | deusex-launcher `2e9a0ac2` 2026-10-26 (+3) (linux-x86_64) | Recovery's Defaults... holds anti-aliasing Off where the GPU multisamples badly |
| `L4b.22` | deusex-launcher `bd353d8e` 2026-10-26 (+1) (linux-x86_64) | Settings' renderer row shows the profile's own renderer where the probe found it does not run here, said so |
| `L4b.23` | deusex-launcher `edbb6ff1` 2026-10-26 (+3) (linux-x86_64) | Back to the defaults on a mod's profile takes the mod's own settings and controls |
| `L4b.33` | deusex-launcher `55b900e6` 2026-10-26 (+3) (linux-x86_64) | Multiplayer looks for servers again after the profile changes |

### L5

In deusex-launcher; its fixes touched `src/platform/win32`, `src/app/run.c`, `src/main.c`,
`src/platform/startup.h`.

**Not cited:** L5.3, L5.7, L5.8, L5.9, L5.11, L5.13, L5.14, L5.15, L5.19

| Item | Cited by | What the commit says |
|---|---|---|
| `L5.1` | deusex-launcher `95bf3478` 2026-10-26 (+4) (linux-x86_64) | test_process proves a 2,000-character environment value reaches the game whole |
| `L5.2` | deusex-launcher `a327b5d0` 2026-10-26 (windows-x86_64) | A line from the engine arrives whole on Windows too, the reader growing its buffer while no read is pending on it |
| `L5.4` | deusex-launcher `70cfe731` 2026-10-26 (windows-x86_64) | 0xC0000409 is named an abort or a stack buffer overrun |
| `L5.5` | deusex-launcher `07e40a73` 2026-10-26 (windows-x86_64) | The home folder is found once and kept |
| `L5.6` | deusex-launcher `488bd637` 2026-10-26 (windows-x86_64) | dxl_fs_real gives the path made full where the volume has no DOS name to give (an ImDisk RAM disk's, some network redirectors') |
| `L5.10` | deusex-launcher `93f9a7ca` 2026-10-26 (windows-x86_64) | A deusex:// link opened by the registered handler is taken whole from the raw command line |
| `L5.12` | deusex-launcher `717da156` 2026-10-26 (+4) (linux-x86_64) | A program says when its run is done (dxl_startup_ended) |
| `L5.16` | deusex-launcher `ce4150fa` 2026-10-26 (windows-x86_64) | The Recycle Bin's delete asks FOF_WANTNUKEWARNING |
| `L5.17` | deusex-launcher `f8615530` 2026-10-26 (+3) (linux-x86_64) | On Windows the engine's DXEngine.pk3 counts beside its exe alone |
| `L5.18` | deusex-launcher `42dfc158` 2026-10-26 (+3) (linux-x86_64) | dxl-cli --print-command prints cmd.exe's lines on Windows |
| `L5.20` | deusex-launcher `f3af8b14` 2026-10-26 (windows-x86_64) | The port README says what check-abi holds the programs to: Windows 10 by their PE headers and Windows' own DLLs by name, not the functions imported, 1903 being the manifest's UTF-8 code page |

### L6

In deusex-launcher; its fixes touched `src/platform/posix`, `src/app/run.c`,
`src/core/settings_table.c`, `ports/trimui-smartpro`.

**Not cited:** L6.3, L6.4, L6.5, L6.6, L6.7, L6.8, L6.9, L6.10, L6.12, L6.13, L6.14, L6.16, L6.17,
L6.18, L6.19

| Item | Cited by | What the commit says |
|---|---|---|
| `L6.1` | deusex-launcher `ca7f9fc2` 2026-10-26 (trimui-smartpro) | A stop that comes during the game hook's before starts no game |
| `L6.2` | deusex-launcher `a18534db` 2026-10-26 (+3) (linux-x86_64) | A helper run for its output is a process group of its own, ended whole at the time limit |
| `L6.11` | deusex-launcher `1d5c805a` 2026-10-26 (trimui-smartpro) | The launcher's renderer row offers OpenGL ES or Software |
| `L6.15` | deusex-launcher `f644d91c` 2026-10-26 (trimui-smartpro) | profile-map.sh ends the run on INT, TERM or HUP |
| `L6.20` | deusex-launcher `61e96dda` 2026-10-26 (linux-aarch64) | The port README lists the shared files the branch changes |

### L7

In deusex-launcher, dx-reverse-info; its fixes touched `src/launch/launch.c`, `src/platform/posix`,
`src/core/ini.c`, `tests/test_ini.c`.

**Not cited:** L7.4, L7.6, L7.7, L7.9, L7.11, L7.12, L7.13, L7.14, L7.15, L7.22, L7.24, L7.30

| Item | Cited by | What the commit says |
|---|---|---|
| `L7.1` | deusex-launcher `b20a9bd6` 2026-10-25 (main) | A key appended after an ini's last, unterminated line gets a line of its own |
| `L7.2` | deusex-launcher `6df119ba` 2026-10-25 (main) | The ini save keeps a linked ini a link and refuses a read-only one |
| `L7.3` | deusex-launcher `ef2d3c04` 2026-10-26 (main) | An ini is saved on a name of its own beside it, and on the disk before it takes the ini's place |
| `L7.5` | deusex-launcher `f2c4a588` 2026-10-25 (main) | INI= and USERINI= are found in any case in every component of their path |
| `L7.8` | deusex-launcher `b7b6d6ee` 2026-10-25 (main) | The crash sentinel's removal takes a Running.ini in another case too |
| `L7.10` | deusex-launcher `db67c868` 2026-10-25 (main) | The save migration takes a .usa's number by strtol, one past an int naming no slot |
| `L7.16` | deusex-launcher `2dcf2efb` 2026-10-26 (main) | The .int files register in NTFS's order, by their upper case |
| `L7.17` | deusex-launcher `f111d5d8` 2026-10-26 (main) | The hand-over channel is a socket file in the user's runtime folder (XDG_RUNTIME_DIR, else one made 0700 under /tmp and checked to be the user's), the lock file beside it, opened with O_NOFOLLOW |
| `L7.18` | deusex-launcher `6d4dcd59` 2026-10-26 (main) | A forwarded command line of the longest length arrives whole |
| `L7.19` | deusex-launcher `6854f05c` 2026-10-26 (main) | The Detail page's CPU figure is the CPU's rated clock, as the original's is its cycle counter's rate |
| `L7.20` | deusex-launcher `12fb15c0` 2026-10-25 (main) | A mnemonic's letter is one byte in a Latin-1 label |
| `L7.21` | deusex-launcher `742c9d3a` 2026-10-25 (main) | A dialog's title is converted as every drawn text is |
| `L7.23` | deusex-launcher `5c312c39` 2026-10-26 (main) | Space clicks a dialog's button as it is let go, as a Win32 button does, and a letter is an edit's |
| `L7.25` | deusex-launcher `4bdf6d03` 2026-10-26 (main) | A message box's text is measured as it is drawn |
| `L7.26` | dx-reverse-info `5de4fbac` 2026-10-26 (+1) | DeusEx.exe: the log |
| `L7.27` | dx-reverse-info `5de4fbac` 2026-10-26 (+1) | DeusEx.exe: the log |
| `L7.28` | deusex-launcher `0557a140` 2026-10-26 (main) | The game's words and environment are made before the fork and handed to execve |
| `L7.29` | deusex-launcher `9bc59966` 2026-10-25 (main) | xdg-open is started with SIGPIPE's default disposition |
| `L7.31` | deusex-launcher `6134ab08` 2026-10-26 (main) | The Renderer page's detection gives way to a quit a signal asks for |
| `L7.32` | deusex-launcher `59826bf0` 2026-10-25 (main) | run-game.sh plays the launcher's package's ini and lets the engine open its error window |
| `L7.33` | deusex-launcher `59826bf0` 2026-10-25 (main) | run-game.sh plays the launcher's package's ini and lets the engine open its error window |
| `L7.34` | deusex-launcher `a844d74b` 2026-10-26 (main) | A run-game.sh that finds no engine is a start that failed, said in the critical box |

### L8

In deusex-launcher; its fixes touched `tests/stub_engine.c`, `tests/test_play.c`,
`tests/test_saves.c`, `tests/upkg_writer.h`.

**Not cited:** L8.1, L8.3, L8.5, L8.6, L8.7, L8.8, L8.9, L8.10, L8.11, L8.12, L8.13, L8.15, L8.17,
L8.18, L8.19, L8.20, L8.21, L8.22, L8.24, L8.25, L8.26

| Item | Cited by | What the commit says |
|---|---|---|
| `L8.2` | deusex-launcher `3ed33868` 2026-10-26 (+3) (linux-x86_64) | The stub engine takes SIGTERM as DXEngine does, quitting at its next turn with exit 0, and test_play's stop sees the stopped-and-clean end |
| `L8.4` | deusex-launcher `08f9e435` 2026-10-26 (+3) (linux-x86_64) | The stub engine can leave a child holding the line, and takes SIGHUP as the engine does |
| `L8.14` | deusex-launcher `4169caef` 2026-10-26 (+3) (linux-x86_64) | The test package writer sizes a string property by its bytes |
| `L8.16` | deusex-launcher `532d0abf` 2026-10-26 (+2) (linux-x86_64) | test_gamefiles detects the classic layout in the real DefUser.ini |
| `L8.23` | deusex-launcher `d8094bc1` 2026-10-26 (+2) (linux-x86_64) | dxl-shots fails a shot on a kit diagnostic in any of its frames |
| `L8.27` | deusex-launcher `361037a9` 2026-10-26 (+1) (linux-x86_64) | check-abi holds the programs to the C library's shared libraries |

### X1

In DXEngine, deusex-launcher; its fixes touched `src/core/cmdline.c`, `tests/test_cmdline.c`,
`src/app/run.c`, `src/core/advice.c`.

**Not cited:** X1.3, X1.4, X1.5, X1.7, X1.9, X1.11

| Item | Cited by | What the commit says |
|---|---|---|
| `X1.1` | DXEngine `c5657916` 2026-10-25 | The display backend, DXEngine.pk3 and the theme are set up inside the app's try, so their failure is reported as every other |
| `X1.2` | DXEngine `82199805` 2026-10-25 | Settings.json's reader passes over a UTF-8 byte order mark, as DXLauncher's does |
| `X1.6` | deusex-launcher `1ec32ef0` 2026-10-26 (+4) (main) | A word of the player's that holds a space is quoted on the joined line, after its NAME=, and on the raw one |
| `X1.8` | DXEngine `ffbdf063` 2026-10-26 (+4) | SIGHUP quits as a closed window does, with SIGTERM and SIGINT |
| `X1.10` | DXEngine `239de0e7` 2026-10-26 | DXL_LAUNCHER_FD is taken out of the engine's environment once its line is open |
| `X1.12` | DXEngine `d4bc3be2` 2026-10-26 | The WindowSystem ini key is gone |

### X2

In DXEngine, deusex-launcher, port-ex-machina; its fixes touched `tools/dxcap.sh`, `scripts/ci.sh`,
`DXWidgets/src`, `DXEngine/RenderDevice`.

**Not cited:** X2.2, X2.3, X2.4, X2.5, X2.6, X2.7, X2.8, X2.10, X2.11, X2.13, X2.17, X2.23, X2.27,
X2.28, X2.30

| Item | Cited by | What the commit says |
|---|---|---|
| `X2.1` | DXEngine `d42408ca` 2026-10-25 | The SDL windows fail at a GL context they cannot make, as the docs say |
| `X2.9` | DXEngine `775296f4` 2026-10-26 | ENGINE.md names every Settings.json key the engine reads |
| `X2.12` | DXEngine `4cccf2c8` 2026-10-26 | ENGINE.md says CPUDriver's GL half is Linux's |
| `X2.14` | DXEngine `792d13da` 2026-10-26 | ENGINE.md names the software device among those that step a mover's surfaces |
| `X2.15` | DXEngine `f1c670ad` 2026-10-26 | GL errors are asked for once a texture made, not once a level or an upload |
| `X2.16` | DXEngine `d25afecd` 2026-10-26 | ENGINE.md says which GL scene buffers are immutable |
| `X2.18` | DXEngine `2e1ccf9f` 2026-10-26 | ENGINE.md says what a sound keeps |
| `X2.19` | DXEngine `bdcda488` 2026-10-26 | LICENSE.md names the third-party code where it lives and the three it lacked |
| `X2.20` | DXEngine `b28f44b1` 2026-10-26 | The harness refuses DXCAP_SETTINGS with DXCAP_USERDIR on a Linux build too |
| `X2.21` | DXEngine `f4aace06` 2026-10-26 | The harness's usage names MultiEditConsole's Keys |
| `X2.22` | DXEngine `7557d47a` 2026-10-26 | PerfTurnConsole is named among the consoles DXCAP_MISSION_MAP drives |
| `X2.24` | deusex-launcher `6bf126d9` 2026-10-26 (+3) (linux-x86_64) | LAUNCHER.md names the Detail page's keys the engine reads |
| `X2.25` | deusex-launcher `760d459d` 2026-10-26 (+3) (linux-x86_64) | LAUNCHER.md says which of the original's safe-mode boxes are not offered and why |
| `X2.26` | deusex-launcher `84bc5686` 2026-10-26 (+3) (linux-x86_64) | LAUNCHER.md says the engine's .pk3 missing has no fix button on Recovery |
| `X2.29` | deusex-launcher `c5afe712` 2026-10-26 (trimui-smartpro) | LAUNCHER.md no longer says a UI script hands a second start over on this branch |
| `X2.31` | deusex-launcher `da6fe0b8` 2026-10-26 (trimui-smartpro) | The port README says what dxl-cli --dry-run prints and that --probe runs alone |
| `X2.32` | deusex-launcher `da6fe0b8` 2026-10-26 (trimui-smartpro) | The port README says what dxl-cli --dry-run prints and that --probe runs alone |
| `X2.33` | deusex-launcher `f3af8b14` 2026-10-26 (windows-x86_64) | The port README says what check-abi holds the programs to: Windows 10 by their PE headers and Windows' own DLLs by name, not the functions imported, 1903 being the manifest's UTF-8 code page |
| `X2.34` | port-ex-machina `3512b0b4` 2026-10-26 | Docs: ci.sh's help prints its whole header (it ended mid-sentence, the RELEASES_TOKEN and DX_OWNER_GIT lines dropped); DEVELOPMENT.md's list of what no build reads names the engine's tools/perf, host-tools.sh and natives_audit.py and the wo… |
| `X2.35` | port-ex-machina `3512b0b4` 2026-10-26 | Docs: ci.sh's help prints its whole header (it ended mid-sentence, the RELEASES_TOKEN and DX_OWNER_GIT lines dropped); DEVELOPMENT.md's list of what no build reads names the engine's tools/perf, host-tools.sh and natives_audit.py and the wo… |
| `X2.36` | port-ex-machina `3512b0b4` 2026-10-26 | Docs: ci.sh's help prints its whole header (it ended mid-sentence, the RELEASES_TOKEN and DX_OWNER_GIT lines dropped); DEVELOPMENT.md's list of what no build reads names the engine's tools/perf, host-tools.sh and natives_audit.py and the wo… |
| `X2.L1` | DXEngine `51410bf5` 2026-10-26 | NATIVES.md's vsync note covers the Wayland window |
| `X2.L2` | DXEngine `6c073e39` 2026-10-26 | NATIVES.md says which spawns a SpawnNotify is not asked about |
| `X2.L4` | DXEngine `22555bc1` 2026-10-26 | The natives audit follows the calls in the code alone |
| `X2.L6` | DXEngine `fbbff9f6` 2026-10-26 | DEVELOPMENT.md says what empties Save/Current |
| `X2.L9` | deusex-launcher `51f03ae2` 2026-10-26 (+2) (linux-x86_64) | Docs: the controller the game takes is the first SDL has a mapping for in the order the system lists them as it starts, and the Controller page's is the first the launcher met |
| `X2.L10` | deusex-launcher `1c687bf1` 2026-10-26 (trimui-smartpro) | Docs: the port README says what a SHOT keeps |

## quality.md (14 items cited)

### L6

In deusex-launcher; its fixes touched `src/screens/window.c`, `CMakeLists.txt`, `src/app/run.c`,
`src/app/run.h`.

**Not cited:** L6.1, L6.2, L6.3, L6.4, L6.5, L6.6, L6.7

| Item | Cited by | What the commit says |
|---|---|---|
| `L6.8` | deusex-launcher `4f79ea8f` 2026-10-26 (trimui-smartpro) | The big screen's switch is gone, with the module that decided it |
| `L6.9` | deusex-launcher `0c2eedc9` 2026-10-26 (trimui-smartpro) | The window's sizing is gone |
| `L6.10` | deusex-launcher `56914695` 2026-10-26 (trimui-smartpro) | dxl_icon_save is gone |

### L7

In deusex-launcher; its fixes touched `src/core/policy.h`, `src/core/config.c`, `src/core/config.h`,
`tests/test_config.c`.

**Not cited:** L7.1, L7.2, L7.3, L7.5, L7.8, L7.10, L7.16, L7.17, L7.18, L7.19, L7.20, L7.21, L7.22,
L7.23

| Item | Cited by | What the commit says |
|---|---|---|
| `L7.4` | deusex-launcher `7dfe79b2` 2026-10-26 (main) | The ini module's four unused helpers are gone |
| `L7.6` | deusex-launcher `37e6699f` 2026-10-26 (main) | dxl_config_clamp_first_run is gone |
| `L7.7` | deusex-launcher `86e4b0b2` 2026-10-26 (main) | The config's user path, dirty flag and game engine getters are gone |
| `L7.9` | deusex-launcher `dee73d5d` 2026-10-26 (main) | The sentinel's path getter and created flag are gone |
| `L7.11` | deusex-launcher `3aa03857` 2026-10-26 (main) | The policy's forward action is gone |
| `L7.12` | deusex-launcher `cfa3022a` 2026-10-26 (main) | The decision's exec_file is gone |
| `L7.13` | deusex-launcher `8a0c0aa3` 2026-10-26 (main) | The log's echo and file switches are gone |
| `L7.14` | deusex-launcher `d74bc298` 2026-10-26 (main) | dxl_path_copy is gone |
| `L7.15` | deusex-launcher `5ded611c` 2026-10-26 (main) | Comments: paths.h and instance.c explain the desktop's code by its own filesystems, not a handheld's card |
| `L7.24` | deusex-launcher `a7538a95` 2026-10-26 (main) | dxl_dialog_set_title and the dialog's alt are gone |

### X1

In deusex-launcher; its fixes touched docs only.

**Not cited:** X1.1, X1.2, X1.3, X1.4, X1.5, X1.6, X1.7, X1.8, X1.9, X1.10, X1.11

| Item | Cited by | What the commit says |
|---|---|---|
| `X1.12` | deusex-launcher `2b6a0075` 2026-10-27 (+3) (linux-x86_64) | LAUNCHER.md swept for the hand-off |
