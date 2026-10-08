---
name: import-carla-ue58-map
description: Gets a custom map into CARLA on UE 5.8 — the four distinct routes and which one to pick — via Util/Tools/Import.py for FBX+OpenDRIVE assets into a source build, CARLA_MAPS_TO_COOK to ship a map inside a package, or client.generate_opendrive_world() for an OpenDRIVE-only map at runtime with no build at all. Covers where imported assets land, why an imported map cannot be loaded from a package by name, and the four capabilities the UE4 skill has that ue58 lacks — standalone asset packages, their installer, semantic-material tagging, and a pedestrian navmesh that is generated but silently skipped. Use when the user asks to "import a map", "add a custom map", "load my .xodr", "package my own town", or "create a standalone map package".
license: MIT
compatibility: Linux with a built ue58-dev tree; the editor commandlets need CARLA_UNREAL_ENGINE_PATH and an importable `carla` client. The OpenDRIVE-only route needs only a running server. VERIFIED end to end on ue58-dev HEAD 718efd7cc, engine 5.8.0 - Route B was run with a real RoadRunner pair (31.7 MB FBX + 515 KB OpenDRIVE 1.4), producing a map that loads on a server with 24 spawn points, 80 topology edges, 80 crosswalks and 36 landmarks, and on which a vehicle spawns and resolves to a lane. Route A (cook scope) and Route C (runtime OpenDRIVE) were verified earlier. The pedestrian nav mesh is NOT produced - see the gap below.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/maps.md
---

# Get a map into CARLA on UE 5.8

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Four different things get called "adding a map", and picking the wrong one wastes
hours. They are not variations of one procedure:

| Route | What you need | Build step | Result |
|---|---|---|---|
| **A. Cook into a package** | the map already in `Content` | `package` target, 1–2 h | map ships inside the `.pak` |
| **B. Import into a source build** | `.fbx` + `.xodr` | editor commandlets, minutes | map in `Content`, usable in the editor |
| **C. OpenDRIVE at runtime** | just a `.xodr` | **none** | procedural road mesh, no scenery |
| **D. Standalone asset package** | — | — | **does not exist on ue58-dev** |

Route D is the UE4 `make package ARGS="--packages=Name"` flow, and it is gone in
both directions — nothing builds such a package and nothing installs one. The docs
that described it (`tuto_A_create_standalone.md`, `tuto_M_add_map_package.md`) are
**absent from this tree**: `Docs/maps_tutorials.md` and
`Docs/custom_assets_tutorials.md` link to 10 files that do not exist. Do not plan
around it — the section below lists this and the three other UE4 capabilities with
no ue58 equivalent.

## What the UE4 skill does that this one cannot

Read this before promising a user anything the UE4 flow could do. Four
capabilities of [[import-carla-map]] have **no equivalent on ue58-dev**, all
verified against both trees:

| UE4 capability | UE4 mechanism | ue58 state |
|---|---|---|
| Build a standalone, distributable asset package | `Util/BuildTools/Package.sh --packages=Name1,Name2` | **gone** — no `Package.sh`; the only targets are whole-server (`package`, `packageShipping`, `packageDebug`, `packageDebugGame`, `packageDevelopment`, `packageTest`, `Unreal/CMakeLists.txt:546-551`), scoped only by `CARLA_MAPS_TO_COOK` |
| Install such a package into an existing release | `Util/ImportAssets.sh`, shipped at the release root | **gone** — a built ue58 package root holds only `CHANGELOG LICENSE Linux PythonAPI README Tools VERSION` |
| Tag materials so imported geometry is segmented | `-run=GenerateTaggedMaterialsRegistry -PackageNames=…`, run by `Package.sh` per package | **gone** — the commandlet exists in 2 UE4 plugin files and **0** here, so `semantic_tags` comes back empty on imported assets ([[import-carla-ue58-prop]] hits the same wall) |
| Generate the pedestrian navmesh on import | `Util/DockerUtils/dist/` ships `RecastBuilder`; `build.sh` runs `FBX2OBJ` then `RecastBuilder` | **silently does nothing** — see below |

### The navmesh gap is silent, and it is the one that bites

