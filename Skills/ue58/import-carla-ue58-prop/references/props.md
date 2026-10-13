# Props on UE 5.8

Everything here was measured on `ue58-dev` HEAD `718efd7cc`, engine 5.8.0,
CARLA 0.10.0: a prop was imported, registered, spawned in the editor and through
the API, then reverted.

## Registration moved between UE4 and ue58

| | UE4 | ue58-dev |
|---|---|---|
| Declared in | any `*.Package.json` under `Content/` | `Content/Carla/Config/PropParameters.json` |
| JSON key | `"props"` | `"Props"` |
| Fields | `name` / `path` / `size` | `Name` / `Mesh` / `Size` |
| Loaded by | `UCarlaBlueprintRegistry::LoadPropDefinitions` | `APropActorFactory` → `LoadPropParametersArrayFromFile` |
| Status | live | **`LoadPropDefinitions` has zero callers — dead code** |

`LoadPropDefinitions` still compiles and still walks `FPaths::ProjectContentDir()`
recursively for `*.Package.json`, merging Default first. Nothing calls it. Verified
twice: `grep -rn LoadPropDefinitions` over the plugin returns only the definition
and its header declaration, and a hand-written
`Content/SkillTest/Config/SkillTest.Package.json` declaring a prop left the
`static.prop.*` count unchanged at 83.

The live path is `PropActorFactory.cpp:24`:

```cpp
LoadPropParametersArrayFromFile("PropParameters.json", PropsParams);
// ... seeds MeshCacheByPath from Params.Mesh ...
UActorBlueprintFunctionLibrary::MakePropDefinitions(PropsParams, Definitions);
```

Entry shape:

```json
{ "Name": "ATM", "Mesh": "/Game/Carla/Static/Static/SM_Atm.SM_Atm", "Size": "Medium" }
```

`Size` is one of `Tiny`, `Small`, `Medium`, `Big`. (`Default.Package.json` — the
now-unused UE4-era file — also contains a lowercase `"big"`, which is why the
loader is case-insensitive there; write the capitalised form.)

Adding one entry took the count 82 → 83 in the file and 83 → 84 in the blueprint
library, so exactly one built-in prop comes from somewhere other than this file.

## What the factory builds

`MakePropDefinition` synthesises the blueprint. Observed for an imported prop:

```
id   : static.prop.testdoorprop
tags : ['testdoorprop', 'prop', 'static']
       mesh_path      = /Game/Carla/Static/Static/TestDoorProp/TestDoorProp.TestDoorProp
       size           = medium
       role_name      = prop
       ros_name       = static.prop.testdoorprop
       ros_topic_name = static.prop.testdoorprop
```

So the blueprint id is `static.prop.<Name lowercased>`, and ROS naming comes for
free — the prop publishes under `rt/carla/static.prop.<name>` on a `-ros2` server.

`APropActorFactory::SpawnActor` spawns an `AStaticMeshActor` from
`ActorDescription.Class` with `ESpawnActorCollisionHandlingMethod::AlwaysSpawn`,
pointing it at the cached mesh. An imported prop is served by exactly the same path
as a built-in one.

**Definitions are read once at startup.** A prop registered against a running
server does not appear until it restarts.

## FBX unit scale — the trap that bites first

Measured on `Content/Carla/HoudiniEngine/Pieces/Door.fbx`:

| `--scale` | measured dimensions | derived size |
|---|---|---|
| `1.0` | 0.43 × 250 × 400 m | `Big` |
| `0.01` | 0.43 × 2.5 × 4.0 m | `Medium` |

At 1.0 it imported, registered and spawned without a single warning from CARLA or
the engine — a 400-metre door standing in Town10HD_Opt. The FBX carries no usable
unit metadata, so `import_uniform_scale` is the only correction.

Consequences: `Size` classification is meaningless until the scale is right, and a
wrong scale is *silent*. This skill prints the measured dimensions on every import
and warns above 30 m.

## Destination path matters for packaging

Keep props under `/Game/Carla/...`. An asset elsewhere in `Content/` imports and
works in the editor, but a packaged server resolves content through a single
hardcoded fallback shape — the same limitation that stops
`/Game/Carla/Maps/Town12/Town12` loading from a package. The skill's `list` mode
flags registered props whose `Mesh` is outside `/Game/Carla`.

## Semantic segmentation is not wired up

An imported prop comes back with `semantic_tags == []`. UE4's packaging ran a
`GenerateTaggedMaterialsRegistry` commandlet across content packages to assign
these; **that commandlet does not exist in ue58** (`PrepareAssetsForCooking`,
`MoveAssets` and `LoadAssetMaterials` survive; it does not). So imported props are
invisible to semantic and instance segmentation until their materials are tagged by
hand.

## Driving the editor headlessly

```bash
UnrealEditor CarlaUnreal.uproject -run=pythonscript -Script=<script.py> \
    -unattended -nullrhi -stdout
```

Three behaviours to design around:

1. **`print()` and `unreal.log()` do not reach the editor log.** Confirmed on 5.8:
   a script whose only output was `unreal.log()` produced
   `LogPythonScriptCommandlet: Python script executed successfully` and not one
   line of its own. Write results to a file.
2. **Errors *do* reach the log** as `LogPython: Error: …` with a full traceback,
   and the commandlet exits **255**. Exit 0 on success. So the exit code is a
   usable pass/fail signal even though stdout is not.
3. **FBX import goes through Interchange** on 5.8 — the log shows
   `LogInterchangeEngine: Interchange start importing source [...]`. The legacy
   `AssetImportTask` + `FbxImportUI` objects still drive it correctly.

`-nullrhi` is safe here: no camera sensor is involved. (It is *not* safe for a
running server — see [[run-carla-ue58-server]].)

### API changes hit on 5.8

- `unreal.EditorStaticMeshLibrary.get_number_triangles` **no longer exists**
  (`AttributeError`); moved to the StaticMesh editor subsystem. Bounds come from
  `StaticMesh.get_bounds().box_extent`.
- `unreal.EditorAssetLibrary.list_assets(path, recursive=, include_folder=)` and
  `unreal.load_asset` behave as on UE4.

## Why not `Util/Tools/Import.py`

It is a map pipeline with props bolted on:

- `generate_json_package` only auto-detects **maps** — "A map is a .fbx and a
  .xodr with the same name" — and hardcodes `{'maps': json_maps, 'props': []}`.
  Props therefore need a hand-written package JSON.
- it registers into `*.Package.json`, the dead path.
- `invoke_commandlet` uses `subprocess.call(..., shell=True)` on POSIX and never
  checks the result, so a failed commandlet is invisible (the Windows branch uses
  `check_call`).
- its wrapper `Util/Tools/Import.sh` referenced `${CARLA_BUILD_TOOLS_FOLDER}`,
  which nothing set, so it ran `python3 /Import.py` and exited 2.

## End-to-end evidence

```
plan    -> /Game/Carla/Static/Static/TestDoorProp, blueprint static.prop.testdoorprop
import  --scale 0.01
        PASS 0.428 x 2.5 x 4.0 m -> size Medium, lods 1, materials 3
        registered 82 -> 83 props
verify --spawn (after restart)
        PASS registered, asset on disk, in blueprint library with mesh_path
        PASS spawned id=25 bbox 2.5 x 0.4 x 4.0 m
        WARN semantic_tags empty
revert --yes
        unregistered (82 left), asset directory deleted
```
