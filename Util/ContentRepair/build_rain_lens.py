"""Build M_LensRain from the versioned RainLens.ush shader.

Run with UnrealEditor-Cmd <project> -run=pythonscript -script=<this file>
-nullrhi -unattended -nosound. Keeps the old M_screenDrops for comparison.
"""
import unreal

DEFAULTS = [("Coverage", 1.0), ("DropScale", 1.2), ("Refraction", 0.75),
            ("Softness", 1.75), ("Seed", 0.0)]

def main():
    mel = unreal.MaterialEditingLibrary
    folder = "/Game/Carla/Static/GenericMaterials/FX/ScreenDust"
    path = folder + "/M_LensRain"
    material = (unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path)
                else unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                    "M_LensRain", folder, unreal.Material, unreal.MaterialFactoryNew()))
    if not isinstance(material, unreal.Material):
        raise RuntimeError(f"Could not create material: {path}")
    if mel.get_num_material_expressions(material):
        # Constructor-loaded material expressions can be rooted in commandlets.
        # Recompile the versioned include without deleting a live graph.
        if not any(isinstance(n, unreal.MaterialExpressionCustom) and
                   "/Plugin/Carla/RainLens.ush" in n.get_editor_property("include_file_paths")
                   for n in mel.get_material_expressions(material)):
            raise RuntimeError("Existing M_LensRain is not the generated rain graph")
        defaults = {"Lens" + name: value for name, value in DEFAULTS}
        for expression in mel.get_material_expressions(material):
            if isinstance(expression, unreal.MaterialExpressionScalarParameter):
                name = str(expression.get_editor_property("parameter_name"))
                if name in defaults:
                    expression.set_editor_property("default_value", defaults[name])
        mel.recompile_material(material)
        if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
            raise RuntimeError("Could not save rain material")
        unreal.log("RAIN_BUILD: recompiled " + path)
        return
    collection = unreal.load_asset(
        "/Game/Carla/Blueprints/Weather/Materials/WeatherMaterialParameters")
    if collection is None:
        raise RuntimeError("Missing weather parameter collection")
    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    material.set_editor_property("blendable_location",
                                unreal.BlendableLocation.BL_SCENE_COLOR_AFTER_TONEMAPPING)

    def node(cls, x, y):
        return mel.create_material_expression(material, cls, x, y)

    scene = node(unreal.MaterialExpressionSceneTexture, -700, -400)
    scene.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
    uv = node(unreal.MaterialExpressionScreenPosition, -700, -200)
    clock = node(unreal.MaterialExpressionTime, -700, 0)
    rain = node(unreal.MaterialExpressionCollectionParameter, -700, 200)
    rain.set_editor_property("collection", collection)
    rain.set_editor_property("parameter_name", "Precipitation")
    custom = node(unreal.MaterialExpressionCustom, 0, 0)
    custom.set_editor_property("description", "Procedural lens water: beads, runs, local refraction")
    custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    custom.set_editor_property("include_file_paths", ["/Plugin/Carla/RainLens.ush"])
    custom.set_editor_property("code", "return CarlaRainLens(UV, SceneColor.rgb, Time, RainAmount, "
                               "Coverage, DropScale, Refraction, Softness, Seed);")
    connections = [("UV", uv, "ViewportUV"), ("SceneColor", scene, "Color"),
                   ("Time", clock, ""), ("RainAmount", rain, "")]
    for i, (name, value) in enumerate(DEFAULTS):
        param = node(unreal.MaterialExpressionScalarParameter, -350, 200 + i * 160)
        param.set_editor_property("parameter_name", "Lens" + name)
        param.set_editor_property("default_value", value)
        param.set_editor_property("group", "Lens Rain")
        connections.append((name, param, ""))
    inputs = []
    for name, _, _ in connections:
        entry = unreal.CustomInput()
        entry.set_editor_property("input_name", name)
        inputs.append(entry)
    custom.set_editor_property("inputs", inputs)
    for name, source, output in connections:
        if not mel.connect_material_expressions(source, output, custom, name):
            raise RuntimeError(f"Could not connect {name}")
    if not mel.connect_material_property(custom, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR):
        raise RuntimeError("Could not connect output")
    mel.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material):
        raise RuntimeError("Could not save rain material")
    unreal.log("RAIN_BUILD: saved " + path)


if __name__ == "__main__":
    main()
