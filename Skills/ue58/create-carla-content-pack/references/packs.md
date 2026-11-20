# Content packs: detail for create-carla-content-pack

The user-facing page is `Docs/content_authoring_packs.md`; this file keeps what a builder needs at hand, plus what was measured while writing the skill.

## Contents

- [Commands](#commands)
- [Catalog shapes](#catalog-shapes)
- [Sidecar files](#sidecar-files)
- [Editor Python against a pack](#editor-python-against-a-pack)
- [What build does](#what-build-does)
- [Measured](#measured)
- [Failure modes](#failure-modes)

## Commands

`carla-pack` is `Util/ContentPacks/carla-pack` (shim) or `python3 Util/ContentPacks/carla_pack.py`. Exit codes: 0 ok, 1 error, 2 usage, 3 not compatible (`inspect --server`).

| Command | Does |
|---|---|
| `init <Pack>` | `Plugins/Packs/<Pack>/` with `.uplugin` (`ExplicitlyLoaded: true`), empty manifest, `Config/PluginSettings.ini`, `Content/{Maps,Maps/OpenDrive,Maps/Nav,Maps/TM,Config,Static,Static/Static,Blueprints}` |
| `add <Pack> --map <name\|/Game/...\|file.umap>` | registers a map; a project map in place, a pack map at `/<Pack>/...`; `--import` Save-As copies a project map into the pack with the editor (Town12: 5 min, Town13: 40+ min and 30 GB RAM) |
| `add <Pack> --xodr/--nav/--tm`, `--no-xodr/--no-nav/--no-tm` | replace or drop one sidecar of an in-pack map |
| `add <Pack> --props/--vehicles/--walkers/--blueprints <json>` | validate and copy a catalog to `Content/Config/` under its canonical name; re-adding replaces it |
| `add <Pack> --asset <file\|dir> --dest <rel>` | copy anything under `Content/<rel>` |
| `create <Pack> <Map>... [--maps-file f]` | `init` + `add --map` each + `build`; only adds maps |
| `build <Pack> --base <meta>` | DLC cook + tarball |
| `inspect <tar\|dir> [--server PKG] [--json]`, `verify <tar\|dir>` | read the manifest / check checksums |
| `check-base <Releases/<rel>\|AssetRegistry.bin>` | fail if a pack leaked into a base release |

There is no `remove`: to drop a map or catalog, delete `Plugins/Packs/<Pack>/` and recreate it, or edit `carla-pack.json` and delete the staged sidecar.

## Catalog shapes

Paths are object paths: `/<Pack>/<dir>/<Asset>.<Asset>` for a mesh, `/<Pack>/<dir>/<BP>.<BP>_C` for a blueprint class. `/Game/...` paths (CARLA's own assets) are fine. Anything else cannot resolve once mounted and is reported.

| Option | Stored as | Shape | Blueprint id |
|---|---|---|---|
| `--props` | `Config/<Pack>.Package.json` | `{"props": [{"name", "path", "size"}], "maps": [...]}` | `static.prop.<name>` (lowercase) |
| `--props` (other shape) | `Config/PropParameters.json` | `{"Props": [{"Name", "Mesh", "Size"}]}` | same |
| `--vehicles` | `Config/Vehicles.json` | `{"Vehicles": [{"Make", "Model", "Class", "NumberOfWheels", "Generation", "ObjectType", "BaseType", "SpecialType", "HasDynamicDoors", "HasLights", "RecommendedColors", "SupportedDrivers"}]}` | `vehicle.<make>.<model>` |
| `--walkers` | `Config/WalkerParameters.json` | `{"Walkers": [{"Id", "Class", "Gender", "Age", "Speeds": [{"Speed"}], "Generation", "CanUseWheelChair"}]}` | `walker.pedestrian.<id>` |
| `--blueprints` | `Config/BlueprintParameters.json` | `{"Blueprints": [{"Name", "Path"}]}` | `blueprint.<name>` (lowercase) |

`size` is `Tiny`, `Small`, `Medium`, `Big` or `Huge`. A prop mesh must sit under `/<Pack>/Static/<Label>/`, and `<Label>` sets its semantic tag; `--allow-untagged` accepts others without a tag. Labels: `Building`, `Fence`, `Pedestrian`, `Pole`, `Other`, `Road`, `RoadLine`, `SideWalk`, `TrafficSign`, `Vegetation`, `Car`, `Wall`, `Sky`, `Ground`, `Bridge`, `RailTrack`, `GuardRail`, `TrafficLight`, `Static`, `Dynamic`, `Water`, `Terrain`, `Truck`, `Motorcycle`, `Bicycle`, `Bus`, `Rider`, `Train`, `Rock`, `Stone`, `Bush`.

For base content on UE 5.8, `*.Package.json` is no longer read and props go in `PropParameters.json` (see import-carla-ue58-prop). Packs are different: the server reads every `*.Package.json` under a pack's `Content/Config/` (`PropActorFactory.cpp`). Measured: both pack props spawned from `<Pack>.Package.json`.

## Sidecar files

| File | Pack path | Without it |
|---|---|---|
| `<Map>.xodr` | `Content/Maps/OpenDrive/` (`OpenDrive/<Map>/` when a `map_logic.json` traffic-light phase file sits next to it) | renders, but no waypoints, no Traffic Manager, no agents |
| `<Map>.bin` (Recast) | `Content/Maps/Nav/` | walkers cannot navigate |
| `<Map>/` Traffic Manager cache | `Content/Maps/TM/` | the Traffic Manager builds its graph from the OpenDRIVE on first use (seconds) |

For a CARLA town, `add` picks them up from `Content/Carla/Maps/{OpenDrive,Nav,TM}/` or `Maps/<Town>/{OpenDrive,TM}/`. The nav `.bin` comes from `RecastBuilder` (`Build/_deps/recastnavigation-build/RecastBuilder/`, or `Tools/RecastBuilder` in a package).

## Editor Python against a pack

`pack_editor.py run <Pack> script.py` launches `UnrealEditor-Cmd <uproject> -run=pythonscript -script=<script> -EnablePlugins=<Pack> -NullRHI -unattended`, with the pack's `ExplicitlyLoaded` set to false for the run (the commandlet does not mount explicitly loaded plugins) and restored afterwards, also on failure. Inside that commandlet:

- The asset registry has not scanned the pack: `unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/<Pack>'], force_rescan=True)` first, or load by object path with `unreal.load_object(None, '/<Pack>/Static/Static/SM_X.SM_X')`.
- `EditorActorSubsystem.spawn_actor_from_object(mesh, ...)` on a pack asset crashes the process (`UPlacementSubsystem::FindAssetFactoryFromAssetData`). Spawn `unreal.StaticMeshActor` with `spawn_actor_from_class` and set its mesh.
- `print()` does not reach the log: use `unreal.log_warning` or write a file.
- `EditorAssetLibrary.duplicate_asset` on a World Partition map copies the `.umap` alone (opens empty), and `load_level` on the copy in the same process is fatal. A real copy is a Save As: `carla-pack add --import`.

`import-meshes` runs `Skills/ue58/import-carla-ue58-prop/scripts/editor/import_prop_editor.py` (Interchange FBX import through `AssetImportTask` + `FbxImportUI`, combined meshes, auto collision), destination `/<Pack>/Static/<Label>/<FbxStem>/`, then derives the size from the bounds (<=0.5 m Tiny, <=2 m Small, <=8 m Medium, else Big). The prop name is the FBX stem without a leading `SM_`.

## What build does

1. Extracts the base metadata into `<pack>/Saved/CarlaPack/Releases/<release>/`.
2. With the `.uplugin` temporarily `ExplicitlyLoaded: false`, runs `RunUAT BuildCookRun -cook -stage -pak -iostore -dlcname=<uplugin> -basedonreleaseversion=<release> -basedonreleaseversionroot=... -AdditionalCookerOptions=-EnablePlugins=<Pack> -MapsToCook=<in-place maps> -DLCIncludeEngineContent`. That last flag cooks into the pack every `/Game`/`/Engine` asset it references that the base did not cook (CARLA towns need it).
3. Strips base-content files UAT put in the pack's `.pak` (measured: 38 for Town01_Opt), copies the cooked `AssetRegistry.bin`, the loose sidecars and catalogs, and the runtime `.uplugin`.
4. Completes the manifest (`base_release`, `engine`, `platform`, SHA-256 per file) and writes `<Pack>-<version>-<release>.tar.gz` plus the unpacked folder.

`--config` changes nothing in the cooked data; the deployed package is Shipping, so `pack.sh` passes `--config Shipping`. The CMake `pack-<Pack>` targets use `CARLA_CONTENT_PACKS_CONFIG` (Development).

## Measured

On ue58-dev `1ad2c15c9`, base = a Shipping package cooking `Town10HD_Opt` + `OpenDriveMap`:

| Pack | Time | Tarball |
|---|---|---|
| `Town01_Opt` (in place) | 49 s (UAT 34 s, warm DDC) | 615 MB, 8 files |
| `Town13` (World Partition, in place) | 463 s | 1.59 GB, 7 files (xodr, tm, no nav) |
| 2 FBX props + 1 vehicle entry on a CARLA class | 27 s | 1.6 MB, 7 files |
| `pack_editor.py import-meshes` per run | 11-12 s editor time | — |

## Failure modes

| Symptom | Cause | Fix |
|---|---|---|
| `several base releases found` | more than one `*-release-metadata.tar.gz` under `Build/` or `Releases/*` | pass `--base` (`pack.sh` does) |
| `no base release found` | package never built | build it, or download the target package's metadata |
| `the base release already ships /Game/...` | town cooked into the package | drop it; delete the half-made pack folder |
| `'<Name>' is a reserved name` | `Game`, `Engine`, `Carla`, `Script`, an existing plugin | rename |
| the package target fails its leak check | pack enabled in `CarlaUnreal.uproject`, or named like a base content folder | remove it from the project; `-DCARLA_PACK_LEAK_IGNORE=<Pack>` for a name clash |
| prop 100x too big | FBX without unit metadata | `--scale 0.01` |
| pack mounts, its town misses meshes or crashes | pack cooked against another package (or a server bug: Town13 crashed a Town10-only local Shipping package from a correctly paired pack) | `get-carla-additional-maps pair` with both metadata files; rebuild against the target package's metadata |
| `Uncooked Engine or Game content ... referenced by DLC` | `--no-base-content` | drop the flag |
