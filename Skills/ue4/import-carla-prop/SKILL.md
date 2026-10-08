---
name: import-carla-prop
description: Imports FBX meshes into a CARLA source build as spawnable static.prop.* blueprints — pass a directory whose subdirectories name the semantic tags, or a single FBX with its tag, and the props are imported into content, registered, added to the prop factory and verified by spawning. Use when the user asks to "add a prop to CARLA", "import an FBX as a prop", "add street furniture/benches/signs", or needs a custom mesh spawnable from the Python API.
license: MIT
compatibility: Linux. Requires a built CarlaUnreal UE 4.26 fork (UE4_ROOT) and a built CARLA checkout — this skill checks for both and defers to build-carla-ue4, it never builds. Importing needs no Python environment; verifying needs one that imports `carla`. A whole directory is one editor boot, a couple of minutes.
metadata:
  group: ue4
  requires: build-carla-ue4
  prerequisites: scripts/check_env.sh
  reference: references/props.md
---

# Import a prop into CARLA

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

```bash
# a directory — each subdirectory names the semantic tag for what is inside
python3 scripts/import_prop.py ~/meshes/props

# a single file — state its tag
python3 scripts/import_prop.py ~/meshes/Windmill.fbx --tag Building
```

Each mesh is imported into `/Game/Carla/Static/<Tag>/<Name>`, registered in
`Content/Carla/Config/Default.Package.json`, and added to `PropFactory` — after
which it spawns exactly like a native CARLA prop:

```py
barrier_bp = bp_lib.find('static.prop.policebarrier')
for spawn_loc in spawn_locations:
    world.spawn_actor(barrier_bp, spawn_loc)
```

> Pipeline detail, schema and the P-lessons: [`references/props.md`](references/props.md).

## Instructions

