---
name: import-carla-ue58-walker
description: Imports a pedestrian into CARLA on UE 5.8 as a spawnable, animating walker — imports the skinned FBX bound to CARLA's shared pedestrian skeleton, duplicates a donor walker blueprint and repoints it at the new mesh, and registers it in WalkerParameters.json as walker.pedestrian.<id>. Can also export a shipped walker to FBX, which is how you obtain a rig-conforming mesh to start from. Use when the user asks to "import a walker/pedestrian", "add a custom character", "clone a walker", or has a walker that spawns but stands still.
license: MIT
compatibility: Linux with a built ue58-dev tree, the content cloned, and a DISPLAY - both halves need a FULL editor, not a commandlet (skeletal FBX export asserts and segfaults in `-run=pythonscript`). Live verification needs a running server and an importable `carla`. VERIFIED end to end on ue58-dev HEAD 718efd7cc, engine 5.8.0: exported walker 0015, re-imported it as walker.pedestrian.0090, spawned it, drove it under WalkerControl at the commanded 1.4 m/s, read 66 bones, reverted.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/walkers.md
---

# Import a walker on UE 5.8

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

A walker is **three** artefacts, not one. Miss any and it either never appears or
appears and never moves:

```
skinned FBX
  --import, bound to the SHARED skeleton-->  SkeletalMesh
  --duplicate a donor BP, repoint its mesh-->  BP_<Name>_C
  --WalkerParameters.json entry-->  walker.pedestrian.<id>
```

Two things make this harder than [[import-carla-ue58-prop]]:

- **The registry points at a blueprint class, not a mesh.** So a blueprint has to
  exist, and it is made by *duplicating* a donor — the skeletal mesh and anim class
  live on the compiled class's CDO, and a fresh subclass would need a recompile,
  which resets both to the parent's defaults.
- **The skeleton binding decides whether it animates**, and getting it wrong fails
  silently. Bind to CARLA's shared `Skel_Pedestrian_G2`/`G3` and the pedestrian
  animation set drives your mesh. Let the importer create its own skeleton — the
  default — and you get a structurally identical private skeleton, a walker that
  registers, spawns, looks right and **stands still**. No warning anywhere.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: donors — pick a donor and see the shared skeletons
- [ ] Step 3: get a rig-conforming FBX (export one if you have no art)
- [ ] Step 4: plan, then import
- [ ] Step 5: RESTART the server, then verify --spawn
```

### Step 2-3: Donor, and where the FBX comes from

```bash
source scripts/env.sh

python3 scripts/import_walker.py donors
```

Lists the 38 shipped walkers with generation/gender/age, checks both shared
skeletons exist, and reports free registry ids.

The FBX must be **skinned to CARLA's pedestrian rig**. If you have no such art,
export a shipped walker — that is also the round-trip that validates the pipeline:

```bash
python3 scripts/import_walker.py export --walker 0015 --out ~/MyPed.fbx
```

Measured on walker `0015`: a 1.72 MB FBX, mesh 1.69 × 0.34 × 1.84 m, skeleton
`Skel_Pedestrian_G2`.

### Step 4: Import

```bash
python3 scripts/import_walker.py plan   ~/MyPed.fbx --name MyPed
python3 scripts/import_walker.py import ~/MyPed.fbx --name MyPed
```

| Flag | Effect |
|---|---|
| `--name` | asset and blueprint name; the FBX is staged as `<name>.fbx` first, because the imported asset takes the **file's** name |
| `--gen 2\|3` | which shared skeleton to bind to (default 2) |
| `--id` | registry id (default: first free from `0090`) |
| `--donor` | donor id or blueprint path (default: lowest id of that generation) |
| `--gender`, `--age` | override the donor's values |
| `--scale` | `import_uniform_scale` |
| `--no-register` | import without touching the registry |

`import` **fails loudly** if the mesh ends up on a private skeleton, rather than
registering a walker that cannot animate.

### Step 5: Restart, verify

**Definitions load once at startup.** A walker registered against a running server
does not appear until it restarts.

```bash
cd ../run-carla-ue58-server && bash scripts/run_server.sh stop
DETACH=1 bash scripts/run_server.sh game

cd ../import-carla-ue58-walker
python3 scripts/import_walker.py verify --id 0090 --spawn
```

`verify --spawn` checks the registry entry, the blueprint on disk, the blueprint
library, then spawns it, **drives it with a `WalkerControl` and asserts a non-zero
peak speed**. That last check is the only proof the shared skeleton's animations
are driving the mesh — a private-skeleton walker spawns and reads exactly
0.00 m/s. It also reports the bone count.

Verified output:

```
PASS registered: gen2 Female Adult -> /Game/Carla/Blueprints/Walkers/BP_SkillPed.BP_SkillPed_C
PASS blueprint on disk   PASS walker.pedestrian.0090 in the blueprint library
PASS spawned id=25 bbox 1.68 x 0.30 x 1.83 m
PASS peak 1.40 m/s under WalkerControl — the shared skeleton's animations are
     driving it (net travel 0.09 m)
