# Content authoring - pedestrians

CARLA's pedestrians share one skeleton, so any model skinned to it works with the animations CARLA already has. This page takes a pedestrian from Blender into the Unreal Editor, registers it as a `walker.pedestrian.*` blueprint, and adds an animation of your own.

- [__The pedestrian skeleton__](#the-pedestrian-skeleton)
- [__Prepare the model in Blender__](#prepare-the-model-in-blender)
- [__Export from Blender__](#export-from-blender)
- [__Import into the Unreal Editor__](#import-into-the-unreal-editor)
- [__Create the walker blueprint__](#create-the-walker-blueprint)
- [__Register the pedestrian__](#register-the-pedestrian)
- [__Spawn it from Python__](#spawn-it-from-python)
- [__Add a custom animation__](#add-a-custom-animation)
- [__Ship the pedestrian__](#ship-the-pedestrian)

Running the Unreal Engine editor is required for this process, therefore you need to [build CARLA from source](build_carla.md).

---

## The pedestrian skeleton

Every pedestrian is bound to one of two skeleton assets in `Content/Carla/Static/Pedestrian/00_GenericComponents/Definitions/`:

| Skeleton | Used by | Animation blueprint |
| --- | --- | --- |
| `Skel_Pedestrian_G2` | Generation 2 pedestrians, `walker.pedestrian.0015` to `0050` | `/Game/Carla/Animations/GEN2/ABP_GEN2_vA` (`ABP_GEN2_kids_vA` for children) |
| `Skel_Pedestrian_G3` | Generation 3 pedestrians, `0051` and `0052` | `/Game/Carla/Animations/GEN3/ABP_GEN3` |

The animations, blend spaces and animation blueprints belong to the skeleton, not to the mesh. A new model bound to `Skel_Pedestrian_G2` idles, walks, runs and turns with no animation work at all.

The bones are prefixed `crl_` and suffixed `__C`, `__L` or `__R`. The hierarchy is:

```text
crl_root
└── crl_hips__C
    ├── crl_spine__C ─ crl_spine01__C
    │   ├── crl_shoulder__L ─ crl_arm__L ─ crl_foreArm__L ─ crl_hand__L ─ 5 fingers × 4 bones
    │   ├── crl_shoulder__R ─ crl_arm__R ─ crl_foreArm__R ─ crl_hand__R ─ 5 fingers × 4 bones
    │   └── crl_neck__C ─ crl_Head__C ─ crl_eye__L, crl_eye__R
    ├── crl_thigh__L ─ crl_leg__L ─ crl_foot__L ─ crl_toe__L ─ crl_toeEnd__L
    └── crl_thigh__R ─ crl_leg__R ─ crl_foot__R ─ crl_toe__R ─ crl_toeEnd__R
```

!!! Warning
    Keep the bone names and the hierarchy exactly as they are. A renamed, added or removed bone stops the mesh from binding to the shared skeleton, and with it from using CARLA's animations.

---

## Prepare the model in Blender

The quickest way to a correct rig is to start from an existing pedestrian. In the Unreal Editor, right-click a pedestrian skeletal mesh such as `/Game/Carla/Static/Pedestrian/EuroF01_G2/SK_EuroF01_A_G2`, choose __Asset Actions > Export__, and import the FBX into Blender. It arrives as three objects:

| Object | Type | What it is |
| --- | --- | --- |
| `SK_<name>.001` | Empty, scale 0.01 | Converts the centimetres of the FBX to Blender's metres. Leave it alone. |
| `crl_root` | Armature, rotated 90° in X | The skeleton. The armature object is the `crl_root` bone. |
| `SK_<name>` | Mesh | The body, with an __Armature__ modifier and one vertex group per bone. |

Edit or replace the mesh, then skin it to the existing armature. Keep in mind:

* __The rest pose is a T-pose__ with the feet on the ground plane and the origin between them. Do not apply a pose to the armature.
* __Keep the axes of the imported pedestrian.__ In Blender the model faces −Y, its left side is +X and up is +Z. After import, the skeletal mesh faces +Y in the Unreal Editor, with its left side at +X, up +Z and units in centimetres. The walker blueprint turns the mesh −90° in yaw so that it faces the actor's +X.
* __Weights go only to the `crl_` vertex groups.__ Any other vertex group is dropped on import.
* __One material slot per material instance__ you want in the Unreal Editor. The slot names become the material slot names of the skeletal mesh.

!!! Note
    The bones imported from the FBX do not point at their children: `crl_hand__L`, for example, points up. This is how the skeleton was built. Pose the rig by rotating the bones, not by aligning them.

---

## Export from Blender

Export the mesh and every animation as __separate FBX files__. In the Outliner, click the `crl_root` armature object and Ctrl+click the mesh object, `SK_<name>`. Select only these two object rows:

* Not the empty `SK_<name>.001` above them. Exported, it becomes an extra root bone.
* Not the data rows nested under each object (the armature with its bone count, the mesh data, __Pose__, __Animation__, __Modifiers__, __Vertex Groups__). They are exported along with their object.

Then use __File > Export > FBX__ with these settings:

| Setting | Mesh FBX | Animation FBX |
| --- | --- | --- |
| __Limit to__ | Selected Objects | Selected Objects |
| __Object Types__ | Armature, Mesh | Armature, Mesh |
| __Smoothing__ | Face | Face |
| __Armature > Add Leaf Bones__ | Off | Off |
| __Bake Animation__ | Off | On |
| __Bake Animation > NLA Strips__ | — | Off |
| __Bake Animation > All Actions__ | — | Off |
| __Bake Animation > Force Start/End Keying__ | — | On |
| __Bake Animation > Simplify__ | — | 0 |

Before exporting the mesh, set the armature's __Object Data Properties > Pose Position__ to __Rest Position__. Set it back to __Pose Position__ before exporting the animation.

!!! Warning
    A mesh exported with an animated pose bakes that pose into the geometry. The skeletal mesh then comes into the Unreal Editor deformed. A crouching or lying model is the usual sign. Export it again in the rest position.

---

## Import into the Unreal Editor

Launch the editor with `cmake --build Build --target launch` and open a folder under `Content/Carla/Static/Pedestrian/`. Drag the mesh FBX into the content browser and set these import options:

| Option | Value |
| --- | --- |
| __Skeleton__ | `Skel_Pedestrian_G2` (or `Skel_Pedestrian_G3`) |
| __Import Mesh__ | On |
| __Import Animations__ | Off |
| __Import Morph Targets__ | Off |
| __Create Physics Asset__ | Off |
| __Material Import Method__ | Do not create material |

Then open the new skeletal mesh and check the following:

* __Size.__ An adult is about 1.7 to 1.9 m tall, and its bounds are close to those of the pedestrian you started from. A model a hundred times too large or too small was exported with the wrong units.
* __Materials.__ Assign the existing material instances to the slots, or create new ones. Clothes are instances of `/Game/Carla/Static/GenericMaterials/000_Masters/Pedestrians/M_FabricMaster`. To recolour a garment, make a child of its instance and set `Color_1`: it tints the garment's diffuse texture, and its alpha is the strength of the tint. The shipped instances leave the alpha at 0, so set it to 1.
* __Physics asset.__ Assign the physics asset of the pedestrian you started from, for example `Phys_EuroF01_G2`, in __Asset Details > Physics__. CARLA pedestrians fall as ragdolls when a vehicle hits them, and that needs a physics asset.

---

## Create the walker blueprint

What CARLA spawns is not the skeletal mesh but a walker blueprint, one per pedestrian, in `Content/Carla/Blueprints/Walkers/`. They are built in three layers:

| Layer | Asset | What it holds |
| --- | --- | --- |
| C++ | `AWalkerBase` (an `ACharacter`) | Alive state, life span after death, wheelchair support. |
| Shared blueprint | `BP_Walker` | The behaviour every pedestrian shares: death when a vehicle hits it, with a death animation picked by the angle of the hit, then physics; the triggers that detect the hit (`PedestrianDeathTrigger`, `PedestrianPropDeathTrigger`); the `CarStopper` box; the props a pedestrian carries; the foot IK. |
| One per pedestrian | `BP_<name>`, for example `BP_EuroF01_A_G2` | Data only, with no graph of its own. It overrides the __Mesh__ component (skeletal mesh, __Anim Class__ and, for some, __Override Materials__), the capsule height and the character movement. |

The names follow `BP_<origin><kind><number>_<variant>_G<generation>`. `<kind>` is `F` for a woman, `M` for a man, `G` for a girl and `B` for a boy, and `<variant>` is a letter for each outfit, or `Cop` for a police officer. `BP_AfroF02_Cop_G2` is the police variant of the second African woman, generation 2.

A new pedestrian is a new `BP_<name>`. Duplicate the blueprint of the pedestrian you started from, so the defaults of its generation come along:

1. Right-click the blueprint, for example `BP_EuroF01_A_G2`, and choose __Duplicate__. Name it after your pedestrian, for example `BP_MyPedestrian_A_G2`.
2. Open it and select the __Mesh__ component (`CharacterMesh0`). Set __Skeletal Mesh Asset__ to your new mesh.
3. Keep the rest as it is:
    * __Anim Class__: `ABP_GEN2_vA` for adults, `ABP_GEN2_kids_vA` for generation 2 children, `ABP_GEN3` for generation 3.
    * The mesh offset and yaw `-90`, and the capsule. Both follow the height of the pedestrian: an adult uses `z = -92` and a half height of 95.75, a generation 2 child `z = -53.5`. Duplicate a pedestrian of the same height as yours, or adjust both together so the feet touch the ground.
4. Compile and save.

Do not change `BP_Walker` to customise one pedestrian. Every walker inherits from it.

!!! Note
    A variant that only changes colours does not need a new mesh. Duplicate the blueprint, keep its mesh, and set the slots to recolour in the __Override Materials__ of the __Mesh__ component. `BP_AfroB01_A_G3` overrides its skin material this way.

---

## Register the pedestrian

The walker blueprint is spawned by the walker factory, `BP_WalkerFactory` in `Content/Carla/Blueprints/Walkers/`. It is a blueprint of the C++ class `AWalkerActorFactory`, and `CarlaGameMode` creates it when the simulation starts. Each time an episode starts, the factory loads its list of walkers from `WalkerParameters.json` and turns every entry into a `walker.pedestrian.<Id>` blueprint. When one is spawned, the factory's `PostProcessWalker` applies the `is_invincible` attribute and gives the walker its AI controller.

Adding a pedestrian to the factory therefore means adding it to `WalkerParameters.json` in `${CARLA_ROOT}/Unreal/CarlaUnreal/Content/Carla/Config`. Add an entry with an `Id` no other walker uses and the class path of your blueprint. The class path ends in `_C`:

```json
{
    "Walkers": [
        ...,
        {
            "Id": "0053",
            "Class": "/Game/Carla/Blueprints/Walkers/BP_MyPedestrian_G2.BP_MyPedestrian_G2_C",
            "Gender": "Female",
            "Age": "Adult",
            "Speeds": [
                { "Speed": 0 },
                { "Speed": 1.7 },
                { "Speed": 4 }
            ],
            "Generation": 2,
            "CanUseWheelChair": true
        }
    ]
}
```

| Key | Values | Becomes the attribute |
| --- | --- | --- |
| `Id` | A string, unique across all walkers | The blueprint ID, `walker.pedestrian.<Id>` |
| `Class` | Path of the walker blueprint class | — |
| `Gender` | `Female`, `Male`, `Other` | `gender` |
| `Age` | `Child`, `Teenager`, `Adult`, `Elderly` | `age` |
| `Speeds` | Speeds in m/s, by convention still, walking and running | The recommended values of `speed` |
| `Generation` | `2` or `3`, matching the skeleton | `generation` |
| `CanUseWheelChair` | `true`, `false` (optional) | `can_use_wheelchair` |
| `SpecialType` | Empty for an ordinary pedestrian (optional) | `special_type` |

The keys are case-sensitive. The file is read again every time an episode starts, so after editing it press __Play__ again in the editor, or call `client.reload_world()` on a running server.

!!! Note
    The __Walkers Params__ array on `BP_WalkerFactory` is empty on purpose, and its __Load Pedestrian Parameters from Json__ and __Write Pedestrian Parameters To Json__ functions only move the list between the array and the file. The JSON is what the simulation reads. Edit it by hand: writing it from the factory adds `SpecialType` and `CanUseWheelChair` to every entry, which turns one new pedestrian into a diff of the whole file.

---

## Spawn it from Python

Start the simulation with __Play__ and spawn the pedestrian like any other. Give it a `controller.ai.walker` to make it walk. The walker navigation advances with the client's ticks, so drive the world in synchronous mode:

```py
import carla

client = carla.Client('localhost', 2000)
world = client.get_world()
bp_lib = world.get_blueprint_library()

original_settings = world.get_settings()
settings = world.get_settings()
settings.synchronous_mode = True
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

try:
    walker_bp = bp_lib.find('walker.pedestrian.0053')
    location = world.get_random_location_from_navigation()
    walker = world.spawn_actor(walker_bp, carla.Transform(location + carla.Location(z=1)))
    world.tick()

    controller = world.spawn_actor(bp_lib.find('controller.ai.walker'), carla.Transform(), walker)
    world.tick()
    controller.start()
    controller.go_to_location(world.get_random_location_from_navigation())
    controller.set_max_speed(float(walker_bp.get_attribute('speed').recommended_values[1]))

    for _ in range(200):  # 10 seconds
        world.tick()

    controller.stop()
    controller.destroy()
    walker.destroy()
finally:
    world.apply_settings(original_settings)
```

---

## Add a custom animation

An animation made for the CARLA skeleton is imported onto that skeleton and works on every pedestrian bound to it.

Drag the animation FBX into the content browser and set these import options:

| Option | Value |
| --- | --- |
| __Skeleton__ | The skeleton the animation was made for |
| __Import Mesh__ | Off |
| __Import Animations__ | On |
| __Animation Length__ | Exported Time |

The importer creates an animation sequence. Open it to check the timing and the pose. It plays on any mesh bound to the same skeleton.

The animation blueprints only blend CARLA's idle, walk, run and turn cycles, by speed. A new sequence is not played by any of them until you put it somewhere that plays it:

* __A pedestrian that only performs this animation.__ Create a walker blueprint as above. On its __Mesh__ component, set __Animation Mode__ to __Use Animation Asset__, __Anim to Play__ to your sequence, and enable __Looping__ and __Playing__. Register it with its own `Id`. This suits a walker standing in one place, such as someone exercising or waiting. If a `controller.ai.walker` moves it, it still plays the same animation.
* __Part of the locomotion.__ Add the sequence to the animation blueprint of its generation, for example as a new state in the state machine of `ABP_GEN2_vA`. This changes every pedestrian that uses that animation blueprint.
* __Driven from Python.__ The bones of a walker can be posed from a client with [`set_bones`](python_api.md#carla.Walker.set_bones), then shown with [`blend_pose`](python_api.md#carla.Walker.blend_pose). No asset needs to change, but the client has to send the pose every frame.

To see an animation in the level without running the simulation, place a __Skeletal Mesh Actor__ with your mesh, set its animation mode as in the first option, and enable __Update Animation in Editor__ on the component.

---

## Ship the pedestrian

Keep the meshes, materials, animations and blueprints under `/Game/Carla`, or they are left out of a package cook. Then build a package with `cmake --build Build --target package`.

To hand pedestrians to someone who runs a CARLA package, without rebuilding the simulator, put them in a [content pack](content_authoring_packs.md). The pack carries its own `Config/WalkerParameters.json`. Its entries are added to the catalogue, and an entry with the same `Id` as an existing walker replaces it.