```
Prop Import Progress:
- [ ] Step 1: Check prerequisites — hand off to build-carla-ue4 on a build FAIL
- [ ] Step 2: Import (one command, one editor boot)
- [ ] Step 3: Verify the props spawn on a running server
- [ ] (opt-in) Export as a standalone package
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

What must exist is a **runnable editor for this project**: `UE4_ROOT`, the
checkout, a built `CarlaUE4Editor`, and `PythonScriptPlugin`.

**This skill never builds.** On a FAIL naming UE4, `CarlaUE4Editor` or missing
content, stop and run [[build-carla-ue4]] against that checkout, then come back.

The check also reports whether `Content/Carla` is a **symlink to a shared content
clone** — if so, everything below lands in a tree every other checkout sees.
`--package NAME` keeps an import out of it.

### Step 2: Import

The directory form takes each prop's tag from the first level below the path you
give — the same shape as the destination:

```
~/meshes/props/
├── Building/Windmill.fbx
├── Static/Bench_Modern.fbx
└── Dynamic/Crate.fbx
```

Any directory works. An FBX loose in the root, or under a directory that is not a
tag, is an error before the editor boots.

**`size` is measured, not asked.** `EPropSize` is defined by physical scale, so
the importer reads the mesh's bounding box and reports it:

```
[import] OK    static.prop.windmill    3.2 x 3.1 x 12.4 m    size=huge (measured)    412 KB
```

Read the dimensions: a mesh exported at the wrong scale shows up here. `--size`
overrides.

**`tag` cannot be measured and is never defaulted.** It sets the semantic
segmentation label, and a wrong one is labelled `None` with nothing raised. When
filling it in from a request rather than from directory names:

| the object is… | tag |
|---|---|
| bolted-down street furniture — bench, ATM, bus stop, fountain, mailbox, vending machine | `Static` |
| portable or knockable — bin, box, bag, cone, chair, cart, barrier | `Dynamic` |
| a structure with mass — kiosk, shed, windmill, tower | `Building` |
| a plant | `Vegetation` |
| a post — street light, sign post, bollard | `Pole` |
| road-surface debris | `Other` |

Say which tag you chose and why in one line, so the user can correct it before
anything is written. When the object is ambiguous, ask.

Other flags:

| flag | why |
|---|---|
| `--name` | prop name for a single file; defaults to the FBX stem |
| `--package NAME` | import into its own `/Game/NAME/` tree — self-contained, exportable, touches no shared content, no factory entry |
| `--no-factory` | skip `PropFactory`; the registry entry alone still makes the prop spawnable |
| `--mesh` | nominate one mesh when a multi-mesh FBX yields several |
| `--verbose` | stream the editor log instead of capturing it |

The importer stats every `.uasset` at the path it registered and fails if one is
missing or too small to hold geometry.

### Step 3: Verify they spawn

Start a server ([[run-carla-server]] — the default `-nullrhi` mode is enough,
props need no rendering to spawn):

```bash
bash ../run-carla-server/scripts/run_server.sh Town10HD 2000 >/tmp/carla_server.log 2>&1 &
until nc -z 127.0.0.1 2000; do sleep 1; done
python3 scripts/verify_prop.py --name Windmill
pkill -x UE4Editor          # clean stop — never pkill -f (run-carla-server S3)
```

`verify_prop.py` filters, finds, spawns and destroys each prop. Blueprint ids are
**lowercase**: `static.prop.windmill`.

Add `--keep` to leave the props standing, paired with a windowed server.

**This is where the skill stops by default.**

### Optional: export as a standalone package

Only for `--package` imports, and only when the request asks to distribute the
props — the stock route has nothing separate to export:

```bash
python3 scripts/import_prop.py ~/meshes/props --package MyProps
PACKAGES=MyProps bash ../package-carla-ue4/scripts/package.sh
# -> Dist/MyProps_<tag>.tar.gz, installable into a release via ImportAssets.sh
```

## Examples

**Example 1: one prop**

User says: "add this windmill FBX to CARLA as a prop"

```bash
bash scripts/check_env.sh
python3 scripts/import_prop.py ~/models/Windmill.fbx --tag Building
python3 scripts/verify_prop.py --name Windmill
```
Result: `static.prop.windmill` spawnable from any `carla.Client`.

**Example 2: a directory of props**

User says: "import all the street furniture in this folder"

Arrange it by tag once, then import in a single boot:

```bash
python3 scripts/import_prop.py ~/meshes/street_furniture
python3 scripts/verify_prop.py --package Carla     # every prop in the stock set
```

**Example 3: import and distribute (opt-in)**

User says: "import these props and give me a package I can drop into a release"

Use `--package MyProps`, verify with `--package MyProps`, then
`PACKAGES=MyProps bash ../package-carla-ue4/scripts/package.sh`.

## Troubleshooting

**Error: `CarlaUE4Editor not built` / `UE4 not built`**
Cause: the checkout is not built; this skill has no build step.
Solution: run [[build-carla-ue4]] against that checkout, then re-run the check.

**Error: `these FBX files sit directly in <dir> with no tag directory`**
Cause: the directory form needs `<dir>/<Tag>/mesh.fbx`.
Solution: move each mesh into a subdirectory named for its semantic tag, or
import it on its own with `--tag`.

**Error: `the editor produced no result`**
Cause: the editor died before finishing.
Solution: re-run with `--verbose` and read the **first** error, not the last.

**Error: `the import produced no StaticMesh`**
Cause: the FBX holds no static mesh geometry — skeletal-only, curves, or an
unreadable FBX version.
Solution: re-export as a static mesh FBX, with the origin of geometry at the
scene origin: that becomes the prop's anchor point.

**Error: `the import produced N static meshes and none was nominated`**
Cause: a multi-node FBX the importer could not merge, or `--no-combine`.
Solution: pass `--mesh <object path>` to choose one, or re-export as a single
mesh. Name collision hulls `UCX_<MeshName>` so they are absorbed rather than
imported as their own asset (P4).

**Error: `the editor reported success but no asset on disk at …`**
Cause: the import was rolled back or written elsewhere; the registry entry would
point at nothing.
Solution: re-run with `--verbose`; the first FBX error explains it.

**Error: `static.prop.<name>` missing from the blueprint library**
Cause: the registry entry is missing, or its `path` points at an asset that does
not exist (P1, P4).
Solution: check the registry file the importer printed. Note the id is
**lowercased** (P6).

**Error: the prop spawns but semantic segmentation labels it `None`**
Cause: a tag `ATagger::GetLabelByFolderName` does not recognise (P2).
Solution: re-import under a tag from the table above.

## Outputs

- `Content/<Root>/Static/<Tag>/<Name>/<Mesh>.uasset` — the imported mesh
  (`<Root>` is `Carla` by default, or your `--package` name).
- `Content/Carla/Config/Default.Package.json` — the registration that makes each
  prop spawnable as `static.prop.<name>`; with `--package`, it is
  `Content/<Name>/Config/<Name>.Package.json`, which is also what
  [[package-carla-ue4]] needs to export the package.
- `Content/Carla/Blueprints/Props/PropFactory.uasset` — the factory entry, unless
  `--no-factory` or `--package`.
