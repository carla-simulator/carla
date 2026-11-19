---
name: create-carla-content-pack
description: Builds a CARLA content pack on a UE 5.8 ue58-dev source build — a tarball of maps, props, vehicles, walkers or blueprints cooked as DLC against one CARLA package, which installs into that package without repackaging it. Covers packs of existing towns the package leaves out (carla-pack create), packs of your own content (init, author or import FBX meshes into the pack, add catalogs and sidecars, build), choosing the right base release, and checking the tarball before shipping it. Use when the user asks to "make a content pack", "pack these towns", "build a pack with my props/vehicles/map", "carla-pack create/build", "ship extra maps for a CARLA package", or a pack build fails.
license: MIT
compatibility: Linux. Needs a ue58-dev checkout with the engine and the carla-unreal-editor target built (the cooker is UnrealEditor-Cmd) and the release metadata of the target package (the package target writes it). Python 3.8+. Verified on ue58-dev 1ad2c15c9, engine 5.8, against a Shipping package of CARLA 1.0.
metadata:
  group: ue58
  requires: package-carla-ue58
  prerequisites: scripts/check_env.sh
  reference: references/packs.md
---

# Create a CARLA content pack

> **Paths.** `scripts/…` and `references/…` below are relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix them with its absolute path or the command is not found.

The deliverable is a checked tarball, `<Pack>-<version>-<base_release>.tar.gz`, under `Unreal/CarlaUnreal/Plugins/Packs/<Pack>/Saved/CarlaPack/out/`. Installing it is the [[install-carla-content-pack]] skill; mounting it into a running server is [[manage-content-packs]]; the official AdditionalMaps pack is [[get-carla-additional-maps]].

`scripts/pack.sh` wraps `Util/ContentPacks/carla_pack.py` (`carla-pack`). It always passes an explicit `--base` and checks the result, because both defaults fail in ordinary checkouts.

## The one decision: which base

A pack is cooked against the asset registry of **one** CARLA package, its release metadata (`<release>-release-metadata.tar.gz`, ~2 MB). Cook against the metadata of the package the pack will be installed into, and nothing else.

- **Your own package:** the tarball the `package` target wrote next to it, `Build/<preset>/Package/carla-<version>-<platform>-release-metadata.tar.gz`. This is `pack.sh`'s default (newest one in `Build/$CARLA_PRESET/Package/`).
- **A downloaded package:** the metadata published next to it (`CARLA_1.0_release-metadata.tar.gz`, `CARLA_UE5_Weekly_release-metadata.tar.gz`). Pass it with `CARLA_PACK_BASE=<file>`.

Every 1.0 build is stamped `carla-1.0-Linux`: the release, the weekly and your local packages. `carla-pack install` and the server compare only that string, so a pack cooked against the wrong 1.0 build installs and mounts without a word, and references assets its base had and the target may lack. Nothing downstream catches a wrong base; this step is the only place it is chosen.

## Instructions

```
Progress:
- [ ] Step 1: bash scripts/check_env.sh — clear FAILs, read the base it picked
- [ ] Step 2: Make the pack: A (existing towns) or B (your own content)
- [ ] Step 3: pack.sh check — verify, containers, sidecars, clashes with other packs
- [ ] (opt-in) install it: install-carla-content-pack
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
bash scripts/pack.sh bases        # every candidate base, the default first
```

### Step 2A: Existing CARLA towns

```bash
bash scripts/pack.sh towns MyTowns Town03_Opt Town11            # or --maps-file maps.json
```

`create` = `init` + `add --map` per town + `build`. Towns register **in place** at `/Game/Carla/Maps/...` in a second each (no editor), with their OpenDRIVE, navigation and Traffic Manager files picked up automatically. Only towns the base package does not ship qualify; `build` refuses the others. Measured: Town01_Opt, 49 s and 615 MB; Town13, 8 min and 1.59 GB.

`create` only **adds**. Running it again with a shorter list keeps every map registered before, and there is no `remove` command. To drop a map, delete `Plugins/Packs/<Pack>/` and create again (registering towns is instant).

A refused map still leaves `Plugins/Packs/<Pack>/` behind with the map registered, because `init` and `add` run before the base check, even with `--dry-run`. Delete the folder before retrying.

### Step 2B: Your own content

```bash
source scripts/env.sh
python3 "$CARLA_PACK_TOOL" init MyPack --description "..."
# meshes: FBX -> /MyPack/Static/<Label>/<Name>/, registered as static.prop.<name>
python3 scripts/pack_editor.py import-meshes MyPack ./fbx/ --label Static [--scale 0.01] [--size Small]
python3 "$CARLA_PACK_TOOL" add MyPack --vehicles vehicles.json      # / --walkers / --blueprints
python3 "$CARLA_PACK_TOOL" add MyPack --map <Pack>/Content/Maps/NewTown.umap --xodr NewTown.xodr --nav NewTown.bin
bash scripts/pack.sh build MyPack
```

