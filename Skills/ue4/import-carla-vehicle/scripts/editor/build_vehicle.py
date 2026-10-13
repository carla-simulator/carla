#!/usr/bin/env python
"""Import a vehicle mesh and assemble its blueprint, from inside UE4Editor.

Not an entry point. Run by ../import_vehicle.py as the FIRST of two boots:

    UE4Editor CarlaUE4.uproject -run=pythonscript -Script="<this file>"

The job description is read from $CARLA_VEHICLE_SPEC.

This is an orchestrator, not an implementation: the heavy lifting belongs to
CarlaTools' `UVehicleAuthoringLibrary` (CARLA PR #9805), which is editor-only C++
exposed to Python. Every asset-shaping step below is one of its functions:

    SetupVehiclePhysicsAsset   dedicated physics asset, convex chassis + kinematic
                               spheres on the four wheel bones
    CreateVehicleAnimBP        duplicate an anim blueprint and retarget its skeleton
    ConfigureWheel             radius / width / mass / steering on a UVehicleWheel CDO
    CreateVehicleBlueprint     duplicate a donor vehicle BP and repoint its inherited
                               native components (Mesh, VehicleMovement WheelSetups)
    CompileAndSaveBlueprint    FKismetEditorUtilities::CompileBlueprint + save

Doing it any other way means reimplementing PhysX wheel setup in Python, so this
script fails loudly when the library is absent rather than improvising.

Registration in VehicleFactory is NOT done here — see CarlaTools'
add_vehicle_to_vehicle_factory.py, driven by ../import_vehicle.py.
"""
import json
import os
import traceback

import unreal

CM_PER_M = 100.0

# The canonical PxVehicleDrive4W bone order: front-left, front-right, rear-left,
# rear-right. CreateVehicleBlueprint pairs Wheels[i] with WheelBones[i], and
# SetupVehiclePhysicsAsset builds a kinematic sphere on each of these names.
WHEEL_BONES = ["Wheel_Front_Left", "Wheel_Front_Right",
               "Wheel_Rear_Left", "Wheel_Rear_Right"]


def log(msg):
    unreal.log("[build_vehicle] {}".format(msg))


def require_library():
    if not hasattr(unreal, "VehicleAuthoringLibrary"):
        raise RuntimeError(
            "CarlaTools does not expose VehicleAuthoringLibrary. It arrived with CARLA "
            "PR #9805; without it there is no scripted way to build a vehicle's physics "
            "asset or wheel setups. Rebuild CarlaTools against a checkout that has it.")
    return unreal.VehicleAuthoringLibrary


def clean_destination(spec):
    """Delete a previous import of this vehicle so this one is never a REIMPORT."""
    for asset in (spec["bp_destination"].rstrip("/") + "/" + spec["bp_name"],):
        if unreal.EditorAssetLibrary.does_asset_exist(asset):
            log("deleting previous blueprint {}".format(asset))
            unreal.EditorAssetLibrary.delete_asset(asset)
    destination = spec["mesh_destination"].rstrip("/")
    if not unreal.EditorAssetLibrary.does_directory_exist(destination):
        return []
    existing = list(unreal.EditorAssetLibrary.list_assets(
        destination, recursive=True, include_folder=False) or [])
    for path in existing:
        package = path.split(".")[0]
        try:
            unreal.EditorAssetLibrary.delete_asset(package)
        except Exception as exc:  # noqa: BLE001
            log("could not delete {}: {}".format(package, exc))
    return existing


def build_import_options(spec):
    """FbxImportUI for a vehicle: a skeletal mesh on its OWN new skeleton.

    Unlike walkers, vehicles do not share one skeleton — each mesh brings its own
    (SK_Ambulance_Skeleton, SM_LincolnMKZ_2K17_Skeleton, ...), because the bones are
    that vehicle's wheels and chassis. So no skeleton is supplied and UE creates one;
    what matters is that the four wheel bone NAMES are present (check_input.py).
    """
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import",
                                unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    # A physics asset is built afterwards by SetupVehiclePhysicsAsset, which needs
    # wheel bodies kinematic and a convex chassis — not what the importer generates.
    options.set_editor_property("create_physics_asset", False)

    mesh_data = options.skeletal_mesh_import_data
    mesh_data.set_editor_property("convert_scene", True)
    mesh_data.set_editor_property("convert_scene_unit", True)
    mesh_data.set_editor_property("normal_import_method",
                                  unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS)
    return options