`Import.py:487 build_binary_for_navigation` shells out to
`Util/DockerUtils/dist/build.sh`, whose every step is guarded:

```bash
if [ -f "FBX2OBJ" ]; then …
if [ -f "RecastBuilder" ]; then …
```

UE4 ships `RecastBuilder` in that folder. ue58's `dist/` contains only
`addOBJ.py`, `build.bat`, `build.sh`, `get_xodr_crosswalks.py`. CMake *does* build
RecastBuilder — to `Build/<preset>/_deps/recastnavigation-build/RecastBuilder/` and
into packages as `Tools/RecastBuilder` — but **nothing copies it into `dist/`**, and
no CMake target references `DockerUtils` at all. `FBX2OBJ` has its own
`Util/DockerUtils/fbx/CMakeLists.txt` that the main build never adds, and no binary
exists under `Build/`. So the import runs, exits 0, writes no `.bin`, and walkers
cannot navigate the new map — with no warning anywhere.

Work around it by putting both binaries where `build.sh` looks:

```bash
cp "$(find "${CARLA_UE58_ROOT}/Build" -name RecastBuilder -type f | head -1)" \
   "${CARLA_UE58_ROOT}/Util/DockerUtils/dist/"
# FBX2OBJ must be built first: it is a standalone CMake project needing the FBX SDK
# cmake -S Util/DockerUtils/fbx -B /tmp/fbx2obj && cmake --build /tmp/fbx2obj
```

**Route C is unaffected.** `CarlaEpisode.cpp:45 BuildRecastBuilderFile()` prefers
`<RootDir>/Tools/RecastBuilder` and falls back to the compiled-in
`RECASTBUILDER_PATH`, so a runtime OpenDRIVE world gets its navmesh properly.

### What is NOT missing

`Util/Tools/Import.py` is UE4's `Util/BuildTools/Import.py` with paths renamed —
the whole diff is **24 lines** (`UE4_ROOT`→`CARLA_UNREAL_ENGINE_PATH`,
`UE4Editor`→`UnrealEditor`, `CarlaUE4`→`CarlaUnreal`, `is not`→`!=`, two imports).
Every function is identical, and the three commandlets it needs —
`PrepareAssetsForCooking`, `MoveAssets`, `LoadAssetMaterials` — are all present.
That is why Route B works at all.

