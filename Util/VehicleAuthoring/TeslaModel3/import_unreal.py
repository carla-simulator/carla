"""Import the prepared Model 3 into a separate UE5 content folder."""
import unreal,json,re,traceback
from pathlib import Path
import os
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'));D=P/'export';manifest=json.loads((D/'manifest.json').read_text())
DEST='/Game/Carla/Static/Car/4Wheeled/TeslaModel3UE58';BPDEST='/Game/Carla/Blueprints/Vehicles/TeslaModel3UE58'
at=unreal.AssetToolsHelpers.get_asset_tools();eal=unreal.EditorAssetLibrary;mel=unreal.MaterialEditingLibrary
report={'meshes':{},'materials':{},'errors':[]}
def save(): (P/'audit/unreal-import.json').write_text(json.dumps(report,indent=2))
def clean(n):return re.sub('[^a-zA-Z0-9_]','_',n)
def import_file(file,dest,name,options=None):
 t=unreal.AssetImportTask();t.filename=str(file);t.destination_path=dest;t.destination_name=name;t.automated=True;t.replace_existing=True;t.save=True
 if options:t.options=options
 at.import_asset_tasks([t]);return [unreal.load_asset(p) for p in t.imported_object_paths]
try:
 textures={}
 for name,file in manifest['images'].items():
  path=DEST+'/Textures/T_'+clean(name);tex=unreal.load_asset(path) if eal.does_asset_exist(path) and not (name.startswith(('WheelDetail_','SurfaceDetail_')) and os.environ.get('CARLA_MODEL3_REIMPORT_DETAIL_TEXTURES')=='1') else import_file(file,DEST+'/Textures','T_'+clean(name))[0];textures[name]=tex
 materials={}
 light_groups={'breaklight_l':'Brake','revlight_L':'Reverse','reverse_indicator_l':'Reverse','reverse_indicator_r':'Reverse','left_front_light':'Low Beam','right_front_light':'Low Beam','left_rear_light':'Position','right_rear_light':'Position','light_night':'Interior','front_position_l':'Position','front_position_r':'Position','foglight_l':'Fog','foglight_r':'Fog','indicator_lf':'Left Blinker','indicator_lr':'Left Blinker','indicator_rf':'Right Blinker','indicator_rr':'Right Blinker'}
 for spec in manifest['materials']:
  name=spec['name'];asset_name=('M_Model3VehicleLightsMaster_' if name in light_groups else 'M_')+clean(name);path=DEST+'/Materials/'+asset_name
  if name=='Bodywork_Mat':
   path=DEST+'/Materials/M_Model3Paint';mat=unreal.load_asset(path) if eal.does_asset_exist(path) else at.create_asset('M_Model3Paint',DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
   # Clear-coat input defaults are 1.0 weight and 0.1 roughness (UE MaterialAttributeDefinitionMap).
   mel.delete_all_material_expressions(mat);mat.set_editor_property('use_material_attributes',False);mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_CLEAR_COAT)
   node=mel.create_material_expression(mat,unreal.MaterialExpressionVectorParameter);node.set_editor_property('parameter_name','Base Color');node.set_editor_property('default_value',unreal.LinearColor(.8,.8,.8,1));mel.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
   for param,value,prop in [('Metallic',.15,unreal.MaterialProperty.MP_METALLIC),('Roughness',.23,unreal.MaterialProperty.MP_ROUGHNESS)]:
    node=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);node.set_editor_property('parameter_name',param);node.set_editor_property('default_value',value);mel.connect_material_property(node,'',prop)
   mel.recompile_material(mat)
  else:
   mat=unreal.load_asset(path) if eal.does_asset_exist(path) else at.create_asset(asset_name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
   mel.delete_all_material_expressions(mat);mat.set_editor_property('use_material_attributes',False)
   if spec['texture']:
    node=mel.create_material_expression(mat,unreal.MaterialExpressionTextureSample);node.set_editor_property('texture',textures[spec['texture']]);mel.connect_material_property(node,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
   else:
    node=mel.create_material_expression(mat,unreal.MaterialExpressionVectorParameter);node.set_editor_property('parameter_name','SurfaceTint');node.set_editor_property('default_value',unreal.LinearColor(*((.025,.033,.04,1) if spec['transmission']>.5 else spec['base'])));mel.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
   for field,prop in [('roughness',unreal.MaterialProperty.MP_ROUGHNESS),('metallic',unreal.MaterialProperty.MP_METALLIC),('specular',unreal.MaterialProperty.MP_SPECULAR)]:
    node=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);node.set_editor_property('parameter_name',field.title());node.set_editor_property('default_value',spec.get(field,.5));mel.connect_material_property(node,'',prop)
   for key,prop in [('normal_texture',unreal.MaterialProperty.MP_NORMAL),('roughness_texture',unreal.MaterialProperty.MP_ROUGHNESS)]:
    if spec.get(key):
     texture=textures[spec[key]];texture.set_editor_property('srgb',False)
     texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP if key=='normal_texture' else unreal.TextureCompressionSettings.TC_MASKS)
     if key=='normal_texture':texture.set_editor_property('flip_green_channel',True)
     eal.save_loaded_asset(texture)
     detail=mel.create_material_expression(mat,unreal.MaterialExpressionTextureSample);detail.set_editor_property('texture',texture)
     detail.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if key=='normal_texture' else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
     mel.connect_material_property(detail,'RGB' if key=='normal_texture' else 'R',prop)
   if spec['transmission']>.5:
    mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT);mat.set_editor_property('two_sided',True)
    mat.set_editor_property('translucency_lighting_mode',unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
    node=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);node.set_editor_property('parameter_name','Opacity');node.set_editor_property('default_value',.82 if name=='glass.1' else (.32 if name.startswith('glass') else .12));mel.connect_material_property(node,'',unreal.MaterialProperty.MP_OPACITY)
   if name=='LCDs.0':
    # An LCD emits its image; treating that image as diffuse white paint adds
    # sunlight a second time and washes out the UI. Keep only dark glass albedo.
    black=mel.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);black.set_editor_property('constant',unreal.LinearColor(.002,.002,.002,1));mel.connect_material_property(black,'',unreal.MaterialProperty.MP_BASE_COLOR)
    display_node=mel.create_material_expression(mat,unreal.MaterialExpressionTextureSample);display_node.set_editor_property('texture',textures[spec['texture']]);display_gain=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);display_gain.set_editor_property('parameter_name','DisplayLuminance');display_gain.set_editor_property('default_value',1200.0)
    display_emission=mel.create_material_expression(mat,unreal.MaterialExpressionMultiply);mel.connect_material_expressions(display_node,'RGB',display_emission,'A');mel.connect_material_expressions(display_gain,'',display_emission,'B');mel.connect_material_property(display_emission,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
   if name in light_groups:
    red_lens=light_groups[name] in ('Brake','Position') and not name.startswith('front_position')
    color=(1,.003,.001,1) if red_lens else ((1,.18,.003,1) if 'Blinker' in light_groups[name] else (1,.93,.85,1))
    if red_lens:
     # A dark red unlit lens remains visible without adding orange diffuse
     # energy to an already bright emitter under physical camera exposure.
     tint=mel.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);tint.set_editor_property('constant',unreal.LinearColor(.12,.002,.001,1));mel.connect_material_property(tint,'',unreal.MaterialProperty.MP_BASE_COLOR)
    c=mel.create_material_expression(mat,unreal.MaterialExpressionVectorParameter);c.set_editor_property('parameter_name','LightColor');c.set_editor_property('default_value',unreal.LinearColor(*color))
    power=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);power.set_editor_property('parameter_name',light_groups[name]);power.set_editor_property('default_value',0.0)
    if light_groups[name]=='Low Beam':
     # Shared visible headlamp emitters respond to both beam modes; a high
     # beam uses twice the low-beam emission, without adding both modes.
     high=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);high.set_editor_property('parameter_name','High Beam');high.set_editor_property('default_value',0.0)
     scale=mel.create_material_expression(mat,unreal.MaterialExpressionMultiply);scale.set_editor_property('const_b',2.0);mel.connect_material_expressions(high,'',scale,'A')
     maximum=mel.create_material_expression(mat,unreal.MaterialExpressionMax);mel.connect_material_expressions(power,'',maximum,'A');mel.connect_material_expressions(scale,'',maximum,'B');power=maximum
    mul=mel.create_material_expression(mat,unreal.MaterialExpressionMultiply);mel.connect_material_expressions(c,'',mul,'A');mel.connect_material_expressions(power,'',mul,'B');gain=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);gain.set_editor_property('parameter_name','LightIntensity');gain.set_editor_property('default_value',{'Low Beam':50000.0,'High Beam':100000.0,'Position':250.0,'Brake':1500.0,'Fog':30000.0,'Interior':25.0}.get(light_groups[name],6000.0))
    emission=mel.create_material_expression(mat,unreal.MaterialExpressionMultiply);mel.connect_material_expressions(mul,'',emission,'A');mel.connect_material_expressions(gain,'',emission,'B');mel.connect_material_property(emission,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if name.startswith('reverse_indicator_'):
     # One optical face carries the two API channels. Amber has priority while
     # the indicator is active, avoiding coplanar opaque emitter surfaces.
     blink=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);blink.set_editor_property('parameter_name','Left Blinker' if name.endswith('_l') else 'Right Blinker');blink.set_editor_property('default_value',0.0)
     active=mel.create_material_expression(mat,unreal.MaterialExpressionSaturate);assert mel.connect_material_expressions(blink,'',active,'')
     inverse=mel.create_material_expression(mat,unreal.MaterialExpressionOneMinus);assert mel.connect_material_expressions(active,'',inverse,'')
     white=mel.create_material_expression(mat,unreal.MaterialExpressionMultiply);mel.connect_material_expressions(emission,'',white,'A');mel.connect_material_expressions(inverse,'',white,'B')
     amber=mel.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);amber.set_editor_property('constant',unreal.LinearColor(250,45,.75,1))
     amber_emission=mel.create_material_expression(mat,unreal.MaterialExpressionMultiply);mel.connect_material_expressions(amber,'',amber_emission,'A');mel.connect_material_expressions(blink,'',amber_emission,'B')
     combined=mel.create_material_expression(mat,unreal.MaterialExpressionAdd);mel.connect_material_expressions(white,'',combined,'A');mel.connect_material_expressions(amber_emission,'',combined,'B');mel.connect_material_property(combined,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)

   mel.recompile_material(mat)
  # Hidden MP_CustomData0/1 enum values are unavailable to Python in UE5.8.
  # MakeMaterialAttributes exposes the corresponding named clear-coat inputs.
  coated=spec.get('coat',0)>0 and spec['transmission']<=.5
  mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_CLEAR_COAT if coated else unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
  if coated:
   attrs=mel.create_material_expression(mat,unreal.MaterialExpressionMakeMaterialAttributes)
   names=mel.get_material_expression_input_names(attrs)
   pins={str(n).replace(' ','').replace('_','').lower():str(n) for n in names}
   def connect_attribute(node,output,pin):
    if not mel.connect_material_expressions(node,output,attrs,pins[pin.lower()]):raise RuntimeError('Cannot connect material attribute '+pin)
   for prop,pin in [(unreal.MaterialProperty.MP_BASE_COLOR,'BaseColor'),(unreal.MaterialProperty.MP_METALLIC,'Metallic'),(unreal.MaterialProperty.MP_SPECULAR,'Specular'),(unreal.MaterialProperty.MP_ROUGHNESS,'Roughness'),(unreal.MaterialProperty.MP_NORMAL,'Normal'),(unreal.MaterialProperty.MP_EMISSIVE_COLOR,'EmissiveColor')]:
    source=mel.get_material_property_input_node(mat,prop)
    if source:connect_attribute(source,mel.get_material_property_input_node_output_name(mat,prop),pin)
   for key,pin,default in [('coat','ClearCoat',0),('coat_roughness','ClearCoatRoughness',.1)]:
    node=mel.create_material_expression(mat,unreal.MaterialExpressionScalarParameter);node.set_editor_property('parameter_name',key);node.set_editor_property('default_value',spec.get(key,default));connect_attribute(node,'',pin)
   mat.set_editor_property('use_material_attributes',True)
   if not mel.connect_material_property(attrs,'',unreal.MaterialProperty.MP_MATERIAL_ATTRIBUTES):raise RuntimeError('Cannot connect material attributes')
  mel.recompile_material(mat)
  eal.save_loaded_asset(mat);materials[clean(name)]=mat;report['materials'][name]=mat.get_path_name()
 meshes={}
 for part in manifest['parts']:
  name=part['part'];path=DEST+'/Meshes/SM_Model3_'+name
  if eal.does_asset_exist(path) and name not in os.environ.get('CARLA_MODEL3_REIMPORT_PARTS','').split(','):mesh=unreal.load_asset(path)
  else:
   opt=unreal.FbxImportUI();opt.import_mesh=True;opt.import_as_skeletal=False;opt.import_materials=False;opt.import_textures=False;opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
   data=opt.static_mesh_import_data;data.combine_meshes=True;data.auto_generate_collision=False;data.generate_lightmap_u_vs=False
   mesh=next(a for a in import_file(part['file'],DEST+'/Meshes','SM_Model3_'+name,opt) if isinstance(a,unreal.StaticMesh))
  slots=list(mesh.get_editor_property('static_materials'));missing=[]
  for i,slot in enumerate(slots):
   key=clean(str(slot.material_slot_name));mat=materials.get(key)
   if not mat:missing.append(key)
   else:slot.material_interface=mat
  mesh.set_editor_property('static_materials',slots);eal.save_loaded_asset(mesh);meshes[name]=mesh
  b=mesh.get_bounds();report['meshes'][name]={'path':mesh.get_path_name(),'extent_cm':[b.box_extent.x,b.box_extent.y,b.box_extent.z],'origin_cm':[b.origin.x,b.origin.y,b.origin.z],'unmatched_materials':missing};save()
 # Refuse a wrongly oriented or scaled import before generating physics.
 b=meshes['Body'].get_bounds()
 if not 180<b.box_extent.x<270 or not 70<b.box_extent.y<130:raise RuntimeError('Import axis/scale mismatch: '+str(report['meshes']['Body']))
 # Simple convex collision of the exterior envelope, not per-triangle driving collision.
 smes=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem) or unreal.new_object(unreal.StaticMeshEditorSubsystem)
 for part in ['Body','DoorFL','DoorFR','DoorRL','DoorRR']:
  smes.remove_collisions(meshes[part]);smes.add_simple_collisions(meshes[part],unreal.ScriptingCollisionShapeType.NDOP26);eal.save_loaded_asset(meshes[part])
 skpath=DEST+'/SK_Model3Rig';phpath=DEST+'/PH_Model3'
 sk=unreal.load_asset(skpath) if eal.does_asset_exist(skpath) else eal.duplicate_asset('/Game/Carla/Blueprints/USDImportTemplates/SK_USDVehicleBase',skpath)
 ph=unreal.load_asset(phpath) if eal.does_asset_exist(phpath) else eal.duplicate_asset('/Game/Carla/Blueprints/USDImportTemplates/SK_USDVehicleBase_PhysicsAsset',phpath)
 # Refresh collision even when updating an existing Blueprint. The importer
 # invalidates cooked Chaos data; copying AggGeom alone retained template data.
 unreal.USDImporterWidget.copy_collision_to_physics_asset(ph,meshes['Body'])
 sk.set_editor_property('physics_asset',ph)
 for physics_body in unreal.ObjectIterator(unreal.BodySetup):
  if physics_body.get_outer()!=ph:continue
  if str(physics_body.get_editor_property('bone_name'))=='Vehicle_Base':
   instance=physics_body.get_editor_property('default_instance')
   instance.set_editor_property('notify_rigid_body_collision',True)
   physics_body.set_editor_property('default_instance',instance)
 # Native EditSkeletalMeshBones edits only the duplicated mesh reference pose.
 # The template skeleton is read for virtual-bone definitions; never saved here.
 wheels=unreal.WheelTemplates();wheelclasses={}
 for part in ['FL','FR','RL','RR']:
  name='BP_Model3_Wheel'+part;path=BPDEST+'/'+name
  if eal.does_asset_exist(path):bp=unreal.load_asset(path)
  else:
   factory=unreal.BlueprintFactory();factory.set_editor_property('parent_class',unreal.ChaosVehicleWheel);bp=at.create_asset(name,BPDEST,unreal.Blueprint,factory)
  cls=unreal.load_class(None,path+'.'+name+'_C');cdo=unreal.get_default_object(cls)
  for prop,value in {'wheel_radius':33.625,'wheel_width':23.5,'max_steer_angle':35.0 if part[0]=='F' else 0.0,'affected_by_engine':True,'affected_by_steering':part[0]=='F','affected_by_handbrake':part[0]=='R','max_brake_torque':1800.0,'suspension_max_raise':7.0,'suspension_max_drop':9.0,'suspension_damping_ratio':.5}.items():cdo.set_editor_property(prop,value)
  unreal.BlueprintEditorLibrary.compile_blueprint(bp);eal.save_loaded_asset(bp);wheelclasses[part]=cls;wheels.set_editor_property('wheel_'+part.lower(),cls)
 parts=unreal.MergedVehicleMeshParts();anchors=unreal.VehicleMeshAnchorPoints()
 for part in manifest['parts']:
  name=part['part']
  if name.startswith('Caliper'):continue
  prop=re.sub(r'([a-z])([A-Z])',r'\1_\2',name).lower();parts.set_editor_property(prop,meshes[name])
  if name!='Body':anchors.set_editor_property(prop,unreal.Vector(part['anchor_m'][0]*100,-part['anchor_m'][1]*100,part['anchor_m'][2]*100))
 parts.set_editor_property('anchors',anchors)
 lights=[]
 for typ,x,z,col in [('Headlight',2.08,.65,(1,.94,.86,1)),('Brakelight',-2.30,.80,(1,.015,.008,1)),('Reverse',-2.32,.77,(1,1,1,1)),('Blinker',2.06,.67,(1,.25,.005,1))]:
  for side,y in [('Left',.67),('Right',-.67)]:
   l=unreal.VehicleLight();l.name=typ+'_'+('r' if x<0 else 'f')+('l' if side=='Left' else 'r');l.location=unreal.Vector(x*100,-y*100,z*100);l.color=unreal.LinearColor(*col);lights.append(l)
 parts.set_editor_property('lights',lights)
 bppath=BPDEST+'/BP_TeslaModel3';world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();base=unreal.load_class(None,'/Game/Carla/Blueprints/USDImportTemplates/BaseUSDImportVehicle.BaseUSDImportVehicle_C')
 if not eal.does_asset_exist(bppath):unreal.USDImporterWidget.generate_new_vehicle_blueprint(world,base,sk,ph,bppath,parts,wheels)
 bp=unreal.load_asset(bppath)
 if not bp:raise RuntimeError('Vehicle blueprint generation failed')
 cdo=unreal.get_default_object(unreal.load_class(None,bppath+'.BP_TeslaModel3_C'));movement=cdo.get_component_by_class(unreal.ChaosWheeledVehicleMovementComponent)
 tick=cdo.get_editor_property('primary_actor_tick');tick.set_editor_property('start_with_tick_enabled',True);cdo.set_editor_property('primary_actor_tick',tick)
 cdo.set_editor_property('tags',list(dict.fromkeys([str(t) for t in cdo.get_editor_property('tags')]+['Carla.StaticBodyworkPaint','Carla.ModularWheelCalipers'])))
 cdo.set_editor_property('constraint_component_names',['PhysConst_FL','PhysConst_FR','PhysConst_RL','PhysConst_RR'])
 setup=[]
 for part,bone in [('FL','Wheel_Front_Left'),('FR','Wheel_Front_Right'),('RL','Wheel_Rear_Left'),('RR','Wheel_Rear_Right')]:
  ws=unreal.ChaosWheelSetup();ws.set_editor_property('wheel_class',wheelclasses[part]);ws.set_editor_property('bone_name',bone);setup.append(ws)
 movement.set_editor_property('wheel_setups',setup);movement.set_editor_property('mass',1850.0)
 # A single-speed EV approximation; calibration is documented separately from geometry.
 engine=movement.get_editor_property('engine_setup');curve=engine.get_editor_property('torque_curve')
 csv=P/'export/EVTorque.csv';csv.write_text('0,1\n5500,1\n10000,0.65\n15000,0.43\n18500,0.32\n')
 torque_path=DEST+'/Curve_EV_Torque'
 if eal.does_asset_exist(torque_path):torque=unreal.load_asset(torque_path)
 else:
  factory=unreal.CSVImportFactory();settings=unreal.CSVImportSettings();settings.import_type=unreal.CSVImportType.ECSV_CURVE_FLOAT;factory.set_editor_property('automated_import_settings',settings)
  task=unreal.AssetImportTask();task.filename=str(csv);task.destination_path=DEST;task.destination_name='Curve_EV_Torque';task.automated=True;task.save=True;task.factory=factory;at.import_asset_tasks([task]);torque=unreal.load_asset(torque_path)
 if not torque:raise RuntimeError('EV torque curve import failed')
 curve.set_editor_property('external_curve',torque);engine.set_editor_property('torque_curve',curve)
 for key,value in {'max_torque':550.0,'max_rpm':18500.0,'engine_idle_rpm':0.0,'engine_rev_up_moi':1.0}.items():engine.set_editor_property(key,value)
 movement.set_editor_property('engine_setup',engine)
 transmission=movement.get_editor_property('transmission_setup');transmission.set_editor_property('forward_gear_ratios',[1.0]);transmission.set_editor_property('reverse_gear_ratios',[1.0]);transmission.set_editor_property('final_ratio',9.0);transmission.set_editor_property('change_up_rpm',18000.0);transmission.set_editor_property('change_down_rpm',0.0);movement.set_editor_property('transmission_setup',transmission)
 movement.set_editor_property('enable_center_of_mass_override',True);movement.set_editor_property('center_of_mass_override',unreal.Vector(0,0,35))
 meshcomp=cdo.get_component_by_class(unreal.SkeletalMeshComponent)
 # The duplicated skeletal mesh supplies bones/physics, not visible bodywork.
 # Keep template geometry out of the separately attached visual assembly.
 meshcomp.set_editor_property('render_in_main_pass',False)
 meshcomp.set_editor_property('render_in_depth_pass',False)
 meshcomp.set_editor_property('cast_shadow',False)
 meshcomp.set_editor_property('visible_in_ray_tracing',False)
 meshcomp.set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
 animpath=DEST+'/ABP_Model3';anim=unreal.load_asset(animpath) if eal.does_asset_exist(animpath) else eal.duplicate_asset('/Game/Carla/Static/Car/4Wheeled/AudiTT/AnimBP_AudiTT',animpath)
 anim.set_editor_property('target_skeleton',sk.get_editor_property('skeleton'));unreal.BlueprintEditorLibrary.compile_blueprint(anim);eal.save_loaded_asset(anim)
 meshcomp.set_anim_instance_class(unreal.load_class(None,animpath+'.ABP_Model3_C'))

 body=meshcomp.get_editor_property('body_instance');report['old_simulate_physics']=body.get_editor_property('simulate_physics');body.set_editor_property('simulate_physics',True);body.set_editor_property('enable_gravity',True);meshcomp.set_editor_property('body_instance',body)
 bounds=cdo.get_component_by_class(unreal.BoxComponent)
 if bounds:
  bounds.set_editor_property('relative_scale3d',unreal.Vector(1,1,1));bounds.set_editor_property('box_extent',unreal.Vector(234.7,104.4,72.15));bounds.set_editor_property('relative_location',unreal.Vector(-6.85,0,72.15))
 # Separate calipers are updated by the opt-in native wheel visual path.
 subsystem=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem);lib=unreal.SubobjectDataBlueprintFunctionLibrary
 handles=subsystem.k2_gather_subobject_data_for_blueprint(bp)
 parent=next(h for h in handles if isinstance(lib.get_object_for_blueprint(lib.get_data(h),bp),unreal.SkeletalMeshComponent))
 # Modular lamps have explicit API-state tags. Authored photometry is retained
 # while native state control toggles visibility after template BP processing.
 authored_lights=[]
 for group,pitch,cone,intensity in [('Low Beam',-3,35,500000),('High Beam',0,20,1000000),('Fog',-6,55,120000)]:
  for side,y in [('L',-67),('R',67)]:
   authored_lights.append((group.replace(' ','_')+'_'+side,group,unreal.SpotLightComponent,(212 if group=='Fog' else 208,y,40 if group=='Fog' else 65),pitch,cone,intensity))
 authored_lights.append(('Interior_Roof','Interior',unreal.PointLightComponent,(19,0,127),0,0,20))
 # Disable the two template low beams superseded by the authored pair.
 # Hiding in game also survives the template's visibility toggles.
 for handle in handles:
  existing=lib.get_object_for_blueprint(lib.get_data(handle),bp)
  if isinstance(existing,unreal.LightComponent) and existing.get_name().startswith('front-low_beam-'):
   existing.set_editor_property('hidden_in_game',True)
 report['authored_lights']=[]
 for name,group,cls,location,pitch,cone,intensity in authored_lights:
  name='Model3_'+name
  comp=next((lib.get_object_for_blueprint(lib.get_data(h),bp) for h in handles if lib.get_object_for_blueprint(lib.get_data(h),bp) and lib.get_object_for_blueprint(lib.get_data(h),bp).get_name().startswith(name)),None)
  if not comp:
   params=unreal.AddNewSubobjectParams();params.parent_handle=parent;params.new_class=cls;params.blueprint_context=bp
   handle,reason=subsystem.add_new_subobject(params);subsystem.rename_subobject(handle,unreal.Text(name));comp=lib.get_object_for_blueprint(lib.get_data(handle),bp)
   if not comp:raise RuntimeError('Lamp component creation failed: '+str(reason))
  comp.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);comp.set_editor_property('component_tags',['Carla.Light.'+group]);comp.set_editor_property('relative_location',unreal.Vector(*location));comp.set_editor_property('relative_rotation',unreal.Rotator(pitch=pitch,yaw=0,roll=0))
  comp.set_editor_property('intensity_units',unreal.LightUnits.LUMENS if group=='Interior' else unreal.LightUnits.CANDELAS);comp.set_editor_property('intensity',float(intensity));comp.set_editor_property('attenuation_radius',150.0 if group=='Interior' else 10000.0);comp.set_editor_property('visible',False)
  if isinstance(comp,unreal.SpotLightComponent):comp.set_editor_property('outer_cone_angle',float(cone));comp.set_editor_property('inner_cone_angle',float(cone*.5))
  report['authored_lights'].append({'name':name,'group':group,'intensity':intensity})
 for index,part in enumerate(['FL','FR','RL','RR']):
  name='Caliper'+part
  comp=next((lib.get_object_for_blueprint(lib.get_data(h),bp) for h in handles if lib.get_object_for_blueprint(lib.get_data(h),bp) and lib.get_object_for_blueprint(lib.get_data(h),bp).get_name().startswith(name)),None)
  if not comp:
   params=unreal.AddNewSubobjectParams();params.parent_handle=parent;params.new_class=unreal.StaticMeshComponent;params.blueprint_context=bp
   handle,reason=subsystem.add_new_subobject(params);subsystem.rename_subobject(handle,unreal.Text(name));comp=lib.get_object_for_blueprint(lib.get_data(handle),bp)
   if not comp:raise RuntimeError('Caliper component creation failed: '+str(reason))
  comp.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);comp.set_static_mesh(meshes[name]);comp.set_editor_property('component_tags',['Carla.Caliper.'+str(index)]);comp.set_collision_profile_name('NoCollision')
  anchor=next(p['anchor_m'] for p in manifest['parts'] if p['part']==name);comp.set_editor_property('relative_location',unreal.Vector(anchor[0]*100,-anchor[1]*100,anchor[2]*100))
 # Wheels are visuals driven by Chaos suspension traces, never independent
 # blocking/auto-welded components. Also applies if geometry is later reimported.
 for handle in subsystem.k2_gather_subobject_data_for_blueprint(bp):
  component=lib.get_object_for_blueprint(lib.get_data(handle),bp)
  if isinstance(component,unreal.StaticMeshComponent) and component.get_name().startswith('Wheel_'):
   component.set_collision_profile_name('NoCollision')
   instance=component.get_editor_property('body_instance');instance.set_editor_property('auto_weld',False);component.set_editor_property('body_instance',instance)
 report['anim_class']=str(meshcomp.get_editor_property('anim_class'))
 unreal.BlueprintEditorLibrary.compile_blueprint(bp)
 for asset in [sk,ph,bp]:eal.save_loaded_asset(asset,only_if_is_dirty=False)
 report['blueprint']=bppath;report['status']='imported; runtime validation required';save()
except Exception:
 report['errors'].append(traceback.format_exc());save();raise