def import_fbx(spec):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", spec["fbx"])
    task.set_editor_property("destination_path", spec["mesh_destination"])
    task.set_editor_property("destination_name", spec["mesh_name"])
    task.set_editor_property("options", build_import_options(spec))
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    # The task saves the mesh package only; the skeleton, materials and textures it
    # also created stay in memory and would read back as None next boot.
    unreal.EditorAssetLibrary.save_directory(
        spec["mesh_destination"], only_if_is_dirty=False, recursive=True)

    paths = list(task.get_editor_property("imported_object_paths") or [])
    if not paths:
        paths = list(unreal.EditorAssetLibrary.list_assets(
            spec["mesh_destination"], recursive=True, include_folder=False) or [])
    return paths


def skeletal_mesh_among(object_paths, hint):
    meshes = {}
    for path in object_paths:
        try:
            asset = unreal.load_asset(path)
        except Exception:
            continue
        if isinstance(asset, unreal.SkeletalMesh):
            meshes[asset.get_path_name()] = asset
    if not meshes:
        return None, meshes
    if len(meshes) == 1:
        return list(meshes.values())[0], meshes
    if hint and hint in meshes:
        return meshes[hint], meshes
    return None, meshes


def bone_names(mesh):
    """Bone names on the mesh's skeleton, via the reference skeleton's bone tree."""
    skeleton = mesh.get_editor_property("skeleton")
    if skeleton is None:
        return []
    try:
        return [str(n) for n in skeleton.get_editor_property("bone_tree")]
    except Exception:
        # bone_tree is not always readable; the mesh's own bone list is enough for
        # reporting, and check_input.py already gated on the FBX names.
        return []


def measure(mesh):
    bounds = mesh.get_bounds()
    half = bounds.box_extent
    return {
        "half_extent_cm": [round(abs(half.x), 2), round(abs(half.y), 2),
                           round(abs(half.z), 2)],
        "length_m": round(2 * abs(half.x) / CM_PER_M, 3),
        "width_m": round(2 * abs(half.y) / CM_PER_M, 3),
        "height_m": round(2 * abs(half.z) / CM_PER_M, 3),
    }


def material_slots(mesh):
    slots = []
    try:
        for entry in mesh.get_editor_property("materials"):
            material = entry.get_editor_property("material_interface")
            slots.append({
                "slot": str(entry.get_editor_property("material_slot_name")),
                "material": material.get_path_name() if material else None,
            })
    except Exception as exc:  # noqa: BLE001
        log("could not read material slots: {}".format(exc))
    return slots


