# Prop import — pipeline, schema and gotchas

Detail layer for `import-carla-prop`: **what the importer does**, the **schema**
it writes, and the **P-lessons** (live failure modes).

> **Provenance.** Derived from the pipeline source in the checkout —
> `Carla/Actor/CarlaBlueprintRegistry.cpp`, `Carla/Actor/PropParameters.h`,
> `Carla/Actor/ActorBlueprintFunctionLibrary.cpp`, `Carla/Game/Tagger.{h,cpp}`,
> `LibCarla/source/carla/actors/BlueprintLibrary.cpp`,
> `Plugins/CarlaTools/Content/Python/add_prop_to_prop_factory.py` — plus
> `Docs/content_authoring_props.md`.

## What the importer does

`scripts/import_prop.py` resolves the input into a list of props, validates every
tag and name, then boots the editor **once**:

```
UE4Editor CarlaUE4.uproject -run=pythonscript -Script=scripts/editor/import_and_register.py
```

with the job in `$CARLA_PROP_SPEC`. Inside the editor, per prop:

1. **Import** — an `unreal.AssetImportTask` with an `FbxImportUI` configured for
   a static mesh (`combine_meshes` on, `auto_generate_collision` on,
   `convert_scene_unit` on), targeting `/Game/<Root>/Static/<Tag>/<Name>`.
2. **Read back** — `task.imported_object_paths` is what UE says it created; each
   is loaded and kept only if it really `isinstance(..., unreal.StaticMesh)`.
   Exactly one mesh is required, otherwise the prop fails with the list (P4).
3. **Measure** — `UStaticMesh::GetBoundingBox()` (BlueprintPure,
   `StaticMesh.h:1116`) gives the extents; `size` is derived from the largest
   dimension unless one was given.
4. **Register** — the mesh's `get_path_name()` goes into the registry JSON, and
   into `PropFactory`'s `DefinitionsMap`.

The host then confirms the artifacts itself: every registered path is resolved to
a `.uasset` on disk and checked against a size floor. The editor exiting 0 is not
evidence — it logs fatal import errors and carries on.

### Size thresholds

Largest dimension, in metres. `EPropSize` is defined purely by physical scale —
`PropParameters.h` describes the values as "smaller than a mailbox / size of a
mailbox / size of a human / size of a bus stop / size of a house or bigger".

| largest dimension | size |
|---|---|
| < 0.5 m | `tiny` |
| 0.5 – 1.2 m | `small` |
| 1.2 – 2.5 m | `medium` |
| 2.5 – 6 m | `big` |
| > 6 m | `huge` |

### Two roots

| | mesh | registry file | factory |
|---|---|---|---|
| default (stock) | `/Game/Carla/Static/<Tag>/<Name>` | `Content/Carla/Config/Default.Package.json` | yes |
| `--package NAME` | `/Game/NAME/Static/<Tag>/<Name>` | `Content/NAME/Config/NAME.Package.json` | no |

## How a prop becomes spawnable

At map load, `UCarlaBlueprintRegistry::LoadPropDefinitions` recursively scans
`Content/` for `*.Package.json`, reads each file's `props` array, loads the
`StaticMesh` at `path`, and hands the result to
`UActorBlueprintFunctionLibrary::MakePropDefinitions`, which produces the actor
id `static.prop.<name>` with a `size` attribute.

`LoadPropDefinitions` is `BlueprintCallable` with no C++ caller — `PropFactory`
is what calls it. So the **JSON entry is what makes a prop spawnable**, and the
`DefinitionsMap` entry is what puts it in the factory alongside the props
authored in the editor. This skill writes both.

`Carla/Config/Default.Package.json` is sorted **first**, and later packages
overwrite same-named entries — see P5.

## Registry schema

```json
{
  "props": [
    { "name": "Windmill",
      "path": "/Game/Carla/Static/Building/Windmill/Windmill.Windmill",
      "size": "Huge" }
  ],
  "maps": []
}
```

| field | meaning |
|---|---|
| `name` | the blueprint id becomes `static.prop.<name lowercased>` (P6) |
| `path` | object path `Package.Object`, as UE reported it — never derived from the FBX filename (P4) |
| `size` | `Tiny` `Small` `Medium` `Big` `Huge` → `EPropSize`; resolved through `FName`, so case-insensitive |

**Tags** — the folder names `ATagger::GetLabelByFolderName` actually recognises
(`Tagger.cpp:29-60`): `Bridge`, `Building`, `Bicycle`, `Bus`, `Car`, `Dynamic`,
`Fence`, `Ground`, `GuardRail`, `Motorcycle`, `Other`, `Pedestrian`, `Pole`,
`RailTrack`, `Rider`, `Road`, `RoadLine`, `SideWalk`, `Sky`, `Static`,
`Terrain`, `TrafficLight`, `TrafficSign`, `Train`, `Truck`, `Vegetation`,
`Wall`, `Water`.

This differs from the list in `Docs/tuto_A_add_props.md`, which also names
`Vehicles` and `Unlabeled`. Neither is matched above; both fall through to
`CityObjectLabel::None`.

### Which tag, empirically

All 98 props in the shipped `Default.Package.json`, by tag folder:

| folder | count | what is in it |
|---|---|---|
| `Dynamic` | 59 | bins, boxes, bags, cones, chairs, carts, helmets, hay bales |
| `Static` | 32 | ATM, benches, bus stop, fountain, mailbox, vending machine, plant pots |
| `Other` | 3 | DirtDebris01-03 |
| `Vegetation` | 3 | trees |
| `Building` | 1 | Kiosk_01 |

The working distinction is **Static vs Dynamic = is it fixed in place, or could
it be moved or knocked over**. A bench is bolted down; a traffic cone is not.

