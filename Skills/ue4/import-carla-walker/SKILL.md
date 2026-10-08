---
name: import-carla-walker
description: Imports an FBX pedestrian into a CARLA source build as a spawnable walker.pedestrian.NNNN — checks the FBX is skinned to CARLA's GEN3 rig, imports the mesh, physics asset and materials, builds the walker blueprint from a GEN3 donor, registers it in WalkerFactory through CARLA's own add_walker_to_walker_factory.py, and verifies it spawns, stands on the ground and walks. Use when the user asks to "add a pedestrian to CARLA", "import a walker/character FBX", or needs a custom human spawnable from the Python API.
license: MIT
compatibility: Linux. Requires a built CarlaUnreal UE 4.26 fork (UE4_ROOT) and a built CARLA checkout — this skill checks for both and defers to build-carla-ue4, it never builds. Importing needs no Python environment; verifying needs one that imports `carla`. Two editor boots, roughly 5-8 minutes. Automatic registration needs WalkerFactory's pedestrian list to be a blueprint MEMBER variable (references, C1); without it the walker is still built and the factory entry is a one-line paste.
metadata:
  group: ue4
  requires: build-carla-ue4
  prerequisites: scripts/check_env.sh
  reference: references/walker_import.md
---

# Import a pedestrian into CARLA

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

```bash
python3 scripts/import_walker.py ~/models/SK_AfroBoy01_G3.fbx \
    --gender male --age child --generation 3 --speed 0.0,1.1,2.0
```

`--gender`, `--age` and `--generation` are **required and asked of the user** (step 0);
`--speed` is optional, defaulting to the stock adult triple `0.0,1.7,4.0`.

The FBX must be skinned to CARLA's **GEN3 pedestrian rig** — 26 `crl_*` bones. That is
the whole contract: match it and the walker inherits `ABP_GEN3` and every `AS_*_G3`
animation with no retargeting. Miss it and the import succeeds while the walker never
moves, which is why the rig is checked before anything boots.

The result spawns exactly like a native CARLA pedestrian:

```py
walker_bp = bp_lib.find('walker.pedestrian.0053')
walker = world.spawn_actor(walker_bp, world.get_random_location_from_navigation())
controller = world.spawn_actor(bp_lib.find('controller.ai.walker'),
                               carla.Transform(), attach_to=walker)
controller.start(); controller.go_to_location(world.get_random_location_from_navigation())
```

> Pipeline, factory schema and constraints: [`references/walker_import.md`](references/walker_import.md).

## Instructions

```
Walker Import Progress:
- [ ] Step 0: ASK THE USER for gender, age and generation (speed optional)
- [ ] Step 1: Check the INPUT FILE first — format, skinned, rig, textures
- [ ] Step 2: Check prerequisites — hand off to build-carla-ue4 on a build FAIL
- [ ] Step 3: Import — boot 1 builds the assets, boot 2 registers them
- [ ] Step 4: Verify it spawns, stands, and walks on a running server
```

### Step 0: Ask the user for the attributes

**Do this before anything else, every time. Do not infer them, do not carry them over
from an earlier import, do not read them off the file name.**

Three questions, and the import will not start without the answers — the CLI makes
`--gender`, `--age` and `--generation` required precisely so this cannot be skipped:

| ask | values | why it must be asked |
|---|---|---|
| gender | `male` / `female` / `other` | not derivable from a mesh at all |
| age | `child` / `teenager` / `adult` / `elderly` | height is measured and reported, but a 1.2 m mesh can be a child or a scaled adult |
| generation | `1` / `2` / `3` | `3` is the GEN3 rig this skill imports; the user still chooses what the attribute claims |

`--speed` is **optional** and defaults to `0.0,1.7,4.0` — the stock adult triple, and
the same default CARLA's `add_walker_to_walker_factory.py` uses. Children and teenagers
normally use `0.0,1.1,2.0`; when the default contradicts the chosen age the importer
says so rather than accepting it quietly. Ask for it only if the user cares about
walking and running speed.