def make_wheels(library, spec, result):
    """Duplicate the donor wheel blueprints and configure each one.

    Wheels are their own blueprints (UVehicleWheel subclasses) because PxVehicle reads
    radius, width, mass, steering angle and handbrake off the wheel CLASS, not the
    vehicle. Four are needed, in the canonical order, and the front pair is the only
    one that steers.
    """
    donors = spec["donor_wheels"]
    if len(donors) != 4:
        raise RuntimeError("need exactly 4 donor wheel blueprints, got %d" % len(donors))

    radius = float(spec["wheel_radius_cm"])
    width = float(spec.get("wheel_width_cm") or radius * 0.45)
    mass = float(spec.get("wheel_mass_kg") or 20.0)
    steer = float(spec.get("steer_angle_deg") or 70.0)

    created = []
    for index, donor in enumerate(donors):
        suffix = ("FLW", "FRW", "RLW", "RRW")[index]
        dest = "{}/{}_{}".format(spec["bp_destination"].rstrip("/"), spec["bp_name"], suffix)
        if unreal.EditorAssetLibrary.does_asset_exist(dest):
            unreal.EditorAssetLibrary.delete_asset(dest)
        if not unreal.EditorAssetLibrary.does_asset_exist(donor):
            raise RuntimeError("donor wheel %s does not exist" % donor)
        if unreal.EditorAssetLibrary.duplicate_asset(donor, dest) is None:
            raise RuntimeError("could not duplicate wheel %s -> %s" % (donor, dest))

        wheel_class = unreal.load_object(None, "{}.{}_C".format(dest, dest.rsplit("/", 1)[-1]))
        if wheel_class is None:
            raise RuntimeError("cannot load duplicated wheel class for %s" % dest)
        # Only the front wheels steer; the rear pair takes the handbrake.
        front = index < 2
        ok = library.configure_wheel(wheel_class, radius, width, mass,
                                     steer if front else 0.0, not front, None)
        created.append({"path": dest, "class": str(wheel_class), "configured": bool(ok),
                        "bone": WHEEL_BONES[index], "steers": front})
    result["wheels"] = created
    return [unreal.load_object(None, "{}.{}_C".format(w["path"], w["path"].rsplit("/", 1)[-1]))
            for w in created]


