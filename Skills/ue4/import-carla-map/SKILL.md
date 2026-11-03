---
name: import-carla-map
description: Imports a custom RoadRunner map (FBX geometry + XODR OpenDRIVE) into a CARLA source build as a drivable map — reads the map from the directory the user names, runs CARLA's Import.py to cook the level, Traffic Manager graph and (standard maps only) pedestrian navmesh, and verifies it loads on a server. Handles both standard maps and large tiled maps; large maps get no navmesh, matching upstream CARLA. Use when the user asks to "import a map into CARLA", "add a custom/RoadRunner map", "ingest an FBX+XODR map", "make import", bring in a "large/tiled map", or fix missing walkers / a missing pedestrian navmesh.
license: MIT
compatibility: Linux. Requires a built CarlaUnreal UE 4.26 fork (UE4_ROOT), a built CARLA checkout, and a python that can `import carla` — Import.py runs in-process. Compiles nothing; defers to build-carla-ue4. A standard map is one editor boot (~minutes); a large map scales with tile count. The pedestrian navmesh needs Blender 3.3+ and is standard-maps-only.
metadata:
  group: ue4
  requires: build-carla-ue4
  prerequisites: scripts/check_env.sh
  reference: references/maps.md
---

# Import a map into CARLA

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Turn a RoadRunner export — an `.fbx` mesh plus its `.xodr` OpenDRIVE — into a
CARLA map you can load and drive on. **The map's directory comes from the
request** — `Import/Maps`, `~/dev/Maps/MyTown`, a mounted share, anywhere. Ask
for it if the request doesn't say; never guess a location. Your files are only
read, never modified in place (Import.py copies them into
`Util/DockerUtils/dist/` for the navmesh stage and deletes the copies again).

```bash
# standard map: MyTown.fbx + MyTown.xodr in the directory the user named
python3 scripts/import_map.py <map-dir> --package MyTown

# large map: MyTown_Tile_0_0.fbx … + MyTown.xodr, imported in memory-bounded batches
python3 scripts/import_map.py <map-dir> --package BigCity --tile-size 1000 --batch-size 200
```

Both go through the same Import.py pipeline; the large-map path is just a
branch on the tile naming. The map lands at
`/Game/<package>/Maps/<name>/<name>`, cooked with its Traffic Manager route
graph and — for standard maps only — its pedestrian navmesh (Step 4).

> Pipeline internals, naming rules, the tiled branch and the M-lessons:
> [`references/maps.md`](references/maps.md).

## Instructions

```
Map Import Progress:
- [ ] Step 1: Check prerequisites — hand off to build-carla-ue4 on a build/carla FAIL
- [ ] Step 2: Import (one command: runs Import.py over the map's directory, verifies artifacts)
- [ ] Step 3: Verify it loads and drives on a running server
- [ ] Step 4: (standard maps + walkers only) build and inspect the pedestrian navmesh
- [ ] (opt-in) Cook it into a distributable package
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh <map-dir>      # map-dir optional: also checks the .fbx/.xodr pair
```

Blocks only on hard failures: a runnable editor for this project (`UE4_ROOT`, the
checkout, `CarlaUE4Editor`) and **a python that imports `carla`** — map import
runs `carla.Map()` in-process, so a missing wheel aborts Import.py on its first
line (M4). On a build FAIL, stop and run [[build-carla-ue4]] against that
checkout.

Read the WARN lines too. Two matter before you import anything: `Content -> …
(shared with other checkouts)` means the map lands in a content clone every
worktree sees, and leftover geometry in `Util/DockerUtils/dist` from an
interrupted run can feed the next navmesh build stale data.

Run it with the map directory as above. The MCP `check_prerequisites` tool takes
no argument, so through that path you get the environment checks only — the
`.fbx`/`.xodr` pair is then first checked by `import_map.py` itself.

**The carla wheel must be importable by the `python3` that runs Step 2** — unlike
prop import, that is a hard requirement here, not a verify-only one. You bring the
env; the skill never creates one:

- Activate it in the same shell before Step 2 (venv/conda/pyenv/system — no
  manager is assumed).
- Non-interactive? Point `CARLA_ENV_ACTIVATE` at its activate script — an
  optional hook `scripts/env.sh` sources, used by both `check_env.sh` and
  `import_map.py`. Nothing else is detected.
- `check_env.sh` prints the interpreter it resolved (`client python: … (imports
  carla)`); that line is the one to trust, not which env you think is active.

