#!/usr/bin/env python
"""Import a walker mesh and build its blueprint, from inside UE4Editor.

Not an entry point. Run by ../import_walker.py as the FIRST of two boots:

    UE4Editor CarlaUE4.uproject -run=pythonscript -Script="<this file>"

The job description is read from the file named by $CARLA_WALKER_SPEC — arguments
go through the environment because the pythonscript commandlet hands `-Script=`
to ExecPythonCommandEx as one command string (PythonScriptCommandlet.cpp:12-33),
where quoting an argv is fragile.

Five steps that are only correct together:

  1. import the FBX as a SkeletalMesh bound to the EXISTING GEN3 skeleton
  2. read back what UE says it created, and confirm the binding
  3. duplicate a GEN3 donor blueprint and repoint it at the new mesh
  4. compare the new mesh against the donor's, and keep the donor's collision
     geometry when they match (references, C3)
  5. persist and report what the import produced: physics asset, material slots

Step 3 is a duplicate rather than a fresh subclass for the reason CarlaTools'
VehicleAuthoringLibrary spells out for vehicles: the skeletal mesh and anim class
live on an inherited NATIVE component (ACharacter::Mesh, "CharacterMesh0"). A
donor already carries those override slots in its generated-class CDO — a slot
proven to serialise — so overwriting the values on the already-compiled duplicate
persists. Establishing brand-new overrides on a fresh blueprint does not.

And for the same reason the duplicate is saved WITHOUT recompiling: a recompile
reverts native-component slots to the parent default, which is how you end up
with a walker whose SkeletalMesh is None.

Registration in WalkerFactory is deliberately NOT done here — see
CARLA's own add_walker_to_walker_factory.py, driven by ../import_walker.py (C1).
"""
import json
import os
import traceback

import unreal

# Unreal works in centimetres, CARLA's Python API in metres.
CM_PER_M = 100.0

# How far the new mesh's unscaled bounds may differ from the donor's before the
# inherited collision numbers stop being trustworthy. Within this, the donor's
# shipped values are kept verbatim (C3); outside it, the caller is told to pass
# explicit overrides.
BOUNDS_TOLERANCE = 0.10


def log(msg):
    unreal.log("[build_walker] {}".format(msg))


def build_import_options(spec):
    """FbxImportUI for a walker: one skeletal mesh, on an existing skeleton."""
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    # The GEN3 animations already exist; importing takes from the FBX only what
    # the FBX is authoritative for, which is the mesh and its skinning.
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import",
                                unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    # Binding to the EXISTING skeleton is the whole point: a new skeleton would
    # be structurally identical and still drive none of the GEN3 animations.
    skeleton = unreal.load_asset(spec["skeleton"])
    if skeleton is None:
        raise RuntimeError("cannot load the GEN3 skeleton at %s" % spec["skeleton"])
    options.set_editor_property("skeleton", skeleton)
    # The import creates <Name>_PhysicsAsset from this; ensure_physics_asset() below
    # falls back to sharing the donor's if it ever produces none.
    options.set_editor_property("create_physics_asset",
                                bool(spec.get("physics_asset", True)))

    mesh_data = options.skeletal_mesh_import_data
    mesh_data.set_editor_property("import_morph_targets", False)
    mesh_data.set_editor_property("update_skeleton_reference_pose", False)
    mesh_data.set_editor_property("convert_scene", True)
    # FBX authored in metres, Unreal in centimetres.
    mesh_data.set_editor_property("convert_scene_unit", True)
    mesh_data.set_editor_property("normal_import_method",
                                  unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS)
    return options