Every one of these is **silent when wrong**: nothing raises, the walker spawns happily,
and the attribute is simply wrong for anyone filtering the blueprint library on
`gender`, `age` or `generation`.

### Step 1: Check the input file — ALWAYS FIRST

```bash
python3 scripts/check_input.py ~/models/SK_MyPed.fbx
```

**Expected input, in one place:**

| requirement | detail |
|---|---|
| format | **FBX**, binary or ASCII. Nothing else is importable — `.ma`, `.mb`, `.blend`, `.max`, `.c4d`, `.obj`, USD, glTF, COLLADA all fail here |
| kind | a **skinned/skeletal** mesh. A static mesh is a prop, not a walker |
| rig | skinned to CARLA's **GEN3 skeleton**: exactly the 26 `crl_*` bones, no more and no fewer |
| textures | **embedded** (FBX export option "Embed Media") or texture files beside the FBX. Absolute paths from the author's machine resolve nowhere |
| contents | one character per file; animation is not needed and is not imported |
| units | authored in cm or m — the importer converts scene units |

Exit codes: `0` usable, `1` rig mismatch, `2` unusable (wrong format, or not skinned).

```
[input] SK_AfroBoy01_G3_test.fbx  1313 KB
[input] PASS  binary FBX
[input] PASS  skinned, 26 crl_* bones
[input] PASS  rig is GEN3 — ABP_GEN3 and every AS_*_G3 animation apply as-is
[input] WARN  2 texture reference(s) are paths from another machine ...
```

The texture line is a **warning, never fatal**: the import proceeds and the walker comes
out flat white. Nothing downstream can invent textures the file does not contain, so
knowing it now saves judging a bad import later.

`import_walker.py` runs this itself and refuses on a `1` or `2`, so running it alone is
for triaging a file before committing to it.

### Step 2: Check prerequisites

```bash
bash scripts/check_env.sh
```

Beyond a runnable editor, this checks the assets no step here can create —
`Skel__GEN3`, `ABP_GEN3`, the donor blueprint, `WalkerFactory` — and whether the
factory's pedestrian list is a member variable, which decides whether registration can
be automatic (C1).

**This skill never builds.** On a FAIL naming UE4, `CarlaUE4Editor` or missing content,
stop and run [[build-carla-ue4]] against that checkout, then come back.

There is **no `--package` escape hatch** here, unlike [[import-carla-prop]]: a walker is
only spawnable through `WalkerFactory`, which is shared content. If `Content/Carla` is a
symlink, the check says so — the walker will appear in every checkout that links it.

### Step 3: Import

```bash
python3 scripts/import_walker.py ~/models/SK_AfroBoy01_G3.fbx \
    --gender male --age child --generation 3 --speed 0.0,1.1,2.0
```

**The id is allocated for you**: 4 digits, `max + 1`. Re-importing the same walker
updates its existing entry in place instead of appending a duplicate. `--id` overrides.

