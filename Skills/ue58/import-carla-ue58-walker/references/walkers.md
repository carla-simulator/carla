# Walkers on UE 5.8 — mechanics, measurements and pitfalls

Everything here was measured on `ue58-dev` HEAD `718efd7cc`, engine 5.8.0,
CARLA 0.10.0, `Content/Carla` at `ue58-dev-carla`.

## 1. The three artefacts

`AWalkerActorFactory` (`Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Actor/`)
builds walker definitions from `Content/Carla/Config/WalkerParameters.json`:

```json
{
  "Walkers": [
    {
      "Id": "0015",
      "Generation": 2,
      "Gender": "Female",
      "Age": "Adult",
      "Speed": [...],
      "Class": "/Game/Carla/Blueprints/Walkers/BP_AfroF01_A_G2.BP_AfroF01_A_G2_C"
    }
  ]
}
```

`Class` is a **blueprint generated class**, not a mesh. That is the structural
difference from props ([[import-carla-ue58-prop]]), where the registry names a
`StaticMesh` directly and no blueprint is needed. So importing a walker means
producing a blueprint, and the blueprint has to be a `WalkerBase` subclass whose
CDO already carries the pedestrian anim class.

The id also names the blueprint: `walker.pedestrian.<Id>`, four digits, zero
padded. Shipped set is `0001`–`0048` with gaps; `0090`+ is free space this skill
allocates from.

## 2. Why duplicate the donor instead of subclassing

The skeletal mesh and the animation blueprint class live on the **generated
class's CDO**, not on the blueprint asset. A newly created `WalkerBase` subclass
has no generated class until it is compiled — and compiling it resets the CDO to
the parent's defaults, wiping the mesh assignment made before the compile.

Duplicating a shipped walker blueprint sidesteps this entirely: the duplicate
arrives with a valid generated class, a correct anim class, a `SkeletalMeshComponent`
named `mesh`, and every collision/capsule/controller setting the shipped walkers
use. Only the `skeletal_mesh` on that component needs to change, and it is saved
**without recompiling** — a recompile would undo it.

Observed CDO class: `WalkerBase`. Component property name: `mesh` (lowercase; the
skill probes `mesh`, `Mesh`, `skeletal_mesh_component` in that order).

## 3. The shared skeletons — the silent failure

```
/Game/Carla/Static/Pedestrian/00_GenericComponents/Definitions/
    Skel_Pedestrian_G2.Skel_Pedestrian_G2
    Skel_Pedestrian_G3.Skel_Pedestrian_G3
```

Note **`Definitions`** in the path. Guessing
`00_GenericComponents/Skel_Pedestrian_G2` loads nothing, and `FbxImportUI` with a
null `skeleton` does **not** error: it creates a private skeleton beside the mesh
(named `<fbx>_Skeleton`) with the identical bone hierarchy. The result:

| | shared skeleton | private skeleton |
|---|---|---|
| imports | yes | yes |
| registers | yes | yes |
| spawns | yes | yes |
| looks right | yes | yes |
| **animates** | **yes** | **no** |
| warning anywhere | — | **none** |

This is why `verify --spawn` does not stop at "it spawned": it applies a
`WalkerControl` and asserts a non-zero peak speed (see the distance trap in §6).
A private-skeleton walker stays put because
the anim class on the donor CDO drives bones by name on *its* skeleton, and the
mesh is bound to a different skeleton object.

`import` raises rather than registering when `bound_skeleton != job["skeleton"]`.

Bone count on a GEN2 walker: **66**, readable via `get_bones_transform()`.

## 4. Full editor, not a commandlet

Export of a `SkeletalMesh` via `Exporter.run_asset_export_task` asserts inside the
engine and takes the process down:

```
Assertion failed: MeshObject
  [.../Runtime/Engine/Private/Components/SkinnedMeshComponent.cpp:4987]
SIGSEGV
```

Reproduced under **both** `-nullrhi` **and** `-RenderOffScreen`, so it is not an
RHI-selection issue — the FBX exporter needs a real render context that
`-run=pythonscript` never sets up. Both halves of this skill therefore run in a
full editor and need `DISPLAY`.

Driving a full editor has two consequences:

- **Use `-ExecutePythonScript=<path>`.** `-ExecCmds="py \"<path>\""` mangles under
  nested quoting and silently runs nothing — no error, no output, exit 0.