The `.Package.json` format also lingers without its pipeline: `Import.py:276
generate_package_file` still writes one, and `Content/Carla/Config` still holds
`Town06_Opt.Package.json` and friends, but nothing on ue58 cooks or loads them.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Pick a route (A / B / C) — see the decision below
- [ ] Step 3: Run it
- [ ] Step 4: Verify the map actually loads
```

### Step 2: Which route

**Only have a `.xodr`, and want roads to drive on?** Route C. No build, seconds,
works against any running server. You get a procedurally generated road mesh and
nothing else — no buildings, no props, no scenery.

**Have `.fbx` geometry and want it in the editor?** Route B, then Route A if you
also need it in a package.

**Map already in `Content` and you want it shipped?** Route A alone.

### Route C — OpenDRIVE only, no build

```bash
source scripts/env.sh
bash scripts/import_map.sh opendrive /path/to/MyTown.xodr
```

which is `client.generate_opendrive_world(xodr_string, params)`. Tunable through
`carla.OpendriveGenerationParameters`: `vertex_distance`, `max_road_length`,
`wall_height`, `additional_width`, `smooth_junctions`,
`enable_mesh_visibility`.

This is the fastest way to test a road network, and the only route that needs no
engine at all. It is also the only one where the map is **not** persisted: it
lives for the session.

### Route B — import FBX + OpenDRIVE into a source build

Lay the assets out under `Import/`:

```
Import/
├── MyTown.fbx          # geometry; name must match the .xodr
├── MyTown.xodr         # road network
└── MyPackage.json      # OPTIONAL - generated if absent
```

then:

```bash
source scripts/env.sh
bash scripts/import_map.sh plan   --package MyPackage    # what will happen, no changes
bash scripts/import_map.sh import --package MyPackage
```

**Do not use `Util/Tools/Import.sh`.** It is broken on this branch:
`Util/Tools/Environment.sh` never sets `CARLA_BUILD_TOOLS_FOLDER`, so its last
line runs `python3 /Import.py`:

```
$ bash Util/Tools/Import.sh --help
parse-options: unrecognized option '--help'
/usr/bin/python3: can't open file '/Import.py': [Errno 2] No such file or directory
$ echo $?
2
```

(`--help` is also missing from its `getopt` list, which is the first line of that
output. The exit code is 2 because `Environment.sh` runs `set -e`; measure it
directly, not through a pipeline, or you will read the exit code of the last
command in the pipe.) Call `Util/Tools/Import.py` directly, which is what this
skill's script does.

What the pipeline does, in order:

1. scans `Import/` for package `.json` files; generates one if none exist,
2. writes `importsetting.json` for the `ImportAssets` commandlet,
3. runs the editor commandlets: `ImportAssets` → `LoadAssetMaterials` →
   `PrepareAssetsForCooking` → `MoveAssets`,
4. **tries** to build the navigation mesh with `RecastBuilder` from
   `Util/DockerUtils/dist` — and silently does nothing, because ue58 ships
   neither `RecastBuilder` nor `FBX2OBJ` there and `build.sh` guards both steps
   with `if [ -f ... ]`. `plan` prints the copy/build commands that fix it,
5. builds the Traffic Manager cache via `carla.Map(...).cook_in_memory_map()` —
   which is why `Import.py` needs an importable `carla` client,
6. writes the package descriptor.

**A measured run**, with a real RoadRunner export (`TestMap.fbx` 31.7 MB binary
FBX + `TestMap.xodr` 515 KB, OpenDRIVE 1.4, RoadRunner 2019.2):

```
Import.py exited 0, ~55 s, four commandlets in sequence
  Content/TestMapPkg/Maps/TestMap/TestMap.umap
  Content/TestMapPkg/Maps/TestMap/OpenDrive/TestMap.xodr
  Content/TestMapPkg/Maps/TestMap/TM/TestMap.bin
  Content/TestMapPkg/Config/TestMapPkg.Package.json   ("maps": 1, "props": 0)
  Content/TestMapPkg/Static/{Road,RoadLine,SideWalk,Terrain}
server: get_available_maps() lists TestMap (30 maps); load_world("TestMap") ->
  24 spawn points, 80 topology edges, 80 crosswalks, 36 landmarks;
  a vehicle spawns and resolves to road 2 / lane -1
NO Nav/TestMap.bin — walkers cannot navigate it (the step-4 gap above)
```

Two log errors are expected noise on a clean run: `LogAutomatedImport: Error:
Invalid Destination Path ()` appears twice, from the empty props list in the
generated descriptor, and does not affect the map. `FBXImport: Warning: No
smoothing group information` reflects the exporter's settings, not the import.

Where things land — **this is the part that decides whether Route A will work
afterwards**:

```
Content/<Package>/Maps/<MapName>              the .umap        -> /Game/<Package>/Maps/<MapName>
Content/<Package>/Maps/<MapName>/TM/<Map>.bin Traffic Manager cache
Content/<Package>/Config/<Package>.Package.json   descriptor
```

### Route A — cook a map into a package

```bash
cd ../build-carla-ue58
MAPS="Town10HD_Opt,MyTown" bash scripts/build.sh configure
cd ../package-carla-ue58 && bash scripts/package.sh build
```

`MAPS=` takes town names and expands them to `+`-separated package paths. For a
map outside `/Game/Carla/Maps` pass the full package path:

```bash
MAPS="/Game/Carla/Maps/Town10HD_Opt,/Game/MyPackage/Maps/MyTown" \
    bash ../build-carla-ue58/scripts/build.sh configure
