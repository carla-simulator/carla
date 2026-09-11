"""Match Town10's saved sky sharpening to the default RGB camera profile.

Run with UnrealEditor-Cmd <project> -run=pythonscript -script=<this file>.
Only changes the placed sky's sharpening; preserves other authored settings.
"""
import json
from pathlib import Path
import unreal

profile = Path(unreal.Paths.project_content_dir()) / 'Carla/Config/PostProcess/Default.json'
data = json.loads(profile.read_text())
# The profile stores the serialized FPostProcessSettings as a nested object.
def find_sharpen(value):
    if isinstance(value, dict):
        if 'sharpen' in value:
            return float(value['sharpen'])
        for child in value.values():
            result = find_sharpen(child)
            if result is not None:
                return result
    return None

sharpen = find_sharpen(data)
assert sharpen is not None, 'Default profile has no sharpening value'
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert level.load_level('/Game/Carla/Maps/Town10HD_Opt')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
skies = [actor for actor in actors if isinstance(actor, unreal.SkyBase)]
assert len(skies) == 1, f'Expected one sky, found {len(skies)}'
component = skies[0].get_component_by_class(unreal.PostProcessComponent)
settings = component.get_editor_property('settings')
unreal.log(f'Town10 sky sharpen: {settings.sharpen} -> {sharpen}')
settings.set_editor_property('sharpen', sharpen)
settings.set_editor_property('override_sharpen', True)
component.set_editor_property('settings', settings)
assert level.save_current_level(), 'Failed to save Town10'