PASS 66 bones readable
```

**The check asserts on peak SPEED, not distance travelled**, and that matters.
Measured on a stock walker: peak 1.40 m/s — exactly the commanded speed — with
0.09 m of net travel, because `get_spawn_points()` returns vehicle bays that can
be boxed in by geometry, so the walker treadmills against a wall. Speed proves
the mesh/skeleton/anim chain is live; displacement only proves the spot was open.
An earlier version of this skill asserted on distance and reported ~107 m, which
was the distance from the world origin: in asynchronous mode a just-spawned actor
reads back `(0,0,0)` until the first tick lands, so a start sampled immediately is
meaningless. `verify` now settles for 20 ticks before its first sample.

### Undo

```bash
python3 scripts/import_walker.py revert --name MyPed         # dry run
python3 scripts/import_walker.py revert --name MyPed --yes
```

Removes the registry entry, the mesh directory and the blueprint.
`import`/`revert` back `WalkerParameters.json` up first.

## Examples

**Example 1: "clone an existing pedestrian so I can modify it"**

`export --walker 0015 --out ~/Ped.fbx`, edit in your DCC tool keeping the skin
weights, then `import ~/Ped.fbx --name MyPed`. Restart, `verify --spawn`.

**Example 2: "my walker spawns but doesn't move"**

The mesh is on a private skeleton. `verify --spawn` catches it (`peak 0.00 m/s`);
note that near-zero *net travel* with a healthy peak speed is not this fault, it
is an enclosed spawn point.
Re-import — `import` refuses to register in that state, so it cannot happen through
this skill; it happens when importing by hand or through the editor UI.

**Example 3: "add a GEN3 pedestrian"**

`--gen 3` picks `Skel_Pedestrian_G3` and a generation-3 donor. Mixing a GEN3 mesh
onto the GEN2 skeleton gives a bound-but-wrong result: the bone names differ.

**Example 4: "the asset is called wexport instead of MyPed"**

The imported asset takes the FBX filename. `--name` handles this by staging a
renamed copy, so pass `--name` rather than renaming files yourself.

## Troubleshooting

**Problem: the process segfaults during export**
Cause: a commandlet. `Exporter.run_asset_export_task` on a SkeletalMesh asserts —
`Assertion failed: MeshObject [SkinnedMeshComponent.cpp:4987]` — under both
`-nullrhi` and `-RenderOffScreen`.
Solution: this skill uses a full editor for both halves; make sure `DISPLAY` is set.

**Problem: the command hangs for minutes and times out**
Cause: a full editor driven with `-ExecutePythonScript` does **not** exit when the
script ends — it opens the GUI and stays.
Solution: the in-editor script calls `quit_editor()` in a `finally`. If you write
your own, do the same.

**Problem: `editor exited -11` but the output is there**
Cause: the known ue58 shutdown segfault, same as the server's.
Solution: ignore it; the skill warns and continues, judging by the result file.

**Problem: `mesh bound to <X> instead of the shared <Y>`**
Cause: the shared skeleton could not be loaded, so the importer made its own.
Solution: `donors` verifies both skeleton paths. They live under
`Static/Pedestrian/00_GenericComponents/**Definitions**/` — a path that is easy to
guess wrong.

**Problem: `no SkeletalMesh produced — is the FBX skinned?`**
Cause: the FBX has no skin weights, or only a static mesh.
Solution: a walker needs a skinned mesh; a static mesh is a prop
([[import-carla-ue58-prop]]).

**Problem: the walker never spawns anywhere**
Cause: often the *location*, not the asset. `get_random_location_from_navigation()`
returns points where no walker spawns — the **stock** walkers fail there too.
Solution: `verify` falls back to map spawn points with a z-offset. Do the same in
your own code before concluding the asset is broken.

**Problem: `duplicate_asset failed`**
Cause: the donor blueprint path is wrong, or a stale target is locked by a running
editor.
Solution: close the editor; `donors` prints exact donor class paths.

## Outputs

`Content/Carla/Static/Pedestrian/<Name>/` (skeletal mesh + materials),
`Content/Carla/Blueprints/Walkers/BP_<Name>`, and an entry in
`WalkerParameters.json` with a backup beside it — giving
`walker.pedestrian.<id>` after the next server start. `donors`, `list`, `plan` and
`verify` are read-only; `export` writes only the FBX; `revert` removes all three.

The three-artefact model, skeleton binding, the editor-vs-commandlet split and the
measured round-trip are in [references/walkers.md](references/walkers.md).
