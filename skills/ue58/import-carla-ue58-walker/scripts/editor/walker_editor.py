"""In-editor half of import-carla-ue58-walker. Runs inside UnrealEditor via
`-ExecutePythonScript`; never invoke directly.

Reads a job JSON (`CARLA_WALKER_JOB`), performs one of two actions, and writes a
result JSON. Progress is flushed after every step, because two of the operations
here can hard-crash the process and the partial trace is what identifies where.

Why a full editor and not a commandlet
--------------------------------------
`Exporter.run_asset_export_task` on a SkeletalMesh asserts inside the engine:

    Assertion failed: MeshObject
      [Runtime/Engine/Private/Components/SkinnedMeshComponent.cpp:4987]

and takes the process down with SIGSEGV. It does this under BOTH `-nullrhi` and
`-RenderOffScreen`, so it is not an RHI-selection problem: the FBX exporter needs
a real render context, which `-run=pythonscript` does not provide. The import path
plus the blueprint duplicate were also validated in a full editor, so both actions
use one.
"""
import json
import os
import traceback

import unreal

JOB = os.environ["CARLA_WALKER_JOB"]
job = json.load(open(JOB))
RESULT = job["result"]

state = {"engine": str(unreal.SystemLibrary.get_engine_version()),
         "action": job["action"], "steps": []}


def mark(step, **extra):
    entry = {"step": step}
    entry.update(extra)
    state["steps"].append(entry)
    with open(RESULT, "w") as handle:
        json.dump(state, handle, indent=2)


def do_export():
    """Export an existing walker's SkeletalMesh to FBX.

    This is how you obtain a rig-conforming FBX without an external asset: export
    a shipped walker, edit it, re-import it. It is also the only way to validate
    the pipeline end to end on a machine with no rigged source art.
    """
    mesh = unreal.load_asset(job["mesh"])
    if mesh is None:
        raise RuntimeError("cannot load " + job["mesh"])
    mark("loaded", asset=job["mesh"], cls=type(mesh).__name__,
         materials=len(mesh.materials))
    skeleton = mesh.get_editor_property("skeleton")
    state["skeleton"] = str(skeleton.get_path_name()) if skeleton else None
    extent = mesh.get_bounds().box_extent
    state["dimensions_m"] = [round(2 * extent.x / 100, 3),
                             round(2 * extent.y / 100, 3),
                             round(2 * extent.z / 100, 3)]
    mark("measured", skeleton=state["skeleton"], dimensions_m=state["dimensions_m"])

    task = unreal.AssetExportTask()
    task.set_editor_property("object", mesh)
    task.set_editor_property("filename", job["fbx"])
    task.set_editor_property("automated", True)
    task.set_editor_property("prompt", False)
    task.set_editor_property("replace_identical", True)
    options = unreal.FbxExportOption()
    options.set_editor_property("collision", False)
    options.set_editor_property("level_of_detail", False)
    task.set_editor_property("options", options)
    mark("about_to_export")          # the step that crashes in a commandlet
    ok = bool(unreal.Exporter.run_asset_export_task(task))
    exists = os.path.isfile(job["fbx"])
    state["export"] = {"ok": ok, "path": job["fbx"], "exists": exists,
                       "bytes": os.path.getsize(job["fbx"]) if exists else 0}
    mark("exported", **state["export"])
    if not (ok and exists):
        raise RuntimeError("export produced no file")


