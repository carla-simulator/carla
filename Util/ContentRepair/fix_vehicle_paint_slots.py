"""Rename the car-paint material slot of vehicle meshes to `Bodywork_Mat`.

BaseVehiclePawn.ApplyColor paints the material slot named `Bodywork_Mat` (hard-coded) with a dynamic instance of the
M_CarPaintMaster child it finds there. Several UE5 vehicle meshes (Lincoln MKZ, Mini Cooper, Dodge Charger) import
their paint slot under another name (`M_BodyWork_Lincoln`, `M_Bodywork`, ...), so the `color` attribute is silently
ignored and the cars spawn in their default paint (black). This renames, on every skeletal and static mesh under the
given content folders, each slot whose material derives from M_CarPaintMaster to `Bodywork_Mat` (the first one; further
paint slots of the same mesh get `Bodywork_Mat2`, ... since the blueprint paints one index).

Run headless from the project root:
  UnrealEditor CarlaUnreal.uproject -run=pythonscript -script="Util/ContentRepair/fix_vehicle_paint_slots.py" -unattended -nosplash -nosound
Set FIX_PAINT_DRY_RUN=1 to only report.
"""
import os
import unreal

FOLDERS = ["/Game/Carla/Static/Car/4Wheeled/LincolnMKZ", "/Game/Carla/Static/Car/4Wheeled/MiniCooper", "/Game/Carla/Static/Car/4Wheeled/DodgeCharger",
           "/Game/Carla/Static/Car/4Wheeled/BMWIsetta", "/Game/Carla/Static/Car/4Wheeled/FordCrown01", "/Game/Carla/Static/Car/4Wheeled/FordCrown02",
           "/Game/Carla/Static/Car/4Wheeled/NissanPatrol", "/Game/Carla/Static/Car/4Wheeled/MercedesSprinter", "/Game/Carla/Static/Car/4Wheeled/Sprinter"]
MASTER = "M_CarPaintMaster"
DRY = os.environ.get("FIX_PAINT_DRY_RUN", "0") == "1"
eal = unreal.EditorAssetLibrary
ar = unreal.AssetRegistryHelpers.get_asset_registry()


def is_carpaint(mat):
    seen = 0
    while mat is not None and seen < 12:
        if mat.get_name() == MASTER: return True
        if isinstance(mat, unreal.MaterialInstance): mat = mat.get_editor_property("parent")
        else: return False
        seen += 1
    return False


changed = []; report = []
for folder in FOLDERS:
    if not eal.does_directory_exist(folder): continue
    for path in eal.list_assets(folder, recursive=True, include_folder=False):
        data = eal.find_asset_data(path); cls = data.asset_class_path.asset_name if hasattr(data, "asset_class_path") else data.asset_class
        if str(cls) not in ("SkeletalMesh", "StaticMesh"): continue
        asset = eal.load_asset(path)
        prop = "materials" if isinstance(asset, unreal.SkeletalMesh) else "static_materials"
        mats = list(asset.get_editor_property(prop)); n_paint = 0; dirty = False
        for m in mats:
            name = str(m.get_editor_property("material_slot_name")); mi = m.get_editor_property("material_interface")
            if not is_carpaint(mi): continue
            n_paint += 1; want = "Bodywork_Mat" if n_paint == 1 else "Bodywork_Mat%d" % n_paint
            report.append("%s: slot '%s' -> %s (%s)" % (path.split("/")[-1], name, mi.get_name(), "ok" if name == want else "rename to " + want))
            if name != want:
                m.set_editor_property("material_slot_name", want); dirty = True
        if dirty and not DRY:
            asset.set_editor_property(prop, mats)
            if eal.save_loaded_asset(asset): changed.append(path)
            else: report.append("SAVE FAILED " + path)
for line in report: unreal.log("fix_vehicle_paint_slots: " + line)
unreal.log("fix_vehicle_paint_slots: %s %d mesh(es)" % ("would change" if DRY else "changed", len(changed)))
for p in changed: unreal.log("  " + p)
