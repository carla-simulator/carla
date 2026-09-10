"""Make M_CarPaintMaster render sanely under path tracing (editor Python commandlet).

The car-paint master is a clear-coat material whose instances carry Roughness values up to 1.6 (invalid; the raster
path clamps them and fakes rough-metal reflections from reflection captures, so the cars look glossy anyway). The path
tracer integrates the material physically: with such a roughness the base metal reflects almost nothing, and its
clear-coat layer loses most of what is left, so every car came out near black (a silver Lincoln MKZ on its shadow side
measured at 6 % of the road luminance against ~100 % in raster; clamping the roughness alone gave 9 %, default-lit 22 %).

Two PathTracingQualitySwitch nodes keep the raster look untouched and give the path tracer a plain glossy metal:
  Roughness      = switch(Normal: instance value, PathTraced: clamp(value, 0, `PT Roughness Max` = 0.2))
  Shading model  = switch(Normal: ClearCoat, PathTraced: DefaultLit)   (shading model from material expression)
Idempotent: each part is skipped when already present. The shading-model part needs `MaterialProperty.MP_SHADING_MODEL`,
which the UE 5.8 Python API does not expose (the enum stops at the visible material outputs), so it is skipped with a
warning until an editor-side C++ helper wires it; the roughness clamp alone took a silver car's side from 0.05 to 0.21 of
the road luminance in the NuRec hybrid composite (with the sky light at skymap.DEFAULT_INTENSITY = 160000), and the
`PT Roughness Max` parameter can be overridden per instance.

  UnrealEditor-Cmd CarlaUnreal.uproject -run=pythonscript -script="Util/ContentRepair/fix_carpaint_pathtracing.py" -unattended -nosplash -nosound -NullRHI
"""
import unreal

MEL = unreal.MaterialEditingLibrary
PATH = "/Game/Carla/Static/GenericMaterials/000_Masters/Vehicles/M_CarPaintMaster"
PT_ROUGHNESS_MAX = 0.2
mat = unreal.load_asset(PATH); changed = []

# 1. roughness
node = MEL.get_material_property_input_node(mat, unreal.MaterialProperty.MP_ROUGHNESS)
if node is None: raise SystemExit("M_CarPaintMaster: nothing connected to Roughness")
if isinstance(node, unreal.MaterialExpressionPathTracingQualitySwitch):
    unreal.log("fix_carpaint_pathtracing: roughness switch already present")
else:
    x, y = node.get_editor_property("material_expression_editor_x"), node.get_editor_property("material_expression_editor_y")
    pt_max = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x + 150, y + 160)
    pt_max.set_editor_property("parameter_name", "PT Roughness Max"); pt_max.set_editor_property("default_value", PT_ROUGHNESS_MAX)
    clamp = MEL.create_material_expression(mat, unreal.MaterialExpressionClamp, x + 300, y + 80)
    clamp.set_editor_property("min_default", 0.0); clamp.set_editor_property("max_default", PT_ROUGHNESS_MAX)
    switch = MEL.create_material_expression(mat, unreal.MaterialExpressionPathTracingQualitySwitch, x + 480, y)
    ok = [MEL.connect_material_expressions(node, "", clamp, "") or MEL.connect_material_expressions(node, "", clamp, "Input"), MEL.connect_material_expressions(pt_max, "", clamp, "Max"),
          MEL.connect_material_expressions(node, "", switch, "Normal"), MEL.connect_material_expressions(clamp, "", switch, "PathTraced"),
          MEL.connect_material_property(switch, "", unreal.MaterialProperty.MP_ROUGHNESS)]
    if not all(ok): raise SystemExit("fix_carpaint_pathtracing: roughness wiring failed %s" % ok)
    changed.append("roughness switch")

# 2. shading model (the Python MaterialProperty enum of UE 5.8 does not expose MP_SHADING_MODEL; without it the switch
#    cannot be wired from a script, so this part is skipped with a warning and the roughness clamp alone is saved)
if mat.get_editor_property("shading_model") == unreal.MaterialShadingModel.MSM_FROM_MATERIAL_EXPRESSION:
    unreal.log("fix_carpaint_pathtracing: shading-model switch already present")
elif not hasattr(unreal.MaterialProperty, "MP_SHADING_MODEL"):
    unreal.log_warning("fix_carpaint_pathtracing: MaterialProperty.MP_SHADING_MODEL not exposed to Python, shading-model switch skipped")
else:
    cc = MEL.create_material_expression(mat, unreal.MaterialExpressionShadingModel, -600, 600); cc.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_CLEAR_COAT)
    dl = MEL.create_material_expression(mat, unreal.MaterialExpressionShadingModel, -600, 700); dl.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    sw = MEL.create_material_expression(mat, unreal.MaterialExpressionPathTracingQualitySwitch, -400, 640)
    ok = [MEL.connect_material_expressions(cc, "", sw, "Normal"), MEL.connect_material_expressions(dl, "", sw, "PathTraced"),
          MEL.connect_material_property(sw, "", unreal.MaterialProperty.MP_SHADING_MODEL)]
    if not all(ok): raise SystemExit("fix_carpaint_pathtracing: shading-model wiring failed %s" % ok)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_FROM_MATERIAL_EXPRESSION)
    changed.append("shading-model switch")

if changed:
    MEL.recompile_material(mat)
    if not unreal.EditorAssetLibrary.save_loaded_asset(mat): raise SystemExit("save failed")
print("fix_carpaint_pathtracing: " + (", ".join(changed) + " saved" if changed else "nothing to do"))