A missing `FBX2OBJ` is a **warning**, and only for a standard map: it imports and
drives, but gets no pedestrian navmesh. Do Step 4 first if that map needs
walkers, so the import builds the navmesh in one pass.

### Step 2: Import

```bash
python3 scripts/import_map.py <map-dir> --package <PackageName>
```

`<map-dir>` is the directory the user named (or the `.xodr`/`.fbx` in it). The
script enforces the one rule that breaks silently otherwise — **the `.fbx` and
`.xodr` must share the same name root** (M1) — detects standard vs. tiled, runs
Import.py against that directory, then confirms the cooked artifacts by path and
size.

This imports into the **source build you are running against**: the map lands in
the Content tree and loads in the editor or on an uncooked server straight away.
Cooking a *distributable* package (a `Dist/` tarball) is the separate opt-in step
at the end.

**Always pass `--package`.** It names the **content package** — the folder under
`Content/` that holds the map and the `Config/<package>.Package.json` registering
it, the same way CARLA's own maps live in the `Carla` content package. It
defaults to the map name; never leave it as upstream's `map_package`, which
collides on the next import (M5). A distinct name also keeps your map out of
`Content/Carla`, which is usually a clone shared by every worktree.

Re-importing an existing content package **overwrites it**, navmesh included; the
script says so before the editor boots.

A RoadRunner tiled export normally holds **both** a combined `<name>.fbx` and the
`<name>_Tile_X_Y.fbx` files. The combined mesh is used by default (and the script
says so); pass `--tiled` to take the large-map path.

| flag | when |
|---|---|
| `--tiled` | the export has both variants and you want the tiled one |
| `--tile-size N` | large maps: tile edge in metres (default 1000; CARLA max 2000) |
| `--batch-size N` | large maps: import tiles in ≤N-MB batches to bound editor RAM |
| `--no-carla-materials` | keep RoadRunner materials instead of CARLA road textures |
| `--json-only` | write the package json and stop, to inspect before importing |
| `--keep-json` | leave the package json behind afterwards |
| `--force` | overwrite a package json this skill did not write; silence the re-import warning |

Import.py finds content packages by walking `Import/` for `*.json`, so the script
writes one `<package>.json` there — naming your `.fbx`/`.xodr` by absolute path —
and deletes it when the import finishes. It refuses to overwrite a json it did
not write (it would delete yours at the end), and warns if another package json
is already in that tree, since those import too (M2).

Import.py is called **directly, not via `make import`** — that target rebuilds
LibCarla, the plugins, the editor and the PythonAPI wheel first, and silently
drops the batch-size flag on the way through (see `references/maps.md`).

