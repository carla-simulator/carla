#!/usr/bin/env python
"""Imports and registers props from inside UE4Editor. Not an entry point.

Run by ../import_prop.py, never by hand:

    UE4Editor CarlaUE4.uproject -run=pythonscript -Script="<this file>"

The job description is read from the file named by $CARLA_PROP_SPEC. Arguments
go through the environment rather than the command line because the pythonscript
commandlet hands `-Script=` to ExecPythonCommandEx as one command string
(PythonScriptCommandlet.cpp:12-33); quoting an argv through that is fragile, an
env var is not.

The spec holds a LIST of props, so a whole directory is one editor boot instead
of one boot per mesh. For each prop, four steps that are only correct together:

  1. import the FBX to /Game/<Root>/Static/<Tag>/<Name>
  2. read back the StaticMesh the import ACTUALLY produced
  3. measure it, deriving `size` from the bounds unless one was given
  4. register it — in the .Package.json the blueprint registry reads, and in
     PropFactory's DefinitionsMap

Step 2 is the point. The old `make import` pipeline derived the asset path from
the FBX filename (`<stem>.<stem>`), which is wrong for any FBX holding more than
one mesh node — see references/props.md, and the dead `SM_Door_Front_L` entry
that mistake left behind. `AssetImportTask.imported_object_paths` is UE telling
us what it created; we never guess.

Results are written as JSON to spec["result"]: the host reports from that, not
from the editor log, and confirms the artifacts on disk itself.
"""
import json
import os
import traceback

import unreal

# EPropSize spellings as they appear in Default.Package.json. StringToPropSizeType
# resolves them through FName, which is case-insensitive, so the capitalisation
# only matches the entries already in the file.
SIZE_NAMES = {
    "tiny": "Tiny",
    "small": "Small",
    "medium": "Medium",
    "big": "Big",
    "huge": "Huge",
}

# Largest-dimension thresholds in metres for deriving `size`. EPropSize is
# defined purely by physical scale — PropParameters.h describes the values as
# "smaller than a mailbox / size of a mailbox / size of a human / size of a bus
# stop / size of a house or bigger" — so this is a measurement, not a judgment
# call. Ordered smallest first; the first threshold the mesh fits under wins.
SIZE_THRESHOLDS_M = (
    (0.5, "tiny"),
    (1.2, "small"),
    (2.5, "medium"),
    (6.0, "big"),
)
SIZE_ABOVE_ALL = "huge"

# Unreal works in centimetres.
CM_PER_M = 100.0

PROP_FACTORY_PATH = "/Game/Carla/Blueprints/Props/PropFactory.PropFactory_C"


def log(msg):
    unreal.log("[import_and_register] {}".format(msg))


def build_import_options(prop):
    """FbxImportUI for a prop: one static mesh, with collision, no skeletal data."""
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", prop.get("import_materials", True))
    options.set_editor_property("import_textures", prop.get("import_textures", True))
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)

    mesh_data = options.static_mesh_import_data
    # combine_meshes is what the old pipeline got wrong: it left it off, so a
    # multi-node FBX became several assets named "<stem>_<node>" while the single
    # path written into the registry resolved to nothing. A prop is one spawnable
    # object, so the nodes are combined.
    mesh_data.set_editor_property("combine_meshes", prop.get("combine_meshes", True))
    mesh_data.set_editor_property("auto_generate_collision", prop.get("auto_collision", True))
    mesh_data.set_editor_property("remove_degenerates", True)
    mesh_data.set_editor_property("convert_scene", True)
    # FBX is authored in metres, Unreal works in centimetres.
    mesh_data.set_editor_property("convert_scene_unit", True)
    return options