def clean_destination(spec):
    """Delete a previous import at the destination, so this one is never a REIMPORT.

    UE's reimport path (SkeletalMeshHelper::RestoreExistingSkelMeshData, called
    when an asset already exists at the destination) SEGFAULTS in a commandlet
    while restoring the existing asset's metadata — measured, reproducible, and it
    takes the whole editor down before the result file is written. So a re-run
    deletes first and imports clean. Idempotency by replacement, not by reimport
    Idempotency by replacement, not by reimport.
    """
    # The blueprint goes FIRST: while it exists it references the mesh, and a
    # referenced asset does not delete — which is how the reimport path gets
    # entered even after an apparent cleanup. The host also clears both before
    # booting (clean_previous), so this is the second line of defence.
    bp_path = "{}/{}".format(spec["bp_destination"].rstrip("/"), spec["bp_name"])
    if unreal.EditorAssetLibrary.does_asset_exist(bp_path):
        log("deleting previous blueprint {}".format(bp_path))
        unreal.EditorAssetLibrary.delete_asset(bp_path)

    destination = spec["mesh_destination"].rstrip("/")
    if not unreal.EditorAssetLibrary.does_directory_exist(destination):
        return []
    existing = list(unreal.EditorAssetLibrary.list_assets(
        destination, recursive=True, include_folder=False) or [])
    for path in existing:
        package = path.split(".")[0]
        log("deleting previous import {}".format(package))
        try:
            unreal.EditorAssetLibrary.delete_asset(package)
        except Exception as exc:  # noqa: BLE001 - report, do not abort the run
            log("could not delete {}: {}".format(package, exc))
    return existing


def import_fbx(spec):
    """Run the import; return the object paths UE reports having created."""
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", spec["fbx"])
    task.set_editor_property("destination_path", spec["mesh_destination"])
    # Name the package deterministically rather than letting UE derive it from the
    # FBX. The OBJECT inside can still take an FBX node's name (a multi-node FBX
    # merges into one mesh named after a node, e.g. "<Name>_SM_Shoes1"), which is
    # why the real object path is read back rather than assumed.
    task.set_editor_property("destination_name", spec["name"])
    task.set_editor_property("options", build_import_options(spec))
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    # AssetImportTask.save saves the MESH package and nothing else: the materials,
    # textures and physics asset the import also creates stay in memory, so the
    # mesh ends up referencing packages that never reach disk. The next boot then
    # logs "Failed to load '/Game/.../blinn1': Can't find file" for each and reads
    # them back as None. Saving the whole destination is what persists them.
    saved_dir = unreal.EditorAssetLibrary.save_directory(
        spec["mesh_destination"], only_if_is_dirty=False, recursive=True)
    log("save_directory({}) -> {}".format(spec["mesh_destination"], saved_dir))

    paths = list(task.get_editor_property("imported_object_paths") or [])
    if not paths:
        # Some import paths populate the asset registry but leave this empty.
        # Listing the destination is second best - it also sees assets from an
        # earlier run - but it beats guessing a name from the FBX stem.
        log("imported_object_paths empty; listing {}".format(spec["mesh_destination"]))
        paths = list(unreal.EditorAssetLibrary.list_assets(
            spec["mesh_destination"], recursive=True, include_folder=False) or [])
    return paths


def skeletal_meshes_among(object_paths):
    """Map object path -> asset, for the paths that really load as a SkeletalMesh."""
    meshes = {}
    for path in object_paths:
        try:
            asset = unreal.load_asset(path)
        except Exception:
            continue
        if isinstance(asset, unreal.SkeletalMesh):
            meshes[asset.get_path_name()] = asset
    return meshes


def measure(mesh):
    """Measure the mesh: HALF extents in cm, plus the unscaled height in metres.

    FBoxSphereBounds::BoxExtent is a HALF extent — centre to face, not full size.
    So the unscaled height is TWICE box_extent.z. This is the mesh as authored,
    NOT as it appears in game: every GEN3 walker blueprint scales its mesh by 0.65,
    so the visible height is this times that scale (see visible_height_m).
    """
    bounds = mesh.get_bounds()
    half = bounds.box_extent
    return {
        "half_extent_cm": [round(abs(half.x), 2), round(abs(half.y), 2),
                           round(abs(half.z), 2)],
        "unscaled_height_m": round(2 * abs(half.z) / CM_PER_M, 3),
    }


