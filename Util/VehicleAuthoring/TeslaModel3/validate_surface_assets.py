"""Fresh-process material integration checks; run as an Unreal Python commandlet."""
import json,os,unreal
from pathlib import Path
root=Path(__file__).resolve().parents[4]
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',root/'artifacts/tesla-model3'))
base='/Game/Carla/Static/Car/4Wheeled/TeslaModel3UE58';r={};mel=unreal.MaterialEditingLibrary
for name in ['M_wheels_6','M_Model3Paint']:
 m=unreal.load_asset(base+'/Materials/'+name);assert m
 assert m.get_editor_property('use_material_attributes'),name
 node=mel.get_material_property_input_node(m,unreal.MaterialProperty.MP_MATERIAL_ATTRIBUTES);assert node
 r[name]={'attributes':node.get_class().get_name(),'coat':mel.get_material_default_scalar_parameter_value(m,'coat'),'coat_roughness':mel.get_material_default_scalar_parameter_value(m,'coat_roughness')}
 assert r[name]['coat']>0
for name in ['SurfaceDetail_Seat_Leather_white_0_Normal','SurfaceDetail_movsteer_1_0_0_Normal']:
 tex=unreal.load_asset(base+'/Textures/T_'+name);assert tex,name
 assert not tex.get_editor_property('srgb');assert tex.get_editor_property('flip_green_channel')
 r[name]={'normal_compression':str(tex.get_editor_property('compression_settings'))}
lamp=unreal.load_asset(base+'/Materials/M_Model3VehicleLightsMaster_left_front_light')
r['low_beam_intensity']=mel.get_material_default_scalar_parameter_value(lamp,'LightIntensity');assert r['low_beam_intensity']>=30
rear=unreal.load_asset(base+'/Materials/M_Model3VehicleLightsMaster_right_rear_light')
r['rear_position_intensity']=mel.get_material_default_scalar_parameter_value(rear,'LightIntensity')
r['rear_diffuse_node']=mel.get_material_property_input_node(rear,unreal.MaterialProperty.MP_BASE_COLOR).get_class().get_name()
assert r['rear_diffuse_node']=='MaterialExpressionConstant3Vector'
reflector=unreal.load_asset(base+'/Materials/M_Model3VehicleLightsMaster_indicator_rf')
assert mel.get_material_property_input_node(reflector,unreal.MaterialProperty.MP_BASE_COLOR).get_class().get_name()=='MaterialExpressionVectorParameter'
flutes=unreal.load_asset(base+'/Textures/T_SurfaceDetail_ReflectorFlutes_Normal');assert flutes
assert not flutes.get_editor_property('srgb') and flutes.get_editor_property('flip_green_channel')
r['reflector_flutes_normal']=flutes.get_path_name()
light_parameters={}
for name,required,forbidden in [('light_night',{'Interior'},{'High Beam'}),('front_position_l',{'Position'},{'Fog'}),('front_position_r',{'Position'},{'Fog'}),('left_front_light',{'Low Beam','High Beam'},{'Interior'}),('right_front_light',{'Low Beam','High Beam'},{'Interior'}),('reverse_indicator_l',{'Reverse','Left Blinker'},{'Right Blinker'}),('reverse_indicator_r',{'Reverse','Right Blinker'},{'Left Blinker'})]:
 material=unreal.load_asset(base+'/Materials/M_Model3VehicleLightsMaster_'+name);assert material,name
 parameters={str(n) for n in mel.get_scalar_parameter_names(material)}
 assert required<=parameters and not forbidden&parameters,(name,parameters)
 if name.startswith('reverse_indicator_'):
  # Parameter names alone do not prove a usable shader: a disconnected unary
  # input can invalidate the entire compiled emissive branch.
  nodes=[n for n in unreal.ObjectIterator(unreal.MaterialExpression) if n.get_outer()==material]
  for kind in (unreal.MaterialExpressionSaturate,unreal.MaterialExpressionOneMinus):
   node=next(n for n in nodes if isinstance(n,kind))
   inputs=mel.get_inputs_for_material_expression(material,node)
   assert len(inputs)==1 and inputs[0] is not None,(name,node.get_name(),'disconnected input')
 light_parameters[name]=sorted(parameters)
r['light_parameters']=light_parameters
bp=unreal.load_asset('/Game/Carla/Blueprints/Vehicles/TeslaModel3UE58/BP_TeslaModel3')
subsystem=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem);lib=unreal.SubobjectDataBlueprintFunctionLibrary;r['authored_lights']=[]
for handle in subsystem.k2_gather_subobject_data_for_blueprint(bp):
 component=lib.get_object_for_blueprint(lib.get_data(handle),bp)
 if isinstance(component,unreal.LightComponent) and component.get_name().startswith('front-low_beam-'):
  assert component.get_editor_property('hidden_in_game'),'Superseded template low beam is still active'
 if isinstance(component,unreal.LightComponent) and component.get_name().startswith('Model3_'):
  tags=[str(t) for t in component.get_editor_property('component_tags')]
  assert any(t.startswith('Carla.Light.') for t in tags)
  assert component.get_editor_property('intensity')>0
  rotation=component.get_editor_property('relative_rotation')
  if 'Carla.Light.Low Beam' in tags:assert abs(rotation.pitch+3)<.01 and abs(rotation.roll)<.01
  if 'Carla.Light.Fog' in tags:assert abs(rotation.pitch+6)<.01 and abs(rotation.roll)<.01
  r['authored_lights'].append({'name':component.get_name(),'tags':tags,'pitch':rotation.pitch,'roll':rotation.roll,'intensity':component.get_editor_property('intensity')})
assert len(r['authored_lights'])==7,r['authored_lights']
lcd=unreal.load_asset(base+'/Materials/M_LCDs_0');node=mel.get_material_property_input_node(lcd,unreal.MaterialProperty.MP_BASE_COLOR)
r['lcd_diffuse_node']=node.get_class().get_name();assert r['lcd_diffuse_node']=='MaterialExpressionConstant3Vector'
r['display_luminance']=mel.get_material_default_scalar_parameter_value(lcd,'DisplayLuminance');assert r['display_luminance']>=500
r['status']='passed';(P/'audit/surface-asset-validation.json').write_text(json.dumps(r,indent=2))