def build(spec):
    result = {"name": spec["name"], "source": spec["fbx"], "ok": False}
    try:
        library = require_library()
        result["authoring_library"] = "CarlaTools VehicleAuthoringLibrary"

        removed = clean_destination(spec)
        if removed:
            result["replaced_previous_import"] = sorted(removed)

        log("importing {} -> {}".format(spec["fbx"], spec["mesh_destination"]))
        imported = import_fbx(spec)
        result["imported_objects"] = sorted(imported)

        mesh, candidates = skeletal_mesh_among(imported, spec.get("mesh_hint"))
        result["skeletal_meshes"] = sorted(candidates)
        if mesh is None:
            result["error"] = (
                "the import produced %d skeletal meshes and none was nominated:\n  %s\n"
                "Re-run with --mesh <object path>, or export one vehicle per FBX."
                % (len(candidates), "\n  ".join(sorted(candidates)))
                if candidates else
                "the import produced no SkeletalMesh. A CARLA vehicle must be a skinned "
                "mesh whose wheels are bones; read the FIRST FBX error in the log.")
            return result
        result["mesh_path"] = mesh.get_path_name()
        skeleton = mesh.get_editor_property("skeleton")
        result["skeleton"] = skeleton.get_path_name() if skeleton else None
        if skeleton is None:
            result["error"] = "the imported mesh has no skeleton — nothing can drive it"
            return result

        result.update(measure(mesh))
        result["material_slots"] = material_slots(mesh)
        log("{} measures {} x {} x {} m".format(
            spec["name"], result["length_m"], result["width_m"], result["height_m"]))

        # 1. physics asset: convex chassis, kinematic spheres on the wheel bones
        physics_ok = library.setup_vehicle_physics_asset(
            mesh, None, float(spec["wheel_radius_cm"]))
        result["physics_asset_built"] = bool(physics_ok)
        physics = mesh.get_editor_property("physics_asset")
        result["physics_asset"] = physics.get_path_name() if physics else None
        if not physics_ok or physics is None:
            result["error"] = ("SetupVehiclePhysicsAsset failed — without wheel bodies "
                              "PxVehicle cannot raycast the suspension. See the editor "
                              "log for the body layout it reported.")
            return result

        # 2. anim blueprint: duplicate a template and retarget to this skeleton
        anim_dest = "{}/AnimBP_{}".format(spec["mesh_destination"].rstrip("/"), spec["name"])
        anim = library.create_vehicle_anim_bp(
            skeleton, unreal.load_asset(spec["donor_anim_bp"]), anim_dest)
        result["anim_bp"] = anim.get_path_name() if anim else None
        if anim is None:
            result["error"] = ("CreateVehicleAnimBP failed — the vehicle would have no "
                               "wheel rotation or suspension animation.")
            return result

        # 3. wheels
        wheels = make_wheels(library, spec, result)

        # 4. the spawnable vehicle blueprint
        # RaycastMesh is the CustomCollision hull CARLA uses for sensor raycasts. It is
        # an SCS-inherited component, so when none is given the duplicate keeps the
        # DONOR's hull — correct-looking for a same-shaped vehicle, wrong for anything
        # else, which is why the inherited value is reported below.
        raycast = unreal.load_asset(spec["collision_mesh"]) if spec.get("collision_mesh") else None
        if spec.get("collision_mesh") and raycast is None:
            raise RuntimeError("cannot load collision mesh %s" % spec["collision_mesh"])
        blueprint = library.create_vehicle_blueprint(
            spec["bp_name"], spec["bp_destination"], unreal.load_asset(spec["donor_bp"]),
            mesh, anim, raycast, wheels, [unreal.Name(b) for b in WHEEL_BONES])
        if blueprint is None:
            result["error"] = "CreateVehicleBlueprint failed — see the editor log"
            return result
        bp_path = blueprint.get_path_name().split(".")[0]
        result["bp_path"] = bp_path
        result["bp_class"] = "{}.{}_C".format(bp_path, spec["bp_name"])

        # The library sets the mesh on the CDO and saves WITHOUT recompiling, because a
        # recompile reverts inherited native-component slots to the parent default. Read
        # it back so a silent revert cannot pass as success.
        generated = unreal.load_object(None, result["bp_class"])
        cdo = unreal.get_default_object(generated) if generated else None
        bound = None
        if cdo is not None:
            component = cdo.get_editor_property("mesh")
            bound = component.get_editor_property("skeletal_mesh")
            result["cdo_skeletal_mesh"] = bound.get_path_name() if bound else None
            anim_class = component.get_editor_property("anim_class")
            result["cdo_anim_class"] = str(anim_class) if anim_class else None
        # Report the raycast hull actually in place, inherited or set.
        try:
            collision = unreal.load_object(generated, "CustomCollision_GEN_VARIABLE")
            hull = collision.get_editor_property("static_mesh") if collision else None
            result["collision_mesh"] = hull.get_path_name() if hull else None
            result["collision_mesh_inherited"] = (
                bool(hull) and not spec.get("collision_mesh"))
        except Exception as exc:  # noqa: BLE001 - reporting only
            log("no CustomCollision component: {}".format(exc))
        if bound is None:
            result["error"] = ("the vehicle blueprint's CDO has no SkeletalMesh — the "
                               "native-component override did not persist.")
            return result

        unreal.EditorAssetLibrary.save_directory(
            spec["mesh_destination"], only_if_is_dirty=False, recursive=True)
        result["persisted_assets"] = sorted(unreal.EditorAssetLibrary.list_assets(
            spec["mesh_destination"], recursive=True, include_folder=False) or [])
        result["ok"] = True
    except Exception as exc:  # noqa: BLE001
        result["error"] = "{}: {}".format(type(exc).__name__, exc)
        result["traceback"] = traceback.format_exc()
        unreal.log_error("[build_vehicle] " + result["traceback"])
    return result


def main():
    spec_path = os.environ.get("CARLA_VEHICLE_SPEC", "")
    result_path = ""
    try:
        if not spec_path or not os.path.isfile(spec_path):
            raise RuntimeError("CARLA_VEHICLE_SPEC does not name a spec file: %r" % spec_path)
        with open(spec_path, "r") as handle:
            spec = json.load(handle)
        result_path = spec.get("result", "")
        result = build(spec)
    except Exception as exc:  # noqa: BLE001
        result = {"ok": False, "error": "{}: {}".format(type(exc).__name__, exc),
                  "traceback": traceback.format_exc()}
        unreal.log_error("[build_vehicle] " + result["traceback"])

    if result_path:
        with open(result_path, "w") as handle:
            json.dump(result, handle, indent=2)
    log("done: ok={}".format(result.get("ok")))


main()