def material_slots(mesh):
    """Slot names AND what is bound to each.

    A slot bound to None, or to a blank material the import wrote because the FBX
    named a material it does not carry, renders untextured — worth reporting per slot
    rather than summarising as "imported" (C4).
    """
    slots = []
    try:
        for entry in mesh.get_editor_property("materials"):
            material = entry.get_editor_property("material_interface")
            slots.append({
                "slot": str(entry.get_editor_property("material_slot_name")),
                "material": material.get_path_name() if material else None,
            })
    except Exception as exc:  # noqa: BLE001 - reporting only
        log("could not read material slots: {}".format(exc))
    return slots


def donor_mesh_of(donor_bp):
    """The SkeletalMesh the donor blueprint uses, or None."""
    name = donor_bp.rsplit("/", 1)[-1]
    generated = unreal.load_object(None, "{}.{}_C".format(donor_bp, name))
    if generated is None:
        return None
    cdo = unreal.get_default_object(generated)
    return cdo.get_editor_property("mesh").get_editor_property("skeletal_mesh")


def ensure_physics_asset(spec, mesh, donor_mesh, result):
    """Make sure the mesh has a physics asset; share the donor's if it has none.

    Without one a walker never ragdolls on death (AWalkerBase::bAlive ->
    AfterLifeSpan). The import normally creates `<Name>_PhysicsAsset`; when it does not,
    the donor's is reused, which is sound only because both meshes share the same
    skeleton and a physics asset addresses bodies by bone name. It is SHARED, not
    copied — editing it affects the donor too, which is why this is reported.
    """
    if mesh.get_editor_property("physics_asset") is not None:
        result["physics_asset_source"] = "created by the import"
        return
    if not spec.get("share_physics", True):
        result["physics_asset_source"] = "none (--no-share-physics)"
        result["physics_asset_warning"] = (
            "the mesh has NO physics asset: the walker will not ragdoll on death. "
            "Create one in the Skeletal Mesh Editor, or drop --no-share-physics.")
        return
    donor_physics = donor_mesh.get_editor_property("physics_asset") if donor_mesh else None
    if donor_physics is None:
        result["physics_asset_source"] = "none available"
        result["physics_asset_warning"] = (
            "neither the import nor the donor provided a physics asset — no ragdoll.")
        return
    mesh.set_editor_property("physics_asset", donor_physics)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    result["physics_asset_source"] = "SHARED with the donor"
    result["physics_asset_warning"] = (
        "the physics asset %s is shared with the donor walker, not a copy — it fits "
        "because both meshes use the same skeleton, but editing it affects both."
        % donor_physics.get_name())


def is_raw_import(material_path, destination):
    """True when this material is one the FBX import just created next to the mesh.

    An FBX that ships no embedded textures still declares material NAMES, so the
    import writes a blank white Material per slot into the destination folder. Those
    are bound, not unassigned — which is why "fill the empty slots" is not enough to
    make a walker look right (C4). Anything living outside the destination is a real
    material somebody authored, and is left alone.
    """
    return bool(material_path) and material_path.startswith(destination.rstrip("/") + "/")


def material_named(name, exclude_under):
    """Find an existing MaterialInterface asset called exactly `name`.

    An FBX exported OUT of Unreal names its material slots after the materials that
    were assigned — so a slot called "MI_AfroKid01_G3" is a direct reference to the
    asset of that name. Resolving it beats binding a donor's material by slot
    position, and it is what makes a round-tripped CARLA mesh come back shaded.

    `exclude_under` MUST be the import destination. The import writes a blank
    material per slot named after that slot, so a search that includes the
    destination would resolve every slot to its own blank material and report a fill
    that changed nothing.

    Searched under the pedestrian content root only: a bare name like "T_KidPants"
    is too generic to match project-wide.
    """
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    excluded = exclude_under.rstrip("/") + "/"
    for asset in registry.get_assets_by_path("/Game/Carla/Static/Pedestrian",
                                             recursive=True) or []:
        if str(asset.asset_name) != name:
            continue
        if str(asset.object_path).startswith(excluded):
            continue
        loaded = unreal.load_asset(asset.object_path)
        if isinstance(loaded, unreal.MaterialInterface):
            return loaded
    return None


