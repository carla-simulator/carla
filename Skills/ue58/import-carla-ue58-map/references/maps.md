# Maps on UE 5.8: the four routes, and what actually happens

Read off `ue58-dev` HEAD `718efd7cc`. Claims marked **measured** come from a
controlled experiment on this machine: the existing 6-map package was deleted, a
1-map package was built from scratch, and both were compared with the engine's own
`UnrealPak -List` and against a live server.

## The taxonomy

| | A. cook into package | B. import to source | C. runtime OpenDRIVE | D. standalone package |
|---|---|---|---|---|
| Inputs | map in `Content` | `.fbx` + `.xodr` | `.xodr` | — |
| Tooling | `CARLA_MAPS_TO_COOK` + `package` | `Util/Tools/Import.py` | `generate_opendrive_world()` | none |
| Time | 1–2 h | minutes | seconds | — |
| Needs the engine | yes | yes (commandlets) | **no** | — |
| Persisted | in the `.pak` | in `Content` | **no** | — |
| Scenery | yes | yes | **roads only** | — |
| On ue58-dev | yes | yes | yes | **NO** |

Route D is UE4's `make package ARGS="--packages=Name1,Name2"`, which cooked named
content packages into separately distributable archives via
`-run=PrepareAssetsForCooking -PackageName=` and
`-run=GenerateTaggedMaterialsRegistry`. On ue58-dev:

- the CMake build has no `--packages` equivalent — the `package*` targets only pass
  `-MapsToCook=`,
- `PrepareAssetsForCooking`, `MoveAssets` and `LoadAssetMaterials` commandlets do
  still exist under `Plugins/Carla/Source/Carla/Commandlet/`, so the machinery is
  partly present and could be driven by hand,
- `GenerateTaggedMaterialsRegistry` is **gone** — no in-tree cross-package instance
  segmentation tagging,
- the docs are gone: `tuto_A_create_standalone.md`, `tuto_M_add_map_package.md`,
  `tuto_M_add_map_source.md`, `tuto_M_add_map_alternative.md`,
  `tuto_M_custom_map_overview.md`, `tuto_M_generate_map.md`,
  `tuto_M_custom_buildings.md`, `tuto_M_custom_road_painter.md`,
  `tuto_A_add_props.md`, `tuto_A_add_vehicle.md` are all linked from
  `maps_tutorials.md` / `custom_assets_tutorials.md` and none exist (38 dead `.md`
  links across 12 files in `Docs/`).

## What is in a package by default

With `CARLA_MAPS_TO_COOK` empty, no `-MapsToCook=` reaches UAT and it falls back to
`Unreal/CarlaUnreal/Config/DefaultGame.ini`:

```ini
bCookAll=False
bCookMapsOnly=False
+MapsToCook=(FilePath="/Game/Carla/Maps/Town10HD_Opt")
+MapsToCook=(FilePath="/Game/Carla/Maps/OpenDriveMap")
+MapsToCook=(FilePath="/Game/Carla/Maps/TestMaps/EmptyMap")
+MapsToCook=(FilePath="/Game/Carla/Maps/Mine_01")
+MapsToCook=(FilePath="/Game/Carla/Maps/Town15/Town15")
```

Five maps — **not** "every map"; `bCookAll=False`. And **Town15 is one of them**,
which cannot be cooked (573 unresolvable `MaterialInstanceDynamic` imports), so the
out-of-the-box `--target package` **fails on this branch**. An explicit
`CARLA_MAPS_TO_COOK` excluding Town15 is effectively mandatory.

The boot map is separate, from `DefaultEngine.ini`:

```ini
EditorStartupMap=/Game/Carla/Maps/Town10HD_Opt.Town10HD_Opt
GameDefaultMap=/Game/Carla/Maps/Town10HD_Opt.Town10HD_Opt
ServerDefaultMap=/Game/Carla/Maps/Town10HD_Opt.Town10HD_Opt
```

so a package with no map argument boots Town10HD_Opt — **measured** on both
packages.

`DirectoriesToAlwaysCook` additionally forces in licence plates, parked vehicles
and walker blueprints regardless of map scope.

## Adding and removing maps — measured

The experiment: same tree, same commit, only `CARLA_MAPS_TO_COOK` changed.

| | 6-map scope | 1-map scope (`Town10HD_Opt`) |
|---|---|---|
| package size | 33 GB (12 GB `.tar.gz`) | **7.4 GB** (6.6 GB `.tar.gz`) |
| `pakchunk0-Linux.ucas` | 9.6 GB | **5.7 GB** |
| cooked top-level `.umap` | `Town10HD_Opt`, `Town_C`, `OpenDriveMap` | **`Town10HD_Opt` only** |
| Town12 / Town13 WP cells | 2,068 / 6,194 entries | **none** |
| boot map | Town10HD_Opt | Town10HD_Opt |
| `load_world('Town10HD_Opt')` | OK | **OK** |
| `load_world('Town_C')` | OK | **fails** (`std::exception`) |
| `load_world('Town12')` | fails | fails |
| `get_available_maps()` | `[]` | `[]` |
| `generate_opendrive_world()` | not tested | **fails** (`std::exception`) |

