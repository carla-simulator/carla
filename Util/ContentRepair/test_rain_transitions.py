"""Unreal Python commandlet regression for rain blendable lifecycle.

Run: UnrealEditor-Cmd <project> -run=pythonscript -script=<this file>
     -nullrhi -unattended -nosound
"""
import unreal


def main():
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    weather_class = unreal.EditorAssetLibrary.load_blueprint_class(
        "/Game/Carla/Blueprints/Weather/BP_CarlaWeather")
    weather = actors.spawn_actor_from_class(weather_class, unreal.Vector())
    camera = None
    try:
        camera = actors.spawn_actor_from_class(unreal.SceneCaptureCamera, unreal.Vector())
        capture = camera.get_capture_component2d()
        # A user-owned post-process pass must survive every weather change.
        unrelated = unreal.load_asset(
            "/Game/Carla/Static/GenericMaterials/FX/ScreenDust/M_screenDrops")
        settings = capture.get_editor_property("post_process_settings")
        blendables = settings.get_editor_property("weighted_blendables")
        entry = unreal.WeightedBlendable()
        entry.set_editor_property("object", unrelated)
        entry.set_editor_property("weight", 0.125)
        blendables.set_editor_property("array", [entry])
        settings.set_editor_property("weighted_blendables", blendables)
        capture.set_editor_property("post_process_settings", settings)
        for rain in (0, 20, 85, 0, 100, 0):
            parameters = unreal.WeatherParameters()
            parameters.set_editor_property("precipitation", rain)
            weather.apply_weather(parameters)
            settings = capture.get_editor_property("post_process_settings")
            entries = settings.get_editor_property("weighted_blendables").get_editor_property("array")
            rows = [(entry.get_editor_property("object").get_path_name(),
                     entry.get_editor_property("weight")) for entry in entries]
            lens = [row for row in rows if row[0].endswith("M_LensRain.M_LensRain")]
            assert len(lens) == int(rain > 0), (rain, lens)
            if rain:
                assert abs(lens[0][1] - rain / 100) < 1e-5, lens
            assert (unrelated.get_path_name(), 0.125) in rows, rows
            unreal.log(f"RAIN_TRANSITION PASS: precipitation={rain}, lens={lens}")
    finally:
        if camera is not None:
            actors.destroy_actor(camera)
        actors.destroy_actor(weather)


if __name__ == "__main__":
    main()