def fill_materials(spec, mesh, donor_mesh, result):
    """Report every slot, and in donor mode replace blank imports with the donor's.

    Two different problems wear the same face here:

      * slot bound to None            -> nothing was imported for it
      * slot bound to a BLANK material -> the FBX named a material but carried no
        textures, so the import made an empty white one

    Both render untextured, so donor mode targets both. What it never overwrites is a
    material from outside the import destination: that one was authored deliberately.
    """
    destination = spec["mesh_destination"]
    slots = material_slots(mesh)
    for slot in slots:
        slot["raw_import"] = is_raw_import(slot["material"], destination)
    result["material_slots"] = slots
    result["material_slots_unassigned"] = [s["slot"] for s in slots if not s["material"]]
    result["material_slots_raw"] = [s["slot"] for s in slots if s["raw_import"]]

    mode = spec.get("materials", "none")
    replaceable = [s for s in slots if not s["material"] or s["raw_import"]]
    if mode != "donor" or not replaceable or donor_mesh is None:
        return

    donor_by_slot = {}
    for entry in donor_mesh.get_editor_property("materials"):
        material = entry.get_editor_property("material_interface")
        if material is not None:
            donor_by_slot[str(entry.get_editor_property("material_slot_name"))] = material

    materials = mesh.get_editor_property("materials")
    filled, missing = [], []
    for index, entry in enumerate(materials):
        slot = str(entry.get_editor_property("material_slot_name"))
        current = entry.get_editor_property("material_interface")
        current_path = current.get_path_name() if current else None
        if current is not None and not is_raw_import(current_path, destination):
            continue
        # Slot name -> the donor's material for that slot, else an existing asset of
        # exactly that name (the UE-export case).
        donor_material = donor_by_slot.get(slot)
        source = "donor slot"
        if donor_material is None:
            donor_material = material_named(slot, destination)
            source = "asset of the same name"
        if donor_material is None:
            missing.append(slot)
            continue
        entry.set_editor_property("material_interface", donor_material)
        materials[index] = entry
        filled.append({"slot": slot, "material": donor_material.get_path_name(),
                       "via": source, "replaced": current_path})
    if filled:
        mesh.set_editor_property("materials", materials)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        # Re-read: the slot list above was captured BEFORE the replacement, so
        # reporting it now would describe a state that no longer exists.
        refreshed = material_slots(mesh)
        for slot in refreshed:
            slot["raw_import"] = is_raw_import(slot["material"], destination)
        result["material_slots"] = refreshed
        result["material_slots_unassigned"] = [s["slot"] for s in refreshed
                                               if not s["material"]]
        result["material_slots_raw"] = [s["slot"] for s in refreshed if s["raw_import"]]
    result["materials_filled_from_donor"] = filled
    result["materials_still_unassigned"] = missing


def duplicate_donor(spec, name):
    """Duplicate the donor GEN3 blueprint into the walker blueprint folder."""
    donor = spec["donor_bp"]
    dest_dir = spec["bp_destination"].rstrip("/")
    dest = "{}/{}".format(dest_dir, name)

    if unreal.EditorAssetLibrary.does_asset_exist(dest):
        # Re-import of the same walker: replace it, so the run is idempotent
        # rather than accumulating BP_Walker_X1, X2, ...
        log("{} exists; deleting before duplicating".format(dest))
        unreal.EditorAssetLibrary.delete_asset(dest)

    if not unreal.EditorAssetLibrary.does_asset_exist(donor):
        raise RuntimeError("donor blueprint %s does not exist" % donor)

    duplicated = unreal.EditorAssetLibrary.duplicate_asset(donor, dest)
    if duplicated is None:
        raise RuntimeError("could not duplicate %s into %s" % (donor, dest))
    return dest, duplicated