So the cook scope is exactly the lever: a map that is not cooked cannot be loaded.

```bash
# add / remove: name the maps you want, then re-package
MAPS="Town10HD_Opt,Town12,Town13,Town_C" bash ../build-carla-ue58/scripts/build.sh configure
MAPS="Town10HD_Opt"                      bash ../build-carla-ue58/scripts/build.sh configure

# a map outside /Game/Carla/Maps needs its full package path
MAPS="/Game/Carla/Maps/Town10HD_Opt,/Game/MyPkg/Maps/MyTown" \
    bash ../build-carla-ue58/scripts/build.sh configure
```

`MAPS=` expands town names to `+`-separated package paths. Small maps are
`/Game/Carla/Maps/<T>`; large World Partition maps are `/Game/Carla/Maps/<T>/<T>`.
A `;` separator or a filesystem path cooks nothing while still reporting success.

### Two non-obvious dependencies

**`OpenDriveMap` is required by `generate_opendrive_world()`.** It is the *host
level* for the procedural road: a generated world reports its name as
`Carla/Maps/OpenDriveMap`. Measured — with `OpenDriveMap` excluded from the cook
scope, `generate_opendrive_world()` fails with a bare `std::exception` and the
server log shows it reaching for
`Content/Carla/Maps/Nav/OpenDriveMap.obj`; the same call against `-game` mode on
the same tree succeeds in 7.3 s with 160 spawn points. So dropping `OpenDriveMap`
to shrink a package silently disables Route C.

**The archive directory is cumulative.**
`Unreal/Package/RemoveUnrealPackageExtraFiles.cmake` says it outright: "The archive
step copies over a previous archive without deleting". Measured: the pre-existing
package contained `Carla/Maps/Town12/OpenDrive/Town12.xodr` even though
`DefaultGame.ini` (pristine) never lists it — a leftover from an earlier build. A
clean 1-map build has no such file. **Delete `Build/<preset>/Package/` before any
build whose contents you intend to reason about**, or you will be inspecting a
union of every build you ever ran.

## Staging — whether the road data ships

`DirectoriesToAlwaysStageAsUFS` in `DefaultGame.ini` (pristine on this branch):

```
Carla/Maps/OpenDrive        Carla/Maps/Nav          Carla/Maps/TM
Carla/Maps/Town15/OpenDrive Carla/Maps/Town15/Nav   Carla/Maps/Town15/TM
Carla/Config                Carla/Config/Mine_01
```

Content layout:

| Map kind | `.xodr` | TM cache |
|---|---|---|
| small (Town01–Town10, Town_C) | `Carla/Maps/OpenDrive/<T>.xodr` | `Carla/Maps/TM/<T>.bin` |
| large (Town11/12/13) | `Carla/Maps/<T>/OpenDrive/<T>.xodr` | `Carla/Maps/<T>/TM/<T>.bin` |
| Town15 | `Carla/Maps/Town15/OpenDrive/...` | `Carla/Maps/Town15/TM/...` |
| imported | `Content/<Pkg>/Maps/<T>/...` | `Content/<Pkg>/Maps/<T>/TM/<T>.bin` |

Measured on the clean 1-map package with `UnrealPak -List` — 80 UFS entries under
`Content/`, of which 20 `.xodr`:

- `Carla/Maps/OpenDrive/*.xodr` — **19 staged** (the whole shared dir, regardless
  of which maps were cooked)
- `Carla/Maps/Town15/OpenDrive/Town15.xodr` — **staged** (Town15 never cooks, but
  its loose data is listed and ships anyway)
- `Carla/Maps/Town12/OpenDrive/Town12.xodr`, `Town13/...` — **NOT staged**
- `Carla/Maps/Town12/TM/Town12.bin`, `Town13/TM/Town13.bin` — **staged**
- `Carla/Config/PostProcess/*` — **4 files staged**, so `Carla/Config` *is*
  recursive; the separate `Carla/Config/Mine_01` entry is redundant, not proof of
  non-recursion

So the real gap is narrower than "five missing entries": **only
`Carla/Maps/Town12/OpenDrive` and `Carla/Maps/Town13/OpenDrive` are missing.** The
TM subdirs and the post-process profiles already ship. Why the TM subdir stages
and the OpenDrive one does not is not established — the ini explains neither.

Consequence for a **cleanly built** package that cooks Town12 or Town13: it ships
with no OpenDRIVE for those towns. On the pre-existing package this was masked by
archive accumulation.

## Route C — `generate_opendrive_world`

```python
params = carla.OpendriveGenerationParameters(
    vertex_distance=2.0, max_road_length=50.0, wall_height=1.0,
    additional_width=0.6, smooth_junctions=True, enable_mesh_visibility=True)
world = client.generate_opendrive_world(open("MyTown.xodr").read(), params)
```