def import_fbx(prop):
    """Run the import; return the object paths UE reports having created."""
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", prop["fbx"])
    task.set_editor_property("destination_path", prop["destination"])
    task.set_editor_property("options", build_import_options(prop))
    # Unattended: no dialogs, overwrite a previous import of the same prop, and
    # flush to disk so the .uasset exists before the host stats it.
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    paths = list(task.get_editor_property("imported_object_paths") or [])
    if not paths:
        # Fallback for import paths that populate the asset registry but leave
        # imported_object_paths empty. Listing the destination is second best —
        # it also sees assets from an earlier run — but beats guessing a name.
        log("imported_object_paths empty; listing {}".format(prop["destination"]))
        paths = list(unreal.EditorAssetLibrary.list_assets(
            prop["destination"], recursive=True, include_folder=False) or [])
    return paths


def static_meshes_among(object_paths):
    """Map object path -> asset, for the paths that really load as a UStaticMesh.

    An FBX import also yields materials and textures; asking each object what it
    is beats pattern-matching asset names.
    """
    meshes = {}
    for path in object_paths:
        try:
            asset = unreal.load_asset(path)
        except Exception:
            continue
        if isinstance(asset, unreal.StaticMesh):
            # Normalise to Package.Object — the form the registry's LoadObject
            # call and every existing Default.Package.json entry use.
            meshes[asset.get_path_name()] = asset
    return meshes


def measure(mesh):
    """Return the mesh's local bounding box as (x, y, z) in metres.

    UStaticMesh::GetBoundingBox is BlueprintPure (StaticMesh.h:1116), so it is
    reachable here. Reporting this is worth as much as the derived size: a mesh
    exported at the wrong scale shows up immediately rather than after it is
    spawned. The CARLA docs hit exactly that with their police barrier, which
    came out of Sketchfab 5 m tall.
    """
    box = mesh.get_bounding_box()
    extent = box.max - box.min
    return (
        round(abs(extent.x) / CM_PER_M, 2),
        round(abs(extent.y) / CM_PER_M, 2),
        round(abs(extent.z) / CM_PER_M, 2),
    )


def size_from_dimensions(dimensions_m):
    """Bucket the largest dimension into an EPropSize value."""
    largest = max(dimensions_m)
    for limit, name in SIZE_THRESHOLDS_M:
        if largest < limit:
            return name
    return SIZE_ABOVE_ALL


def register_in_registry(prop, mesh_path, size):
    """Add or update the prop entry in the .Package.json the registry reads.

    UCarlaBlueprintRegistry::LoadPropDefinitions scans Content/ for *.Package.json
    at map load and turns each entry into `static.prop.<name lowercased>`. This
    file is the registration that makes the prop spawnable.
    """
    config_path = prop["registry_json"]
    directory = os.path.dirname(config_path)
    if not os.path.isdir(directory):
        os.makedirs(directory)

    data = {"props": [], "maps": []}
    if os.path.isfile(config_path):
        with open(config_path, "r") as handle:
            data = json.load(handle)
    data.setdefault("props", [])
    data.setdefault("maps", [])

    entry = {"name": prop["name"], "path": mesh_path, "size": SIZE_NAMES[size]}

    # Replace by name so re-importing updates rather than duplicates. Same-named
    # entries otherwise shadow each other silently (references/props.md P5).
    for index, existing in enumerate(data["props"]):
        if existing.get("name") == prop["name"]:
            data["props"][index] = entry
            break
    else:
        data["props"].append(entry)

    with open(config_path, "w") as handle:
        json.dump(data, handle, indent=4)
    log("registered {} in {}".format(prop["name"], config_path))
    return config_path