> The large-map branch — RoadRunner export options, tile levels, editing
> tile-by-tile — is in [`references/maps.md`](references/maps.md#standard-vs-large-tiled-maps).

### Step 3: Verify it loads

Boot an uncooked headless server directly on the new base level
([[run-carla-server]]; `-game -nullrhi` is enough — a map needs no rendering to
load and drive), then check it from a client:

```bash
bash ../run-carla-server/scripts/run_server.sh /Game/MyTown/Maps/MyTown/MyTown 2000 \
  >/tmp/carla_server.log 2>&1 &
SERVER=$!
until nc -z 127.0.0.1 2000; do sleep 1; done
python3 scripts/verify_map.py --map MyTown --port 2000 --nav
kill "${SERVER}" 2>/dev/null      # stop the server WE started
```

Stop it by PID. `pkill -x UE4Editor` is the fallback if that leaves anything
behind, but it kills *every* editor on the host — including one the user has open
in a window. Never `pkill -f` anything matching this shell's own command line
(run-carla-server S3).

`verify_map.py` confirms the right map loaded, the OpenDRIVE parsed into a road
network (spawn points + a test vehicle), and — with `--nav` — that a pedestrian
can be placed on the navmesh. **Never pass `--nav` for a large map** (Step 4).
For a standard map, drop `--nav` only until Step 4 has installed `FBX2OBJ` and
built one.

### Step 4: Pedestrian navmesh (standard maps, walkers only)

Skip this step entirely for a **large map** or a vehicles-only scenario.

**Large maps get no navmesh — do not try to build one.** That is the upstream
position, not a limitation of this skill: CARLA's own large maps ship none
(Town11 22×24 km, Town12 12×12 km and Town13 16×12 km have no `Nav/` directory
at all), the import never builds one, and Detour cannot address a single mesh
past ~6.5 km per side. Walkers are unsupported there; vehicles, Traffic Manager
and everything else work normally. `build_navmesh.py` refuses a tiled map, and
building per tile and merging is a trap that produces seam gaps (M8).

For a standard map that needs walkers:

`make build.utils` can no longer supply `FBX2OBJ` (M3), and without it the map
imports with **no** `Nav/<map>.bin`. Install the Blender-backed replacement once
per checkout, then build the navmesh:

```bash
bash scripts/install_fbx2obj.sh                    # once; needs blender, or $BLENDER
python3 scripts/build_navmesh.py --package MyTown --fbx <map-dir>/MyTown.fbx
```

`install_fbx2obj.sh` keeps the original name and CLI, so a later import
produces the navmesh on its own. `build_navmesh.py` checks every stage; the stock
path reports success on a broken build (M3).

Inspect the result. CARLA walkers use a Detour navmesh loaded by LibCarla, **not**
UE4's navigation system, so the editor's navmesh view always shows nothing:

```bash
python3 scripts/navmesh_to_obj.py --package MyTown                    # report: tiles, polys per area
python3 scripts/navmesh_to_obj.py --package MyTown --coverage         # walkable area, and where it is
python3 scripts/navmesh_to_obj.py --package MyTown --obj nav.obj --ue4  # importable overlay mesh
python3 scripts/draw_navmesh.py   --package MyTown --spectator --loop   # draw over a live server
```

Both tools resolve `Nav/<map>.bin` from `--package` under the checkout (add
`--map` when the map name differs), or take any `.bin` path as a positional
argument — a navmesh can be inspected before it is installed.

If `--coverage` reports a large `area63 (unclassified)` share, that is the terrain
mesh: walkers will wander over it. Rebuild with `CARLA_TERRAIN_AREA=grass` to
classify it as grass, which carries a traversal cost that keeps them on the
pavement (`references/maps.md`).

`navmesh_to_obj.py` exits non-zero on an empty navmesh, and its per-area counts
(`road=…, sidewalk=…`) are the cheapest proof the whole chain worked. `--coverage`
goes further and is worth the extra second: it reports walkable area per class and
how much sits far from any road or sidewalk, which is what catches a navmesh built
over the wrong geometry — one that parses and still leaves walkers stranded.
`draw_navmesh.py` needs a rendering server (`WINDOW=1`); debug lines never appear
under `-nullrhi`, and they expire unless you pass `--loop`.

> Every flag of the three navmesh tools, and what a healthy report looks like:
> [`references/maps.md`](references/maps.md#regenerating-and-inspecting).

Maps over **~6.5 km per side** cannot have a navmesh at all (M7).

**This is where the skill stops by default.**

### Optional: cook into a distributable package

Only when the request asks to distribute the map — importing already leaves it
loadable on a source server:

```bash
PACKAGES=MyTown bash ../package-carla-ue4/scripts/package.sh
# -> Dist/MyTown_*.tar.gz, installable into a release via ImportAssets.sh
```

Cooking belongs to [[package-carla-ue4]]; to fold the map into the main CARLA
cook instead, see [`references/maps.md`](references/maps.md#cooking--packaging-for-distribution).

## Examples

**Example 1: a standard map with walkers**

User says: "import the map in ~/dev/Maps/MyTown into CARLA"

```bash
bash scripts/check_env.sh ~/dev/Maps/MyTown
bash scripts/install_fbx2obj.sh            # once per checkout, for the navmesh
python3 scripts/import_map.py ~/dev/Maps/MyTown --package MyTown
# then verify as in Step 3, with --nav
```
Result: `/Game/MyTown/Maps/MyTown/MyTown` loads in the editor and on a source
server, with traffic and pedestrian nav. Nothing is cooked — see Example 4 to
ship it.

**Example 2: walkers won't spawn on an already-imported map**

```bash
bash scripts/install_fbx2obj.sh                     # if FBX2OBJ is missing
python3 scripts/build_navmesh.py --package MyTown --fbx ~/dev/Maps/MyTown/MyTown.fbx
python3 scripts/navmesh_to_obj.py --package MyTown --coverage
```

**Example 3: a large (tiled) map**

User says: "add the large tiled city in Import/Maps/BigCity — the FBX is split into tiles"

```bash
python3 scripts/import_map.py Import/Maps/BigCity --package BigCity --tiled \
  --tile-size 1000 --batch-size 200
# no --nav: large maps have none (Step 4)
python3 scripts/verify_map.py --map BigCity
```

`--tiled` is only needed when the export also contains a combined `BigCity.fbx`;
without it the tiles are detected on their own.

**Example 4: import and distribute (opt-in)**

User says: "import MyTown and give me a package I can drop into a release"

Import as in Example 1 — that already gives a map the editor and a source server
can load. Then, because the request asked for something shippable, cook the
content package into a **distributable package**:
`PACKAGES=MyTown bash ../package-carla-ue4/scripts/package.sh` → `Dist/MyTown_*.tar.gz`.

## Troubleshooting

**Error: `no .fbx matching the .xodr '<name>'`**
Cause: the mesh and OpenDRIVE have different name roots (M1).
Solution: rename so the stems match — `MyTown.fbx` + `MyTown.xodr`, or
`MyTown_Tile_X_Y.fbx` + `MyTown.xodr` for tiles.

**Error: the import aborts immediately with a `carla` ImportError**
Cause: the active python can't import `carla`; Import.py needs it at module
scope (M4).
Solution: activate the env with the CARLA wheel (build-carla-ue4 step 04), or set
`CARLA_ENV_ACTIVATE` to its activate script, then re-run.

**Error: map imports, but no `Nav/<map>.bin` / walkers won't spawn**
Cause: for a **standard** map, `FBX2OBJ` is not installed, so the FBX→OBJ step
was skipped and RecastBuilder got an empty `.obj` (M3) — `make build.utils`
cannot fix it.
Solution: `bash scripts/install_fbx2obj.sh` (or `export BLENDER=…` if blender
isn't on `PATH`), then `build_navmesh.py --package <pkg> --fbx <the .fbx>`.

**Large map has no `Nav/<map>.bin` and walkers won't spawn**
Not an error — see Step 4. Use a standard map if the scenario needs walkers.

**Error: RecastBuilder runs for many minutes and writes no `.bin`**
Cause: the map exceeds Detour's tile budget (M7) — check its header for
`Max Polys 256` and a `Tiles NxM` product above `Max Tiles`.
Solution: shrink the map's extent (the terrain mesh usually sets it, not the
roads), or rebuild RecastBuilder with a larger tile size. It will not finish
usefully otherwise.

**Error: Import.py exits non-zero mid-way**
Cause: the editor commandlet died. The log scrolls past the real error.
Solution: read the **first** error in the output, not the last. For large maps,
lower `--batch-size` if it ran out of memory.

**Error: an old/other map got imported too**
Cause: another package json under `Import/` — Import.py imports every one it
finds (M2).
Solution: move the other jsons out of `Import/` and re-import.

**Error: `<package>.json already exists and was not written by this skill`**
Cause: a package json of that name is already there, and the import would both
overwrite it and delete it on the way out.
Solution: pick another `--package`, move that json aside, or `--force` if it is
disposable.

**Error: `install_fbx2obj.sh` refuses: `FBX2OBJ.orig already exists`**
Cause: a real, SDK-compiled FBX2OBJ was backed up by an earlier run, and a second
non-shim binary has appeared since — refusing keeps the original backup.
Solution: decide which one you want, remove or rename the other, then re-run.

**The import warns `package '<name>' already exists`**
Not an error: re-importing overwrites that content package, navmesh included. Use
a different `--package` to import alongside it, or `--force` to silence.

## Outputs

Under `Unreal/CarlaUE4/Content/<package>/`:

- `Maps/<name>/<name>.umap` (+ `_Tile_X_Y.umap` for large maps) — the level(s).
- `Maps/<name>/OpenDrive/<name>.xodr` — the road network the server loads.
- `Maps/<name>/TM/<name>.bin` — Traffic Manager route graph.
- `Maps/<name>/Nav/<name>.bin` — pedestrian navmesh. **Standard maps only**, and
  needs `FBX2OBJ` installed. Written during the import, or by Step 4 after the
  fact. A large map has no `Nav/` at all — expected, not a failure (M8).
- `Config/<package>.Package.json` — registers the content package; also what
  [[package-carla-ue4]] reads to cook a standalone distributable package.

Nothing here is cooked: this is a source-build import, loadable in the editor and
on an uncooked server as `/Game/<package>/Maps/<name>/<name>`.