Measured against `-game` on Town01's `.xodr` (973 kB): generated in **7.3 s**, 160
spawn points, 160 topology edges, resulting map name `Carla/Maps/OpenDriveMap`.
Roads only — no buildings, props, vegetation. Not persisted.

## Route B — the import pipeline

Input layout under `Import/`:

```
Import/
├── MyTown.fbx           geometry; base name must match the .xodr
├── MyTown.xodr          road network
├── MyPackage.json       optional; generated when absent
└── roadpainter_decals.json   optional
```

`Util/Tools/Import.py`:

1. `get_packages_json_list(Import/)`, or `generate_json_package(...)` if none,
2. `generate_decals_file(...)`, `copy_roadpainter_config_files(package)`,
3. per batch (`--batch-size`, default 300 MB):
   `generate_import_setting_file` → `ImportAssets` → `LoadAssetMaterials` →
   `PrepareAssetsForCooking` → `MoveAssets` →
   `build_binary_for_navigation` (RecastBuilder) →
   `build_binary_for_tm` (`carla.Map(...).cook_in_memory_map()`) →
   `generate_package_file`.

Output:

```
Content/<Package>/Maps/<MapName>                     the .umap
Content/<Package>/Maps/<MapName>/TM/<MapName>.bin    Traffic Manager cache
Content/<Package>/Config/<Package>.Package.json      descriptor
```

Package path `/Game/<Package>/Maps/<MapName>` — **not** under `/Game/Carla/Maps`,
which drives the discovery problem below.

Prerequisites easy to miss: `CARLA_UNREAL_ENGINE_PATH` (read directly by
`invoke_commandlet`), an importable `carla` (module-scope import for the TM cache),
and `Util/DockerUtils/dist` with `RecastBuilder`.

### Two defects in the wrapper

**`Util/Tools/Import.sh` does not work.** `Util/Tools/Environment.sh` defines only
`log` and `fatal_error` and never sets `CARLA_BUILD_TOOLS_FOLDER`, which the last
line uses. Measured:

```
$ bash Util/Tools/Import.sh --help
parse-options: unrecognized option '--help'
/usr/bin/python3: can't open file '/Import.py': [Errno 2] No such file or directory
$ echo $?
0
```

Exits **0**, so a wrapper reads it as success. `--help` is also missing from the
`getopt` long list although the usage string advertises it. Call `Import.py`
directly — measured working: `python3 Util/Tools/Import.py --json-only` runs clean
on an empty `Import/`.

**`Import.py` ignores commandlet failures.** The POSIX branch of
`invoke_commandlet` uses `subprocess.call([...], shell=True)` and never checks the
result (the Windows branch uses `check_call`), so a failed commandlet lets the
import continue and appear to succeed.

## Map discovery — why an imported map may not load

`Plugins/Carla/Source/Carla/Game/CarlaStatics.cpp`:

```cpp
TArray<FString> UCarlaStatics::GetAllMapNames()          // filesystem only, no pak fallback
{
  PathList.Add(FPaths::ProjectContentDir());
  PathList.Append(GetAllPluginContentPaths());
  for (const FString &Path : PathList)
    IFileManager::Get().FindFilesRecursive(MapNameList, *Path, TEXT("*.umap"), true, false, false);
  MapNameList.RemoveAll([](const FString& Name) {
      return Name.Contains("TestMaps") || Name.Contains("OpenDriveMap") || Name.Contains("Sublevels");
  });
}

FString UCarlaStatics::FindMapPath(const FString &MapName)
{
  // ... the same FindFilesRecursive search, then ONE pak fallback:
  const FString ProjectMapPackage = FString::Printf(TEXT("/Game/Carla/Maps/%s"), *MapName);
  if (FPackageName::DoesPackageExist(ProjectMapPackage)) return ProjectMapPackage;
  return FString();
}
```

Measured, same tree, packaged vs `-game`:

| Call | `-game` | packaged |
|---|---|---|
| `get_available_maps()` | **29** | **`[]`** |
| `load_world('Town10HD_Opt')` | OK | OK |
| `load_world('Town_C')` | OK | OK *(when cooked)* |
| `load_world('Town12')` | OK | **fails** |

Every result follows from the code. `Town10HD_Opt` and `Town_C` sit at
`/Game/Carla/Maps/<Name>`; `Town12` is at `/Game/Carla/Maps/Town12/Town12`, which
the single fallback never tries. `GetAllMapNames` has no fallback at all, hence the
empty list. The 29 names in `-game` mode include Town15 sublevels and map-generator
templates, so it is discovery output rather than a list of playable towns.

**So:** an imported map at `/Game/<Package>/Maps/<Map>` is editor- and
`-game`-loadable but **not** package-loadable by name; the same is true of nested
large maps. To be package-loadable a map must sit exactly at
`/Game/Carla/Maps/<Name>`.
