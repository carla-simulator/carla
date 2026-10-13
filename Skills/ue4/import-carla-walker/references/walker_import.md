# Walker import — pipeline, schema and constraints

Detail layer for `import-carla-walker`: what the importer does, the schema it writes,
and the constraints that make an import correct.

> **Provenance.** CARLA documents importing props (`Docs/tuto_A_add_props.md`) and
> vehicles (`Docs/tuto_A_add_vehicle.md`) but **not pedestrians**. Everything here is
> derived from the source and shipped content of a 0.9.16 checkout:
> `Carla/Actor/PedestrianParameters.h`,
> `Carla/Actor/ActorBlueprintFunctionLibrary.cpp:1663` (`MakePedestrianDefinition`),
> `Carla/Walker/WalkerBase.h`, `Carla/Game/Tagger.cpp:32`,
> `Content/Carla/Blueprints/Walkers/*`, `Content/Carla/Static/Pedestrian/**`, and
> CarlaTools' `VehicleAuthoringLibrary` plus its `add_*_to_*_factory.py` scripts.

## Expected input

| requirement | detail |
|---|---|
| format | **FBX**, binary or ASCII. `.ma`, `.mb`, `.blend`, `.max`, `.c4d`, `.obj`, USD, glTF and COLLADA are not importable by Unreal |
| kind | skinned/skeletal mesh (skin clusters present) |
| rig | exactly the 26 GEN3 `crl_*` bones below |
| textures | embedded ("Embed Media") or beside the FBX; author-machine absolute paths resolve nowhere and yield blank white materials |
| contents | one character per file; animation is not imported |
| units | cm or m — scene units are converted on import |

`scripts/check_input.py` enforces the first three and reports the fourth. It is the
first step of the skill for a reason: every one of these is cheaper to catch in a
sub-second file scan than after an editor boot.

## The GEN3 rig contract

CARLA's current pedestrian generation is GEN3, and it is one skeleton:

```
/Game/Carla/Static/Pedestrian/ZBAsiaM/Gen3_test/Skel__GEN3      26 bones

crl_root
crl_hips__C  crl_spine__C  crl_spine01__C  crl_neck__C  crl_Head__C
crl_eye__L/R  crl_shoulder__L/R  crl_arm__L/R  crl_foreArm__L/R  crl_hand__L/R
crl_thigh__L/R  crl_leg__L/R  crl_foot__L/R  crl_toe__L/R  crl_toeEnd__L/R
```

No finger bones — the rig is deliberately small. These are also the names
`walker.get_bones()` returns over the Python API, which is what makes the bone check
in `verify_walker.py` meaningful.

An FBX skinned to exactly these names binds to the existing skeleton, and then
`ABP_GEN3`, `BS_GEN3` and every `AS_*_G3` sequence apply with **no retargeting**.
`check_input.py` compares names only: a rig matching by name but not by joint structure
imports and animates *wrongly* rather than not at all.

`FbxImportUI.skeleton` is a request, not a guarantee — a mismatched FBX ends up on some
other skeleton, looks perfect in the content browser, and no animation applies. The
importer reads `mesh.skeleton` back after importing and fails when it is not GEN3.

## The asset chain of a GEN3 walker

Read off `BP_Walker_AG001_G3`, the shape every import reproduces:

| piece | example |
|---|---|
| walker blueprint | `/Game/Carla/Blueprints/Walkers/BP_Walker_AG001_G3` (child of `BP_Walker`) |
| skeletal mesh | `…/Static/Pedestrian/AG001_G3/Meshes/SK_AfroGirl01_G3` |
| skeleton | `…/Pedestrian/ZBAsiaM/Gen3_test/Skel__GEN3` |
| physics asset | `…/AG001_G3/Meshes/Phys_AfroGirl01_G3` (ragdoll on death) |
| anim class | `…/Pedestrian/Animations/GEN3/ABP_GEN3` |
| materials | `…/AG001_G3/MI/MI_*` |
| hair | a `GroomComponent` (`Groom_GEN_VARIABLE`, HairStrandsCore) |
| factory entry | `WalkerFactory.Pedestrians` → `walker.pedestrian.0050` |

`BP_Walker` supplies the inherited machinery: `CapsuleComponent`,
`PedestrianDeathTrigger`, `CarStopper`, and the wheelchair hooks driven by
`AWalkerBase::bUsesWheelChair`.

**The `Pedestrian` folder is the semantic label.** `ATagger` derives the segmentation
label from a folder name in the asset path (`Tagger.cpp:32`: `String == "Pedestrian"`
→ `CityObjectLabel::Pedestrians`). A mesh imported outside `…/Static/Pedestrian/…`
spawns and animates perfectly while the semantic camera labels it wrong, with nothing
raised. The destination is not cosmetic.

## What the importer does

### Boot 1 — `-run=pythonscript`, `editor/build_walker.py`

1. **Clean** — remove any previous import at the destination, so a re-run is always a
   fresh import rather than a reimport. The host clears the packages before the editor
   boots, because a blueprint still referencing the mesh prevents its deletion from
   inside the editor.