**Height is measured and reported** as the *visible* height (unscaled bounds × the
blueprint's mesh scale), with a note when it contradicts the chosen `--age`. The
unscaled figure is ~1.84 m for every GEN3 mesh whatever the character, so only the
scaled one means anything (C3).

**Collision is inherited from the donor** — mesh scale, capsule and offset are a
generation-wide convention, not a fit to the mesh (C3). The importer warns when the new
mesh's bounds drift more than 10% from the donor's, and names the overrides.

**Materials are imported raw and reported per slot**, with what is bound to each:

```
[walker] persisted  8 asset(s) saved in the mesh folder
[walker] materials  6 slots: 0 unassigned, 6 blank from the FBX, 0 taken from the donor
[walker]            - blinn1: blank FBX material - blinn1.blinn1
[walker] WARNING    6 slot(s) have no usable material — the walker renders FLAT WHITE.
```

An FBX that embeds no textures still declares material NAMES, so the import writes one
blank white material per slot. Nothing can recover textures the file does not contain —
check it first with `strings file.fbx | grep -iE '\.(png|tga|jpg)'`. `--materials donor`
binds a donor's textured `MI_*` by slot name, which only looks right when the mesh
shares the donor's UV layout; see C4 before trusting it.

**Hair is inherited from the donor unless you say otherwise.** The line reading
`hair ... (INHERITED from the donor)` means this walker is wearing another character's
hairstyle. `--groom /Game/...` sets it.

Other flags:

| flag | why |
|---|---|
| `--name` | asset name; defaults to the FBX stem |
| `--donor` | which GEN3 blueprint to duplicate (default `BP_Walker_AB001_G3`) |
| `--materials {none,donor}` | `donor` binds materials by slot name; default imports raw |
| `--capsule-half-height`, `--capsule-radius`, `--mesh-z`, `--mesh-scale` | override the inherited GEN3 collision convention |
| `--no-share-physics` | do not reuse the donor's physics asset if the import made none |
| `--wheelchair` | set `can_use_wheelchair` on the definition |
| `--id` | force a 4-digit factory id instead of `max + 1` |
| `--no-register` | build the assets, leave `WalkerFactory` alone (not spawnable) |
| `--register-only` | register an already-built blueprint, skipping the import |
| `--mesh` | nominate one mesh when a multi-character FBX yields several |
| `--verbose` | stream the editor log instead of capturing it |

#### The two boots

Boot 1 (`-run=pythonscript`) imports the mesh and builds the blueprint. Boot 2 (a full
editor session) runs **CARLA's own registration script**:

```
Unreal/CarlaUE4/Plugins/CarlaTools/Content/Python/add_walker_to_walker_factory.py
```

The walker counterpart of `add_vehicle_to_vehicle_factory.py`: it appends the entry to
`WalkerFactory.Pedestrians` and compiles + saves the blueprint through Kismet, which is
what pressing Compile and Save does. It lives in the CARLA repo, not in this skill, and
is runnable by hand:

```bash
UE4Editor CarlaUE4.uproject -ExecutePythonScript="add_walker_to_walker_factory.py \
    -w /Game/Carla/Blueprints/Walkers/BP_Walker_MyKid --gender male --age child \
    --generation 3 --speed 0.0,1.1,2.0"
```

Two boots rather than one because a failed import must never reach the factory, and
because the commandlet cannot compile blueprints.

**If the list is not a member variable** the importer says so before spending the second
boot and prints a paste-ready entry instead:

```
 REGISTRATION SKIPPED — no 'Pedestrians' member variable in WalkerFactory
    (Id="0053",Class=BlueprintGeneratedClass'"/Game/…/BP_Walker_X.BP_Walker_X_C"',
     Gender=Male,Age=Child,Speed=(0.000000,1.100000,2.000000),Generation=3,bCanUseWheelChair=False)
```

Exit code `3`: the mesh and blueprint are built and correct, and one paste in Class
Defaults finishes it. Promoting the list is a one-time content change (C1) after which
this is fully automatic.

### Step 4: Verify

Walkers need a running server. **Assume one is already up** — do not start one; if the
connection fails, say so and let the user start it.

```bash
python3 scripts/verify_walker.py --id 0053
```

A server keeps content in memory from its startup, so **it must be restarted after an
import** for the check to mean anything.

Four checks, because each catches a different way an import looks fine and is not:

| check | what it catches |
|---|---|
| in the library and spawns | a missing or broken factory entry |
| `get_bones()` returns 26 `crl_*` bones | a blueprint whose SkeletalMesh is `None` |
| bounding-box **bottom** within 0.6 m of the road | a capsule/offset that does not fit this mesh |
| moves ≥ 0.3 m under an AI controller | a mesh the GEN3 animations do not drive |

The ground check uses the bounding box, not the actor's z: a walker's origin is its
capsule centre and a GEN3 capsule is 1.86 m around a 1.2 m body, so actor z against the
road fails on stock walkers (C3).

`--keep` leaves the walker walking, for looking at it with a windowed server.

## Examples

**Example 1: a child pedestrian**

User says: "add this kid FBX to CARLA as a pedestrian"

```bash
# step 0: ask gender / age / generation first
bash scripts/check_env.sh
python3 scripts/import_walker.py ~/models/SK_AfroBoy01_G3.fbx \
    --gender male --age child --generation 3 --speed 0.0,1.1,2.0
python3 scripts/verify_walker.py --id 0053      # after the user restarts the server
```

**Example 2: an adult, default speed, its own hair**

```bash
python3 scripts/import_walker.py ~/models/SK_EuroWoman04.fbx \
    --gender female --age adult --generation 3 \
    --groom /Game/Carla/Static/Hair/Hair_Types/AfroGirl_V01/twoTailAfro_eyebrows
```

**Example 3: build now, register later**

```bash
python3 scripts/import_walker.py ~/models/SK_Guard.fbx \
    --gender other --age adult --generation 3 --no-register
# later
python3 scripts/import_walker.py ~/models/SK_Guard.fbx --register-only \
    --name SK_Guard --gender other --age adult --generation 3
```

## Troubleshooting

**`rig is NOT GEN3` / `missing crl_...`**
Cause: the FBX is skinned to a different skeleton.
Solution: re-export skinned to the GEN3 rig, or retarget first. `--skip-input-check`
imports anyway and gives a walker that cannot animate.

**`no skin cluster — this is not a skinned mesh`**
Cause: the FBX holds static geometry.
Solution: that is a prop — use [[import-carla-prop]].

**`the import produced no SkeletalMesh`**
Cause: an unreadable FBX version, or curves/static data only.
Solution: re-run with `--verbose` and read the **first** FBX error, not the last.

**`the mesh was bound to <X>, not Skel__GEN3`**
Cause: UE refused the existing skeleton, nearly always because bone names differ from
what `check_input.py` compares (joint structure is not checked).
Solution: compare the rig against `Skel__GEN3` bone by bone.

**`REGISTRATION SKIPPED`**
Cause: `WalkerFactory`'s pedestrian list is still a function-local variable (C1).
Solution: promote it once in the editor, or paste the emitted entry.

**Walker spawns but is invisible**
Cause: the blueprint's `SkeletalMesh` is `None`, which is what a recompile after the CDO
was written produces (C2).
Solution: re-import.

**Walker spawns but is flat white**
Cause: the FBX carries no textures, so every slot got a blank material (C4).
Solution: re-export the FBX with **Embed Media**, supply the texture files, or
`--materials donor` if the mesh shares the donor's UVs.

**Walker floats above or sinks into the pavement**
Cause: the inherited capsule and mesh offset do not fit this mesh (C3).
Solution: `verify_walker.py` reports the offset; re-run with `--capsule-half-height` /
`--mesh-z`, or check the FBX scale.

**Walker stands still while the controller runs**
Cause: the anim class is not `ABP_GEN3`, or the mesh is on a foreign skeleton.
Solution: check the donor was a GEN3 walker; `--donor` selects another.

**Walker does not ragdoll when killed**
Cause: no physics asset on the mesh.
Solution: the import creates `<Name>_PhysicsAsset`; if it did not, the donor's is shared
instead, and `--no-share-physics` disables even that.

**Every walker load logs `Failed to load .../GEN3/Nos_/AS_walkingG3`**
Cause: the stock GEN3 blueprints reference an animation absent from the shipped content.
Harmless — `ABP_GEN3` drives the walker.
Solution: nothing to fix; the import clears that slot in its copy.

## Outputs

- `Content/Carla/Static/Pedestrian/<Name>/Meshes/SK_<Name>.uasset` — the mesh, bound to
  `Skel__GEN3`. The `Pedestrian` folder is required: `Tagger.cpp:32` derives the
  `Pedestrians` segmentation label from that folder name.
- `…/Meshes/<Name>_PhysicsAsset.uasset` — created by the import (ragdoll on death).
- `…/Meshes/*.uasset` — the FBX's own materials, one package per slot (C4).
- `Content/Carla/Blueprints/Walkers/BP_Walker_<Name>.uasset` — the walker, duplicated
  from the donor, repointed and resized.
- `Content/Carla/Blueprints/Walkers/WalkerFactory.uasset` — the entry that makes it
  spawnable as `walker.pedestrian.<id>`; untouched with `--no-register` or when the
  pedestrian list is not a member variable.
