import unreal,json
from pathlib import Path
import os
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'));D='/Game/Carla/Static/Car/4Wheeled/TeslaModel3UE58/Meshes'
smes=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem) or unreal.new_object(unreal.StaticMeshEditorSubsystem)
r=[]
for path in unreal.EditorAssetLibrary.list_assets(D,recursive=False,include_folder=False):
 mesh=unreal.load_asset(path)
 if not isinstance(mesh,unreal.StaticMesh):continue
 options=unreal.StaticMeshReductionOptions();options.auto_compute_lod_screen_size=False
 settings=[]
 # The tire has far fewer triangles than the rim. Whole-wheel reduction
 # collapses its sidewall/tread into spikes while spending the budget on spokes.
 # Keep its authored silhouette until purpose-built wheel LODs are available.
 levels=[(1.0,1.0)] if '_Wheel' in mesh.get_name() else [(1.0,1.0),(.45,.35),(.12,.12),(.03,.035)]
 for fraction,screen in levels:
  s=unreal.StaticMeshReductionSettings();s.percent_triangles=fraction;s.screen_size=screen;settings.append(s)
 options.reduction_settings=settings
 result=smes.set_lods(mesh,options);unreal.EditorAssetLibrary.save_loaded_asset(mesh);r.append(dict(mesh=path,lods=mesh.get_num_lods(),result=result))
(P/'audit/lods.json').write_text(json.dumps(r,indent=2))
