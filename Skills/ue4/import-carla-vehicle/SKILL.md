---
name: import-carla-vehicle
description: Imports an FBX vehicle into a CARLA source build as a spawnable vehicle.<make>.<model> — checks the FBX carries CARLA's 4-wheel bone rig, imports the mesh, then assembles the vehicle through CarlaTools' VehicleAuthoringLibrary (physics asset with kinematic wheel bodies, retargeted anim blueprint, four configured wheel blueprints, the vehicle blueprint), registers it in VehicleFactory through CARLA's own add_vehicle_to_vehicle_factory.py, and verifies it spawns, drives and steers. Use when the user asks to "add a car/truck/van to CARLA", "import a vehicle FBX", or needs a custom drivable vehicle spawnable from the Python API.
license: MIT
compatibility: Linux. Requires a built CarlaUnreal UE 4.26 fork (UE4_ROOT), a built CARLA checkout, and CarlaTools built WITH VehicleAuthoringLibrary (CARLA PR #9805) — this skill checks all three and defers to build-carla-ue4, it never builds. Importing needs no Python environment; verifying needs one that imports `carla`. Two full editor boots, roughly 8-12 minutes.
metadata:
  group: ue4
  requires: build-carla-ue4
  prerequisites: scripts/check_env.sh
  reference: references/vehicle_import.md
---

# Import a vehicle into CARLA

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

```bash
python3 scripts/import_vehicle.py ~/models/SK_MyVan.fbx \
    --make ford --model transit --base-type truck --generation 3 --wheel-radius 40
```

The FBX must be a **skeletal** mesh whose wheels are bones named
`Wheel_Front_Left`, `Wheel_Front_Right`, `Wheel_Rear_Left`, `Wheel_Rear_Right`.
PhysX's `PxVehicleDrive4W` finds wheels by those exact names — the physics bodies, the
`WheelSetups` and the animation blueprint all address them — so the rig is checked
before anything boots.

The result spawns and drives like any native CARLA vehicle:

```py
bp = bp_lib.find('vehicle.ford.transit')
car = world.spawn_actor(bp, world.get_map().get_spawn_points()[0])
car.apply_control(carla.VehicleControl(throttle=0.6, steer=0.2))
```

> Pipeline, schema and constraints: [`references/vehicle_import.md`](references/vehicle_import.md).

## Instructions

```
Vehicle Import Progress:
- [ ] Step 0: ASK THE USER for make, model, base type, generation and wheel radius
- [ ] Step 1: Check the INPUT FILE first — format, skinned, rig, textures
- [ ] Step 2: Check prerequisites — hand off to build-carla-ue4 on a build FAIL
- [ ] Step 3: Import — boot 1 assembles the vehicle, boot 2 registers it
- [ ] Step 4: Verify it spawns, drives and steers on a running server
```

### Step 0: Ask the user for the attributes

**Do this before anything else, every time. Do not infer them from the file name.**

| ask | values | why it must be asked |
|---|---|---|
| make | e.g. `ford`, `tesla`, `carla` | first half of the blueprint id |
| model | e.g. `transit`, `model3` | second half of the blueprint id |
| base type | `car` / `truck` / `van` / `bus` / `motorcycle` / `bicycle` | Traffic Manager and most example scripts filter on it |
| generation | `1` / `2` / `3` | asset generation the attribute claims |
| wheel radius | centimetres | **PhysX sizes the suspension from it and nothing in the mesh states it** |

Optional and worth offering: `--wheel-width` (defaults to 45% of the radius),
`--wheel-mass`, `--steer-angle` (default 70°), `--special-type` (`electric`,
`emergency`, `taxi`), `--has-lights`, `--object-type`.

Wheel radius is the one number with physical consequences: too small and the car sits
in the road, too large and it bounces. Measure it in the authoring scene; failing that,
a normal car's wheel diameter is roughly a third of its total height.

### Step 1: Check the input file — ALWAYS FIRST

```bash
python3 scripts/check_input.py ~/models/SK_MyCar.fbx
```

**Expected input, in one place:**

| requirement | detail |
|---|---|
| format | **FBX**, binary or ASCII. Nothing else is importable — `.ma`, `.mb`, `.blend`, `.max`, `.c4d`, `.obj`, USD, glTF, COLLADA all fail here |
| kind | a **skinned/skeletal** mesh whose wheels are BONES. A rigid body is a prop, not a vehicle |
| rig | the four canonical wheel bones: `Wheel_Front_Left`, `Wheel_Front_Right`, `Wheel_Rear_Left`, `Wheel_Rear_Right`, plus a chassis bone (`Vehicle_Base` by convention; otherwise inferred) |
| textures | **embedded** (FBX export option "Embed Media") or texture files beside the FBX. Absolute paths from the author's machine resolve nowhere |
| contents | one vehicle per file; animation is not needed and is not imported |
| units | authored in cm or m — the importer converts scene units |
| known separately | the **wheel radius** in cm. It is not in the file and PhysX sizes the suspension from it |

Exit codes: `0` usable, `1` wheel bones missing, `2` unusable (wrong format, or not
skinned).

```
[input] SK_Ambulance_export.fbx  2493 KB
[input] PASS  binary FBX
[input] PASS  skinned
[input] PASS  all 4 wheel bones present
```

Extra bones beyond the wheels and chassis are fine — doors, steering wheel, suspension
helpers all pass. The texture line is a **warning, never fatal**.

`import_vehicle.py` runs this itself and refuses on a `1` or `2`.

### Step 2: Check prerequisites

```bash
bash scripts/check_env.sh
```

Beyond a runnable editor this checks the things no step here can create: **CarlaTools
built with `VehicleAuthoringLibrary`** (hard blocker — see C4),
`add_vehicle_to_vehicle_factory.py`, the donor vehicle blueprint, its four donor wheel
blueprints, the donor anim blueprint, and that `VehicleFactory` has its `Vehicles`
member.

**This skill never builds.** On a FAIL naming UE4, `CarlaUE4Editor` or CarlaTools, stop
and run [[build-carla-ue4]] against that checkout.

### Step 3: Import

```bash
python3 scripts/import_vehicle.py ~/models/SK_MyVan.fbx \
    --make ford --model transit --base-type truck --generation 3 --wheel-radius 40
```

Assembly is done by CarlaTools' `VehicleAuthoringLibrary`, one library call per step, so
the skill never reimplements PhysX wheel setup:

| step | library function | what it produces |
|---|---|---|
| physics | `SetupVehiclePhysicsAsset` | convex chassis (simulated) + KINEMATIC spheres on the four wheel bones |
| animation | `CreateVehicleAnimBP` | a retargeted copy of a template anim blueprint |
| wheels | `ConfigureWheel` ×4 | four duplicated wheel blueprints; front pair steers, rear pair takes the handbrake |
| vehicle | `CreateVehicleBlueprint` | duplicated donor BP with mesh, anim class and `WheelSetups` repointed |

Reported per run:

```
[vehicle] mesh       /Game/Carla/Static/Vehicles/TestAmbulance/SK_TestAmbulance...
[vehicle] size       6.37 x 2.35 x 2.43 m
[vehicle] physics    .../SK_TestAmbulance_PhysicsAsset
[vehicle] anim BP    .../AnimBP_TestAmbulance
[vehicle] wheel      Wheel_Front_Left: BP_TestAmbulance_FLW  steers=True
[vehicle] blueprint  /Game/Carla/Blueprints/Vehicles/TestAmbulance/BP_TestAmbulance
[vehicle] CDO mesh   .../SK_TestAmbulance      <- read back, not assumed
```

The `CDO mesh` line matters: the mesh lives on an inherited native component, and a
recompile would silently revert it to `None` (C1), so the importer reads it back.

Other flags:

| flag | why |
|---|---|
| `--name` | asset name; defaults to the FBX stem |
| `--donor` | donor vehicle blueprint to duplicate (default `BP_Ambulance`) |
| `--donor-anim-bp` | anim blueprint template to retarget |
| `--wheel-width`, `--wheel-mass`, `--steer-angle` | wheel tuning |
| `--object-type`, `--special-type`, `--has-lights` | extra `FVehicleParameters` fields |
| `--mesh` | nominate one mesh when a multi-mesh FBX yields several |
| `--no-register` | build the assets, leave `VehicleFactory` alone (not spawnable) |
| `--register-only` | register an already-built blueprint |
| `--skip-input-check` | import without the canonical wheel bones (it will not drive) |
| `--verbose` | stream the editor log |

#### The two boots

**Both are full editor sessions**, not the fast commandlet. `CreateVehicleAnimBP`
retargets animations, and UE's retarget path syncs the Content Browser, which builds a
Slate window — in a commandlet that asserts and takes the process down mid-assembly
(C6). Registration needs a full editor anyway, to compile the factory blueprint.

Boot 2 runs **CARLA's own script**, which ships with CARLA and is runnable by hand:

```
Unreal/CarlaUE4/Plugins/CarlaTools/Content/Python/add_vehicle_to_vehicle_factory.py
```

It appends an `FVehicleParameters` to `VehicleFactory.Vehicles` and saves. The skill
supplies its arguments and confirms the factory package changed on disk.

### Step 4: Verify

Vehicles need a running server. **Assume one is already up** — do not start one; if the
connection fails, say so and let the user start it. A server keeps content from its
startup, so it must be **restarted after an import** for a pass to mean anything.

```bash
python3 scripts/verify_vehicle.py --id vehicle.ford.transit
```

Five checks, because each catches a different way an import looks fine and is not:

| check | what it catches |
|---|---|
| in the library and spawns | a missing factory entry, an unloadable mesh |
| registered attributes | a wrong `base_type` / `generation` reaching the library |
| `get_physics_control()` reports 4 wheels | wheel setups or physics bodies wrong |
| drives ≥ 3 m at full throttle | simulated (not kinematic) wheel bodies — the classic frozen car |
| heading changes under full steer | front wheels bound to the wrong bones |

`--keep` leaves the vehicle in the world to look at.

## Examples

**Example 1: a van**

User says: "add this van FBX to CARLA"

```bash
# step 0: ask make / model / base type / generation / wheel radius first
bash scripts/check_env.sh
python3 scripts/import_vehicle.py ~/models/SK_MyVan.fbx \
    --make ford --model transit --base-type van --generation 3 --wheel-radius 38
python3 scripts/verify_vehicle.py --id vehicle.ford.transit   # after a server restart
```

**Example 2: an emergency vehicle with lights**

```bash
python3 scripts/import_vehicle.py ~/models/SK_Ambulance.fbx \
    --make carla --model ambulance2 --base-type truck --generation 3 \
    --wheel-radius 40 --special-type emergency --has-lights
```

**Example 3: build now, register later**

```bash
python3 scripts/import_vehicle.py ~/models/SK_Prototype.fbx \
    --make acme --model proto --base-type car --generation 3 \
    --wheel-radius 35 --no-register
# later
python3 scripts/import_vehicle.py ~/models/SK_Prototype.fbx --register-only \
    --name SK_Prototype --make acme --model proto --base-type car \
    --generation 3 --wheel-radius 35
```

## Troubleshooting

**`does not carry CARLA's 4-wheeled rig` / missing wheel bones**
Cause: the FBX's joints are not named as PhysX expects.
Solution: rename them in the authoring scene and re-export. `--skip-input-check` imports
anyway and produces a car that cannot drive.

**`no skin cluster — a CARLA vehicle is a SKELETAL mesh`**
Cause: a rigid-body FBX.
Solution: the wheels must be bones. A static mesh belongs to [[import-carla-prop]].

**`CarlaTools does not expose VehicleAuthoringLibrary`**
Cause: CarlaTools predates CARLA PR #9805.
Solution: rebuild CarlaTools against a checkout that has it (C4). There is no fallback:
building wheel bodies from editor Python is not a reasonable substitute.

**`SetupVehiclePhysicsAsset failed`**
Cause: the wheel bones exist in the FBX but not in the imported skeleton, or the mesh
has no skinned geometry to hull.
Solution: check the reported skeleton, then re-export with skinning intact.

**`the vehicle blueprint's CDO has no SkeletalMesh`**
Cause: the native-component override did not persist — what a recompile after writing
the CDO produces (C1).
Solution: re-import; never recompile the assembled blueprint.

**Vehicle spawns but will not move at full throttle**
Cause: wheel bodies simulated instead of kinematic, or no bodies on the wheel bones
(C2).
Solution: re-import so `SetupVehiclePhysicsAsset` rebuilds them; check the wheel radius
is sane.

**Vehicle drives but will not steer**
Cause: the front wheel blueprints have no steer angle, or the wheel order does not match
the bone order (C3).
Solution: re-import; `--steer-angle` sets the angle.

**Vehicle sits in the road, or bounces**
Cause: `--wheel-radius` does not match the mesh (C2).
Solution: measure the wheel and re-import.

**Vehicle renders untextured**
Cause: the FBX embeds no textures, so each slot got a blank material (C5).
Solution: re-export with **Embed Media**, or supply the texture files.

## Outputs

- `Content/Carla/Static/Vehicles/<Name>/` — `SK_<Name>`, its skeleton,
  `<Name>_PhysicsAsset`, `AnimBP_<Name>`, and the FBX's materials.
- `Content/Carla/Blueprints/Vehicles/<Name>/BP_<Name>.uasset` — the spawnable vehicle.
- `…/BP_<Name>_{FLW,FRW,RLW,RRW}.uasset` — the four configured wheel blueprints.
- `Content/Carla/Blueprints/Vehicles/VehicleFactory.uasset` — the entry that makes it
  spawnable as `vehicle.<make>.<model>`; untouched with `--no-register`.