def configure_blueprint(spec, bp_path, mesh, measurement, donor_mesh):
    """Repoint the duplicate's CDO at the new mesh.

    Everything here is set on the ALREADY-COMPILED duplicate's CDO and saved
    without a recompile, for the native-component reason in the module docstring.

    Collision geometry is INHERITED from the donor by default (references, C3). Every
    shipped GEN3 walker carries the same numbers — mesh scale 0.65, capsule half-height
    93.0, radius 18.77, mesh z -94.70 — for a 1.2 m visible character, whatever the
    individual mesh measures. They are a generation-wide convention, not a measurement,
    so deriving them from bounds invents values no stock walker uses. What IS checked is
    that the new mesh is close enough to the donor's for those numbers to still fit.
    """
    generated = unreal.load_object(None, "{}.{}_C".format(bp_path, bp_path.rsplit("/", 1)[-1]))
    if generated is None:
        raise RuntimeError("cannot load the generated class of %s" % bp_path)
    cdo = unreal.get_default_object(generated)

    applied = {}

    mesh_component = cdo.get_editor_property("mesh")
    before = mesh_component.get_editor_property("skeletal_mesh")
    mesh_component.set_editor_property("skeletal_mesh", mesh)
    applied["skeletal_mesh"] = mesh.get_path_name()
    applied["skeletal_mesh_was"] = before.get_path_name() if before else None
    applied["anim_class"] = str(mesh_component.get_editor_property("anim_class"))

    # The donor's SingleAnimationPlayData points at AS_walkingG3 under a folder
    # that is missing from the shipped content ("Nos_"), so every load of a GEN3
    # walker logs a linker warning. ABP_GEN3 drives the walker, so this slot is
    # dead weight: clear it in the duplicate rather than inherit the warning.
    if spec.get("clear_single_anim", True):
        try:
            play_data = mesh_component.get_editor_property("animation_data")
            play_data.set_editor_property("anim_to_play", None)
            mesh_component.set_editor_property("animation_data", play_data)
            mesh_component.set_editor_property("animation_mode",
                                               unreal.AnimationMode.ANIMATION_BLUEPRINT)
            applied["single_anim_cleared"] = True
        except Exception as exc:  # noqa: BLE001 - cosmetic, never fatal
            log("could not clear SingleAnimationPlayData: {}".format(exc))
            applied["single_anim_cleared"] = "failed: %s" % exc

    capsule = cdo.get_editor_property("capsule_component")
    scale = mesh_component.get_editor_property("relative_scale3d")
    location = mesh_component.get_editor_property("relative_location")

    applied["inherited"] = {
        "mesh_scale": [round(scale.x, 4), round(scale.y, 4), round(scale.z, 4)],
        "capsule_half_height": capsule.get_editor_property("capsule_half_height"),
        "capsule_radius": round(capsule.get_editor_property("capsule_radius"), 2),
        "mesh_relative_z": round(location.z, 2),
    }

    # Does the donor's collision still fit this mesh? Compare UNSCALED bounds,
    # which is what the shared scale is applied to.
    if donor_mesh is not None:
        donor_half_z = abs(donor_mesh.get_bounds().box_extent.z)
        new_half_z = measurement["half_extent_cm"][2]
        drift = abs(new_half_z - donor_half_z) / donor_half_z if donor_half_z else 0.0
        applied["donor_mesh_half_extent_z"] = round(donor_half_z, 2)
        applied["bounds_drift"] = round(drift, 4)
        if drift > BOUNDS_TOLERANCE:
            applied["bounds_warning"] = (
                "this mesh is %.0f%% %s than the donor's (%.1f vs %.1f cm half-height). "
                "The inherited capsule and mesh offset were sized for the donor and "
                "will leave this walker floating, sunk or mis-collided — pass "
                "--capsule-half-height / --mesh-z / --mesh-scale, and check "
                "verify_walker.py's ground result." % (
                    drift * 100.0,
                    "taller" if new_half_z > donor_half_z else "shorter",
                    new_half_z, donor_half_z)
            )

    # Explicit overrides win over the inherited convention.
    if spec.get("mesh_scale"):
        factor = float(spec["mesh_scale"])
        mesh_component.set_editor_property("relative_scale3d",
                                           unreal.Vector(factor, factor, factor))
        applied["mesh_scale"] = factor
    if spec.get("capsule_half_height"):
        capsule.set_editor_property("capsule_half_height", float(spec["capsule_half_height"]))
        applied["capsule_half_height"] = float(spec["capsule_half_height"])
    if spec.get("capsule_radius"):
        capsule.set_editor_property("capsule_radius", float(spec["capsule_radius"]))
        applied["capsule_radius"] = float(spec["capsule_radius"])
    if spec.get("mesh_z") is not None:
        location.z = float(spec["mesh_z"])
        mesh_component.set_editor_property("relative_location", location)
        applied["mesh_relative_z"] = float(spec["mesh_z"])

    final_scale = mesh_component.get_editor_property("relative_scale3d")
    applied["visible_height_m"] = round(
        measurement["unscaled_height_m"] * abs(final_scale.z), 3)

    # Hair is per-character art: the donor's groom fits the donor's head. Swapped
    # only when asked, and the inherited value is reported either way so a wrong
    # hairstyle is visible in the log rather than only in a screenshot.
    groom_component = None
    try:
        groom_component = unreal.load_object(generated, "Groom_GEN_VARIABLE")
    except Exception as exc:  # noqa: BLE001
        log("no groom component on the donor: {}".format(exc))
    if groom_component is not None:
        current = groom_component.get_editor_property("groom_asset")
        applied["groom_inherited"] = current.get_path_name() if current else None
        if spec.get("groom"):
            groom_asset = unreal.load_asset(spec["groom"])
            if groom_asset is None:
                raise RuntimeError("cannot load groom asset %s" % spec["groom"])
            groom_component.set_editor_property("groom_asset", groom_asset)
            applied["groom"] = spec["groom"]

    if spec.get("wheelchair") is not None:
        cdo.set_editor_property("bUsesWheelChair", bool(spec["wheelchair"]))
        applied["uses_wheelchair"] = bool(spec["wheelchair"])

    # Save without recompiling: see the module docstring.
    applied["saved"] = bool(unreal.EditorAssetLibrary.save_asset(bp_path, False))
    return applied


