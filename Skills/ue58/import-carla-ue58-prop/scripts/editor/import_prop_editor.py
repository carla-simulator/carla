"""In-editor half of import-carla-ue58-prop. Runs inside UnrealEditor via
`-run=pythonscript`; never invoke directly.

Reads a job JSON, imports each FBX as a StaticMesh, measures it, and writes a
result JSON. Three UE 5.8 facts shape this file:

  * print() and unreal.log() from the pythonscript commandlet do NOT reach the
    editor log, so every result goes through the result file. Errors DO appear as
    `LogPython: Error`, and the commandlet exits 255, so failures are still
    visible to the caller.
  * FBX import goes through the Interchange framework in 5.8. AssetImportTask +
    FbxImportUI still drives it (the log shows LogInterchangeEngine), so the
    legacy option object is the right one to build.
  * EditorStaticMeshLibrary.get_number_triangles no longer exists; bounds come
    from StaticMesh.get_bounds().
"""
import json
import os
import traceback

import unreal

JOB = os.environ.get("CARLA_PROP_JOB", "")

# EPropSize spellings CARLA accepts, and the longest-dimension cut-offs used to
# pick one. Matches the distribution of the 82 built-in props.
SIZE_THRESHOLDS = (
    (0.5, "Tiny"),
    (2.0, "Small"),
    (8.0, "Medium"),
    (float("inf"), "Big"),
)


def size_for(longest_m):
    for limit, name in SIZE_THRESHOLDS:
        if longest_m <= limit:
            return name
    return "Big"


def build_options(scale, combine, collision):
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    data = options.static_mesh_import_data
    data.set_editor_property("combine_meshes", combine)
    data.set_editor_property("generate_lightmap_u_vs", True)
    data.set_editor_property("auto_generate_collision", collision)
    # The FBX's own unit metadata is often absent or wrong on assets authored for
    # other pipelines, which is how a door arrives 400 m tall. An explicit uniform
    # scale is the only reliable correction.
    data.set_editor_property("import_uniform_scale", float(scale))
    return options


def import_one(entry, defaults):
    fbx = entry["fbx"]
    dest = entry["dest"]
    scale = entry.get("scale", defaults["scale"])
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", fbx)
    task.set_editor_property("destination_path", dest)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("options", build_options(
        scale, defaults["combine"], defaults["collision"]))
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    out = {"fbx": fbx, "dest": dest, "scale": scale, "assets": [], "mesh": None}
    for path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True, include_folder=False):
        asset = unreal.load_asset(path)
        cls = type(asset).__name__
        out["assets"].append({"path": str(path), "class": cls})
        if isinstance(asset, unreal.StaticMesh):
            extent = asset.get_bounds().box_extent
            longest = 2.0 * max(extent.x, extent.y, extent.z) / 100.0
            out["mesh"] = {
                "path": str(path),
                "extent_cm": [round(extent.x, 1), round(extent.y, 1), round(extent.z, 1)],
                "dimensions_m": [round(2 * extent.x / 100.0, 3),
                                 round(2 * extent.y / 100.0, 3),
                                 round(2 * extent.z / 100.0, 3)],
                "longest_m": round(longest, 3),
                "size": entry.get("size") or size_for(longest),
                "num_lods": asset.get_num_lods(),
                "materials": len(asset.static_materials),
            }
    return out


def main():
    result = {"engine": str(unreal.SystemLibrary.get_engine_version()),
              "imported": [], "errors": []}
    job = json.load(open(JOB))
    defaults = {
        "scale": job.get("scale", 1.0),
        "combine": job.get("combine", True),
        "collision": job.get("collision", True),
    }
    for entry in job["props"]:
        try:
            result["imported"].append(import_one(entry, defaults))
        except Exception:
            result["errors"].append({"fbx": entry.get("fbx"), "traceback": traceback.format_exc()})
    with open(job["result"], "w") as handle:
        json.dump(result, handle, indent=2)
    if result["errors"]:
        # Make the commandlet exit non-zero: the caller cannot see stdout.
        raise RuntimeError("{} import(s) failed; see {}".format(
            len(result["errors"]), job["result"]))


main()