2. **Import** — `AssetImportTask` + `FbxImportUI`:
   `mesh_type_to_import = FBXIT_SKELETAL_MESH`, `skeleton = Skel__GEN3`,
   `create_physics_asset = True`, `import_animations = False`,
   `convert_scene_unit = True`, into `/Game/Carla/Static/Pedestrian/<Name>/Meshes`.
3. **Read back** — `task.imported_object_paths` is what UE says it created; each is
   loaded and kept only if it really is a `unreal.SkeletalMesh`. The skeleton binding
   is verified here, and the mesh is measured.
4. **Persist** — `save_directory` on the destination. `AssetImportTask.save` writes the
   mesh package only, leaving materials and the physics asset as references to packages
   that never reach disk; the host re-checks every referenced package against the
   filesystem and warns per dangling slot.
5. **Build the blueprint** — duplicate a GEN3 donor, repoint its mesh, apply the
   collision convention, optionally set the groom, save without recompiling (C2).

### Boot 2 — full editor, CARLA's own registration script

```
Unreal/CarlaUE4/Plugins/CarlaTools/Content/Python/add_walker_to_walker_factory.py
```

Part of the CARLA repo, sitting next to `add_vehicle_to_vehicle_factory.py` and
runnable by hand. It appends — or updates in place, matching on the blueprint class —
one `FPedestrianParameters` in `WalkerFactory.Pedestrians`, then compiles and saves the
blueprint through Kismet, the scripted equivalent of the editor's Compile and Save. It
allocates the id from the array and refuses to save if the array would shrink. The
skill only supplies its arguments and reads its JSON summary.

A full editor session rather than the commandlet, because the factory blueprint has to
compile.

The host then confirms the artifacts itself: every reported path is resolved to a
`.uasset` and checked against a size floor. The editor exiting 0 is not evidence.

## Schema: `FPedestrianParameters`

`Carla/Actor/PedestrianParameters.h`, consumed by `MakePedestrianDefinition`:

| field | type | becomes |
|---|---|---|
| `Id` | `FString` | the blueprint id: `walker.pedestrian.<Id>`, **lowercased** |
| `Class` | `TSubclassOf<ACharacter>` | the walker blueprint's generated class |
| `Gender` | `EPedestrianGender` | attribute `gender` = female / male / other |
| `Age` | `EPedestrianAge` | attribute `age` = child / teenager / adult / elderly |
| `Speed` | `TArray<float>` | variation `speed`, recommended values (idle, walk, run) |
| `Generation` | `int32` | attribute `generation` |
| `bCanUseWheelChair` | `bool` | attribute `can_use_wheelchair` |

`Speed` is a *variation* with `bRestrictToRecommended = false`
(`ActorBlueprintFunctionLibrary.cpp:1713-1723`), so a client may request any speed; the
three values are the recommendations `spawn_walkers` reads. Stock triples: adults and
elderly `(0.0, 1.7, 4.0)`, children and teenagers `(0.0, 1.1, 2.0)`.

Ids are 4-digit and dense — `0001`…`0052` in stock 0.9.16 — so the next is `max + 1`.

---

## Constraints

### C1 — The pedestrian array must be a blueprint MEMBER variable

Registration reads and writes the array by reflection (`get_editor_property`), exactly
as CARLA's vehicle script does with `VehicleFactory.Vehicles`. That works only for a
**member** variable.

Stock 0.9.16 keeps the list in `Walkers`, a variable **local to the
`GenerateDefinitions` function**. Function locals are not class properties, so no name
reaches them: every editor-side read returns 0 entries while a running server reports
all 52 walkers, because a local is materialised from its default only when the function
executes.

**Promote it once, in the editor:** add a member variable `Pedestrians` (Array of
`PedestrianParameters`), move the entries into its default value, repoint the `Get`
node inside `GenerateDefinitions`, then Compile and Save. `VehicleFactory` is already
authored this way, which is why vehicles register from a script and stock walkers
cannot.

`check_env.sh` reports whether the member exists and the importer pre-flights it before
spending an editor boot. Without it the import still builds the walker and prints a
paste-ready entry to finish by hand. `CARLA_WALKER_FACTORY_ARRAY` overrides the name
(default `Pedestrians`).

### C2 — Never recompile the walker blueprint after writing its CDO

The skeletal mesh and anim class live on the inherited **native** component
`ACharacter::Mesh` (`CharacterMesh0`). A donor already carries those override slots in
its generated-class CDO — a slot that serialises — so overwriting the values on the
already-compiled duplicate and saving persists. **A recompile reverts native-component
slots to the parent default**, producing a walker whose `SkeletalMesh` is `None`. Hence
`save_asset`, never a compile, in `build_walker.py`.

This is also why a walker is built by duplicating a donor rather than subclassing
`BP_Walker`: brand-new overrides on a fresh blueprint do not serialise. CarlaTools'
`VehicleAuthoringLibrary` documents the same rule for vehicles.