def build(spec):
    result = {
        "name": spec["name"],
        "source": spec["fbx"],
        "ok": False,
    }
    try:
        removed = clean_destination(spec)
        if removed:
            result["replaced_previous_import"] = sorted(removed)
        log("importing {} -> {}".format(spec["fbx"], spec["mesh_destination"]))
        imported = import_fbx(spec)
        log("import produced {} object(s)".format(len(imported)))
        result["imported_objects"] = sorted(imported)

        meshes = skeletal_meshes_among(imported)
        result["skeletal_meshes"] = sorted(meshes)
        if not meshes:
            result["error"] = (
                "the import produced no SkeletalMesh. Read the FIRST FBX error in "
                "the log: the file may hold only static geometry, or an FBX version "
                "this engine cannot read."
            )
            return result

        hint = spec.get("mesh_hint") or ""
        if len(meshes) == 1:
            mesh_path = list(meshes)[0]
        elif hint and hint in meshes:
            mesh_path = hint
        else:
            result["error"] = (
                "the import produced %d skeletal meshes and none was nominated:\n  %s\n"
                "Re-run with --mesh <object path>, or export one character per FBX."
                % (len(meshes), "\n  ".join(sorted(meshes)))
            )
            return result
        mesh = meshes[mesh_path]
        result["mesh_path"] = mesh_path
        # UE names the object after an FBX mesh node, which need not match the
        # package: report both so the path in the blueprint is traceable.
        result["mesh_object_name"] = mesh.get_name()

        # A mesh bound to the wrong skeleton drives no GEN3 animation. check_input.py
        # compares bone NAMES on the host; this confirms what UE actually bound.
        bound = mesh.get_editor_property("skeleton")
        result["skeleton"] = bound.get_path_name() if bound else None
        expected = spec["skeleton"].rsplit("/", 1)[-1]
        if bound is None or expected not in bound.get_path_name():
            result["error"] = (
                "the mesh was bound to %s, not %s — every GEN3 animation would be "
                "inapplicable. This usually means the FBX bone names do not match "
                "the skeleton (run check_input.py)." % (result["skeleton"], spec["skeleton"])
            )
            return result

        measurement = measure(mesh)
        result.update(measurement)
        try:
            result["verts_lod0"] = unreal.EditorSkeletalMeshLibrary.get_num_verts(mesh, 0)
        except Exception:  # noqa: BLE001 - diagnostic only
            pass
        log("{} half-extents {} cm -> unscaled height {} m".format(
            spec["name"], measurement["half_extent_cm"], measurement["unscaled_height_m"]))

        donor_mesh = donor_mesh_of(spec["donor_bp"])
        result["donor_mesh"] = donor_mesh.get_path_name() if donor_mesh else None

        ensure_physics_asset(spec, mesh, donor_mesh, result)
        fill_materials(spec, mesh, donor_mesh, result)

        # Save again: ensure_physics_asset and fill_materials both mutate the mesh,
        # and the physics asset may itself be a freshly created package.
        unreal.EditorAssetLibrary.save_directory(
            spec["mesh_destination"], only_if_is_dirty=False, recursive=True)
        result["persisted_assets"] = sorted(unreal.EditorAssetLibrary.list_assets(
            spec["mesh_destination"], recursive=True, include_folder=False) or [])
        physics = mesh.get_editor_property("physics_asset")
        result["physics_asset"] = physics.get_path_name() if physics else None

        bp_path, _ = duplicate_donor(spec, spec["bp_name"])
        result["bp_path"] = bp_path
        result["bp_class"] = "{}.{}_C".format(bp_path, spec["bp_name"])
        result["blueprint"] = configure_blueprint(spec, bp_path, mesh, measurement,
                                                  donor_mesh)

        result["ok"] = True
    except Exception as exc:  # noqa: BLE001 - the traceback is the deliverable
        result["error"] = "{}: {}".format(type(exc).__name__, exc)
        result["traceback"] = traceback.format_exc()
        unreal.log_error("[build_walker] " + result["traceback"])
    return result


def main():
    spec_path = os.environ.get("CARLA_WALKER_SPEC", "")
    result_path = ""
    try:
        if not spec_path or not os.path.isfile(spec_path):
            raise RuntimeError("CARLA_WALKER_SPEC does not name a spec file: %r" % spec_path)
        with open(spec_path, "r") as handle:
            spec = json.load(handle)
        result_path = spec.get("result", "")
        result = build(spec)
    except Exception as exc:  # noqa: BLE001
        result = {"ok": False,
                  "error": "{}: {}".format(type(exc).__name__, exc),
                  "traceback": traceback.format_exc()}
        unreal.log_error("[build_walker] " + result["traceback"])

    if result_path:
        with open(result_path, "w") as handle:
            json.dump(result, handle, indent=2)
    log("done: ok={}".format(result.get("ok")))


main()