## Why the tag folder depth matters

`ATagger::GetLabelByPath` (`Tagger.h:85-90`) splits the asset path on `/` and
reads **index 4**:

```
/Game/Carla/Static/Building/Windmill/Windmill.Windmill
  0    1      2       3        4
```

So `/Game/<Root>/Static/<Tag>/<Name>/` is load-bearing at runtime. Depth *below*
index 4 is free, which is why a per-prop subfolder is safe. The comparison is
`FString ==`, i.e. `Stricmp` (`UnrealString.h:1117`), so casing does not matter —
spelling does.

`Content/Carla/Static/` also holds 13 directories that are **not** labels:
`CubeMaps`, `Decals`, `GenericMaterials`, `Hair`, `HDRi`, `Imported`, `LUTs`,
`Niagara`, `Particles`, `StreetLight`, `TestHair`, `TestWindowsParts`,
`VolumetricClouds`. Two are traps: `Imported/` is the obvious place to drop a new
asset, and `StreetLight/` looks semantic. Anything under either is labelled
`None`; a street light belongs in `Pole/`.

---

## P-lessons

### P1 — registration comes from the `.Package.json`, not the editor
- **Symptom:** looking for where to "register" the prop and finding `PropFactory`
  in the content browser.
- **Root cause:** `LoadPropDefinitions` reads the JSON at map load;
  `DefinitionsMap` is a separate, editor-authored set. Either alone will spawn.
- **Fix:** if `static.prop.<name>` is missing, check the registry file exists and
  that its `path` resolves to a real `.uasset`.

### P2 — a bad `tag` or `size` is not an error
- **Symptom:** the prop imports and spawns, but semantic segmentation labels it
  `None`, or its `size` attribute reads `unknown`.
- **Root cause:** the tag is a folder name matched by `GetLabelByFolderName`,
  which returns `None` for anything unrecognised; an unmatched size becomes
  `EPropSize::INVALID`. Nothing raises.
- **Fix:** `import_prop.py` validates tags against the tagger's own list before
  the editor boots, and measures `size` rather than accepting a guess.
  `verify_prop.py` warns on `size=unknown`.

### P3 — two places a prop can live; pick by where it belongs
- **Stock content set** (default): mesh into `/Game/Carla/Static/<Tag>/`, entry in
  `Default.Package.json`, entry in `PropFactory`. Matches
  `Docs/content_authoring_props.md`; the prop sits alongside the shipped props.
  Mutates shared content (P7).
- **Own package** (`--package NAME`): a self-contained `/Game/NAME/` tree plus its
  own `.Package.json`, exportable as a standalone tarball via
  [[package-carla-ue4]]. Touches nothing shared, and nothing in the factory.

### P4 — one FBX does not always mean one asset
- **Symptom:** the registry points at `<Stem>.<Stem>` but the folder holds
  `<Stem>_<NodeName>.uasset` files instead, and the prop lists without ever
  spawning.
- **Root cause:** with `combine_meshes` off, UE imports each FBX mesh node as its
  own static mesh named after the node.
- **Fix:** import with `combine_meshes` on and read back what was created. If
  several meshes survive, refuse and list them rather than register half a model.
  Name collision hulls `UCX_<MeshName>` so they are absorbed.

### P5 — same-named props silently override each other
- **Symptom:** a prop resolves to a mesh from a different package.
- **Root cause:** `LoadPropDefinitions` sorts the registry files with
  `Default.Package.json` first, then applies each in turn, replacing any entry
  whose `name` already exists. Last file wins.
- **Fix:** `import_prop.py` rejects a batch containing two meshes that would
  produce the same prop name. Across packages, namespace the names.

### P6 — blueprint ids are lowercase
- **Symptom:** `bp_lib.find('static.prop.Windmill')` raises, on a completely
  successful import.
- **Root cause:** `FillIdAndTags` builds both id and tags with `.ToLower()`
  (`ActorBlueprintFunctionLibrary.cpp:207-208`), and `BlueprintLibrary::Find` is
  an exact `std::map` lookup with no case folding (`BlueprintLibrary.cpp:67-70`).
  The docs say the same: `static.prop.<name_lower_case>`.
- **Fix:** `verify_prop.py` lowercases before filtering and finding. The `name` in
  the registry keeps its original casing; only the id is folded.

### P7 — `Content/Carla` is often shared between checkouts
- **Symptom:** a prop imported from one worktree appears in all of them; the
  content repo shows unexpected modifications.
- **Root cause:** `Unreal/CarlaUE4/Content/Carla` is commonly a symlink to one
  content clone reused by every worktree. The stock route writes the mesh,
  `Default.Package.json` and `PropFactory.uasset` there — and that last one is a
  binary that cannot be diffed or merged.
- **Fix:** `import_prop.py` and `check_env.sh` both report the resolved target
  when it lies outside the checkout. `--package NAME` keeps an import
  self-contained; `--no-factory` avoids the binary write.

---

## Verifying

Spawn the prop to test it:

```bash
python3 scripts/verify_prop.py --name <Name>
```

It filters, finds, spawns and destroys, mirroring `Docs/content_authoring_props.md`.

The default `-nullrhi` mode of [[run-carla-server]] is sufficient; props need no
rendering to spawn. Use a windowed server with `verify_prop.py --keep` to look at
the prop.

## Removing a prop

There is no uninstall step. Delete the entry from the registry JSON and remove
the asset folder under `Content/<Root>/Static/<Tag>/<Name>/`; the registry
rebuilds from the files on the next map load. Also remove the entry from
`PropFactory`'s `DefinitionsMap` in the editor.