### C3 — GEN3 collision numbers are a convention, not a measurement

| blueprint | mesh scale | capsule half-height | mesh z | mesh half-extent z | visible height |
|---|---|---|---|---|---|
| `BP_Walker_AB001_G3` (kid) | 0.65 | 93.0 | -94.70 | 92.13 | 1.20 m |
| `BP_Walker_AG001_G3` (teen) | 0.65 | 93.0 | -94.70 | 93.15 | 1.21 m |
| `BP_WalkerKid1_v1` (GEN1) | 1.0 | 65.0 | -58.00 | 97.06 | 1.94 m |
| `BP_Walker_MaleEuro_v2` (GEN2) | 1.0 | 93.0 | -92.00 | 91.76 | 1.84 m |

Two things follow. Every GEN3 mesh is authored at ~1.84 m and scaled to 0.65 in the
blueprint, so the mesh's own bounds say nothing about the walker's height — only
`2 × box_extent.z × mesh_scale.z` does (`box_extent` is a HALF extent). And the GEN3
collision numbers are identical for kid and teen — a 1.86 m capsule around a 1.2 m
body — i.e. a generation-wide convention rather than a fit to the mesh.

So the importer **inherits** the donor's scale, capsule and offset, and checks that the
new mesh's unscaled bounds are within 10% of the donor's; outside that it warns and
names the overrides (`--capsule-half-height`, `--capsule-radius`, `--mesh-z`,
`--mesh-scale`). Radius is not derivable — a T-pose x extent is the arm span — so the
donor's `18.77` cm is kept.

Consequence for verification: a walker's origin is its capsule centre, ~0.93 m above
its feet, so comparing actor z to the road fails on *stock* walkers.
`verify_walker.py` compares the BOTTOM of the actor's bounding box instead.

### C4 — Textures come from the FBX, or not at all

An FBX that embeds no textures still declares material NAMES, so the import writes one
blank white material per slot — bound, not unassigned — and the walker renders flat
white. Diagnose before importing:

```
strings file.fbx | grep -iE '\.(png|tga|jpg|bmp)'   # absolute author paths, or nothing
strings file.fbx | grep -c LayerElementUV           # UVs are a separate question
```

Absolute paths from the author's machine (`D:/…/T_Shoes_d.png`) resolve to nothing
elsewhere. The fix is the FBX re-exported with **Embed Media**, or the texture files
placed alongside it.

`--materials donor` binds materials by slot name over blank and unassigned slots: first
from the donor's matching slot, then from any material asset under `Static/Pedestrian`
whose name equals the slot name — an FBX exported *out of* Unreal names its slots after
the materials that were assigned, which makes that second route work. **It only looks
right when the mesh shares the donor's UV layout**: on a different-topology variant of
the same character the bindings are correct and the result is still flat gray. Look at
the result. Flat white (no materials) is a more honest failure than flat gray (wrong
materials), which is why the default is to import raw and report every slot.

Materials are art: instancing `Static/GenericMaterials/Pedestrian_Shaders` per
character stays manual.

### C5 — A duplicated walker wears the donor's hair

Hair is a `GroomComponent` added in the donor's construction script, reachable as
`unreal.load_object(generated_class, "Groom_GEN_VARIABLE")`. Which groom to use is an
art choice, so the importer reports the inherited value on every run and `--groom` sets
it.

### C6 — Editor invocation

`PythonScriptCommandlet::Main` runs inside `FEngineLoop::PreInitPostStartupScreen`,
before editor init: the fast `-run=pythonscript` path cannot compile blueprints, and
anything needing a viewport is unavailable there (`EditorLevelLibrary.spawn_actor_*`
segfaults without one). Registration therefore uses a full editor session.

`-ExecutePythonScript=<path> <argv>` passes arguments through to the script, which is
how CARLA's registration scripts are driven. The host polls for the script's result
file and stops the editor itself, because `quit_editor` is best-effort headless.

Both boots pass `-nocrashreports`, but note it does **not** suppress the upload:
`-unattended` makes CrashReportClient auto-agree, so a crashing editor still sends a
minidump and project paths to `datarouter.ol.epicgames.com`. Silencing it needs
`bAgreeToCrashUpload=False` in the engine's `CrashReportClient` config — an
engine-level change this skill does not make.

---

## Relationship to the sibling skills

| skill | boundary |
|---|---|
| [[build-carla-ue4]] | builds UE4 + CARLA; this skill checks and defers, never builds |
| [[run-carla-server]] | starts the server verification needs (`-game -nullrhi` is enough) |
| [[spawn-walkers]] | spawns walkers at runtime, including newly imported ones |
| [[import-carla-prop]] | static meshes; a non-skinned FBX belongs there |
| [[import-carla-vehicle]] | the 4-wheeled counterpart, built on CarlaTools' `VehicleAuthoringLibrary` |
| [[package-carla-ue4]] | packaging; a walker cannot be a self-contained package — `WalkerFactory` is shared content |
