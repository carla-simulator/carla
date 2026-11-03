# CARLA map import — pipeline, naming, and failure modes

Detail layer for `import-carla-map`. Read when a step misbehaves or you need the
exact shape of an input or output.

## Contents

- [What the import actually does](#what-the-import-actually-does)
- [Naming rules (non-negotiable)](#naming-rules-non-negotiable)
- [The package JSON](#the-package-json)
- [Standard vs large (tiled) maps](#standard-vs-large-tiled-maps)
- [Pedestrian navmesh](#pedestrian-navmesh)
- [Outputs on disk](#outputs-on-disk)
- [Cooking / packaging for distribution](#cooking--packaging-for-distribution)
- [M-lessons (failure modes)](#m-lessons-failure-modes)

## What the import actually does

`import_map.py` runs `Util/BuildTools/Import.py` directly (see "Why not `make
import`" below). For every content package it finds under `Import/` it:

1. Reads the package `<name>.json` (or auto-generates one by pairing each
   `*.xodr` with the `*.fbx` of the same name root).
2. Runs the **`ImportAssets`** editor commandlet to bring the FBX geometry into
   `/Game/<package>/Maps/<name>`. Large maps are imported in tile batches.
3. Copies the `.xodr` to `Content/<package>/Maps/<name>/OpenDrive/<name>.xodr`.
4. **`MoveAssets`** sorts meshes into semantic-tag folders (segmentation).
5. **`PrepareAssetsForCooking`** builds the streaming base level (+ one level per
   tile for large maps) and **`LoadAssetMaterials`** applies materials.
6. **Traffic Manager graph**: `carla.Map(name, xodr).cook_in_memory_map()` writes
   `Content/<package>/Maps/<name>/TM/<name>.bin`. *This is why the active python
   must import `carla`* — it runs in-process in Import.py, not in the editor.
7. **Pedestrian navmesh** (background thread): see below.
8. Writes `Content/<package>/Config/<package>.Package.json` registering the map.

The whole thing is one editor boot per batch. A single standard map is a couple
of minutes; a large map scales with tile count.

Import.py needs exactly two things from the environment: `UE4_ROOT` (it reads it
to locate the editor binary) and an interpreter that can `import carla`. It finds
the checkout from its own path, so it can be run from anywhere.

It also leaves two traces outside `Content/`, both upstream's and both harmless
once you know: `importsetting.json` is written and deleted in the **current
working directory** (we run with cwd = the checkout root), and the `.fbx`/`.xodr`
are copied into `Util/DockerUtils/dist/` for the navmesh stage and removed
afterwards — an interrupted run leaves them there (see M2).

### Why not `make import`

`make import` is the documented upstream entry point and this skill deliberately
does not use it. Both reasons were verified against `Util/BuildTools/`:

1. **It is not build-free.** `import: CarlaUE4Editor PythonAPI` (Linux.mk), and
   those recipes unconditionally run `BuildUE4Plugins.sh --build`,
   `BuildCarlaUE4.sh --build` and `BuildPythonAPI.sh`, plus
   `LibCarla.server.release`, `LibCarla.client.release`, `osm2odr` and
   `downloadplugins`. Importing a map would relink the project and rebuild the
   client wheel first — slow, network-touching, and able to fail for reasons that
   have nothing to do with your map. This skill checks that the editor is built
   instead (`check_env.sh`) and defers building to [[build-carla-ue4]].
2. **Flags do not survive the trip.** `make import ARGS=…` reaches `Import.sh`,
   whose getopt knows `--batch`, not `--batch-size`; an unrecognised option makes
   getopt fail, and because `Import.sh` does not check its status before
   `eval set -- "$OPTS"`, **every** argument is dropped. Even the accepted
   `--batch N` is then discarded by Import.py, whose flag is `--batch-size` and
   which parses with `parse_known_args`. So through `make`, batch size is always
   Import.py's default of 300 MB, whatever you asked for. Called directly, it
   works.

If you ever need the make path (e.g. to reproduce an upstream bug report), expect
those two behaviours.

## Naming rules (non-negotiable)

Import.py pairs files **by name root**. Break this and the map silently does not
import (M1).

| Variant | FBX | XODR | Notes |
|---|---|---|---|
| standard | `MyTown.fbx` | `MyTown.xodr` | identical stems |
| large | `MyTown_Tile_0_0.fbx`, `MyTown_Tile_0_1.fbx`, … | `MyTown.xodr` | tiles are `<name>_Tile_X_Y.fbx`; one xodr for the whole map |

Export from RoadRunner via `File > Export > Carla (.fbx, .xodr, ...)`; it
follows this convention. Centre the map at (0,0) before export — the road layout
cannot be changed after export.

## The package JSON

`import_map.py` writes this for you into `Import/<package>.json`. The **json
filename is the content package name** Import.py uses
(`package_name = filename.replace(".json", "")`). Shape:

```json
{
  "maps": [
    {
      "name": "MyTown",
      "xodr": "/abs/path/MyTown.xodr",
      "use_carla_materials": true,
      "source": "/abs/path/MyTown.fbx"
    }
  ],
  "props": [],
  "_written_by": "carla-agentic-tools/import-carla-map"
}
```

`_written_by` is ours, ignored by Import.py, and the reason the script can delete
the json afterwards without risking a hand-written one: a json without that marker
is never overwritten or removed (pass `--force` to override).

- `source` (single string) → **standard** map path.
- `tiles` (list) + `tile_size` → **large** map path. Presence of `tiles` is the
  switch Import.py reads; never put both `source` and `tiles`.
- `use_carla_materials: true` uses CARLA road textures; `false` keeps RoadRunner
  materials. A value here overrides the `--no-carla-materials` CLI flag.

## Standard vs large (tiled) maps

Large maps are split into tiles so only the tiles near the ego vehicle are
resident in graphics memory. The pipeline is the same import, branched on the
JSON:

- **Tile size**: `--tile-size` (metres). CARLA's hard max is 2000; ~1000 is the
  usual recommendation. RoadRunner must be exported with *Export to Tiles* +
  *Export Individual Tiles*, and *Split by Segmentation*.
- **Batching**: `--batch-size` (MB) makes Import.py import tiles in groups so the
  editor doesn't exhaust RAM on a big map. Lower it on memory-tight machines.
- **You cannot import a large map and a standard map in the same run** — clear
  `Import/` between them (M2).
- **A RoadRunner tiled export usually contains both variants**: a combined
  `<name>.fbx` *and* the `<name>_Tile_X_Y.fbx` files. That is normal, not a
  broken export. `import_map.py` takes the combined mesh by default and prints
  which it chose; pass `--tiled` for the large-map path.
- **Editing**: work tile-by-tile in the editor by opening each
  `MyTown_Tile_X_Y` level; the base `MyTown` level is the streaming shell, not an
  editing surface. Standard-map customization tools (road painter, procedural
  buildings) are not recommended on large maps.

## Pedestrian navmesh

The walkable navmesh (`Nav/<name>.bin`) drives pedestrian pathfinding, and is
**standard maps only** — large maps have none (see below). It is built in
`Util/DockerUtils/dist/build.sh`:

```
FBX  --FBX2OBJ-->  OBJ  --+
XODR --crosswalks--> OBJ -+--> RecastBuilder --> <name>.bin
```

### FBX2OBJ: required, and `make build.utils` can no longer supply it

`RecastBuilder` ships in `dist/`, but `FBX2OBJ` is compiled against the Autodesk
FBX SDK, and `BuildUtilsDocker.sh` fetches that SDK from a hardcoded
`www.autodesk.com` path that no longer serves it (**HTTP 403** when this was
last exercised) — curl saves the "Access Denied" page, `tar` fails on it, and the
build dies. Confirm by reading what the fetch actually saved: a few hundred bytes
of HTML rather than an archive. Without `FBX2OBJ`, `build.sh` skips FBX→OBJ,
RecastBuilder gets an empty `.obj`, and **no `Nav/*.bin` is written** (M3).

`scripts/install_fbx2obj.sh` installs a Blender-backed replacement — no Autodesk
SDK — writing `dist/FBX2OBJ` (a shim) and `dist/fbx2obj_blender.py` under the
original name and CLI, so `build.sh` and `Import.py` use them unchanged.

The converter reproduces `FBX2OBJ.cpp`: one material per mesh node, chosen by
substring on the node name, first match wins, exported Y-up.

| node name contains | material | Recast area |
|---|---|---|
| `Road(s)_Road`, `_Marking`, `_Curb`, `_Gutter` | `road` | `CARLA_AREA_ROAD` |
| `Road(s)_Sidewalk` | `sidewalk` | `CARLA_AREA_SIDEWALK` |
| `Road(s)_Crosswalk` | `crosswalk` | `CARLA_AREA_CROSSWALK` |
| `Road(s)_Grass` | `grass` | `CARLA_AREA_GRASS` |
| anything else | `block` | `CARLA_AREA_BLOCK` (not walkable) |

Those names are the contract with RecastBuilder; the ids are in
`LibCarla/source/carla/nav/Navigation.h`. A terrain mesh normally falls through
to `block`, which is correct — walkers belong on sidewalks.

**`CARLA_TERRAIN_AREA=grass`** (env var, off by default) is the one place the
replacement can behave differently from `FBX2OBJ.cpp`: it maps `Terrain_*` nodes
to `CARLA_AREA_GRASS` instead of letting them fall through. Why it exists: an
unclassified terrain mesh comes back out of Recast as `RC_WALKABLE_AREA` (63) at
default traversal cost, so walkers wander across it — visible in
`navmesh_to_obj.py --coverage` as a large `area63 (unclassified)` share. Grass
carries `AREA_GRASS_COST`, which keeps them on the pavement. Off by default
because the stock converter does not do it:

```bash
CARLA_TERRAIN_AREA=grass python3 scripts/build_navmesh.py --package MyTown --fbx …
```

### Regenerating and inspecting

`scripts/build_navmesh.py --package <p>` runs the chain against an
already-imported map, so a navmesh costs minutes instead of a full editor
re-import. It checks every stage and exits non-zero — unlike `build.sh`, which
gates each step behind `if [ -f … ]`, and `Import.py`, which calls it via
`subprocess.call` and ignores the status. That combination is what silently
ships maps with no nav.

The navmesh is a Recast/Detour `NAVMESHSET` binary loaded by LibCarla, **not** a
UE4 `RecastNavMesh`, so UE4's navigation view never renders it.
`scripts/navmesh_to_obj.py` decodes it and `scripts/draw_navmesh.py` draws it over
a running server with `world.debug.draw_line`. Both parse the file directly, so a
`.bin` can be inspected before it is installed. Per-area polygon counts
(`road=…, sidewalk=…`) are the cheapest end-to-end proof the chain worked.

Both name the navmesh the same way: `--package <name>` (plus `--map` when the map
name differs from the package) resolves it under the checkout, or pass any `.bin`
path directly.

`build_navmesh.py`:

| flag | why |
|---|---|
| `--package` / `--map` | which imported map to build for (`--map` defaults to the package) |
| `--fbx` | the map's source `.fbx` — the same file that was imported; a tiled map's per-tile FBXs are rejected |
| `--carla-root` | the checkout, when `$CARLA_UE4_ROOT` isn't set and `$PWD` isn't one |
| `--timeout N` | seconds allowed for RecastBuilder (default 3600). A wide flat map is slow; raise it rather than assuming a hang |
| `--keep-temp` | keep the working directory with the intermediate `.obj` — the first thing to inspect when a stage produces nothing |

`navmesh_to_obj.py`:

| flag | why |
|---|---|
| *(none)* | the report: tile grid, polygons per area, bounds. Exits non-zero on an empty navmesh |
| `--coverage` | walkable area per class, and how much of it sits beyond ~50 m of any road or sidewalk. This is the check that catches a navmesh built over the wrong geometry — one that parses, reports polygons, and still leaves walkers stranded |
| `--obj FILE` | write the walkable polygons as a mesh grouped by area |
| `--ue4` | emit UE4 world space (cm, Z-up) instead of Recast space (m, Y-up), for importing the overlay into the editor |

A healthy standard map reports `road` and `sidewalk` in the thousands of polygons
with off-road walkable area close to 0% beyond that radius; `grass` and an
unclassified `area63` bulk are normal (terrain Recast kept but walkers avoid).

`draw_navmesh.py` needs a rendering server (`WINDOW=1`): debug lines never appear
under `-nullrhi`, and they expire after `--life-time` unless `--loop` redraws
them. `--areas sidewalk,crosswalk` narrows a busy map, `--spectator` moves the
camera to the navmesh centre.

### Detour caps the map at ~6.5 km per side

RecastBuilder tiles at ~51.2 m and splits a fixed 22-bit index budget between
tile and poly bits, saturating at 14 tile bits = **16,384 tiles**. The largest
addressable extent is therefore 128 × 51.2 m ≈ **6,553 m per side**. Past that it
reports `Max Polys 256` (down from 262,144) and cannot hold the tile set, so it
never produces a usable navmesh however long it runs (M7):

```
Tiles 256 x 263 / Max Tiles 16384 / Max Polys 256      <- over budget
Tiles 8 x 2     / Max Tiles 16    / Max Polys 262144   <- healthy
```

The extent is driven by the **terrain** mesh, usually far wider than the roads.
Raising the limit means rebuilding RecastBuilder with a larger tile size from
`Build/recast-c10-source/`.

### Large (tiled) maps have no navmesh, and cannot be given one

This is upstream's position, not a gap in this skill. Import.py never builds
one — `build_binary_for_navigation()` skips any map entry without a `source` key,
which is every tiled map, and the upstream comment says the tiled Recast path is
"disabled until we have a new Recast adapted to work with tiles". A large map
imports with no `Nav/` at all, and that is the finished state.

The shipped content agrees. Of CARLA's own maps, only non-tiled ones have a
navmesh:

| map | tiled? | extent | `Nav/` |
|---|---|---|---|
| Town11 | 11×12 tiles @ 2000 m | 22 × 24 km | **none** |
| Town12 | 6×6 @ 2000 m | 12 × 12 km | **none** |
| Town13 | 8×6 @ 2000 m | 16 × 12 km | **none** |
| Town15 | no — single `.umap` | 2.2 × 1.8 km | 578 tiles, 27 MB |
| Town01-10 | no | small | 0.3–3.9 MB |

Town11/12/13 carry only `TM/<name>.bin`, the Traffic Manager route graph — that
is vehicles, not pedestrians. Town15 is the largest map with a navmesh and it is
a conventional single-level map, built in one Recast pass.

Three independent reasons it cannot be worked around:

1. **The loader takes one file.** `WalkerNavigation`'s constructor reads
   `GetRequiredFiles("Nav")` and loads `files[0]` only
   (`LibCarla/source/carla/client/detail/WalkerNavigation.cpp`). Extra `.bin`
   files are silently ignored.
2. **`Navigation::Load` replaces, never adds.** One `dtNavMesh *_nav_mesh`; each
   `Load()` builds a new mesh then frees the old one and rebuilds the query and
   crowd. There is no "add a second navmesh".
3. **Detour has no cross-mesh references.** A `dtPolyRef` encodes
   *(salt, tile, poly)* within one mesh, and `dtNavMeshQuery`/`dtCrowd` bind to
   exactly one `dtNavMesh`. Off-mesh connections are intra-mesh. A walker on mesh
   A cannot name a polygon on mesh B, so two meshes can never be pathed across.

A `dtNavMesh` **is** internally partitioned — Town15 holds 578 tiles of 51.2 m,
added one by one with `addTile()` from the single NAVMESHSET file. That
partitioning bounds memory and lets Detour link neighbours, but it does not
extend reach: every tile shares the one 22-bit budget below. Detour does support
`addTile`/`removeTile` at runtime, the natural design for a streaming map, but
CARLA never calls them outside `Load()`.

Building per tile and merging the results **looks** like the answer and is a
trap — see M8.

## Outputs on disk

Under `Unreal/CarlaUE4/Content/<package>/`:

| Path | What |
|---|---|
| `Maps/<name>/<name>.umap` | the base level (streaming shell for large maps) |
| `Maps/<name>/<name>_Tile_X_Y.umap` | per-tile levels (large maps only) |
| `Maps/<name>/OpenDrive/<name>.xodr` | the road network the server loads |
| `Maps/<name>/TM/<name>.bin` | Traffic Manager route graph |
| `Maps/<name>/Nav/<name>.bin` | pedestrian navmesh — **standard maps only**, ≤ ~6.5 km/side, FBX2OBJ installed. the import writes it; `build_navmesh.py` adds it to an already-imported map. A large (tiled) map has no `Nav/` at all and cannot be given one (M8) |
| `Config/<package>.Package.json` | registration; makes the map selectable |

Load path on a server: `/Game/<package>/Maps/<name>/<name>`.

## Cooking / packaging for distribution

Importing leaves the map **uncooked** — enough to load on an uncooked
`-game`/editor server and drive on. To ship it, cook it with the
[[package-carla-ue4]] skill, which produces `Dist/<package>_*.tar.gz`:

```bash
PACKAGES=<package> bash ../package-carla-ue4/scripts/package.sh
```

To fold the map into the main CARLA cook instead of a standalone package, add it
to `Unreal/CarlaUE4/Config/DefaultGame.ini` under
`[/Script/UnrealEd.ProjectPackagingSettings]`:

```
+MapsToCook=(FilePath="/Game/<package>/Maps/<name>/<name>")
```

## M-lessons (failure modes)

**M1 — FBX and XODR name roots differ.** The map does not import and nothing is
raised: Import.py pairs by name, finds no pair, imports nothing.
*Fix*: make the stems identical (`MyTown.fbx` + `MyTown.xodr`).

**M2 — leftovers get imported too.** Import.py walks `Import/` recursively for
`*.json` and imports **every** package it finds, so a stale package json from an
earlier map is cooked again alongside yours, and mixing a large map with a
standard one in one run fails. Loose `.fbx`/`.xodr` are only picked up when there
is no json anywhere (`generate_json_package` auto-generates one under the name
`map_package` — M5); since this skill always writes a json, that path stays off.
*Fix*: keep one map per import; move other package jsons out of `Import/` first —
`import_map.py` and `check_env.sh` both warn when they see one.

There is a second leftover site: `Util/DockerUtils/dist/`, where Import.py copies
the `.fbx`/`.xodr` for the navmesh stage and deletes them afterwards. A run
interrupted mid-navmesh leaves them, plus an `.obj`/`.bin`, and the next navmesh
build can pick up the stale geometry. `check_env.sh` warns; delete them.

**M3 — no pedestrian navmesh.** `Nav/<name>.bin` is absent and walkers can't
path. Cause: `FBX2OBJ` was never built — and `make build.utils` can no longer
build it, because the Autodesk FBX SDK URL returns HTTP 403 (it "succeeds" far
enough to save a 493-byte error page, then `tar` fails).
*Fix*: `bash scripts/install_fbx2obj.sh` (Blender-backed, no SDK), then
`python3 scripts/build_navmesh.py --package <p>` — no re-import needed.

**M4 — the import aborts on line one with a `carla` ImportError.** Import.py
does `import carla` at module scope. The active interpreter lacks the wheel.
*Fix*: activate the env holding the CARLA client wheel (build-carla-ue4 step 04),
or set `CARLA_ENV_ACTIVATE` to its activate script.

**M5 — default package name `map_package`.** Re-using it on a second import
collides with the first. Always pass `--package <name>`; the skill defaults it to
the map name, not `map_package`.

**M6 — map loads but is untextured / wrong materials.** `use_carla_materials`
mismatch, or `LoadAssetMaterials` had nothing to bind.
*Fix*: re-import with the intended `use_carla_materials`; for RoadRunner
materials keep `--no-carla-materials` AND no overriding value in the JSON.

**M7 — map too large for Detour.** RecastBuilder prints `Max Polys 256` and
never writes a `.bin`, however long it runs. Cause: >16,384 tiles at ~51.2 m
each, i.e. an extent over ~6.5 km per side; the tile/poly index budget is
exhausted. The **terrain** mesh usually sets the extent, not the roads.
*Fix*: shrink the map's extent, or rebuild RecastBuilder with a larger tile size
from `Build/recast-c10-source/` — note a fresh checkout has only
`Build/recast-c10-install/`, so `Setup.sh` has to re-fetch the source first.
Vehicles are unaffected — only walkers need the navmesh.

**M8 — per-tile navmeshes merge with gaps at the seams.** Building a large map's
navmesh from its per-tile FBXs and merging the results yields a mesh that looks
complete but is broken along every tile boundary. Measured on a 2×2, 1.75 km map:
**348 of 17,488 OpenDRIVE lane points (2.0%) had no walkable surface within 3 m**,
76% of them within 30 m of the `x=0` seam and 38% within 30 m of `y=0`. Building
the same map in one pass from the combined FBX gave **17,488/17,488 (100%)**.

Two causes, both structural:

- Recast trims the walkable surface by agent radius and border size at the edge
  of *whatever geometry it is handed*, so each tile's mesh stops short of its own
  boundary. Roads running parallel to a seam look fine; roads crossing one break.
- RecastBuilder derives its lattice origin from each input's own bbox minimum, so
  per-tile lattices are out of phase. Tiles then collide in a merged
  `(x, y, layer)` slot and one is discarded — 55 of them on that map, and 4 ×
  18×18 chunks merged to a 34×35 lattice instead of 36×36.

Worse than the missing coverage is missing *connectivity*: Detour links adjacent
tiles via `connectExtLinks`/`findConnectingPolys`, which match portal edges lying
on the shared boundary. Trimmed-back polygons produce no portal edge, so no link
is created even where both sides have surface nearby.

*Fix*: do not do it. Large maps get no navmesh (see above). This entry exists
because the approach is the obvious one to try.