def register_in_factory(prop, mesh_path, size):
    """Add the prop to PropFactory's DefinitionsMap.

    Mirrors Plugins/CarlaTools/Content/Python/add_prop_to_prop_factory.py, which
    is the shipped way to do this from the editor. The key format
    ('static.prop.<Name>') matches that script so entries stay consistent with
    any added by hand.
    """
    factory_class = unreal.load_object(None, PROP_FACTORY_PATH)
    defaults = unreal.get_default_object(factory_class)
    definitions = defaults.get_editor_property("DefinitionsMap")

    prop_id = "static.prop." + prop["name"]

    params = unreal.PropParameters()
    params.name = prop["name"]
    params.mesh = unreal.load_object(None, mesh_path)
    params.size = getattr(unreal.PropSize, size.upper())

    # Assigning over an existing key keeps a re-import idempotent; the shipped
    # script skips instead, which leaves a stale mesh pointer after a re-import.
    existed = prop_id in definitions
    definitions[prop_id] = params
    unreal.EditorAssetLibrary.save_asset(PROP_FACTORY_PATH, False)
    log("{} {} in PropFactory DefinitionsMap".format(
        "updated" if existed else "added", prop_id))
    return True


def import_one(prop):
    """Import, measure and register a single prop. Never raises."""
    result = {
        "name": prop["name"],
        "tag": prop["tag"],
        "blueprint_id": "static.prop." + prop["name"].lower(),
        "source": prop["fbx"],
        "ok": False,
    }
    try:
        log("importing {} -> {}".format(prop["fbx"], prop["destination"]))
        imported = import_fbx(prop)
        log("import produced {} object(s)".format(len(imported)))

        meshes = static_meshes_among(imported)
        result["static_meshes"] = sorted(meshes)

        if not meshes:
            result["error"] = (
                "the import produced no StaticMesh. Read the FIRST FBX error in the "
                "log; the file may be an unsupported FBX version, or hold only "
                "skeletal or curve data."
            )
            return result

        hint = prop.get("mesh_hint") or ""
        if len(meshes) == 1:
            mesh_path = list(meshes)[0]
        elif hint and hint in meshes:
            mesh_path = hint
        else:
            # With combine_meshes on this is unusual, so report rather than pick
            # one arbitrarily and register half a model.
            result["error"] = (
                "the import produced %d static meshes and none was nominated:\n  %s\n"
                "Re-run that file with --mesh <object path>, or re-export the FBX "
                "as a single mesh." % (len(meshes), "\n  ".join(sorted(meshes)))
            )
            return result

        result["mesh_path"] = mesh_path

        dimensions = measure(meshes[mesh_path])
        result["dimensions_m"] = list(dimensions)
        size = prop.get("size") or size_from_dimensions(dimensions)
        result["size"] = size
        result["size_source"] = "given" if prop.get("size") else "measured"
        log("{} measures {} m -> size={}".format(prop["name"], dimensions, size))

        result["registry_json"] = register_in_registry(prop, mesh_path, size)
        if prop.get("factory"):
            result["factory_updated"] = register_in_factory(prop, mesh_path, size)

        result["ok"] = True
    except Exception as exc:  # noqa: BLE001 - one bad prop must not sink the batch
        result["error"] = "{}: {}".format(type(exc).__name__, exc)
        result["traceback"] = traceback.format_exc()
        unreal.log_error("[import_and_register] " + result["traceback"])
    return result


def run(spec):
    props = [import_one(prop) for prop in spec["props"]]
    failed = [p for p in props if not p.get("ok")]
    return {"ok": not failed, "props": props, "failed": len(failed)}


def main():
    spec_path = os.environ.get("CARLA_PROP_SPEC", "")
    result_path = ""
    try:
        if not spec_path or not os.path.isfile(spec_path):
            raise RuntimeError("CARLA_PROP_SPEC does not name a spec file: %r" % spec_path)
        with open(spec_path, "r") as handle:
            spec = json.load(handle)
        result_path = spec.get("result", "")
        result = run(spec)
    except Exception as exc:  # noqa: BLE001 - the traceback is the deliverable
        result = {"ok": False, "props": [], "failed": 0,
                  "error": "{}: {}".format(type(exc).__name__, exc),
                  "traceback": traceback.format_exc()}
        unreal.log_error("[import_and_register] " + result["traceback"])

    if result_path:
        with open(result_path, "w") as handle:
            json.dump(result, handle, indent=2)
    log("done: {} prop(s), {} failed".format(
        len(result.get("props", [])), result.get("failed", 0)))


main()
