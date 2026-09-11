"""Run in a fresh Unreal Python commandlet to check saved assembly references."""
import json,os
from pathlib import Path
import unreal
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'))
root='/Game/Carla/Static/Car/4Wheeled/TeslaModel3UE58';path='/Game/Carla/Blueprints/Vehicles/TeslaModel3UE58/BP_TeslaModel3'
bp=unreal.load_asset(path);cdo=unreal.get_default_object(unreal.load_class(None,path+'.BP_TeslaModel3_C'));mesh=cdo.get_component_by_class(unreal.SkeletalMeshComponent);sk=mesh.get_editor_property('skeletal_mesh_asset');ph=unreal.load_asset(root+'/PH_Model3')
assert sk.get_editor_property('physics_asset')==ph,'Saved rig still references template collision'
assert mesh.get_editor_property('physics_asset_override') in (None,ph),'Unexpected component physics override'
assert not mesh.get_editor_property('render_in_main_pass'),'Template rig must not render as vehicle geometry'
assert mesh.get_editor_property('visibility_based_anim_tick_option')==unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES
subsystem=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem);lib=unreal.SubobjectDataBlueprintFunctionLibrary;wheels=[]
for handle in subsystem.k2_gather_subobject_data_for_blueprint(bp):
 comp=lib.get_object_for_blueprint(lib.get_data(handle),bp)
 if isinstance(comp,unreal.StaticMeshComponent) and comp.get_name().startswith('Wheel_'):
  static_mesh=comp.get_editor_property('static_mesh')
  assert str(comp.get_collision_profile_name())=='NoCollision','Visual wheel has blocking collision'
  assert not comp.get_editor_property('body_instance').get_editor_property('auto_weld')
  assert static_mesh.get_num_lods()==1,'Unreviewed wheel decimation reintroduced'
  wheels.append(static_mesh.get_path_name())
assert len(wheels)==4
base=next(body for body in unreal.ObjectIterator(unreal.BodySetup) if body.get_outer()==ph and str(body.get_editor_property('bone_name'))=='Vehicle_Base')
assert base.get_editor_property('default_instance').get_editor_property('notify_rigid_body_collision'),'Chassis collision notifications are disabled'
report={'status':'passed','saved_physics_asset':ph.get_path_name(),'wheel_meshes':wheels,'wheel_lods':1,'rig_rendered':False,'rig_bones_always_updated':True}
(P/'audit/asset-regression.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))