- **Where content lives.** Everything the pack ships is saved under `Plugins/Packs/<Pack>/Content/` (mount point `/<Pack>/`). A mesh's semantic label is the folder after `Static/`: `/<Pack>/Static/Building/...` is tag 3, `/<Pack>/Static/Static/...` tag 20.
- **`pack_editor.py import-meshes`** runs the import-carla-ue58-prop skill's editor script in `UnrealEditor-Cmd` with `-EnablePlugins=<Pack>`, flipping the pack's `ExplicitlyLoaded` to false for the run and restoring it afterwards. It then merges the meshes into `Config/<Pack>.Package.json` through `carla-pack add --props`. Check the printed dimensions: an FBX without unit metadata arrives 100x too big (measured 300 m for a 3 m piece); re-run with `--scale 0.01`.
- **`pack_editor.py run <Pack> script.py`** runs any editor Python with the pack mounted. In that commandlet, scan the pack before loading from it (`AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/<Pack>'], True)`) and never `spawn_actor_from_object` a pack asset (it crashes); details in [references/packs.md](references/packs.md).
- **Catalog shapes** (props, vehicles, walkers, blueprints) and sidecar files: [references/packs.md](references/packs.md). A vehicle entry may point at a CARLA class (`/Game/Carla/Blueprints/Vehicles/...`): the pack is then a few MB.
- **Interactive editor:** the Carla plugin mounts every pack in `Plugins/Packs/` when the full editor opens (content browser: *<Pack> Content*). Save, then close the editor before `build`.
- **Never** add the pack to `CarlaUnreal.uproject`: it would be cooked into the package, and the `package` target then fails its leak check.

### Step 3: Check the tarball

`towns` and `build` run this themselves; run it again against the target package before handing the tarball over:

```bash
bash scripts/pack.sh check MyPack --server /path/to/Carla-1.0-Linux-Shipping
```

It fails on checksum errors, a missing `.pak`/`.utoc`/`.ucas` (the cook produced nothing), an empty `base_release`, a map without OpenDRIVE, or a pack with neither maps nor catalogs. It warns on a map without navigation, and on another pack in `Plugins/Packs/` that ships the same town or a map of the same name: both mount, and only one copy can be loaded. `add --import` keeps the town's name, and while the copy is in `Plugins/Packs/` every other pack's `add`/`create` of that town by name (the `package-additional-maps` target included) fails with `2 maps are called <Town>`: pass `/Game/Carla/Maps/<Town>` there. Also, to ship an edited copy next to the original, save it under a new name in the editor (or a `pack_editor.py run` script) and add that map instead.

`pack.sh dry-run <Pack>` prints the UAT command without cooking. `pack.sh tool <args>` runs `carla-pack` with any other subcommand. CMake has the same build as `cmake --build Build/<preset> --target pack-<Pack>` (Development, base `Unreal/CarlaUnreal/Releases/<release>`, output `Build/<preset>/Packs/`). The target appears without a reconfigure.

## Examples

**Example 1: "pack Town03 and Town11 for our package"**

`check_env.sh` → `pack.sh towns OurTowns Town03_Opt Town11` → the check prints the tarball path, its size and both maps with their sidecars. Stop there unless asked to install.

**Example 2: "make a pack with these traffic cones and our red Lincoln variant"**

`init ConePack` → `pack_editor.py import-meshes ConePack ./cones --label Static` → `add ConePack --vehicles vehicles.json` (make/model of your choice, class `/Game/Carla/Blueprints/Vehicles/LincolnMKZ/BP_LincolnMKZ.BP_LincolnMKZ_C`) → `pack.sh build ConePack`. Measured: 27 s, 1.6 MB, and on the server `static.prop.<name>` and `vehicle.<make>.<model>` spawn and drive.

**Example 3: "the pack is for the official 1.0 release"**

Download `CARLA_1.0_release-metadata.tar.gz` from the same folder as the release, then `CARLA_PACK_BASE=$PWD/CARLA_1.0_release-metadata.tar.gz bash scripts/pack.sh towns ...`. Cooking against your local package's metadata instead installs fine and is wrong.

## Troubleshooting

**`several base releases found, pass --base to pick one`**: bare `carla-pack` without `--base` sees every `*-release-metadata.tar.gz` under `Build/` and every `Releases/*`. Use `pack.sh`, or pass `--base`.

**`the base release already ships /Game/Carla/Maps/<Town>`**: that town is cooked into the package; a pack cannot replace it. Delete `Plugins/Packs/<Pack>/` (the refused map is already registered) and create without it.

**`'<Name>' is a reserved name`**: pick another pack name (letters, digits, `_`; not `Game`, `Engine`, `Carla` or an existing plugin).

**`Uncooked Engine or Game content ... is being referenced by DLC`**: only with `--no-base-content`. Drop that flag, or move the referenced assets into the pack.

**The pack installs but its town is broken on the target package**: check the base. The pack's `base_release` matches any build of the same version; see "The one decision" above.

More failure modes, the manifest format and the build internals: [references/packs.md](references/packs.md).

## Outputs

- `Plugins/Packs/<Pack>/` — the pack sources (gitignored): `<Pack>.uplugin`, `carla-pack.json`, `Config/PluginSettings.ini`, `Content/`.
- `Plugins/Packs/<Pack>/Saved/CarlaPack/out/<Pack>-<version>-<release>.tar.gz` and the unpacked folder beside it.
