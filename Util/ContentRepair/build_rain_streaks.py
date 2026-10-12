"""Soften the legacy falling-rain sprites for the cinematic rain preset.

Run with UnrealEditor-Cmd <project> -run=pythonscript -script=<this file>
-nullrhi -unattended -nosound. Rewires M_Rain without deleting rooted nodes.
The original particle system, world positions, and depth testing are retained.
"""
import unreal


def main():
    mel = unreal.MaterialEditingLibrary
    material = unreal.load_asset("/Game/Carla/Static/FX/Particles/Rain/M_Rain")
    if material is None:
        raise RuntimeError("Missing M_Rain")
    expressions = mel.get_material_expressions(material)

    def node(cls, name, x, y):
        tag = "Carla cinematic rain: " + name
        matches = [n for n in expressions if n.get_editor_property("desc") == tag]
        if matches:
            return matches[0]
        result = mel.create_material_expression(material, cls, x, y)
        result.set_editor_property("desc", tag)
        return result

    def connect(source, output, target, pin):
        if not mel.connect_material_expressions(source, output, target, pin):
            raise RuntimeError(f"Could not connect {pin}")

    # Additive compositing cannot subtract scene radiance into dark lines.
    # This is a cinematic highlight model, not a drop-light transport solver.
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    uv = node(unreal.MaterialExpressionTextureCoordinate, "UV", -1000, 700)
    particle = node(unreal.MaterialExpressionParticleColor, "Particle", -1000, 900)
    position = node(unreal.MaterialExpressionParticlePositionWS, "Position", -1000, 1100)
    camera = node(unreal.MaterialExpressionCameraPositionWS, "Camera", -1000, 1300)
    shape = node(unreal.MaterialExpressionCustom, "Soft streak", -600, 700)
    shape.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    shape.set_editor_property("code", """
float2 p = UV - 0.5;
float core = exp(-2.0 * pow(p.x / 0.10, 2.0));
float ends = exp(-pow(abs(p.y) / 0.28, 4.0));
float distanceFade = saturate((distance(Position, Camera) - 50.0) / 200.0);
return core * ends * saturate(Alpha) * saturate(Opacity) * distanceFade;
""")
    inputs = []
    for name in ("UV", "Alpha", "Position", "Camera", "Opacity"):
        entry = unreal.CustomInput()
        entry.set_editor_property("input_name", name)
        inputs.append(entry)
    shape.set_editor_property("inputs", inputs)
    connect(uv, "", shape, "UV")
    connect(particle, "A", shape, "Alpha")
    connect(position, "", shape, "Position")
    connect(camera, "", shape, "Camera")
    opacity = node(unreal.MaterialExpressionScalarParameter, "Opacity", -1000, 1500)
    opacity.set_editor_property("parameter_name", "StreakOpacity")
    opacity.set_editor_property("default_value", 0.65)
    opacity.set_editor_property("group", "Cinematic Rain")
    connect(opacity, "", shape, "Opacity")
    fade = node(unreal.MaterialExpressionDepthFade, "Surface fade", -200, 700)
    fade.set_editor_property("fade_distance_default", 35.0)
    connect(shape, "", fade, "Opacity")

    brightness = node(unreal.MaterialExpressionScalarParameter, "Brightness", -600, 1000)
    brightness.set_editor_property("parameter_name", "StreakBrightness")
    brightness.set_editor_property("default_value", 0.18)
    brightness.set_editor_property("group", "Cinematic Rain")
    exposure = node(unreal.MaterialExpressionEyeAdaptationInverse, "Exposure", -200, 1000)
    connect(brightness, "", exposure, mel.get_material_expression_input_names(exposure)[0])
    # Cancel sprite surface refraction: rapidly falling rain should not warp
    # entire building edges into ribbons. Lens beads handle local refraction.
    no_refraction = node(unreal.MaterialExpressionConstant, "No surface refraction", -200, 1300)
    no_refraction.set_editor_property("r", 1.0)
    for expression, prop in [(fade, unreal.MaterialProperty.MP_OPACITY),
                             (exposure, unreal.MaterialProperty.MP_EMISSIVE_COLOR),
                             (no_refraction, unreal.MaterialProperty.MP_REFRACTION)]:
        if not mel.connect_material_property(expression, "", prop):
            raise RuntimeError(f"Could not connect {prop}")
    mel.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError("Could not save M_Rain")
    unreal.log("RAIN_STREAK_BUILD: saved soft additive rain")


if __name__ == "__main__":
    main()