def do_import():
    name = job["name"]
    dest = job["dest"]

    # Binding to the SHARED skeleton is the whole game. Let the importer create
    # its own and you get a structurally identical private skeleton that drives
    # none of CARLA's pedestrian animations -- silently, with no warning.
    skeleton = unreal.load_asset(job["skeleton"])
    if skeleton is None:
        raise RuntimeError(
            "skeleton not found: {} -- without it the import creates a private "
            "skeleton and the walker will not animate".format(job["skeleton"]))
    mark("skeleton_loaded", path=job["skeleton"], cls=type(skeleton).__name__)

    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_materials", job.get("materials", True))
    options.set_editor_property("import_textures", False)
    # The pedestrian animation set already exists on the shared skeleton; taking
    # animations from the FBX would duplicate or conflict with it.
    options.set_editor_property("import_animations", False)
    options.set_editor_property("mesh_type_to_import",
                                unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    options.set_editor_property("skeleton", skeleton)
    data = options.skeletal_mesh_import_data
    data.set_editor_property("import_morph_targets", True)
    data.set_editor_property("update_skeleton_reference_pose", False)
    data.set_editor_property("preserve_smoothing_groups", True)
    data.set_editor_property("import_uniform_scale", float(job.get("scale", 1.0)))

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", job["fbx"])
    task.set_editor_property("destination_path", dest)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("options", options)
    mark("about_to_import", fbx=job["fbx"], dest=dest)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mark("import_returned")

    mesh_path, bound_skeleton, created = None, None, []
    for path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True,
                                                      include_folder=False):
        asset = unreal.load_asset(path)
        created.append({"path": str(path), "class": type(asset).__name__})
        if isinstance(asset, unreal.SkeletalMesh) and mesh_path is None:
            mesh_path = str(path)
            bound = asset.get_editor_property("skeleton")
            bound_skeleton = str(bound.get_path_name()) if bound else None
            extent = asset.get_bounds().box_extent
            state["dimensions_m"] = [round(2 * extent.x / 100, 3),
                                     round(2 * extent.y / 100, 3),
                                     round(2 * extent.z / 100, 3)]
    state["created"] = created
    state["mesh_path"] = mesh_path
    state["bound_skeleton"] = bound_skeleton
    # A private skeleton means the mesh will not animate; report it as a failure
    # rather than letting a silently-broken walker reach the registry.
    state["skeleton_shared"] = (bound_skeleton == job["skeleton"])
    mark("imported", mesh_path=mesh_path, bound_skeleton=bound_skeleton,
         skeleton_shared=state["skeleton_shared"],
         dimensions_m=state.get("dimensions_m"))
    if mesh_path is None:
        raise RuntimeError("no SkeletalMesh produced -- is the FBX skinned?")
    if not state["skeleton_shared"]:
        raise RuntimeError(
            "mesh bound to {} instead of the shared {} -- it would not animate"
            .format(bound_skeleton, job["skeleton"]))

    # Duplicate the donor rather than subclassing: the skeletal mesh and the anim
    # class live on the generated class's CDO, and a fresh subclass would need a
    # recompile to have one -- which resets both to the parent's defaults.
    new_bp = job["blueprint"]
    if unreal.EditorAssetLibrary.does_asset_exist(new_bp):
        unreal.EditorAssetLibrary.delete_asset(new_bp)
        mark("deleted_stale_blueprint", path=new_bp)
    if not unreal.EditorAssetLibrary.duplicate_asset(job["donor_blueprint"], new_bp):
        raise RuntimeError("duplicate_asset failed: " + job["donor_blueprint"])
    mark("duplicated_blueprint", donor=job["donor_blueprint"], new=new_bp)

    blueprint = unreal.load_asset(new_bp)
    cdo = unreal.get_default_object(blueprint.generated_class())
    state["cdo_class"] = type(cdo).__name__
    new_mesh = unreal.load_asset(mesh_path)
    applied = None
    for prop in ("mesh", "Mesh", "skeletal_mesh_component"):
        try:
            component = cdo.get_editor_property(prop)
        except Exception:
            continue
        if component:
            component.set_editor_property("skeletal_mesh", new_mesh)
            applied = prop
            break
    if applied is None:
        raise RuntimeError("no skeletal mesh component on the CDO ({})".format(
            state["cdo_class"]))
    mark("mesh_applied", via=applied, cdo=state["cdo_class"])

    # Save without recompiling, for the reason above.
    unreal.EditorAssetLibrary.save_asset(new_bp, only_if_is_dirty=False)
    state["blueprint_class"] = "{}.{}_C".format(new_bp, new_bp.rsplit("/", 1)[1])
    mark("saved", blueprint_class=state["blueprint_class"])


try:
    mark("start")
    {"export": do_export, "import": do_import}[job["action"]]()
    mark("done")
except Exception:
    state["error"] = traceback.format_exc()
    mark("ERROR")
    with open(RESULT, "w") as handle:
        json.dump(state, handle, indent=2)
finally:
    # A commandlet (-run=pythonscript) exits when its script ends; a full editor
    # driven with -ExecutePythonScript does NOT -- it opens the GUI and stays,
    # so the caller's subprocess.run() would block forever. Ask it to quit.
    if job.get("quit", True):
        try:
            unreal.SystemLibrary.quit_editor()
        except Exception:
            # Fall back to the console command if the library call is missing.
            try:
                unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
            except Exception:
                pass

if state.get("error"):
    raise RuntimeError(state["error"])
