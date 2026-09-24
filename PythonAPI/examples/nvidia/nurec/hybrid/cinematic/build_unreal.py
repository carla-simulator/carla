"""Run in the UE editor with HYBRID_SHOT=/absolute/shot.
Creates isolated map, baked animation, sequence and Movie Render Graph assets.
HYBRID_RENDER=1 starts the graph; HYBRID_LIMIT=1 permits a first-frame smoke test.
"""
import json,os,math,traceback
from pathlib import Path
import unreal as u

ROOT=Path(os.environ['HYBRID_SHOT']);M=json.loads((ROOT/'manifest.json').read_text());F=json.loads((ROOT/'capture.json').read_text())
ASSET='/Game/HybridCinematicMVP/'+M['shot'];TOOLS=u.AssetToolsHelpers.get_asset_tools()

def prop(obj,name,value):obj.set_editor_property(name,value)
def setting(obj,name,value):
    prop(obj,('override_b_' if isinstance(value,bool) else 'override_')+name,True);prop(obj,name,value)
def rot(r):return u.Rotator(pitch=r[0],yaw=r[1],roll=r[2])
def loc(v):return u.Vector(*[x*100 for x in v])
def asset(name,cls,factory=None):
    path=ASSET+'/'+name
    if u.EditorAssetLibrary.does_asset_exist(path):return u.load_asset(path)
    return TOOLS.create_asset(name,ASSET,cls,factory)

def animate(seq,actor,samples):
    binding=seq.add_possessable(actor);track=binding.add_track(u.MovieScene3DTransformTrack);section=track.add_section();section.set_range(969,1049)
    channels=section.get_all_channels()
    for f,data in samples:
        values=[x*100 for x in data['location']]+[data['rotation'][2],data['rotation'][0],data['rotation'][1]]+[1,1,1]
        for ch,value in zip(channels,values):ch.add_key(u.FrameNumber(f),float(value),interpolation=u.MovieSceneKeyInterpolation.LINEAR)
    return binding

