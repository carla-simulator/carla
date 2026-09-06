# Content packs

A __content pack__ is a self-contained bundle of CARLA content — maps, props, vehicles, walkers, blueprints — that is built once against a CARLA release and then dropped into any CARLA package of that release. A pack is loaded by the packaged server without re-cooking or re-packaging CARLA, and can be mounted from the Python API while the server is running.

* __[What a pack is](#what-a-pack-is)__
* __[Requirements](#requirements)__
* __[Authoring flow](#authoring-flow)__
    * __[1. Create the pack](#1-create-the-pack)__
    * __[2. Author the content in the editor](#2-author-the-content-in-the-editor)__
    * __[3. Add maps, catalogs and assets](#3-add-maps-catalogs-and-assets)__
    * __[Catalog JSON shapes](#catalog-json-shapes)__
* __[Build against a release](#build-against-a-release)__
* __[Install a pack](#install-a-pack)__
    * __[Drop-in](#drop-in)__
    * __[From the Python API](#from-the-python-api)__
* __[Limits](#limits)__
* __[FAQ](#faq)__

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
```

The containers carry the name the Unreal stager gives them, `<Pack>CarlaUnreal-<Platform>.*`; `carla-pack build --rename` ships them as `NewPack-Linux.*` instead. The server mounts either — it globs `Content/Paks/<Platform>/*.pak` and the engine finds the `.utoc`/`.ucas` by the `.pak`'s base name, so the three files only have to share it.

Inside the containers the pack's assets live under their own mount point, `/NewPack/`, with the same layout as CARLA's own `Content/Carla` so two packs can each ship a `Maps/Town.umap` without clashing:

```
/NewPack/Maps/NewTown.umap  (+ World Partition cells)     /NewPack/Static/<Label>/SM_Thing.uasset
/NewPack/Blueprints/Vehicles/BP_Foo.uasset                 /NewPack/Blueprints/Walkers/BP_Bar.uasset
```

Everything that is not a cooked asset — the catalogs, `.xodr`, navigation and Traffic Manager files — ships as loose files under the pack's `Content/` folder, where the runtime reads them. (Files inside a DLC container are addressed relative to the engine root and would be unreachable once the pack folder lives elsewhere.)

The manifest, `carla-pack.json`, is what CARLA reads when it mounts the pack. It is authoritative: maps and catalogs are registered from it, not by scanning directories.

```json
{
  "name": "NewPack", "version": "1.0.0",
  "carla_version": "0.10.0", "base_release": "carla-0.10.0-Linux",
  "engine": { "version": "5.8.0", "commit": "abcdef012345" }, "platform": "Linux",
  "maps": [ { "name": "NewTown", "package": "/NewPack/Maps/NewTown",
              "xodr": "Maps/OpenDrive/NewTown.xodr", "world_partition": true } ],
  "catalogs": [ "Config/NewPack.Package.json", "Config/Vehicles.json" ],
  "files": { "Content/Paks/Linux/NewPackCarlaUnreal-Linux.utoc": "sha256:…", "…": "…" }
}
```

`base_release` binds the pack to the CARLA package it was built for: the packaged server carries the same string in `CarlaUnreal/BaseRelease` and refuses packs built against another release (see [FAQ](#faq)).

---

## Requirements

* __To author and build a pack__: a CARLA [source build](build_carla.md) with the Unreal Editor (the cooker is the editor binary) and the __release metadata__ of the CARLA package the pack targets: `<release>-release-metadata.tar.gz`, produced next to the package tarball by the `package` target (see [Building CARLA on Linux](build_linux_ue5.md#content-packs)). `CARLA_UNREAL_ENGINE_PATH` must point at the engine, as for any CARLA build.
* __To use a pack__: a CARLA package of that release, nothing else.

The tool is `Util/ContentPacks/carla-pack` (a shim around `carla_pack.py`, Python 3.8+, standard library only). It can be run from anywhere — it finds the project from its own location, `--project` overrides. `carla-pack <command> --help` lists every option; exit codes are `0` ok, `1` error, `2` usage error, `3` "pack not compatible with `--server`" (`inspect`).

---

## Authoring flow

```sh
carla-pack init NewPack
# existing CARLA towns: registered in place, OpenDRIVE / navigation / Traffic Manager files picked up
carla-pack add NewPack --map Town12
carla-pack add NewPack --map Town13
# your own content: author NewTown, props and vehicles in the CARLA editor, saved under the NewPack content root
carla-pack add NewPack --map Unreal/CarlaUnreal/Plugins/Packs/NewPack/Content/Maps/NewTown.umap --xodr NewTown.xodr --nav NewTown.bin
carla-pack add NewPack --props props.json --vehicles vehicles.json
carla-pack build NewPack --base Build/Package/carla-0.10.0-Linux-release-metadata.tar.gz
# -> Unreal/CarlaUnreal/Plugins/Packs/NewPack/Saved/CarlaPack/out/NewPack-1.0.0-carla-0.10.0-Linux.tar.gz
carla-pack inspect NewPack-1.0.0-carla-0.10.0-Linux.tar.gz --server /path/to/carla-package/Linux
```

The shortest useful pack is a set of CARLA's own towns that the package does not ship (`Town12`, `Town13`, …): `init`, one `add --map <Town>` per town (seconds), `build` (the cook, minutes). No editor session, nothing opened by hand.

### 1. Create the pack

```sh
carla-pack init NewPack [--carla-version 0.10.0] [--pack-version 1.0.0] [--description "..."]
```

creates `Unreal/CarlaUnreal/Plugins/Packs/NewPack/` with the plugin descriptor, an empty manifest, `Config/PluginSettings.ini` (tells the cook to stage the sidecar folders as loose files; `init` and `build` merge into an existing file — your sections, keys and comments stay, only the missing `+AdditionalNonUSFDirectories` lines are added to `[StageSettings]`) and the content folders `Content/{Maps, Maps/OpenDrive, Maps/Nav, Maps/TM, Config, Static, Static/Static, Blueprints}` (`Static/Static/` is the default home for props: it carries CARLA's `Static` semantic label, like the base's `Content/Carla/Static/Static/`). Names reserved by the engine or the project (`Game`, `Engine`, `Carla`, `Script`, the names of existing plugins, …) are refused: the pack name becomes a mount point. Pack names must be valid identifiers (letters, digits, `_`), they become the mount point `/NewPack/`. Use `--root` to keep packs elsewhere than `Plugins/Packs`.

Do not add the pack to `CarlaUnreal.uproject`: an enabled plugin would be cooked into the CARLA package itself. The `package` target checks this after its cook (`Unreal/Package/CheckNoPackLeak.cmake`): it fails when a package name under `/<Pack>/` appears in the base asset registry, or a `Plugins/Packs/<Pack>/` file was staged, for any pack under `Plugins/Packs/`. The same registry check is `carla-pack check-base <Releases/<release> | AssetRegistry.bin> [--pack NewPack]`.

### 2. Author the content in the editor

Open the CARLA editor (`cmake --build Build --target launch`). The Carla plugin mounts every pack under `Plugins/Packs/` when the editor starts, so the pack appears in the content browser as __NewPack Content__ (verified: the editor logs `Content pack plugin 'NewPack' mounted` and `/NewPack/` resolves in the asset registry). In a plain Unreal Editor session without the Carla plugin, or in a commandlet — the Carla plugin deliberately skips commandlets so a base cook never sees pack content — start the editor with `-EnablePlugins=NewPack` after temporarily setting `"ExplicitlyLoaded": false` in `NewPack.uplugin`, and set it back to `true` afterwards:

```sh
$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor Unreal/CarlaUnreal/CarlaUnreal.uproject -EnablePlugins=NewPack
```

Save everything the pack ships under the pack root, following CARLA's layout: maps in `Maps/`, static meshes in `Static/`, vehicle and walker blueprints in `Blueprints/`. __New maps live under the pack root__ (CARLA's own towns need no authoring at all: step 3 registers them in place). To start a new map from an existing one, duplicate it into the pack from the editor (content browser: right click > Duplicate, or File > Save As into `NewPack Content/Maps`) or with `carla-pack add --map <Town> --import`. Copying a `.umap` file from another content root does not work, because the file keeps its original package name (`/Game/...`) and, for World Partition maps, its actors reference that path. The usual CARLA authoring guides apply unchanged — [props](content_authoring_props.md), [vehicles](tuto_content_authoring_vehicles.md) — only the destination folder changes from `Content/Carla/...` to the pack root. Referencing CARLA's own assets (`/Game/Carla/...` materials, the vehicle base classes) is fine and keeps the pack small; they are not duplicated.

__Semantic tags.__ A mesh is labelled for semantic segmentation by the folder that follows `Static` in its path, exactly as in `Content/Carla/Static`: `/NewPack/Static/<Label>/SM_Thing` with `<Label>` one of `Building`, `Fence`, `Pedestrian`, `Pole`, `Other`, `Road`, `RoadLine`, `SideWalk`, `TrafficSign`, `Vegetation`, `Car`, `Wall`, `Sky`, `Ground`, `Bridge`, `RailTrack`, `GuardRail`, `TrafficLight`, `Static`, `Dynamic`, `Water`, `Terrain`, `Truck`, `Motorcycle`, `Bicycle`, `Bus`, `Rider`, `Train`, `Rock`, `Stone`, `Bush`. A mesh anywhere else gets no label, so `carla-pack add --props` refuses a catalog entry whose pack mesh is not under `Static/<Label>/` (`--allow-untagged` accepts it with a warning); `carla-pack add --help` prints the label list. Generic props go under `Static/Static/`, the folder `init` creates.

!!! Note
    `NewPack.uplugin` is authored with `"ExplicitlyLoaded": true`, the setting the packaged server needs to mount the pack on demand. The editor and the cooker, however, only mount a plugin that is *not* explicitly loaded, so `carla-pack build` temporarily rewrites the descriptor with `"ExplicitlyLoaded": false` for the duration of the cook and restores the original file afterwards (also when the cook fails or is interrupted). Nothing to edit by hand for the build; only the manual `-EnablePlugins` editor session described in step 2 needs the flag set to `false` for its duration.

For maps, generate the OpenDRIVE file, the pedestrian navigation (`.bin`) and optionally Traffic Manager data as for any CARLA map; the pack ships them as sidecar files. A map without an `.xodr` renders but has no map API (waypoints, Traffic Manager).

__Authoring from a script (commandlet).__ The same work can be done headless with the editor's Python (`UnrealEditor-Cmd CarlaUnreal.uproject -run=pythonscript -script=make_assets.py -EnablePlugins=NewPack -NullRHI -unattended`, with `"ExplicitlyLoaded": false` set for the session as above). Two things differ from an interactive session:

* the asset registry has not scanned the pack when the script starts, so `unreal.EditorAssetLibrary.load_asset('/NewPack/...')` reports "could not be found in the Asset Registry". Either scan the mount point first — `unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/NewPack'], force_rescan=True)` (`IAssetRegistry::ScanPathsSynchronous`, exposed to Blueprint/Python) — or load by object path: `unreal.load_object(None, '/NewPack/Static/Static/SM_Thing.SM_Thing')`;
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

Duplicating a map (`EditorAssetLibrary.duplicate_asset`) and opening the copy with `load_level` in the same commandlet process is fatal ("Old world ... not cleaned up by garbage collection"): duplicate in one session, edit in the next. `print()` output is swallowed by the commandlet; use `unreal.log_warning` or write a file.

### 3. Add maps, catalogs and assets

```sh
carla-pack add NewPack --map Town12                      # an existing CARLA town, by name
carla-pack add NewPack --map /Game/Carla/Maps/Town13/Town13   # or by package path
carla-pack add NewPack --map <path>/NewTown.umap --xodr NewTown.xodr [--nav NewTown.bin] [--tm <file|dir>] [--world-partition]
carla-pack add NewPack --props props.json | --vehicles vehicles.json | --walkers walkers.json | --blueprints blueprints.json
carla-pack add NewPack --asset SM_Thing.uasset --dest Static/Static/
```

* `--map` takes a map name (`Town12`, looked up under the project's and the pack's content folders), a package path (`/Game/Carla/Maps/Town12/Town12`) or a `.umap` file.
    * A __project map__ (anything under the project's `Content`, i.e. CARLA's own towns) is registered __in place__: the manifest names its `/Game/...` package, `build` puts it on the cook's map list and the server loads it from the pack's containers at that same path. Nothing is copied or opened, so it takes seconds for any town. The base release must not already ship the map (`build` checks the base asset registry and refuses, say, `Town10HD_Opt`).
    * A map __already under the pack's `Content`__ is registered as it is, under its own path (`/NewPack/Maps/NewTown`).
    * `--import` (or a map of another plugin, which has no path in the packaged server) makes a __copy under the pack root__ instead: `carla-pack` starts the headless editor (`UnrealEditor-Cmd`, from `CARLA_UNREAL_ENGINE_PATH` or `--engine`) with the pack enabled and performs a Save As of the map to `/NewPack/Maps/<same layout>`, which carries a World Partition map's external actors along; the copy is then yours to edit without touching CARLA's content. The editor's Save As also re-saves the source map; `carla-pack` puts the original bytes back and reports any other file the editor touched next to it. This is slow for large World Partition towns (Town12, 81k actors: 5 minutes; Town13, 359k actors: over 40 minutes and 30 GB of RAM) and `<pack>/Saved/CarlaPack/import-<Map>.log` has the editor output if it fails. `--copy` skips the editor and copies the file as-is, which is only right for a map whose package name was already rewritten.
    * The sidecar files are copied to `Maps/OpenDrive/<Map>.xodr`, `Maps/Nav/<Map>.bin` and `Maps/TM/<Map>/`. Without `--xodr`/`--nav`/`--tm` they are picked up from where CARLA keeps them: `<Maps>/OpenDrive/<Map>.xodr` or `<Map folder>/OpenDrive/<Map>.xodr`, `<Maps>/Nav/<Map>.bin`, `<Maps>/TM/<Map>.bin` or `<Map folder>/TM/`. World Partition maps are detected automatically from their `__ExternalActors__/…/<Map>` folder; `--world-partition` forces the flag.
* The catalog options validate the JSON against the shapes below, copy the file to `Content/Config/` under its canonical name and list it in the manifest. Asset paths that are neither under `/NewPack/` nor `/Game/` are reported: they cannot resolve once the pack is mounted.
* `--asset` copies any file or folder under `Content/<dest>`.

Re-running `add` for the same map or catalog replaces the previous entry.

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

---

## Build against a release

```sh
carla-pack build NewPack --base <release-metadata.tar.gz | Releases/<release>> \
    [--config Development|Shipping] [--platform Linux] \
    [--engine <UE root>] [--project <CarlaUnreal.uproject>] [--out <dir>] [--dry-run]
```

`--base` is the `<release>-release-metadata.tar.gz` published with the CARLA package (about 1 MB: the base asset registry) or the `Unreal/CarlaUnreal/Releases/<release>` directory of a source tree that built that package. The build extracts it and runs the Unreal cooker as a DLC cook based on it:

```
RunUAT.sh BuildCookRun -project=<uproject> -nocompileeditor -nop4 -skipbuild -cook -stage -pak -iostore \
    -clientconfig=Development -TargetPlatform=Linux -Platform=Linux -dlcname=<abs path>/NewPack.uplugin \
    -basedonreleaseversion=<release> -basedonreleaseversionroot=<...>/Releases \
    -stagingdirectory=<work>/Staged -AdditionalCookerOptions=-EnablePlugins=NewPack -DLCIncludeEngineContent
```

The cook takes seconds to minutes, not the hours of a full CARLA package: everything under `/NewPack/` is cooked, plus the project maps the manifest ships in place, which `build` puts on `-MapsToCook` (`--maps` adds more entries). The engine is found through `CARLA_UNREAL_ENGINE_PATH` (`--engine` overrides), the project is this checkout's `Unreal/CarlaUnreal/CarlaUnreal.uproject` (`--project` overrides). `--dry-run` prints the exact UAT command line and exits (a tarball `--base` is still extracted into the work dir; the pack sources are not touched); `--uat-arg=-Flag` appends flags to it. `-DLCIncludeEngineContent` is passed by default: it makes the cook include the `/Game` and `/Engine` assets the pack references that the base release did not cook (every existing CARLA town has some, its vegetation for one), instead of stopping with "Uncooked Engine or Game content ... is being referenced by DLC". `--no-base-content` drops the flag when you want that error, to keep a pack free of base-project assets.

The staged output is assembled into the pack layout (containers — `--rename` names them `NewPack-Linux.*` —, the cooked `AssetRegistry.bin`, the loose sidecar files, and the descriptor: your `.uplugin` with `ExplicitlyLoaded` and `CanContainContent` set to `true`, `EnabledByDefault` to `false` and an empty `Modules` list dropped; a pack that declares code modules is refused, everything else in the file is passed through), the manifest is completed (`base_release`, `engine`, `platform`, one SHA-256 per file) and the result is written to `--out` (default `Plugins/Packs/NewPack/Saved/CarlaPack/out/`):

```
out/NewPack/                                    # the pack folder
out/NewPack-1.0.0-carla-0.10.0-Linux.tar.gz     # what you distribute
```

Check it with `carla-pack inspect <tar.gz> [--json]` (manifest, maps, catalogs, file sizes) and `carla-pack verify <tar.gz|dir>` (every listed file must match its checksum and no unlisted file may be present — `carla-pack.json` is the only implicit one). `inspect --server <package>` also tells whether the pack fits that server (base release and platform), exit code `3` if not.

The same build is available as CMake targets once the package target has produced the base release: `cmake --build Build --target pack-NewPack` (and `pack-NewPack-dry-run`); `CARLA_CONTENT_PACKS_BASE` points them at a downloaded metadata tarball instead. `content-packs-test` runs the tool's unit tests.

!!! Note
    Development or Shipping server: both are fine, cooked data does not depend on the configuration. `--config` only selects which client configuration the cook is staged for.

---

## Install a pack

A CARLA package finds packs in three places, all read at start: `<package>/Linux/CarlaUnreal/Packs/*/` (next to the package's own `Content/Paks`), every directory passed as `-carla-packs=<dir>` on the server command line, and `$CARLA_PACKS`. `Packs/` is the only supported drop-in location — do not put packs under the package's `Plugins/` folder. A pack found there is mounted before the first episode: its maps show up in `client.get_available_maps()`, its vehicles, walkers and props in `world.get_blueprint_library()`.

### Drop-in

```sh
carla-pack install NewPack-1.0.0-carla-0.10.0-Linux.tar.gz --server /path/to/carla-package [--force]
```

verifies every checksum (refusing files the manifest does not list), the base release against the server's `BaseRelease` stamp and the platform, then extracts the pack into `<package>/Linux/CarlaUnreal/Packs/NewPack/`. `--server` accepts the package root (the directory that holds `Linux/`), the `Linux/` directory (where `CarlaUnreal.sh` is) or `Linux/CarlaUnreal` itself. `--force` replaces an installed pack and overrides the base-release check (do not, unless you know the cook settings match). Extracting the tarball into `CarlaUnreal/Packs/` by hand is equivalent, minus the verification.

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

`mount_content_pack` raises `RuntimeError` with the reason when the manifest is missing, the base release or platform does not match the server, or a pack of the same name is already mounted; it returns the pack's `ContentPackInfo`. The pack's vehicles, walkers and props are available immediately: mounting binds the new actor definitions to every live factory, so `world.get_blueprint_library()` lists them without an episode change. Loading one of the pack's __maps__ still requires `client.load_world(...)`. `unmount_content_pack` returns `True`, and raises `RuntimeError` when the pack is unknown or an object from it is still alive — load another map first.

---

## Limits

* A pack targets __one base release__: same CARLA package, same engine binary, same cook settings. A new CARLA release means rebuilding the pack (`carla-pack build` again, minutes) — never re-packaging CARLA.
* Building a pack needs the CARLA source build with the editor, exactly as authoring content did before. Using a pack needs only the package.
* Assets a pack references that are __not__ in the base release are cooked into the pack. That is correct, but a pack that references a whole asset library from another pack, or from a map that the base did not cook, gets correspondingly large. Two packs never share content with each other.
* Updating a pack whose content has been loaded needs a server restart; between episodes `unmount_content_pack` + `mount_content_pack` works.
* A pack ships whole maps and new actors: CARLA towns the package left out (`Town12`, `Town13`, …, imported with `add --map`) or maps of your own. Adding actors to a town the base package already ships (Town10, Town15, …) is out of scope for now.
* Pack names are global on a server: two packs called `NewPack` cannot be mounted at the same time.

---

## FAQ

__`base release mismatch: pack built against 'carla-0.10.0-Linux', server is 'carla-0.10.1-Linux'`__

The pack was cooked against another CARLA release and the server refuses it — cooked data is only valid against the asset registry it was cooked with. Rebuild the pack with that server's release metadata: `carla-pack build NewPack --base carla-0.10.1-Linux-release-metadata.tar.gz`. `carla-pack inspect <pack> --server <dir>` shows both strings; `<package>/BaseRelease` (and `<package>/Linux/CarlaUnreal/BaseRelease`) holds the server's.

__`server has no BaseRelease stamp`__

The package was built without content pack support (before the `package` target stamped the release). Rebuild the package, or pass `--force` to `install` only if you cooked the pack from that exact source tree and package.

__I updated a pack but the server still shows the old content__

Unreal cannot reload a package that is already in memory. If any map, mesh or blueprint of the pack has been loaded, restart the server after installing the new version. If nothing from it was loaded since the last episode change, `client.unmount_content_pack('NewPack')` followed by `client.mount_content_pack(...)` picks up the new files.

__How do I ship Town12 and Town13, which the package does not include?__

```sh
carla-pack init TownPack
carla-pack add TownPack --map Town12
carla-pack add TownPack --map Town13
carla-pack build TownPack --base carla-0.10.0-Linux-release-metadata.tar.gz
```

`add --map <Town>` registers the town in place (seconds) and picks up its OpenDRIVE and Traffic Manager files; the towns' walker navigation is part of the map itself. `build` cooks the two towns and what they reference that the base did not cook, about 2.5 GB. On the consumer side `carla-pack install` and a server restart, then `client.load_world('Town12')` as for any map.

__How big is a pack?__

Roughly the size of the cooked assets it ships, plus the OpenDRIVE/navigation files. A map with a few hundred unique meshes is typically in the hundreds of MB; a pack that only lists new vehicles built from existing CARLA meshes is a few MB. Everything the pack references from the base release costs nothing. `carla-pack inspect` lists the files with their sizes hashed in the manifest; the `.ucas` container is where the bulk lives.

__Can I mount a pack that is not under `Linux/CarlaUnreal/Packs`?__

Yes: `mount_content_pack` takes any folder on the server's file system that contains a `carla-pack.json`, and `-carla-packs=<dir>` / `$CARLA_PACKS` register extra directories to scan at start. A tarball must be extracted first.

__Does a pack work with a Shipping server?__

Yes. Cooked content is configuration independent; the same pack mounts on Development, Test and Shipping packages of the same release.
