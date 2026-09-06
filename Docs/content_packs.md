# Content packs

A __content pack__ is a self-contained bundle of CARLA content — maps, props, vehicles, walkers, blueprints — built once against a CARLA release and then installed into any CARLA package of that release. The packaged server loads it without re-cooking or re-packaging CARLA; packs can also be mounted from the Python API while the server runs.

Two roles, two halves of this page:

* __Making a pack__ needs a CARLA source build with the editor. A pack of existing CARLA towns is one command; a pack of your own content is the usual authoring flow plus two commands.
* __Installing a pack__ needs only a CARLA package. One command, or one Python call.

Contents:

* __[Quick start](#quick-start)__
* __[Making a pack](#making-a-pack)__
    * __[Requirements](#requirements)__
    * __[A. Existing CARLA towns: `create`](#a-existing-carla-towns-create)__
    * __[B. Your own content: `init`, author, `add`, `build`](#b-your-own-content-init-author-add-build)__
    * __[Catalog JSON shapes](#catalog-json-shapes)__
    * __[Build details](#build-details)__
    * __[Checking a pack](#checking-a-pack)__
* __[Installing a pack](#installing-a-pack)__
    * __[Drop-in install](#drop-in-install)__
    * __[From the Python API](#from-the-python-api)__
    * __[Updating and removing](#updating-and-removing)__
* __[What a pack is](#what-a-pack-is)__
* __[Limits](#limits)__
* __[FAQ](#faq)__

---

## Quick start

On a CARLA source checkout that has built the package (or has the package's release metadata, see [Requirements](#requirements)):

```sh
# five towns the CARLA package does not ship, as one pack
carla-pack create Towns5 Town01_Opt Town03_Opt Town05_Opt Town12 Town13
# -> Unreal/CarlaUnreal/Plugins/Packs/Towns5/Saved/CarlaPack/out/Towns5-1.0.0-carla-0.10.0-Linux.tar.gz
```

Registering the towns takes a second; the cook takes minutes (about 9 for these five, 3.9 GB). Ship the tarball.

On any machine with the CARLA package of that release:

```sh
carla-pack install Towns5-1.0.0-carla-0.10.0-Linux.tar.gz --server /opt/carla/Linux
/opt/carla/Linux/CarlaUnreal.sh
```

```py
client = carla.Client('localhost', 2000)
client.get_available_maps()        # ... 'Town01_Opt', 'Town03_Opt', 'Town05_Opt', 'Town12', 'Town13' ...
world = client.load_world('Town12')
```

`carla-pack` is `Util/ContentPacks/carla-pack` in the source tree, a shim around `carla_pack.py` (Python 3.8+, standard library only). The consumer side needs only that one file: copy `carla_pack.py` next to the package and run `python3 carla_pack.py install ...`.

---

## Making a pack

### Requirements

* A CARLA [source build](build_carla.md) with the Unreal Editor (the cooker is the editor binary). `CARLA_UNREAL_ENGINE_PATH` must point at the engine, as for any CARLA build (`--engine` overrides).
* The __release metadata__ of the CARLA package the pack targets: `<release>-release-metadata.tar.gz` (about 2 MB, the base asset registry). The `package` target writes it next to the package tarball in `Build/Release/Package/`, and it is published alongside the official package (see [Building CARLA on Linux](build_linux_ue5.md#content-packs)). `carla-pack` finds it by itself when it is the only one under this checkout's `Build/`; `--base <file>` otherwise.

`carla-pack <command> --help` lists every option. Exit codes: `0` ok, `1` error, `2` usage error, `3` "pack not compatible with `--server`" (`inspect`). The tool finds the project from its own location; `--project` overrides.

### A. Existing CARLA towns: `create`

```sh
carla-pack create <Pack> <Map> [<Map> ...] [--base <release-metadata.tar.gz>] [--config Shipping] [--description "..."]
```

`create` is `init` + `add --map` for every map + `build` in one go. Maps are given by name (`Town12`), by package path (`/Game/Carla/Maps/Town12/Town12`) or as a `.umap` file. Run it again with more maps to add them to the same pack.

Any map under the project's `Content` (all of CARLA's own towns) is registered __in place__: the manifest names its `/Game/...` package, the cook takes the map and whatever it references that the base release did not cook, and the server loads it from the pack's containers at that same path. Nothing is copied, nothing is opened in the editor, so a World Partition town with hundreds of thousands of actors takes a second to add. Its OpenDRIVE, walker navigation and Traffic Manager files are picked up from where CARLA keeps them (`Maps/OpenDrive/<Map>.xodr` or `Maps/<Map>/OpenDrive/`, `Maps/Nav/<Map>.bin`, `Maps/TM/<Map>.bin` or `Maps/<Map>/TM/`).

Which towns qualify: any map the base package does __not__ ship. The default package ships `Town10HD_Opt`, `Town15`, `Mine_01`, `EmptyMap` and `OpenDriveMap`; `Town01_Opt` … `Town07_Opt`, `Town11`, `Town12`, `Town13` are pack material. `build` reads the base asset registry and refuses a map the base already contains.

Measured on the Shipping package: `Town12 Town13` cook in 8 minutes to 2.4 GB; the five-town example above in 9 minutes to 3.9 GB.

### B. Your own content: `init`, author, `add`, `build`

```sh
carla-pack init NewPack
# author NewTown, props and vehicles in the CARLA editor, saved under "NewPack Content"
carla-pack add NewPack --map Unreal/CarlaUnreal/Plugins/Packs/NewPack/Content/Maps/NewTown.umap --xodr NewTown.xodr --nav NewTown.bin
carla-pack add NewPack --props props.json --vehicles vehicles.json
carla-pack add NewPack --map Town12                     # existing towns mix in freely
carla-pack build NewPack
# -> Unreal/CarlaUnreal/Plugins/Packs/NewPack/Saved/CarlaPack/out/NewPack-1.0.0-carla-0.10.0-Linux.tar.gz
```

#### 1. `init`

```sh
carla-pack init NewPack [--carla-version 0.10.0] [--pack-version 1.0.0] [--description "..."]
```

creates `Unreal/CarlaUnreal/Plugins/Packs/NewPack/` with the plugin descriptor, an empty manifest, `Config/PluginSettings.ini` (makes the cook stage the sidecar folders as loose files; `init` and `build` merge into an existing file, your sections and comments stay) and the content folders `Content/{Maps, Maps/OpenDrive, Maps/Nav, Maps/TM, Config, Static, Static/Static, Blueprints}`. `Static/Static/` is the default home for props: it carries CARLA's `Static` semantic label, like the base's `Content/Carla/Static/Static/`. The pack name becomes the mount point `/NewPack/`, so it must be an identifier (letters, digits, `_`) and names taken by the engine or the project (`Game`, `Engine`, `Carla`, `Script`, existing plugins, …) are refused. `--root` keeps packs elsewhere than `Plugins/Packs`.

Do not add the pack to `CarlaUnreal.uproject`: an enabled plugin would be cooked into the CARLA package itself. The `package` target checks this after its cook (`Unreal/Package/CheckNoPackLeak.cmake`) and fails when a `/<Pack>/` package name appears in the base asset registry or a `Plugins/Packs/<Pack>/` file was staged. The same check is `carla-pack check-base <Releases/<release> | AssetRegistry.bin> [--pack NewPack]`.

#### 2. Author in the editor

Open the CARLA editor (`cmake --build Build --target launch`). The Carla plugin mounts every pack under `Plugins/Packs/` when the editor starts, so the pack appears in the content browser as __NewPack Content__ (the editor logs `Content pack plugin 'NewPack' mounted`). In a plain Unreal Editor session without the Carla plugin, or in a commandlet — the Carla plugin deliberately skips commandlets so a base cook never sees pack content — start the editor with `-EnablePlugins=NewPack` after temporarily setting `"ExplicitlyLoaded": false` in `NewPack.uplugin`, and set it back to `true` afterwards:

```sh
$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor Unreal/CarlaUnreal/CarlaUnreal.uproject -EnablePlugins=NewPack
```

Save everything the pack ships under the pack root, following CARLA's layout: maps in `Maps/`, static meshes in `Static/`, vehicle and walker blueprints in `Blueprints/`. The usual CARLA authoring guides apply unchanged — [props](content_authoring_props.md), [vehicles](tuto_content_authoring_vehicles.md) — only the destination folder changes from `Content/Carla/...` to the pack root. Referencing CARLA's own assets (`/Game/Carla/...` materials, the vehicle base classes) is fine and keeps the pack small; they are not duplicated.

__New maps live under the pack root.__ To start one from an existing town, duplicate it into the pack from the editor (content browser: right click > Duplicate, or File > Save As into `NewPack Content/Maps`) or with `carla-pack add NewPack --map Town12 --import` (see `add` below). Copying a `.umap` file from another content root does not work: the file keeps its original package name and, for World Partition maps, its actors reference that path. For a map you make, generate the OpenDRIVE file, the pedestrian navigation (`.bin`) and optionally Traffic Manager data as for any CARLA map; the pack ships them as sidecar files. A map without an `.xodr` renders but has no map API (waypoints, Traffic Manager).

__Semantic tags.__ A mesh is labelled for semantic segmentation by the folder that follows `Static` in its path, exactly as in `Content/Carla/Static`: `/NewPack/Static/<Label>/SM_Thing` with `<Label>` one of `Building`, `Fence`, `Pedestrian`, `Pole`, `Other`, `Road`, `RoadLine`, `SideWalk`, `TrafficSign`, `Vegetation`, `Car`, `Wall`, `Sky`, `Ground`, `Bridge`, `RailTrack`, `GuardRail`, `TrafficLight`, `Static`, `Dynamic`, `Water`, `Terrain`, `Truck`, `Motorcycle`, `Bicycle`, `Bus`, `Rider`, `Train`, `Rock`, `Stone`, `Bush`. A mesh anywhere else gets no label, so `carla-pack add --props` refuses a catalog entry whose pack mesh is not under `Static/<Label>/` (`--allow-untagged` accepts it with a warning). Generic props go under `Static/Static/`, the folder `init` creates.

!!! Note
    `NewPack.uplugin` is authored with `"ExplicitlyLoaded": true`, the setting the packaged server needs to mount the pack on demand. The editor and the cooker only mount a plugin that is *not* explicitly loaded, so `carla-pack build` (and `add --import`) temporarily rewrite the descriptor with `"ExplicitlyLoaded": false` for the duration of the run and restore the original file afterwards, also when the run fails or is interrupted. Only a manual `-EnablePlugins` editor session needs the flag set by hand.

__Authoring from a script (commandlet).__ The same work can be done headless with the editor's Python (`UnrealEditor-Cmd CarlaUnreal.uproject -run=pythonscript -script=make_assets.py -EnablePlugins=NewPack -NullRHI -unattended`, with `"ExplicitlyLoaded": false` set for the session as above). Two things differ from an interactive session:

* the asset registry has not scanned the pack when the script starts, so `unreal.EditorAssetLibrary.load_asset('/NewPack/...')` reports "could not be found in the Asset Registry". Either scan the mount point first — `unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/NewPack'], force_rescan=True)` — or load by object path: `unreal.load_object(None, '/NewPack/Static/Static/SM_Thing.SM_Thing')`;
* do not place meshes with `EditorActorSubsystem.spawn_actor_from_object(mesh, ...)`: for an asset the registry has not seen it crashes the process (`UPlacementSubsystem::FindAssetFactoryFromAssetData`). Spawn the actor class and assign the mesh instead:

```py
import unreal
unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/NewPack'], force_rescan=True)
mesh = unreal.load_object(None, '/NewPack/Static/Static/SM_Thing.SM_Thing')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level('/NewPack/Maps/NewTown')
actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(300, 0, 20), unreal.Rotator(0, 0, 0))
actor.static_mesh_component.set_static_mesh(mesh)
actor.set_mobility(unreal.ComponentMobility.STATIC)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
```

Duplicating a map (`EditorAssetLibrary.duplicate_asset`) copies the `.umap` alone — a World Partition copy opens empty — and opening the copy with `load_level` in the same commandlet process is fatal ("Old world ... not cleaned up by garbage collection"). A real copy of a World Partition map is a Save As (`EditorLoadingAndSavingUtils.save_map`), which is what `add --import` runs. `print()` output is swallowed by the commandlet; use `unreal.log_warning` or write a file.

#### 3. `add`

```sh
carla-pack add NewPack --map <Map|/Game/.../Map|file.umap> [--map ...] [--xodr f.xodr] [--nav f.bin] [--tm <file|dir>] [--world-partition] [--import|--copy]
carla-pack add NewPack --props props.json | --vehicles vehicles.json | --walkers walkers.json | --blueprints blueprints.json
carla-pack add NewPack --asset SM_Thing.uasset --dest Static/Static/
```

* `--map` (repeatable) registers a map in the manifest and copies its sidecar files to `Maps/OpenDrive/<Map>.xodr`, `Maps/Nav/<Map>.bin` and `Maps/TM/<Map>/`. The map may be a name (looked up under the project's and the pack's content folders), a package path or a `.umap` file:
    * a __project map__ (under the project's `Content`, i.e. a CARLA town) is registered in place at its `/Game/...` package, exactly as `create` does;
    * a map __already under the pack's `Content`__ is registered under its own path (`/NewPack/Maps/NewTown`);
    * `--import` makes a __copy under the pack root__ instead of registering in place (required for a map of another plugin, which has no path in the packaged server): `carla-pack` starts the headless editor (`UnrealEditor-Cmd`, from `CARLA_UNREAL_ENGINE_PATH` or `--engine`) with the pack enabled and performs a Save As of the map to `/NewPack/Maps/<same layout>`, which carries a World Partition map's external actors along; the copy is then yours to edit without touching CARLA's content. Save As also re-saves the source map; `carla-pack` puts the original bytes back and reports any other file the editor touched next to it. Slow for large World Partition towns (Town12, 81k actors: 5 minutes; Town13, 359k actors: over 40 minutes and 30 GB of RAM); `<pack>/Saved/CarlaPack/import-<Map>.log` has the editor output if it fails. `--copy` skips the editor and copies the file as-is, only right for a map whose package name was already rewritten.
    * Without `--xodr`/`--nav`/`--tm` the sidecars are picked up from CARLA's locations (see A). With several `--map`, they are always picked up. World Partition maps are detected from their `__ExternalActors__/…/<Map>` folder; `--world-partition` forces the flag.
* The catalog options validate the JSON against the shapes below, copy the file to `Content/Config/` under its canonical name and list it in the manifest. Asset paths that are neither under `/NewPack/` nor `/Game/` are reported: they cannot resolve once the pack is mounted.
* `--asset` copies any file or folder under `Content/<dest>`.

Re-running `add` for the same map or catalog replaces the previous entry.

#### 4. `build`

```sh
carla-pack build NewPack [--base <release-metadata.tar.gz | Releases/<release>>] [--config Development|Shipping] [--out <dir>] [--dry-run]
```

Cooks the pack as DLC against the base release and writes `out/NewPack-1.0.0-<release>.tar.gz` (default `--out`: `Plugins/Packs/NewPack/Saved/CarlaPack/out/`), plus the unpacked pack folder next to it. Minutes, not the hours of a full CARLA package. `--base` defaults as described in [Requirements](#requirements). Details in [Build details](#build-details).

### Catalog JSON shapes

Catalogs are the same JSON files CARLA reads from `Content/Carla/Config`; a pack's copies are merged with CARLA's when the pack is mounted, and later entries override earlier ones by name. Paths are Unreal object paths: `/NewPack/<folder>/<Asset>.<Asset>` for a mesh, `/NewPack/<folder>/<BP>.<BP>_C` for a blueprint class.

__Props__ — `--props`, written as `Config/NewPack.Package.json` (`static.prop.<name>` blueprints; `size` is one of `Tiny`, `Small`, `Medium`, `Big`, `Huge`). The optional `maps` list mirrors CARLA's own `*.Package.json` files.

```json
{
  "maps":  [ { "name": "NewTown", "path": "/NewPack/Maps/NewTown", "use_carla_materials": true } ],
  "props": [ { "name": "TrafficCone02", "path": "/NewPack/Static/Static/SM_Cone02.SM_Cone02", "size": "Small" } ]
}
```

A file in the `PropParameters.json` shape (`{"Props": [{"Name", "Mesh", "Size"}]}`) is accepted too and stored as `Config/PropParameters.json`.

__Vehicles__ — `--vehicles`, written as `Config/Vehicles.json` (`vehicle.<make>.<model>` blueprints):

```json
{
  "Vehicles": [
    {
      "Make": "Acme", "Model": "Rover",
      "Class": "/NewPack/Blueprints/Vehicles/Rover/BP_Rover.BP_Rover_C",
      "NumberOfWheels": 4, "Generation": 3,
      "ObjectType": "", "BaseType": "car", "SpecialType": "",
      "HasDynamicDoors": true, "HasLights": true,
      "RecommendedColors": [ { "R": 0, "G": 0, "B": 0, "A": 0 } ],
      "SupportedDrivers": []
    }
  ]
}
```

__Walkers__ — `--walkers`, written as `Config/WalkerParameters.json` (`walker.pedestrian.<id>` blueprints; speeds in m/s):

```json
{
  "Walkers": [
    {
      "Id": "9001", "Class": "/NewPack/Blueprints/Walkers/BP_Worker01.BP_Worker01_C",
      "Gender": "Female", "Age": "Adult",
      "Speeds": [ { "Speed": 0 }, { "Speed": 1.7 }, { "Speed": 4 } ],
      "Generation": 2, "CanUseWheelChair": false
    }
  ]
}
```

__Blueprints__ — `--blueprints`, written as `Config/BlueprintParameters.json` (arbitrary spawnable actors, `static.blueprint.<name>`):

```json
{ "Blueprints": [ { "Name": "SmartTrafficLight", "Path": "/NewPack/Blueprints/BP_SmartTrafficLight.BP_SmartTrafficLight" } ] }
```

### Build details

`build` extracts the release metadata and runs the Unreal cooker as a DLC cook based on it:

```
RunUAT.sh BuildCookRun -project=<uproject> -nocompileeditor -nop4 -skipbuild -cook -stage -pak -iostore \
    -clientconfig=Development -TargetPlatform=Linux -Platform=Linux -dlcname=<abs path>/NewPack.uplugin \
    -basedonreleaseversion=<release> -basedonreleaseversionroot=<...>/Releases \
    -stagingdirectory=<work>/Staged -AdditionalCookerOptions=-EnablePlugins=NewPack \
    -MapsToCook=/Game/Carla/Maps/Town12/Town12+... -DLCIncludeEngineContent
```

* Everything under `/NewPack/` is cooked, plus the project maps the manifest ships in place, which go on `-MapsToCook` (`--maps` adds more entries).
* `-DLCIncludeEngineContent` is passed by default: it makes the cook include the `/Game` and `/Engine` assets the pack references that the base release did not cook (every CARLA town has some, its vegetation for one) instead of stopping with "Uncooked Engine or Game content ... is being referenced by DLC". `--no-base-content` drops the flag when you want that error, to keep a pack free of base-project assets.
* Project maps already in the base asset registry are refused before the cook starts.
* `--dry-run` prints the exact UAT command line and exits (a tarball `--base` is still extracted into the work dir; the pack sources are not touched). `--uat-arg=-Flag` appends flags. `--platform`, `--engine`, `--project`, `--work`, `--staged`, `--asset-registry`, `--skip-cook`: see `--help`.
* Development or Shipping: cooked data does not depend on the configuration, `--config` only selects which client configuration the cook is staged for. The same pack mounts on Development, Test and Shipping packages of the same release.

The staged output is assembled into the pack layout: the containers (`--rename` names them `NewPack-Linux.*` instead of UAT's `NewPackCarlaUnreal-Linux.*`; the server mounts either), the cooked `AssetRegistry.bin`, the loose sidecar files, and the descriptor — your `.uplugin` with `ExplicitlyLoaded` and `CanContainContent` set to `true`, `EnabledByDefault` to `false` and an empty `Modules` list dropped; a pack that declares code modules is refused. The manifest is completed (`base_release`, `engine`, `platform`, one SHA-256 per file).

The same build is available as CMake targets once the package target has produced the base release: `cmake --build Build --target pack-NewPack` (and `pack-NewPack-dry-run`); `CARLA_CONTENT_PACKS_BASE` points them at a downloaded metadata tarball instead. `content-packs-test` runs the tool's unit tests.

### Checking a pack

```sh
carla-pack inspect NewPack-1.0.0-carla-0.10.0-Linux.tar.gz [--json] [--server /path/to/package]
carla-pack verify  NewPack-1.0.0-carla-0.10.0-Linux.tar.gz
```

`inspect` prints the manifest: maps, catalogs, file sizes; with `--server` it also says whether the pack fits that package (base release and platform), exit code `3` if not. `verify` checks every listed file against its checksum and that no unlisted file is present (`carla-pack.json` is the only implicit one). Both work on the tarball or on an extracted pack folder.

---

## Installing a pack

A CARLA package finds packs in three places, all read at start: `<package>/Linux/CarlaUnreal/Packs/*/` (next to the package's own `Content/Paks`), every directory passed as `-carla-packs=<dir>` on the server command line, and `$CARLA_PACKS`. A pack found there is mounted before the first episode: its maps show up in `client.get_available_maps()`, its vehicles, walkers and props in `world.get_blueprint_library()`. `Packs/` is the only supported drop-in location — do not put packs under the package's `Plugins/` folder.

### Drop-in install

```sh
carla-pack install NewPack-1.0.0-carla-0.10.0-Linux.tar.gz --server /path/to/carla-package [--force]
```

verifies every checksum (refusing files the manifest does not list), checks the base release against the server's `BaseRelease` stamp and the platform, then extracts the pack into `<package>/Linux/CarlaUnreal/Packs/NewPack/`. `--server` accepts the package root (the directory that holds `Linux/`), the `Linux/` directory (where `CarlaUnreal.sh` is) or `Linux/CarlaUnreal` itself. `--force` replaces an installed pack of the same name and overrides the base-release check (do not, unless you know the cook settings match). Extracting the tarball into `CarlaUnreal/Packs/` by hand is equivalent, minus the verification.

Start (or restart) the server. Then, from any client:

```py
client = carla.Client('localhost', 2000)
print([p.name for p in client.get_content_packs()])   # ['NewPack']
print(client.get_available_maps())                    # base maps + the pack's
world = client.load_world('NewTown')                  # as for any map
```

### From the Python API

A running server can mount packs from any path it can read, and unmount them between episodes:

```py
import carla

client = carla.Client('localhost', 2000)

# packs the server knows about (installed or mounted): carla.ContentPackInfo with
# name, version, base_release, path, mounted and maps
for pack in client.get_content_packs():
    print(pack)

# mount a pack folder (a path on the server's file system, or its carla-pack.json)
info = client.mount_content_pack('/opt/carla/Linux/CarlaUnreal/Packs/NewPack')   # RuntimeError on failure
print(info.maps)                                                                   # ['NewTown']

# blueprint definitions are rebuilt at episode start: load one of the pack's maps
# (or reload_world()) before looking for its blueprints
world = client.load_world('NewTown')
bp_library = world.get_blueprint_library()
rover = bp_library.find('vehicle.acme.rover')
cone = bp_library.find('static.prop.trafficcone02')

# unmount again (only when no episode is using its content)
client.load_world('Town10HD_Opt')
client.unmount_content_pack('NewPack')                                             # True; RuntimeError when refused
```

`mount_content_pack` raises `RuntimeError` with the reason when the manifest is missing, the base release or platform does not match the server, or a pack of the same name is already mounted; it returns the pack's `ContentPackInfo`. A tarball must be extracted first. The pack's vehicles, walkers and props are available immediately: mounting binds the new actor definitions to every live factory, so `world.get_blueprint_library()` lists them without an episode change. Loading one of the pack's __maps__ still requires `client.load_world(...)`. `unmount_content_pack` returns `True`, and raises `RuntimeError` when the pack is unknown, one of its maps is the current world, or an object from it is still alive — load another map first.

### Updating and removing

* __New version of a pack__: `carla-pack install <new tarball> --server ... --force`, then restart the server. Unreal cannot reload a package that is already in memory, so a restart is the only safe path once anything from the pack has been loaded; if nothing from it was loaded since the last episode change, `unmount_content_pack` followed by `mount_content_pack` picks up the new files.
* __New CARLA release__: the pack must be rebuilt against the new release metadata (`carla-pack build`, minutes). The server refuses a pack built for another release (see [FAQ](#faq)).
* __Remove__: delete `<package>/Linux/CarlaUnreal/Packs/<Pack>/` while the server is stopped.

---

## What a pack is

Under the hood a pack is a content-only Unreal plugin, cooked as DLC against the asset registry of a CARLA release (the __base release__) and wrapped with a manifest. Only what the base does not already contain ends up in the pack; everything a pack references from CARLA (materials, vehicle base classes, road meshes) stays a reference. A pack is distributed as `<Pack>-<version>-<base_release>.tar.gz` with this layout:

```
NewPack/
  carla-pack.json                                   # manifest
  NewPack.uplugin                                   # content-only plugin descriptor, no modules
  AssetRegistry.bin                                 # registry state for the pack's cooked assets
  Content/Paks/Linux/NewPackCarlaUnreal-Linux.pak   # cooked assets (IoStore container)
  Content/Paks/Linux/NewPackCarlaUnreal-Linux.utoc
  Content/Paks/Linux/NewPackCarlaUnreal-Linux.ucas
  Content/Config/NewPack.Package.json               # catalogs, OpenDRIVE, navigation, Traffic Manager
  Content/Config/Vehicles.json                      # data: loose files next to the containers
  Content/Maps/OpenDrive/NewTown.xodr
  Content/Maps/Nav/NewTown.bin
  Content/Maps/TM/Town12/Town12.bin
```

The server globs `Content/Paks/<Platform>/*.pak` and the engine finds the `.utoc`/`.ucas` by the `.pak`'s base name, so the three files only have to share it.

Inside the containers, content authored in the pack lives under its own mount point, `/NewPack/`, with the same layout as CARLA's own `Content/Carla`, so two packs can each ship a `Maps/Town.umap` without clashing:

```
/NewPack/Maps/NewTown.umap  (+ World Partition cells)     /NewPack/Static/<Label>/SM_Thing.uasset
/NewPack/Blueprints/Vehicles/BP_Foo.uasset                 /NewPack/Blueprints/Walkers/BP_Bar.uasset
```

CARLA towns shipped in place keep their `/Game/Carla/Maps/...` packages inside the pack's containers, together with the base-project assets they reference that the base did not cook. Two packs shipping the same town would collide, one pack per town.

Everything that is not a cooked asset — the catalogs, `.xodr`, navigation and Traffic Manager files — ships as loose files under the pack's `Content/` folder, where the runtime reads them. (Files inside a DLC container are addressed relative to the engine root and would be unreachable once the pack folder lives elsewhere.)

The manifest, `carla-pack.json`, is what CARLA reads when it mounts the pack. It is authoritative: maps and catalogs are registered from it, not by scanning directories.

```json
{
  "name": "NewPack", "version": "1.0.0",
  "carla_version": "0.10.0", "base_release": "carla-0.10.0-Linux",
  "engine": { "version": "5.8.0", "commit": "abcdef012345" }, "platform": "Linux",
  "maps": [ { "name": "NewTown", "package": "/NewPack/Maps/NewTown",
              "xodr": "Maps/OpenDrive/NewTown.xodr", "nav": "Maps/Nav/NewTown.bin", "world_partition": true },
            { "name": "Town12", "package": "/Game/Carla/Maps/Town12/Town12",
              "xodr": "Maps/OpenDrive/Town12.xodr", "tm": "Maps/TM/Town12", "world_partition": true } ],
  "catalogs": [ "Config/NewPack.Package.json", "Config/Vehicles.json" ],
  "files": { "Content/Paks/Linux/NewPackCarlaUnreal-Linux.utoc": "sha256:…", "…": "…" }
}
```

`base_release` binds the pack to the CARLA package it was built for: the packaged server carries the same string in `CarlaUnreal/BaseRelease` and refuses packs built against another release.

---

## Limits

* A pack targets __one base release__: same CARLA package, same engine binary, same cook settings. A new CARLA release means rebuilding the pack (`carla-pack build` again, minutes) — never re-packaging CARLA.
* Building a pack needs the CARLA source build with the editor. Using a pack needs only the package.
* Assets a pack references that are __not__ in the base release are cooked into the pack. That is correct, but a pack that references a whole asset library from another pack, or from a map the base did not cook, gets correspondingly large. Two packs never share content with each other.
* A pack ships whole maps and new actors: CARLA towns the package left out, or maps of your own. Adding actors to a town the base package already ships (Town10, Town15, …) is out of scope for now, and a pack cannot replace a base map.
* Pack names are global on a server: two packs called `NewPack` cannot be mounted at the same time. Two packs shipping the same town in place collide too.
* Updating a pack whose content has been loaded needs a server restart.

---

## FAQ

__`base release mismatch: pack built against 'carla-0.10.0-Linux', server is 'carla-0.10.1-Linux'`__

The pack was cooked against another CARLA release and the server refuses it — cooked data is only valid against the asset registry it was cooked with. Rebuild the pack with that server's release metadata: `carla-pack build NewPack --base carla-0.10.1-Linux-release-metadata.tar.gz`. `carla-pack inspect <pack> --server <dir>` shows both strings; `<package>/BaseRelease` (and `<package>/Linux/CarlaUnreal/BaseRelease`) holds the server's.

__`server has no BaseRelease stamp`__

The package was built without content pack support (before the `package` target stamped the release). Rebuild the package, or pass `--force` to `install` only if you cooked the pack from that exact source tree and package.

__`no base release found` / `several base releases found`__

`carla-pack` looks for a single `*-release-metadata.tar.gz` under this checkout's `Build/`. Build the package first (`cmake --build Build --target package`), or pass `--base` with the metadata tarball published with the package you target.

__`the base release already ships /Game/Carla/Maps/Town10HD_Opt`__

That town is in the CARLA package already; a pack cannot replace it. Drop it from the map list.

__I updated a pack but the server still shows the old content__

Unreal cannot reload a package that is already in memory. If any map, mesh or blueprint of the pack has been loaded, restart the server after installing the new version. If nothing from it was loaded since the last episode change, `client.unmount_content_pack('NewPack')` followed by `client.mount_content_pack(...)` picks up the new files.

__The client prints `InMemoryMap cache rejected ... rebuilding the local map from the OpenDRIVE`__

The Traffic Manager was handed a cached map (`Maps/TM/<Map>.bin`) that does not match the town's OpenDRIVE — the cache was generated for an older version of the road network. It is discarded and rebuilt from the OpenDRIVE, which takes a few seconds on the first `set_autopilot` in that town; nothing else changes. Regenerating the cache file for that town makes the warning go away.

__How big is a pack?__

Roughly the size of the cooked assets it ships, plus the sidecar files. Measured: `Town12 Town13` 2.4 GB; `Town01_Opt Town03_Opt Town05_Opt Town12 Town13` 3.9 GB; a map with a few hundred unique meshes is typically in the hundreds of MB; a pack that only lists new vehicles built from existing CARLA meshes is a few MB. Everything the pack references from the base release costs nothing. `carla-pack inspect` lists the files with their sizes; the `.ucas` container is where the bulk lives.

__Can I mount a pack that is not under `Linux/CarlaUnreal/Packs`?__

Yes: `mount_content_pack` takes any folder on the server's file system that contains a `carla-pack.json`, and `-carla-packs=<dir>` / `$CARLA_PACKS` register extra directories to scan at start. A tarball must be extracted first.

__Does a pack work with a Shipping server?__

Yes. Cooked content is configuration independent; the same pack mounts on Development, Test and Shipping packages of the same release.

__Can I edit a CARLA town for my pack?__

Make your own copy first: `carla-pack add NewPack --map Town12 --import` performs the editor's Save As into `/NewPack/Maps/Town12/Town12` (minutes for a large World Partition town, see `add`), then edit that copy in the editor and `build`. The pack then ships the copy under `/NewPack/`, so it can be installed next to a pack that ships the original in place.