```

**A cooked imported map still may not load by name.** `UCarlaStatics::FindMapPath`
first does a filesystem `FindFilesRecursive("*.umap")`, which finds nothing inside
a `.pak`, then falls back to exactly one package path:

```cpp
const FString ProjectMapPackage = FString::Printf(TEXT("/Game/Carla/Maps/%s"), *MapName);
if (FPackageName::DoesPackageExist(ProjectMapPackage)) return ProjectMapPackage;
```

So from a package, `load_world("X")` resolves **only** if the map sits at
`/Game/Carla/Maps/X`. An imported map at `/Game/MyPackage/Maps/MyTown` does not,
and neither does a nested large map at `/Game/Carla/Maps/Town12/Town12` —
measured: `load_world('Town12')` raises `std::exception` from a package while
`load_world('Town_C')` works.

**Consequence:** to be loadable from a package, an imported map must end up under
`/Game/Carla/Maps/<Name>`. In the editor it works from anywhere, because the
filesystem search succeeds there.

### Step 4: Verify

```bash
bash scripts/import_map.sh verify --map MyTown
```

Checks `Content` for the `.umap`, the `.xodr`, the TM cache and the nav mesh,
reports the package path the map will have, whether that path is loadable from a
package, and — if a server is up — whether it appears in `get_available_maps()`
and loads.

## Examples

**Example 1: "I have a .xodr, show me the road network"**

Route C: `bash scripts/import_map.sh opendrive MyTown.xodr` against a running
server. Seconds, no build. Roads only.

**Example 2: "import my town with buildings"**

Route B into `Import/`, then open the editor
([[run-carla-ue58-server]] `editor`) and check it there first — the editor finds
maps anywhere in `Content`, so this isolates import problems from packaging ones.

**Example 3: "ship my imported map in a package"**

Route B, then move/import it so it lands under `/Game/Carla/Maps/<Name>`, then
Route A with `MAPS="...,<Name>"`. Verify with
[[package-carla-ue58]] `inspect` that its `.xodr`/`TM` staged — per-map
subdirectories are **not** in `DirectoriesToAlwaysStageAsUFS`, which is the same
gap that leaves Town12/Town13 without road data.

**Example 4: "make a standalone map package to give someone"**

Not available on ue58-dev (Route D). Options: ship the whole package, or give them
the `.xodr` and let them use Route C.

## Troubleshooting

**Problem: `python3: can't open file '/Import.py'`**
Cause: `Util/Tools/Import.sh` — `CARLA_BUILD_TOOLS_FOLDER` is unset in
`Environment.sh`, and the script still exits 0.
Solution: run `Util/Tools/Import.py` directly.

**Problem: `ModuleNotFoundError: No module named 'carla'` from Import.py**
Cause: it imports `carla` at module scope for the TM cache
(`carla.Map(...).cook_in_memory_map()`).
Solution: install the wheel — [[build-carla-ue58]] `pythonapi`.

**Problem: the import runs but the map is not in the editor**
Cause: the commandlets failed while `Import.py` continued — it uses
`subprocess.call` for the POSIX path and does not check the return code.
Solution: read the commandlet output; `plan` prints the exact commands so you can
run one by hand.

**Problem: `KeyError: 'xodr'` or nav/TM steps skipped**
Cause: the generated package JSON has no `xodr`/`source` for the map, so
`build_binary_for_navigation` `continue`s and no nav mesh is produced.
Solution: name the `.xodr` exactly like the `.fbx`, or write the package `.json`
by hand.

**Problem: imported map works in the editor, `load_world` fails from a package**
Cause: the pak fallback only checks `/Game/Carla/Maps/<Name>`.
Solution: get the map to that path, or use the editor / `-game` mode.

**Problem: imported map loads but has no traffic and walkers cannot spawn**
Cause: missing TM cache or navigation mesh. Town_C ships without either, so this
is not unique to imported maps.
Solution: `verify` reports both; re-run the import with a matching `.xodr`.

**Problem: instance segmentation tags are wrong for imported assets**
Cause: the `GenerateTaggedMaterialsRegistry` commandlet that UE4's packaging ran
across packages does not exist in this tree.
Solution: no in-tree equivalent; tag materials manually.

## Outputs

Route A: a map cooked into `Build/<preset>/Package/...`. Route B: assets under
`Content/<Package>/` plus a `.Package.json` descriptor, a TM cache and a nav mesh.
Route C: a world that exists only for the session. `plan` and `verify` are
read-only.

The route taxonomy, the exact import output paths, map-discovery code paths and the
staging implications are in [references/maps.md](references/maps.md).