def main():
    levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
    world_path=ASSET+'/World'
    if u.EditorAssetLibrary.does_asset_exist(world_path):
        levels.load_level(world_path)
        actor_sub=u.get_editor_subsystem(u.EditorActorSubsystem)
        for old in actor_sub.get_all_level_actors():
            if not isinstance(old,(u.WorldSettings,u.Brush)):
                actor_sub.destroy_actor(old)
    else:
        if not levels.new_level(world_path):raise RuntimeError('Could not create isolated shot world')
    actors=u.get_editor_subsystem(u.EditorActorSubsystem)
    camera=actors.spawn_actor_from_class(u.CineCameraActor,loc(F[0]['camera']['location']),rot(F[0]['camera']['rotation']))
    camera.set_actor_label('CalibratedCamera');cc=camera.get_cine_camera_component();w,h=M['resolution'];fx=M['K'][0][0]
    film=cc.get_editor_property('filmback');film.sensor_width=36;film.sensor_height=36*h/w;cc.set_editor_property('filmback',film);cc.set_editor_property('current_focal_length',fx*36/w)
    focus=cc.get_editor_property('focus_settings');focus.focus_method=u.CameraFocusMethod.DISABLE;cc.set_editor_property('focus_settings',focus)
    pp=cc.get_editor_property('post_process_settings')
    for k,v in {'override_auto_exposure_method':True,'auto_exposure_method':u.AutoExposureMethod.AEM_MANUAL,'override_auto_exposure_apply_physical_camera_exposure':True,'auto_exposure_apply_physical_camera_exposure':False,'override_auto_exposure_bias':True,'auto_exposure_bias':0.,'override_motion_blur_amount':True,'motion_blur_amount':0.,'override_bloom_intensity':True,'bloom_intensity':0.,'override_vignette_intensity':True,'vignette_intensity':0.}.items():prop(pp,k,v)
    cc.set_editor_property('post_process_settings',pp)
    # Independent engine projection versus recorded optical K for three markers.
    view=cc.get_camera_view(0.)
    vm,pm,vp=u.GameplayStatics.get_view_projection_matrix(view)
    markers=[]
    for optical in [(-1.,-.5,6.),(1.,.5,10.),(.2,-.8,15.)]:
        t=F[0]['camera_to_world_optical']
        point=[sum(t[row][col]*optical[col] for col in range(3))+t[row][3] for row in range(3)]
        projected=vp.transform_position(loc(point))
        ue=[(projected.x/projected.w+1)*w/2,(1-projected.y/projected.w)*h/2]
        expected=[fx*optical[0]/optical[2]+w/2,fx*optical[1]/optical[2]+h/2]
        error=math.hypot(ue[0]-expected[0],ue[1]-expected[1]);markers.append({'world_m':point,'ue_pixel':ue,'nurec_pinhole_pixel':expected,'error_px':error})
    (ROOT/'camera-numeric-gate.json').write_text(json.dumps({'markers':markers,'passed':max(x['error_px'] for x in markers)<1,'note':'Engine projection matrix checked; rendered-marker gate remains separate'},indent=2))
    if max(x['error_px'] for x in markers)>=1:raise RuntimeError('Camera projection mismatch')
    # Load related blueprints before holding component references: compilation can
    # reconstruct existing MetaHuman actors and invalidate Python wrappers.
    walker_cls=u.EditorAssetLibrary.load_blueprint_class('/Game/Carla/Characters/MetaHumans/German/BP_German_Walker')
    walker=actors.spawn_actor_from_class(walker_cls,u.Vector(),u.Rotator())
    native=next(x for x in walker.get_components_by_class(u.SkeletalMeshComponent) if x.get_name()=='CharacterMesh0')
    mesh_offset=native.get_editor_property('relative_location');mesh_rotation=native.get_editor_property('relative_rotation');actors.destroy_actor(walker)
    hero=actors.spawn_actor_from_class(u.EditorAssetLibrary.load_blueprint_class(M['hero_asset']),u.Vector(),u.Rotator());hero.set_actor_label('Hero')
    components=hero.get_components_by_class(u.SkeletalMeshComponent)
    body=next(x for x in components if x.get_name()=='Body')
    quality={'body_mesh_offset_cm':[mesh_offset.x,mesh_offset.y,mesh_offset.z],'body_mesh_rotation':str(mesh_rotation),'grooms':[]}
    for sync in hero.get_components_by_class(u.LODSyncComponent):sync.set_editor_property('forced_lod',0)
    for comp in components:comp.set_forced_lod(1)
    for groom in hero.get_components_by_class(u.GroomComponent):
        # LODSync drives GroomComponent through its native LOD interface.
        quality['grooms'].append({'component':groom.get_name(),'asset':str(groom.get_editor_property('groom_asset'))})
    components=hero.get_components_by_class(u.SkeletalMeshComponent)
    body=next(x for x in components if x.get_name()=='Body')
    (ROOT/'component-inventory.json').write_text(json.dumps([{'name':x.get_name(),'mesh':str(x.skeletal_mesh_asset)} for x in components],indent=2))
    # Bake original local skeletal samples, not a guessed walk-loop phase.
    factory=u.AnimSequenceFactory();factory.target_skeleton=body.skeletal_mesh_asset.skeleton
    anim=asset('CapturedBody',u.AnimSequence,factory);controller=anim.controller
    controller.open_bracket('Import CARLA body samples',False);controller.remove_all_bone_tracks(False);controller.set_frame_rate(u.FrameRate(120,1),False);controller.set_frame_rate(u.FrameRate(24,1),False);controller.set_number_of_frames(u.FrameNumber(48),False)
    for b in F[0]['actors'][0]['bones']:
        name=b['name'];samples=[next(x['relative'] for x in f['actors'][0]['bones'] if x['name']==name) for f in F]
        samples.append(samples[-1])
        if controller.add_bone_curve(name,False):
            controller.set_bone_track_keys(name,[loc(x['location']) for x in samples],[rot(x['rotation']).quaternion() for x in samples],[u.Vector(1,1,1)]*49,False)
    controller.close_bracket(False);u.EditorAssetLibrary.save_loaded_asset(anim)
    sequence=asset('Shot',u.LevelSequence,u.LevelSequenceFactoryNew());sequence.set_display_rate(u.FrameRate(24,1));sequence.set_tick_resolution_directly(u.FrameRate(24000,1));sequence.set_playback_start(1001);sequence.set_playback_end(1001+int(os.environ.get('HYBRID_LIMIT','48')))
    # Replace only this generated sequence's previous bindings/tracks on retry.
    for b in sequence.get_bindings():b.remove()
    for t in sequence.get_tracks():sequence.remove_track(t)
    cb=animate(sequence,camera,[(f['frame'],f['camera']) for f in F])
    hero_samples=[]
    for f in F:
        d=f['actors'][0]['transform'];p=list(d['location']);p[2]+=mesh_offset.z/100
        r=list(d['rotation']);r[1]+=mesh_rotation.yaw
        hero_samples.append((f['frame'],{'location':p,'rotation':r}))
    hb=animate(sequence,hero,hero_samples)
    bb=sequence.add_possessable(body);bb.set_parent(hb)
    section=bb.add_track(u.MovieSceneSkeletalAnimationTrack).add_section();section.set_range(1001,1049);params=section.params;params.animation=anim;section.params=params
    outfit=actors.spawn_actor_from_class(u.SkeletalMeshActor,u.Vector(),u.Rotator());outfit.set_actor_label('HeroOutfit')
    outfit.skeletal_mesh_component.set_skeletal_mesh_asset(u.load_asset('/Game/Carla/Characters/MetaHumans/German/Outfit/SK_German_Outfit_Runtime'))
    outfit.skeletal_mesh_component.set_forced_lod(1)
    body.set_material(0,u.load_asset('/Game/Carla/Characters/MetaHumans/German/CinematicReview/Common/Materials/M_Hide'))
    ob=animate(sequence,outfit,hero_samples)
    oc=sequence.add_possessable(outfit.skeletal_mesh_component);oc.set_parent(ob)
    outfit_section=oc.add_track(u.MovieSceneSkeletalAnimationTrack).add_section();outfit_section.set_range(1001,1049);params=outfit_section.params;params.animation=anim;outfit_section.params=params
    cut=sequence.add_track(u.MovieSceneCameraCutTrack).add_section();cut.set_range(1001,1049);binding_id=u.MovieSceneObjectBindingID();binding_id.set_editor_property('guid',cb.get_id());cut.set_camera_binding_id(binding_id)
    # Minimal interaction proxy. Geometry fit is a validation gate, not inferred truth.
    road=actors.spawn_actor_from_class(u.StaticMeshActor,loc([F[0]['actors'][0]['transform']['location'][0],F[0]['actors'][0]['transform']['location'][1],M['proxy_road_height_m']]),u.Rotator())
    road.set_actor_label('ProxyRoad');road.static_mesh_component.set_static_mesh(u.load_asset('/Engine/BasicShapes/Plane'));road.set_actor_scale3d(u.Vector(30,30,1))
    # Low-frequency sky and dominant source are editable shot lighting.
    sky=actors.spawn_actor_from_class(u.SkyLight,u.Vector(),u.Rotator());sky.light_component.set_editor_property('intensity',1.)
    sun=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(),u.Rotator(pitch=-35,yaw=80));sun.light_component.set_editor_property('intensity',3.)
    world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
    for command in ['r.PostProcessing.PropagateAlpha 1','sg.ViewDistanceQuality 4','sg.ShadowQuality 4','sg.TextureQuality 4','sg.EffectsQuality 4','r.ForceLOD 0','r.SkeletalMeshLODBias 0','r.HairStrands.UseCardsInsteadOfStrands 0','r.MotionBlurQuality 0','r.PathTracing.MaxBounces 8']:
        u.SystemLibrary.execute_console_command(world,command)
    if u.EditorAssetLibrary.does_asset_exist(ASSET+'/RenderGraph'):
        u.EditorAssetLibrary.delete_asset(ASSET+'/RenderGraph')
    graph=asset('RenderGraph',u.MovieGraphConfig)
    output=graph.get_output_node();settings=graph.create_node_by_class(u.MovieGraphGlobalOutputSettingNode)
    setting(settings,'output_resolution',u.MovieGraphLibrary.named_resolution_from_size(w,h));setting(settings,'output_directory',u.DirectoryPath(str(ROOT/'renders/ue_review')))
    sampling=graph.create_node_by_class(u.MovieGraphSamplingMethodNode);setting(sampling,'temporal_sample_count',1)
    warm=graph.create_node_by_class(u.MovieGraphWarmUpSettingNode);setting(warm,'num_warm_up_frames',32)
    exr=graph.create_node_by_class(u.MovieGraphImageSequenceOutputNode_EXR)
    graph.add_labeled_edge(settings,'',sampling,'');graph.add_labeled_edge(sampling,'',warm,'');graph.add_labeled_edge(warm,'',exr,'');graph.add_labeled_edge(exr,'',output,'Globals')
    depth_material=asset('DepthMetres',u.Material,u.MaterialFactoryNew())
    depth_material.set_editor_property('material_domain',u.MaterialDomain.MD_POST_PROCESS)
    depth_material.set_editor_property('blendable_location',u.BlendableLocation.BL_SCENE_COLOR_AFTER_TONEMAPPING)
    u.MaterialEditingLibrary.delete_all_material_expressions(depth_material)
    sd=u.MaterialEditingLibrary.create_material_expression(depth_material,u.MaterialExpressionSceneDepth)
    div=u.MaterialEditingLibrary.create_material_expression(depth_material,u.MaterialExpressionDivide);div.set_editor_property('const_b',100.)
    u.MaterialEditingLibrary.connect_material_expressions(sd,'',div,'A');u.MaterialEditingLibrary.connect_material_property(div,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    u.MaterialEditingLibrary.recompile_material(depth_material);u.EditorAssetLibrary.save_loaded_asset(depth_material)
    def render_branch(name, modifications):
        branch=graph.add_output();branch.set_member_name(name)
        pt=graph.create_node_by_class(u.MovieGraphPathTracerRenderPassNode)
        setting(pt,'spatial_sample_count',int(os.environ.get('HYBRID_SPP','32')))
        setting(pt,'disable_tone_curve',True);setting(pt,'enable_denoiser',True)
        if name=='integration':
            depth=u.MoviePipelinePostProcessPass();depth.enabled=True;depth.name='depth_m';depth.material=depth_material;depth.high_precision_output=True;depth.use_lossless_compression=True
            setting(pt,'additional_post_process_materials',[depth])
        previous=pt
        for label,changes in modifications:
            collection=graph.create_node_by_class(u.MovieGraphCollectionNode)
            collection.collection.set_collection_name(name+'_'+label)
            query=collection.collection.add_condition_group().add_query(u.MovieGraphConditionGroupQuery_ActorName)
            query.set_editor_property('wildcard_search',label)
            modifier=graph.create_node_by_class(u.MovieGraphModifierNode)
            modifier.add_collection(name+'_'+label)
            render_properties=next((x for x in modifier.get_all_modifiers() if isinstance(x,u.MovieGraphRenderPropertyModifier)),None)
            if render_properties is None:render_properties=modifier.add_modifier(u.MovieGraphRenderPropertyModifier)
            for k,v in changes.items():setting(render_properties,k,v)
            graph.add_labeled_edge(previous,'',collection,'');graph.add_labeled_edge(collection,'',modifier,'');previous=modifier
        layer=graph.create_node_by_class(u.MovieGraphRenderLayerNode);setting(layer,'layer_name',name)
        graph.add_labeled_edge(previous,'',layer,'');graph.add_labeled_edge(layer,'',output,name)
    render_branch('beauty',[('ProxyRoad',{'holdout':True})])
    render_branch('integration',[])
    render_branch('clean',[('Hero*',{'is_hidden':True,'cast_shadow_while_hidden':False,'affect_indirect_lighting_while_hidden':False})])
    u.EditorAssetLibrary.save_loaded_asset(sequence);u.EditorAssetLibrary.save_loaded_asset(graph);u.get_editor_subsystem(u.LevelEditorSubsystem).save_current_level()
    quality['camera_focal_mm']=cc.current_focal_length;quality['filmback_mm']=[film.sensor_width,film.sensor_height];quality['status']='built; visual gates pending';(ROOT/'unreal-build.json').write_text(json.dumps(quality,indent=2))
    if os.environ.get('HYBRID_RENDER')=='1':
        subsystem=u.get_editor_subsystem(u.MoviePipelineQueueSubsystem);queue=subsystem.get_queue();queue.delete_all_jobs();job=queue.allocate_new_job(u.MoviePipelineExecutorJob);job.map=u.SoftObjectPath(ASSET+'/World');job.sequence=u.SoftObjectPath(sequence.get_path_name());job.set_graph_preset(graph)
        global executor
        executor=u.MoviePipelinePIEExecutor()
        def finished(ex,success):
            (ROOT/'unreal-render-status.json').write_text(json.dumps({'success':success}));u.SystemLibrary.quit_editor()
        executor.on_executor_finished_delegate.add_callable_unique(finished)
        subsystem.render_queue_with_executor_instance(executor)

try:main()
except Exception:
    (ROOT/'unreal-error.txt').write_text(traceback.format_exc());raise