- **The editor does not exit when the script ends.** A commandlet does; a full
  editor opens the GUI and waits, so a caller's `subprocess.run()` blocks until
  its timeout. The in-editor script calls `unreal.SystemLibrary.quit_editor()` in
  a `finally` block.

Editor exit code on shutdown is regularly **-11** (SIGSEGV), the same shutdown
crash the ue58 server shows. It happens *after* the work, so the skill treats the
result JSON as the source of truth and only warns.

The result JSON is also the only output channel: `print()` and `unreal.log()` from
Python inside the editor do not reliably reach the log. Progress is flushed after
every step so a hard crash still leaves a trace of where it died.

## 5. The filename trap

The imported asset takes the **FBX file's** basename, not the destination path's
leaf. Exporting to `/tmp/wexport.fbx` and importing into
`/Game/Carla/Static/Pedestrian/TestWalker/` produces an asset called
`wexport`, and the registry then points at a path that does not exist. The skill
stages a copy named `<name>.fbx` in a temp dir before importing.

## 6. Measured round trip

The validation run, end to end, on a machine with no external rigged art:

| step | result |
|---|---|
| `export --walker 0015` | 1 721 104-byte FBX, mesh 1.69 × 0.34 × 1.84 m, skeleton `Skel_Pedestrian_G2` |
| `import --name SkillPed` | mesh at `/Game/Carla/Static/Pedestrian/SkillPed/SkillPed`, bound to shared `Skel_Pedestrian_G2`, 1.682 × 0.303 × 1.833 m |
| blueprint | `BP_SkillPed` duplicated from `BP_AfroF01_A_G2`, CDO `WalkerBase`, `mesh` repointed, saved uncompiled |
| registry | Id `0090`, walker count 38 → 39 |
| spawn (fresh server) | actor id 25, bbox 1.68 × 0.30 × 1.83 m |
| `WalkerControl` | **peak 1.40 m/s**, the commanded speed (net travel 0.09 m — see below) |
| bones | 66 readable |
| `revert --yes` | registry 39 → 38, mesh dir and `BP_SkillPed.uasset` deleted |

Slight dimension drift between export and re-import (1.69/0.34 vs 1.682/0.303) is
FBX round-trip precision plus bounds recomputation, not a scale error.

### The distance trap, and why `verify` asserts on speed

An earlier version of this skill asserted "moved more than a metre" and reported
~107 m. That number was the distance from the **world origin**: in asynchronous
mode `get_location()` on a just-spawned actor reads back `(0,0,0)` until the first
tick lands, so a start sampled immediately measures the spawn point's distance
from the origin and passes no matter what the walker does.

Corrected, then measured on a **stock** walker (`walker.pedestrian.0015`):

```
peak 1.40 m/s        <- exactly the commanded WalkerControl speed
net travel 0.09 m    <- the walker is boxed in
```

Both numbers are real. `map.get_spawn_points()` returns *vehicle* bays, and one
can be enclosed by geometry, so the walker animates on the spot. That is why
`verify` settles for 20 ticks before its first sample, samples velocity every tick
during the control window, and asserts on **peak speed** — a walker on a private
skeleton reads exactly `0.00 m/s`, which is the failure this check exists to
catch. Net travel is reported for information only.

## 7. Spawning walkers actually works differently than the docs suggest

`world.get_random_location_from_navigation()` returns locations where
`try_spawn_actor` fails — and it fails for the **stock** walkers too, which is the
control test that rules out the imported asset. The navigation mesh and the
collision world disagree on ue58.

`verify` falls back to `map.get_spawn_points()` with a z-offset, which works.
Check this before concluding a custom walker is broken.

## 8. Animation control

The walker moves under `carla.WalkerControl` (direction, speed, jump). A
`carla.WalkerAIController` is a separate actor
(`controller.ai.walker`) that has to be spawned attached and given
`go_to_location`; `verify` uses raw `WalkerControl` because it is the narrower
test — it proves the mesh/skeleton/anim chain without involving navigation.

## 9. What this skill does not do

- **No vehicles.** A vehicle needs a wheeled rig, physics asset and Chaos wheel
  setup; the donor-duplicate trick applies but the constraints are different.
- **No new animations.** `import_animations` is off deliberately: the pedestrian
  animation set already lives on the shared skeleton, and importing the FBX's own
  animations duplicates or conflicts with it.
- **No semantic tags.** As with props, `semantic_tags` comes back empty and ue58
  has no tagging commandlet.
- **No LOD generation.** Shipped walkers carry LODs; a re-imported mesh has one.
