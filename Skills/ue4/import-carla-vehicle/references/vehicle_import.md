# Vehicle import — pipeline, schema and constraints

Detail layer for `import-carla-vehicle`: what the importer does, the schema it writes,
and the constraints that make an import correct.

> **Provenance.** Derived from the source and shipped content of a 0.9.16 checkout:
> `Carla/Actor/VehicleParameters.h`, `Carla/Actor/ActorBlueprintFunctionLibrary.cpp`,
> `Plugins/CarlaTools/.../VehicleAuthoringLibrary.{h,cpp}` (CARLA PR #9805),
> `Plugins/CarlaTools/Content/Python/add_vehicle_to_vehicle_factory.py`,
> `Content/Carla/Blueprints/Vehicles/*`, `Content/Carla/Static/{Car,Truck}/**`.
> CARLA's own prose guide is `Docs/tuto_A_add_vehicle.md`, which describes the manual
> editor route; this skill automates it.

## Expected input

| requirement | detail |
|---|---|
| format | **FBX**, binary or ASCII. `.ma`, `.mb`, `.blend`, `.max`, `.c4d`, `.obj`, USD, glTF and COLLADA are not importable by Unreal |
| kind | skinned/skeletal mesh whose wheels are bones |
| rig | the four canonical wheel bones plus a chassis bone; extra bones are fine |
| textures | embedded ("Embed Media") or beside the FBX; author-machine absolute paths resolve nowhere |
| contents | one vehicle per file; animation is not imported |
| units | cm or m — scene units are converted on import |
| supplied separately | wheel radius in cm, which is not in the file |

`scripts/check_input.py` enforces the first three and reports the fourth, as the first
step of the skill.

## The 4-wheel rig contract

PhysX's `PxVehicleDrive4W` finds wheels by BONE NAME. The physics-asset bodies, the
`WheelSetups` on the movement component and the animation blueprint all address the
same four names:

```
Wheel_Front_Left   Wheel_Front_Right   Wheel_Rear_Left   Wheel_Rear_Right
```

plus a chassis root — `Vehicle_Base` in the shipped meshes, though any single non-wheel
bone works: `SetupVehiclePhysicsAsset` treats "whatever is not a wheel" as the chassis,
because the shipped Mustang skeleton and the USD importer disagree on its name.

`check_input.py` reads those names straight out of the FBX in under a second. A mesh
without them imports cleanly and then gives a car that cannot steer or roll.

Unlike walkers, vehicles do **not** share one skeleton: each mesh brings its own
(`SK_Ambulance_Skeleton`, `SM_LincolnMKZ_2K17_Skeleton`, …) because the bones *are* that
vehicle's wheels and chassis. So the import creates a new skeleton and the bone names
are the whole contract.

## The asset chain of a CARLA vehicle

Read off `BP_Ambulance`, the shape every import reproduces:

| piece | example |
|---|---|
| vehicle blueprint | `/Game/Carla/Blueprints/Vehicles/Ambulance/BP_Ambulance` (child of `BaseVehiclePawn`) |
| wheel blueprints | `BP_Ambulance_FLW`, `_FRW`, `_RLW`, `_RRW` (`UVehicleWheel` subclasses) |
| skeletal mesh | `/Game/Carla/Static/Truck/Ambulance/SK_Ambulance` |
| skeleton | `…/SK_Ambulance_Skeleton` |
| physics asset | `…/Phys_Ambulance` |
| anim blueprint | `…/AnimBP_Ambulance` |
| materials | `…/MI_BodyWork_Ambulance`, `MI_GlassExt_…`, … |
| factory entry | `VehicleFactory.Vehicles` → `vehicle.<make>.<model>` |

Wheels are their own blueprints because PxVehicle reads radius, width, mass, steering
angle and handbrake off the wheel CLASS, not the vehicle. Four are needed and only the
front pair steers.

## What the importer does

### Boot 1 — full editor session, `editor/build_vehicle.py`

Every asset-shaping step is a function of CarlaTools' `UVehicleAuthoringLibrary`; the
script is an orchestrator, not a reimplementation.

1. **Clean** — remove a previous import of the same vehicle, so a re-run is a fresh
   import rather than a reimport.
2. **Import** — `FbxImportUI` with `FBXIT_SKELETAL_MESH`, no skeleton supplied (one is
   created), `create_physics_asset = False` — the importer's generated asset is not what
   a vehicle needs — then `save_directory`, because `AssetImportTask.save` writes the
   mesh package only and would leave the skeleton and materials as dangling references.
3. **`SetupVehiclePhysicsAsset`** — a dedicated `<Mesh>_PhysicsAsset`: a convex hull for
   the chassis, left simulated, and a KINEMATIC sphere of the given radius on each wheel
   bone. Kinematic matters: the PxVehicle SDK owns wheel motion through the raycast
   `WheelSetups`, so a simulated wheel body fights the suspension and freezes the car.
4. **`CreateVehicleAnimBP`** — duplicates a template anim blueprint and retargets it to
   the new skeleton, so wheel spin and suspension travel work without rebuilding an
   AnimGraph.
5. **`ConfigureWheel`** ×4 — duplicates the donor wheel blueprints and writes radius,
   width, mass, steering angle and handbrake onto each CDO. Front wheels get the steer
   angle; rear wheels get the handbrake.
6. **`CreateVehicleBlueprint`** — duplicates the donor vehicle blueprint and repoints its
   inherited native components: the CDO `Mesh` gets the new skeletal mesh and anim class,
   the 4W movement component's `WheelSetups` get the four wheel classes paired with the
   four bone names.

The script then reads the CDO's `SkeletalMesh` back, because a silent revert (C1) would
otherwise pass as success.

### Boot 2 — full editor, CARLA's own registration script

```
Unreal/CarlaUE4/Plugins/CarlaTools/Content/Python/add_vehicle_to_vehicle_factory.py
```

Shipped with CARLA and runnable by hand. It appends an `FVehicleParameters` to
`VehicleFactory.Vehicles` and saves the factory. The skill supplies its arguments and
confirms the factory package changed on disk.

A full editor session rather than the commandlet, because the factory blueprint has to
compile.

## Schema: `FVehicleParameters`

`Carla/Actor/VehicleParameters.h`:

| field | type | becomes |
|---|---|---|
| `Make` | `FString` | first half of the id: `vehicle.<make>.<model>`, lowercased |
| `Model` | `FString` | second half of the id |
| `Class` | `TSubclassOf<ACarlaWheeledVehicle>` | the vehicle blueprint's generated class |
| `NumberOfWheels` | `int32` | attribute `number_of_wheels` (4 here) |
| `Generation` | `int32` | attribute `generation` |
| `ObjectType` | `FString` | attribute `object_type` — free-form classification |
| `BaseType` | `FString` | attribute `base_type` — car / truck / van / bicycle / motorcycle |
| `SpecialType` | `FString` | attribute `special_type` — electric / emergency / taxi … |
| `HasDynamicDoors` | `bool` | attribute `has_dynamic_doors` |
| `HasLights` | `bool` | attribute `has_lights` |
| `RecommendedColors` | `TArray<FColor>` | the `color` variation |
| `SupportedDrivers` | `TArray<int32>` | walker ids that can drive it |

`BaseType` is what Traffic Manager and many example scripts filter on, so a wrong value
is silent and consequential.

---

## Constraints

### C1 — Never recompile the vehicle blueprint after writing its CDO

The skeletal mesh, anim class and wheel setups live on inherited **native** components
(`AWheeledVehicle::Mesh`, `VehicleMovement`, created with `CreateDefaultSubobject`). A
donor blueprint already carries those override slots in its generated-class CDO — a slot
that serialises — so overwriting the values on the already-compiled duplicate and saving
persists. **A recompile reverts native-component slots to the parent default**, leaving
a vehicle whose `SkeletalMesh` is `None`.

This is why a vehicle is built by duplicating a donor rather than subclassing
`BaseVehiclePawn`: brand-new overrides on a fresh blueprint do not serialise.
`VehicleAuthoringLibrary` documents the same rule in its own comments, and the importer
reads the CDO back afterwards to prove the value stuck.

### C2 — Wheel bodies must be kinematic, and sized

`SetupVehiclePhysicsAsset` gives each wheel bone a kinematic sphere of the radius passed
in. Two failure modes follow from getting this wrong:

- **Simulated wheel bodies** fight the raycast suspension and the car will not move.
- **A wrong radius** changes ride height and suspension travel; the car can spawn
  intersecting the road or bounce.

The radius is `--wheel-radius`, in centimetres, and it is required because nothing in the
mesh states it. Measure it in the authoring scene (or from the mesh bounds: for a normal
car, wheel diameter is roughly a third of the vehicle height).

The physics asset is per-mesh and never shared: a freshly duplicated mesh otherwise
still references the donor's, and editing that would corrupt the donor vehicle.

### C3 — The wheel order is the bone order

`CreateVehicleBlueprint` pairs `Wheels[i]` with `WheelBones[i]`, and the importer passes
them in the canonical order:

```
0 Wheel_Front_Left    1 Wheel_Front_Right    2 Wheel_Rear_Left    3 Wheel_Rear_Right
```

Only indices 0 and 1 receive a steering angle; 2 and 3 get the handbrake. A vehicle
whose front pair ends up at the back steers from the rear axle, which `verify_vehicle.py`
catches as a heading that barely changes.

### C4 — `CarlaTools` must expose `VehicleAuthoringLibrary`

The library arrived with CARLA PR #9805. Without it there is no scripted route to a
vehicle's physics asset or wheel setups — reimplementing PhysX wheel setup in editor
Python is not a reasonable substitute — so `check_env.sh` treats its absence as a hard
blocker and `build_vehicle.py` fails immediately rather than improvising.

### C4b — A duplicated donor keeps its CustomCollision hull

`CustomCollision` is the static-mesh hull CARLA raycasts sensors against, and it is an
SCS-added component rather than a CDO sub-object — `CreateVehicleBlueprint` repoints it
only when a raycast mesh is supplied and reachable. Without `--collision-mesh` the new
vehicle inherits the donor's hull, so sensor hits follow the donor's silhouette. The
importer reports the hull in place on every run, marked inherited or set.

### C5 — Textures come from the FBX, or not at all

As with any FBX import: a file that embeds no textures still declares material names, so
the import writes one blank material per slot and the vehicle renders untextured. Check
before importing:

```
strings file.fbx | grep -iE '\.(png|tga|jpg)'   # absolute author paths, or nothing
```

A vehicle exported *out of* Unreal names its slots after the materials that were
assigned (`MI_BodyWork_Ambulance`, …), which makes those slots trivially rebindable by
name; an FBX from an authoring package usually does not. The importer reports every slot
with what is bound to it and warns on unassigned ones.

### C6 — Both boots need a full editor session

`CreateVehicleAnimBP` calls `EditorAnimUtils::RetargetAnimations`, which syncs the
Content Browser (`FContentBrowserSingleton::SyncBrowserToAssets`) and therefore builds
an `SWindow`. A `-run=pythonscript` commandlet has no Slate, so that asserts in
`FSlateInvalidationRoot` and kills the process mid-assembly — after the physics asset
is built and before anything is saved. Assembly therefore runs in a full editor session
with `-nullrhi`.

Registration needs a full session too: `PythonScriptCommandlet::Main` runs before editor
init and cannot compile blueprints.
`-ExecutePythonScript=<path> <argv>` passes arguments through, which is how CARLA's
registration scripts are driven.

`add_vehicle_to_vehicle_factory.py` writes no result file, so the host detects
completion by the factory package's mtime changing and then stops the editor itself.

Note the shipped script builds its class path as `<arg> + '_C'`, so it expects the
`Package.Object` form; the skill passes that form rather than a Content Browser path.

---

## Test fixtures: what a round-tripped CARLA mesh can and cannot prove

Exporting a shipped vehicle out of Unreal and importing it back is a convenient fixture
— the rig, physics assembly, wheel setup, blueprint wiring and factory registration are
all exercised against a known-good asset. It cannot vouch for geometry or shading.

UE 4.26's FBX exporter writes the RENDER mesh, not the source model: vertices are split
at every UV and smoothing seam and LODs are dropped. Measured on `SK_Ambulance`:

| | verts | LODs | material slots |
|---|---|---|---|
| original | 86 165 | 4 | 4 |
| after export + import | 93 669 | 1 | 4 |

Material bindings survive (all four slots resolve to the original `MI_*` instances), but
the split geometry has damaged UV seams, so large sections render as if untextured while
others look correct. Stray vertices pulled by re-derived skin weights show up as thin
spikes.

So a round-tripped fixture proves the pipeline and misrepresents the art. For a faithful
test, use an FBX exported from the authoring package (Maya/Blender) with **Embed Media**
enabled. Note that `.ma`/`.mb` cannot be imported at all — UE takes FBX.

## Relationship to the sibling skills

| skill | boundary |
|---|---|
| [[build-carla-ue4]] | builds UE4 + CARLA + CarlaTools; this skill checks and defers |
| [[run-carla-server]] | starts the server verification needs |
| [[spawn-vehicles]] | spawns vehicles at runtime, including newly imported ones |
| [[control-vehicle]] | drives one, which is what verification does in miniature |
| [[import-carla-walker]] | the pedestrian counterpart; same two-boot shape |
| [[import-carla-prop]] | static meshes with no rig |
