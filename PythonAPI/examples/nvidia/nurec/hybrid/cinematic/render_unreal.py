"""Execute in a full editor; render an existing shot's saved Movie Render Graph."""
import os,json,time,traceback,math
from pathlib import Path
import unreal as u
ROOT=Path(os.environ['HYBRID_SHOT']);m=json.loads((ROOT/'manifest.json').read_text());base='/Game/HybridCinematicMVP/'+m['shot']
try:
    # ExecutePythonScript otherwise exits as soon as this asynchronous setup returns.
    u.EditorPythonScripting.set_keep_python_script_alive(True)
    levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
    if not levels.load_level(base+'/World'):raise RuntimeError('Shot world missing')
    world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property('default_game_mode',u.MoviePipelineGameMode)
    if (ROOT/'proxy-fit.json').exists() and not (ROOT/'receiver.fbx').exists():
        fit=json.loads((ROOT/'proxy-fit.json').read_text())
        if fit['rms_m']>.03:raise RuntimeError('Road fit exceeds 3 cm residual')
        for actor in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors():
            if actor.get_actor_label()=='ProxyRoad':
                # The moving lead begins beyond the old 30m receiver footprint.
                actor.static_mesh_component.set_relative_scale3d(u.Vector(2,2,1))
                actor.set_actor_location(u.Vector(*[x*100 for x in fit['origin_xy_m']],(fit['height_m']+fit.get('receiver_vertical_offset_m',0))*100),False,False)
                actor.set_actor_rotation(u.Rotator(pitch=math.degrees(math.atan(fit['slope_x'])),roll=math.degrees(math.atan(-fit['slope_y']/math.sqrt(1+fit['slope_x']**2)))),False)
    if (ROOT/'lighting.json').exists():
        light=json.loads((ROOT/'lighting.json').read_text())
        task=u.AssetImportTask();task.filename=str(ROOT/light['environment']);task.destination_path=base;task.destination_name='Environment';task.automated=True;task.replace_existing=True;task.save=True
        u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        cube=u.load_asset(base+'/Environment')
        if not isinstance(cube,u.TextureCube):raise RuntimeError('Environment did not import as a TextureCube')
        for actor in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors():
            if isinstance(actor,u.SkyLight):
                actor.light_component.set_editor_property('source_type',u.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
                actor.light_component.set_editor_property('cubemap',cube)
                actor.light_component.set_editor_property('intensity',light['sky_intensity'])
            if isinstance(actor,u.DirectionalLight):
                r=light['sun_rotation_degrees'];actor.set_actor_rotation(u.Rotator(pitch=r[0],yaw=r[1],roll=r[2]),False)
                actor.light_component.set_editor_property('intensity',light['sun_intensity'])
    levels.save_current_level()
    # The interactive DLSS-RR preset discards coverage. Keep raw path-traced
    # samples until a cinematic denoiser has passed the alpha/temporal gates.
    u.SystemLibrary.execute_console_command(world,'r.PathTracing.Denoiser 0')
    u.SystemLibrary.execute_console_command(world,'MoviePipeline.FlushLayersDebug 1')
    u.SystemLibrary.execute_console_command(world,'r.PathTracing.FilterWidth 1')
    if os.environ.get('HYBRID_DENOISER')=='nne':
        for command in ['NNEDenoiser.Asset 6','r.PathTracing.SpatialDenoiser.Type 0','r.PathTracing.Denoiser.Name NNEDenoiser','r.PathTracing.Denoiser -1']:
            u.SystemLibrary.execute_console_command(world,command)
    for cmd in ['MoviePipeline.AlphaOutputOverride 1','r.PostProcessing.PropagateAlpha 1','r.PathTracing.BackgroundAlpha 0','r.PathTracing 1','r.PathTracing.MaxBounces 8','r.HairStrands.UseCardsInsteadOfStrands 0','r.HairStrands.Raytracing 1','r.Streaming.FullyLoadUsedTextures 1','r.SkinCache.SceneMemoryLimitInMB 1024','r.BasePassForceOutputsVelocity 1','r.MotionBlurQuality 0','sg.TextureQuality 4','sg.ShadowQuality 4','sg.EffectsQuality 4']:
        u.SystemLibrary.execute_console_command(world,cmd)
    start=int(os.environ.get('HYBRID_START_FRAME',m['frame_start']));limit=int(os.environ.get('HYBRID_LIMIT','48'))
    sequence=u.load_asset(base+'/Shot')
    if os.environ.get('HYBRID_WHEEL_DIAGNOSTIC')=='1':
        import sys,copy
        sys.path.insert(0,str(Path(__file__).resolve().parent))
        from crossing_unreal import animate_camera
        path=base+'/ShotWheelDiagnostic'
        if u.EditorAssetLibrary.does_asset_exist(path):u.EditorAssetLibrary.delete_asset(path)
        sequence=u.EditorAssetLibrary.duplicate_asset(base+'/Shot',path)
        cut=next(t for t in sequence.get_tracks() if isinstance(t,u.MovieSceneCameraCutTrack))
        camera_id=cut.get_sections()[0].get_camera_binding_id().get_editor_property('guid')
        camera_actor=next(a for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors() if isinstance(a,u.CineCameraActor))
        binding=next(b for b in sequence.get_bindings() if str(b.get_display_name())==camera_actor.get_actor_label())
        for track in binding.get_tracks():binding.remove_track(track)
        frames=copy.deepcopy(json.loads((ROOT/'capture.json').read_text()))
        for f in frames:
            pose=next(a for a in f['actors'] if a['id']=='lead')['transform'];x,y,z=pose['location'];yaw=pose['rotation'][1];theta=math.radians(yaw)
            f['camera']={'location':[x-1.4*math.cos(theta)+2.5*math.sin(theta),y-1.4*math.sin(theta)-2.5*math.cos(theta),z+.65], 'rotation':[-6.4,yaw+90,0]}
        animate_camera(binding,frames,start,start+limit)
        u.EditorAssetLibrary.save_loaded_asset(sequence)
    sequence.set_playback_start(start);sequence.set_playback_end(start+limit)
    if m.get('actor_render_type')=='geometry_cache':
        hero=next(a for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors() if a.get_actor_label()=='HeroMary')
        component=hero.get_geometry_cache_component()
        mapping=[r for r in json.loads((ROOT/'mary-frame-map.json').read_text()) if start<=r['frame']<start+limit]
        if not u.LevelSequenceEditorBlueprintLibrary.open_level_sequence(sequence):raise RuntimeError('Cannot evaluate Mary sequence')
        audit=[]
        traffic_audit=[]
        captured={f['frame']:f for f in json.loads((ROOT/'capture.json').read_text())}
        traffic={a.get_actor_label().removeprefix('HeroTraffic_'):a for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors() if a.get_actor_label().startswith('HeroTraffic_')}
        for row in mapping:
            position=u.MovieSceneSequencePlaybackParams();position.frame=u.FrameTime(u.FrameNumber(row['frame']));position.position_type=u.MovieScenePositionType.FRAME
            u.LevelSequenceEditorBlueprintLibrary.set_global_position(position)
            for role,vehicle in traffic.items():
                expected=next(a for a in captured[row['frame']]['actors'] if a['id']==role)['transform']
                loc=vehicle.get_actor_location();xy_error=math.hypot(loc.x/100-expected['location'][0],loc.y/100-expected['location'][1])
                if xy_error>.001:raise RuntimeError('Cinematic traffic XY differs from capture')
                traffic_audit.append({'frame':row['frame'],'role':role,'xy_error_m':xy_error,'location_cm':[loc.x,loc.y,loc.z]})
            seconds=component.get_animation_time();sample=1+math.floor(seconds*30+.5)
            material=component.get_material(0).get_path_name()
            origin,extent=hero.get_actor_bounds(False);location=hero.get_actor_location()
            audit.append({'frame':row['frame'],'cache_time_s':seconds,'source_frame':sample,'material':material,
                          'world_bounds_origin_cm':[origin.x,origin.y,origin.z],
                          'actor_location_cm':[location.x,location.y,location.z]})
            if sample!=row['source_frame'] or material!=row['texture']:
                raise RuntimeError('Mary mesh/texture synchronization failed: '+str(audit[-1]))
        (ROOT/'mary-sequence-validation.json').write_text(json.dumps({'passed':True,'scope':'Sequencer time and texture binding; sample inferred from installed GeometryCacheSceneProxy nearest-sample policy. Rendered UV correspondence also requires visual review.','frames':audit},indent=2))
        (ROOT/'traffic-sequence-validation.json').write_text(json.dumps({'passed':True,'frames':traffic_audit,'scope':'Captured vehicle XY versus evaluated Sequencer; Z is local NuRec surface derivative.'},indent=2))
        u.LevelSequenceEditorBlueprintLibrary.close_level_sequence()
    if (ROOT/'grounding.json').exists():
        grounding=json.loads((ROOT/'grounding.json').read_text())
        if grounding['source_capture_sha256']!=m['capture_sha256']:raise RuntimeError('Grounding belongs to another capture')
        capture={f['frame']:f for f in json.loads((ROOT/'capture.json').read_text())}
        mesh_offset=json.loads((ROOT/'unreal-build.json').read_text())['body_mesh_offset_cm'][2]/100
        adjusted=[]
        for binding in sequence.get_bindings():
            label=str(binding.get_display_name())
            if label not in ['Hero','HeroOutfit']:continue
            for track in binding.get_tracks():
                if not isinstance(track,u.MovieScene3DTransformTrack):continue
                for section in track.get_sections():
                    for key in section.get_all_channels()[2].get_keys():
                        frame=key.get_time(u.MovieSceneTimeUnit.DISPLAY_RATE).frame_number.value
                        delta=grounding['offset_z_m_by_frame'][str(frame)]
                        if abs(delta)>.1:raise RuntimeError('Grounding exceeds 10 cm correction limit')
                        key.set_value((capture[frame]['actors'][0]['transform']['location'][2]+mesh_offset+delta)*100)
                adjusted.append(label)
        if sorted(adjusted)!=['Hero','HeroOutfit']:raise RuntimeError('Grounding did not find both actor tracks: '+str(adjusted))
    if os.environ.get('HYBRID_CAMERA_MARKERS')=='1':
        # Isolated render-only marker scene; never save these world changes.
        captured=json.loads((ROOT/'capture.json').read_text())
        f=next(f for f in captured if f['frame']==start);t=f['camera_to_world_optical'];ft=m['ftheta']
        actors=u.get_editor_subsystem(u.EditorActorSubsystem)
        for actor in actors.get_all_level_actors():
            if actor.get_actor_label().startswith(('Hero','Proxy')):actor.set_actor_hidden_in_game(True)
        markers=[];w,h=m['resolution']
        for index,(px,py) in enumerate([(.15,.2),(.5,.2),(.85,.2),(.25,.8),(.75,.8)]):
            x=int(px*w)+.5;y=int(py*h)+.5;dx=x-ft['principal_point_x'];dy=y-ft['principal_point_y'];r=math.hypot(dx,dy)
            theta=sum(c*r**i for i,c in enumerate(ft['pixeldist_to_angle_poly']))
            v=[2*math.sin(theta)*dx/r,2*math.sin(theta)*dy/r,2*math.cos(theta)]
            pos=[sum(t[row][col]*v[col] for col in range(3))+t[row][3] for row in range(3)]
            marker=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*[v*100 for v in pos]),u.Rotator())
            marker.set_actor_label('HeroCalibration_'+str(index));marker.static_mesh_component.set_static_mesh(u.load_asset('/Engine/BasicShapes/Sphere'))
            marker.set_actor_scale3d(u.Vector(.03,.03,.03))
            markers.append({'pixel_centre':[x,y],'expected_image_index':[x-.5,y-.5],'world_m':pos,'expected_optical_z_m':v[2]})
        (ROOT/'camera-markers.json').write_text(json.dumps(markers,indent=2))
    graph=u.load_asset(base+'/RenderGraph')
    def configure_output(target):
        node=target.get_output_node().get_input_pin('Globals').get_connected_nodes()[0]
        while not isinstance(node,u.MovieGraphGlobalOutputSettingNode):
            node=node.get_input_pin('').get_connected_nodes()[0]
        node.set_editor_property('override_output_resolution',True)
        node.set_editor_property('output_resolution',u.MovieGraphLibrary.named_resolution_from_size(*m['resolution']))
        node.set_editor_property('override_output_directory',True)
        output=ROOT/'diagnostics/wheels' if os.environ.get('HYBRID_WHEEL_DIAGNOSTIC')=='1' else ROOT/'renders/ue_review'
        if os.environ.get('HYBRID_CAMERA_MARKERS')=='1':output=ROOT/'diagnostics/camera'
        node.set_editor_property('output_directory',u.DirectoryPath(str(output)))
        node=target.get_output_node().get_input_pin('integration').get_connected_nodes()[0]
        while not isinstance(node,u.MovieGraphPathTracerRenderPassNode):
            node=node.get_input_pin('').get_connected_nodes()[0]
        passes=list(node.additional_post_process_materials)
        for p in passes:
            if p.name=='depth_m':p.material=u.load_asset(base+'/DepthMetres')
        node.set_editor_property('additional_post_process_materials',passes)
    configure_output(graph)
    # Soft material references otherwise load only on the last spatial sample.
    # Keep utility shaders resident before warm-up starts.
    utility_materials=[u.load_asset(base+'/DepthMetres')]
    if m.get('actor_render_type')=='geometry_cache':
        u.SystemLibrary.execute_console_command(world,'r.RayTracing.Geometry.GeometryCache 1')
        character_material=u.load_asset(base+'/MaryMaterial')
        if character_material is None:raise RuntimeError('Missing Mary material')
        u.MaterialEditingLibrary.get_statistics(character_material)
    if os.environ.get('HYBRID_DIAGNOSTIC')=='1':
        utility_materials.append(u.load_asset('/MovieRenderPipeline/Materials/MovieRenderQueue_WorldDepth'))
    for material in utility_materials:
        if material is None:raise RuntimeError('Missing utility material')
        # In this engine GetStatistics submits missing shader permutations and
        # calls FinishCompilation. Preloading or waiting a fixed time does not.
        if u.MaterialEditingLibrary.get_statistics(material).num_pixel_shader_instructions<=0:
            raise RuntimeError('Utility shader did not compile: '+material.get_path_name())
    (ROOT/'utility-materials.json').write_text(json.dumps([
        {'path':mat.get_path_name(),'domain':str(mat.material_domain),
         'emissive_input':str(u.MaterialEditingLibrary.get_material_property_input_node(mat,u.MaterialProperty.MP_EMISSIVE_COLOR)),
         'expressions':[str(x) for x in u.MaterialEditingLibrary.get_material_expressions(mat)],
         'statistics':str(u.MaterialEditingLibrary.get_statistics(mat))}
        for mat in utility_materials],indent=2))
    for name in ['beauty','integration','clean']:
        node=graph.get_output_node().get_input_pin(name).get_connected_nodes()[0]
        while not isinstance(node,u.MovieGraphPathTracerRenderPassNode):
            node=node.get_input_pin('').get_connected_nodes()[0]
        node.override_spatial_sample_count=True
        node.spatial_sample_count=int(os.environ.get('HYBRID_SPP','32'))
    if os.environ.get('HYBRID_DIAGNOSTIC')=='1':
        inspected=[]
        for name in ['beauty','integration','clean']:
            node=graph.get_output_node().get_input_pin(name).get_connected_nodes()[0]
            while True:
                if isinstance(node,u.MovieGraphPathTracerRenderPassNode):
                    node.set_editor_property('override_b_enable_denoiser',True)
                    node.set_editor_property('enable_denoiser',False)
                    inspected.append({'layer':name,'disable_tone_curve':node.disable_tone_curve,'spp':node.spatial_sample_count,'denoiser':node.enable_denoiser})
                    break
                node=node.get_input_pin('').get_connected_nodes()[0]
        branch=graph.add_output();branch.set_member_name('utility')
        deferred=graph.create_node_by_class(u.MovieGraphDeferredRenderPassNode)
        deferred.override_b_disable_tone_curve=True;deferred.disable_tone_curve=True
        deferred.override_anti_aliasing_method=True;deferred.anti_aliasing_method=u.AntiAliasingMethod.AAM_NONE
        depth=u.MoviePipelinePostProcessPass();depth.enabled=True;depth.name='engine_depth';depth.material=u.load_asset('/MovieRenderPipeline/Materials/MovieRenderQueue_WorldDepth');depth.high_precision_output=True;depth.use_lossless_compression=True
        deferred.override_additional_post_process_materials=True;deferred.additional_post_process_materials=[depth]
        layer=graph.create_node_by_class(u.MovieGraphRenderLayerNode);layer.override_layer_name=True;layer.layer_name='utility'
        graph.add_labeled_edge(deferred,'',layer,'');graph.add_labeled_edge(layer,'',graph.get_output_node(),'utility')
        (ROOT/'render-diagnostic-settings.json').write_text(json.dumps(inspected,indent=2))
    subsystem=u.get_editor_subsystem(u.MoviePipelineQueueSubsystem);queue=subsystem.get_queue();queue.delete_all_jobs()
    # Vulkan retains per-sample ray-tracing binding tables across frames. Three
    # simultaneous PT layers exhaust VRAM even at this small image size. Run
    # each layer as a separate job, preserving the same sequence/camera times.
    job_graphs=[graph]
    if os.environ.get('HYBRID_DIAGNOSTIC')!='1':
        u.EditorAssetLibrary.save_loaded_asset(graph,only_if_is_dirty=False)
        job_graphs=[]
        selected_layers=['integration'] if os.environ.get('HYBRID_WHEEL_DIAGNOSTIC')=='1' else ['beauty','integration','clean']
        for selected in selected_layers:
            path=base+'/RenderGraph_'+selected
            if u.EditorAssetLibrary.does_asset_exist(path):u.EditorAssetLibrary.delete_asset(path)
            isolated=u.EditorAssetLibrary.duplicate_asset(base+'/RenderGraph',path)
            configure_output(isolated)
            for name in ['beauty','integration','clean']:
                if name==selected:continue
                output_node=isolated.get_output_node()
                upstream=output_node.get_input_pin(name).get_connected_nodes()[0]
                isolated.remove_labeled_edge(upstream,'',output_node,name)
            job_graphs.append(isolated)
    for job_graph in job_graphs:
        job=queue.allocate_new_job(u.MoviePipelineExecutorJob);job.map=u.SoftObjectPath(base+'/World');job.sequence=u.SoftObjectPath(sequence.get_path_name());job.set_graph_preset(job_graph)
    executor=u.MoviePipelinePIEExecutor()
    errors=[]
    wheel_samples=[]
    def audit_pie_wheels(delta):
        pie=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_game_world()
        if pie is None:return
        vehicles=[a for a in u.GameplayStatics.get_all_actors_of_class(pie,u.Actor) if a.get_actor_label()=='HeroTraffic_lead']
        if not vehicles:return
        mesh=vehicles[0].get_component_by_class(u.SkeletalMeshComponent)
        q=mesh.get_socket_transform('Wheel_Rear_Left',u.RelativeTransformSpace.RTS_COMPONENT).rotation
        row=[q.x,q.y,q.z,q.w]
        if not wheel_samples or sum(abs(a-b) for a,b in zip(row,wheel_samples[-1]['quaternion']))>1e-4:
            p=vehicles[0].get_actor_location();wheel_samples.append({'quaternion':row,'actor_xy_cm':[p.x,p.y]})
    wheel_audit_handle=u.register_slate_post_tick_callback(audit_pie_wheels)
    def error(ex,pipeline,fatal,text):errors.append({'fatal':fatal,'message':str(text)})
    def finished(ex,success):
        u.unregister_slate_post_tick_callback(wheel_audit_handle)
        (ROOT/'wheel-render-validation.json').write_text(json.dumps({'scope':'rear-left wheel bone observed in actual PIE render world','distinct_poses':len(wheel_samples),'rotation_observed':len(wheel_samples)>2,'samples':wheel_samples},indent=2))
        (ROOT/'unreal-render-status.json').write_text(json.dumps({'success':success,'errors':errors},indent=2))
        # Let PIE teardown and render-resource release finish before editor exit.
        global quit_handle
        deadline=time.monotonic()+5
        def quit_when_idle(delta):
            if time.monotonic()>=deadline:
                u.unregister_slate_post_tick_callback(quit_handle)
                u.EditorPythonScripting.set_keep_python_script_alive(False)
        quit_handle=u.register_slate_post_tick_callback(quit_when_idle)
    executor.on_executor_errored_delegate.add_callable_unique(error)
    executor.on_executor_finished_delegate.add_callable_unique(finished)
    subsystem.render_queue_with_executor_instance(executor)
except Exception:
    (ROOT/'unreal-render-error.txt').write_text(traceback.format_exc())
    u.EditorPythonScripting.set_keep_python_script_alive(False)
    raise
