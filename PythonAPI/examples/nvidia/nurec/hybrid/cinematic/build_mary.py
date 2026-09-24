"""Create an isolated Alembic/texture-sequence comparison shot in Unreal."""
import unreal as u
import os,json,math,traceback,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
ROOT=Path(os.environ['HYBRID_SHOT']);M=json.loads((ROOT/'manifest.json').read_text())
F=json.loads((ROOT/'capture.json').read_text());BASE='/Game/HybridCinematicMVP/'+M['shot']
SRC='/Game/HybridCinematicMVP/sh010';FILES=Path(M['source_asset'])
TOOLS=u.AssetToolsHelpers.get_asset_tools()
START=M['frame_start'];END=M['frame_end']+1;COUNT=END-START
CROSSING=M.get('scenic_crossing',False)
CONTACT={r['frame']:r['actors'] for r in json.loads((ROOT/'contact-surfaces.json').read_text())['frames']} if (ROOT/'contact-surfaces.json').exists() else {}
SOLE={r['frame']:r for r in json.loads((ROOT/'sole-contact.json').read_text())['frames']} if (ROOT/'sole-contact.json').exists() else {}
MARY_BASE=M.get('mary_asset_base',BASE)

def asset(name,cls,factory):
    return u.load_asset(BASE+'/'+name) if u.EditorAssetLibrary.does_asset_exist(BASE+'/'+name) else TOOLS.create_asset(name,BASE,cls,factory)

def animate(seq,actor,samples):
    binding=seq.add_possessable(actor);section=binding.add_track(u.MovieScene3DTransformTrack).add_section();section.set_range(START-32,END)
    for frame,location,yaw in samples:
        for channel,value in zip(section.get_all_channels(),[*location,0,0,yaw,1,1,1]):
            channel.add_key(u.FrameNumber(frame),float(value),interpolation=u.MovieSceneKeyInterpolation.LINEAR)
    return binding

def main():
    levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
    if u.EditorAssetLibrary.does_asset_exist(BASE+'/World'):levels.load_level(BASE+'/World')
    elif not levels.new_level_from_template(BASE+'/World',M.get('world_template',SRC+'/World')):raise RuntimeError('Cannot create Mary world')
    actors=u.get_editor_subsystem(u.EditorActorSubsystem)
    for actor in actors.get_all_level_actors():
        if actor.get_actor_label().startswith('Hero'):actors.destroy_actor(actor)
    cache_name=M.get('mary_cache_name','MaryPerformance')
    cache_path=M.get('mary_cache_base',MARY_BASE)+'/'+cache_name
    if not u.EditorAssetLibrary.does_asset_exist(cache_path):
        export_end=M.get('mary_export_end',61)
        task=u.AssetImportTask();task.filename=str(ROOT/f'capture/mary/mary_0001_{export_end:04d}_cm.abc');task.destination_path=BASE;task.destination_name=cache_name;task.automated=True;task.save=True
        options=u.AbcImportSettings();options.import_type=u.AlembicImportType.GEOMETRY_CACHE
        sampling=options.sampling_settings;sampling.frame_start=1;sampling.frame_end=export_end;sampling.skip_empty=False;options.sampling_settings=sampling
        geo=options.geometry_cache_settings;geo.flatten_tracks=True;geo.compressed_position_precision=.001;geo.compressed_texture_coordinates_number_of_bits=16;options.geometry_cache_settings=geo
        task.options=options;TOOLS.import_asset_tasks([task])
        (ROOT/'alembic-import-paths.json').write_text(json.dumps(list(task.imported_object_paths)))
    cache=u.load_asset(cache_path)
    if not isinstance(cache,u.GeometryCache):raise RuntimeError('Alembic import did not produce GeometryCache')
    mat=asset('MaryMaterial',u.Material,u.MaterialFactoryNew());u.MaterialEditingLibrary.delete_all_material_expressions(mat)
    tex=u.MaterialEditingLibrary.create_material_expression(mat,u.MaterialExpressionTextureSampleParameter2D);tex.set_editor_property('parameter_name','FrameTexture')
    # GeometryCacheSceneProxy's non-interpolated render path rounds its
    # interpolation factor; the track's flooring lookup is NOT the final
    # rendered sample. UV-changing caches require nearest-sample textures.
    source_frames=sorted({f.get('mary_source_frame',1+(i*30+12)//24) for i,f in enumerate(F)})
    textures={};instances={};tasks=[]
    for n in source_frames:
        name=f'MaryTex_{n:04d}';path=MARY_BASE+'/'+name
        if not u.EditorAssetLibrary.does_asset_exist(path):
            task=u.AssetImportTask();task.filename=str(FILES/f'tex_4k/rp_mary_4d_006_walkingCalling_dif_4k.{n:04d}.jpg');task.destination_path=BASE;task.destination_name=name;task.automated=True;task.save=True;tasks.append(task)
    if tasks:TOOLS.import_asset_tasks(tasks)
    for n in source_frames:
        texture=u.load_asset(MARY_BASE+f'/MaryTex_{n:04d}')
        if not texture:raise RuntimeError('Missing texture frame '+str(n))
        texture.set_editor_property('srgb',True);textures[n]=texture
    tex.set_editor_property('texture',textures[1])
    assert u.MaterialEditingLibrary.connect_material_property(tex,'RGB',u.MaterialProperty.MP_BASE_COLOR)
    for value,property in [(.95,u.MaterialProperty.MP_ROUGHNESS),(.1,u.MaterialProperty.MP_SPECULAR)]:
        c=u.MaterialEditingLibrary.create_material_expression(mat,u.MaterialExpressionConstant);c.set_editor_property('r',value);u.MaterialEditingLibrary.connect_material_property(c,'',property)
    u.MaterialEditingLibrary.set_material_usage(mat,u.MaterialUsage.MATUSAGE_GEOMETRY_CACHE)
    u.MaterialEditingLibrary.recompile_material(mat);u.EditorAssetLibrary.save_loaded_asset(mat)
    for n in source_frames:
        mi=asset(f'MaryFrame_{n:04d}',u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
        u.MaterialEditingLibrary.set_material_instance_parent(mi,mat);u.MaterialEditingLibrary.set_material_instance_texture_parameter_value(mi,'FrameTexture',textures[n]);u.EditorAssetLibrary.save_loaded_asset(mi);instances[n]=mi
    hero=actors.spawn_actor_from_class(u.GeometryCacheActor,u.Vector(),u.Rotator());hero.set_actor_label('HeroMary');component=hero.get_geometry_cache_component();component.set_geometry_cache(cache);component.set_material(0,instances[1]);component.set_looping(False);component.set_interpolate_frames(False);component.set_extrapolate_frames(False);component.set_editor_property('manual_tick',True);component.play()
    bounds=[]
    for t in [0.,1.,2.]:
        component.tick_at_this_time(t,True,False,False);origin,extent=hero.get_actor_bounds(False)
        bounds.append({'time':t,'origin':[origin.x,origin.y,origin.z],'extent':[extent.x,extent.y,extent.z]})
    (ROOT/'cache-bounds.json').write_text(json.dumps(bounds,indent=2))
    (ROOT/'cache-info.txt').write_text('duration_seconds='+str(component.get_duration()))
    if not 140<bounds[0]['extent'][2]*2<210:raise RuntimeError('Unexpected Alembic scale/orientation; inspect cache-bounds.json')
    d=[bounds[1]['origin'][i]-bounds[0]['origin'][i] for i in (0,1)]
    if math.hypot(*d)<10:raise RuntimeError('Geometry cache did not advance during sampling')
    target_yaw=F[0]['actors'][0]['transform']['rotation'][1];yaw=target_yaw-math.degrees(math.atan2(d[1],d[0]));theta=math.radians(yaw)
    xy=F[0]['actors'][0]['transform']['location'][:2];fit=json.loads((ROOT/'proxy-fit.json').read_text())
    samples=[];mapping=[]
    blender_samples={r['source_frame']:r for r in json.loads((ROOT/'capture/mary/blender-samples.json').read_text())}
    for i,f in enumerate(F):
        n=f.get('mary_source_frame',1+(i*30+12)//24);component.tick_at_this_time((n-1)/30+1e-5,True,False,False);origin,extent=hero.get_actor_bounds(False)
        if CROSSING:
            # Remove the baked Blender root displacement and replace it with
            # Scenic's authoritative pedestrian root; preserve local gait.
            root=blender_samples[n]['matrix_world'];rx=root[0][3];ry=-root[1][3]
            target=f['actors'][0]['transform']['location']
            xy=[target[0]-(rx*math.cos(theta)-ry*math.sin(theta)),target[1]-(rx*math.sin(theta)+ry*math.cos(theta))]
        dx=(origin.x*math.cos(theta)-origin.y*math.sin(theta))/100;dy=(origin.x*math.sin(theta)+origin.y*math.cos(theta))/100
        road=fit['height_m']+fit['slope_x']*(xy[0]+dx-fit['origin_xy_m'][0])+fit['slope_y']*(xy[1]+dy-fit['origin_xy_m'][1])
        if f['frame'] in CONTACT: road=CONTACT[f['frame']]['mary_proxy']['height_m']
        z=road*100-(origin.z-extent.z)+.3
        if f['frame'] in SOLE:
            # Current evaluated sole vertices, rather than the cache's bounds.
            z=SOLE[f['frame']]['root_height_m']*100
        samples.append((f['frame'],[xy[0]*100,xy[1]*100,z],yaw));mapping.append({'frame':f['frame'],'source_frame':n,'source_time_s':(n-1)/30,'texture':instances[n].get_path_name()})
    sequence=asset('Shot',u.LevelSequence,u.LevelSequenceFactoryNew());sequence.set_display_rate(u.FrameRate(24,1));sequence.set_tick_resolution_directly(u.FrameRate(24000,1));sequence.set_playback_start(START);sequence.set_playback_end(END)
    for b in sequence.get_bindings():b.remove()
    for track in sequence.get_tracks():sequence.remove_track(track)
    camera=next(a for a in actors.get_all_level_actors() if isinstance(a,u.CineCameraActor))
    from cinematic_camera import configure_camera
    configure_camera(camera,M,F,ROOT)
    cb=sequence.add_possessable(camera)
    if CROSSING:
        from crossing_unreal import animate_camera, spawn_traffic
        animate_camera(cb,F,START,END)
        spawn_traffic(actors,sequence,F,BASE,fit,START,END)
        if (ROOT/'receiver.fbx').exists():
            road_actor=next(a for a in actors.get_all_level_actors() if a.get_actor_label()=='ProxyRoad')
            material=road_actor.static_mesh_component.get_material(0)
            task=u.AssetImportTask();task.filename=str(ROOT/'receiver.fbx');task.destination_path=BASE;task.destination_name='ContactReceiver';task.automated=True;task.replace_existing=True;task.save=True
            options=u.FbxImportUI();options.import_mesh=True;options.import_materials=False;options.import_textures=False;options.import_as_skeletal=False;options.mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH
            options.static_mesh_import_data.set_editor_property('combine_meshes',True)
            task.options=options;TOOLS.import_asset_tasks([task]);terrain=u.load_asset(BASE+'/ContactReceiver')
            if not isinstance(terrain,u.StaticMesh):raise RuntimeError('Contact receiver import failed')
            road_actor.static_mesh_component.set_static_mesh(terrain);road_actor.static_mesh_component.set_material(0,material)
            road_actor.static_mesh_component.set_relative_scale3d(u.Vector(1,1,1));road_actor.set_actor_scale3d(u.Vector(1,1,1));road_actor.set_actor_rotation(u.Rotator(),False)
            road_actor.set_actor_location(u.Vector(fit['origin_xy_m'][0]*100,fit['origin_xy_m'][1]*100,fit['height_m']*100),False,False)
            bounds=terrain.get_bounding_box();(ROOT/'receiver-import.json').write_text(json.dumps({'min_cm':[bounds.min.x,bounds.min.y,bounds.min.z],'max_cm':[bounds.max.x,bounds.max.y,bounds.max.z]},indent=2))
        elif CONTACT:
            # Keep the shared receiver just below the lowest local contact
            # surface at each shot time, rather than a fixed worst-case offset.
            road_actor=next(a for a in actors.get_all_level_actors() if a.get_actor_label()=='ProxyRoad')
            road_binding=sequence.add_possessable(road_actor)
            road_section=road_binding.add_track(u.MovieScene3DTransformTrack).add_section();road_section.set_range(START-32,END)
            receiver=[]
            pitch=math.degrees(math.atan(fit['slope_x']));roll=math.degrees(math.atan(-fit['slope_y']/math.sqrt(1+fit['slope_x']**2)))
            for f in F:
                offsets=[]
                for a in f['actors']:
                    x,y=a['transform']['location'][:2]
                    plane=fit['height_m']+fit['slope_x']*(x-fit['origin_xy_m'][0])+fit['slope_y']*(y-fit['origin_xy_m'][1])
                    offsets.append(CONTACT[f['frame']][a['id']]['height_m']-plane)
                offset=min(offsets)-.003
                values=[fit['origin_xy_m'][0]*100,fit['origin_xy_m'][1]*100,(fit['height_m']+offset)*100,roll,pitch,0,30,30,1]
                for ch,value in zip(road_section.get_all_channels(),values):ch.add_key(u.FrameNumber(f['frame']),float(value),interpolation=u.MovieSceneKeyInterpolation.LINEAR)
                receiver.append({'frame':f['frame'],'offset_m':offset,'contact_offsets_m':offsets})
            (ROOT/'receiver-frame-map.json').write_text(json.dumps(receiver,indent=2))
    cut=sequence.add_track(u.MovieSceneCameraCutTrack).add_section();cut.set_range(START,END);bid=u.MovieSceneObjectBindingID();bid.set_editor_property('guid',cb.get_id());cut.set_camera_binding_id(bid)
    hb=animate(sequence,hero,samples);gb=sequence.add_possessable(component);gb.set_parent(hb)
    cache_track=gb.add_track(u.MovieSceneGeometryCacheTrack)
    if CROSSING:
        # One exact sample per sharp delivery frame supports waiting and
        # stopped poses without looping the walking performance.
        for row in mapping:
            section=cache_track.add_section();section.set_range(row['frame'],row['frame']+1)
            params=section.params;params.geometry_cache_asset=cache;params.play_rate=1.
            params.first_loop_start_frame_offset=u.FrameNumber((row['source_frame']-1)*800+1);section.params=params
    else:
        section=cache_track.add_section();section.set_range(START,END);params=section.params;params.geometry_cache_asset=cache;params.play_rate=1.;params.first_loop_start_frame_offset=u.FrameNumber(1);section.params=params
    mt=gb.add_track(u.MovieScenePrimitiveMaterialTrack);info=u.ComponentMaterialInfo();info.material_slot_index=0;info.material_type=u.ComponentMaterialType.INDEXED_MATERIAL;mt.set_material_info(info)
    ms=mt.add_section();ms.set_range(START-32,END);ch=ms.get_all_channels()[0];ch.add_key(u.FrameNumber(START-32),instances[1])
    for row in mapping:ch.add_key(u.FrameNumber(row['frame']),instances[row['source_frame']])
    for name in ['RenderGraph','DepthMetres']:
        if not u.EditorAssetLibrary.does_asset_exist(BASE+'/'+name):u.EditorAssetLibrary.duplicate_asset(SRC+'/'+name,BASE+'/'+name)
        if not u.EditorAssetLibrary.save_asset(BASE+'/'+name):raise RuntimeError('Cannot save '+name)
    graph=u.load_asset(BASE+'/RenderGraph');node=graph.get_output_node().get_input_pin('Globals').get_connected_nodes()[0]
    while not isinstance(node,u.MovieGraphGlobalOutputSettingNode):node=node.get_input_pin('').get_connected_nodes()[0]
    node.override_output_directory=True;node.output_directory=u.DirectoryPath(str(ROOT/'renders/ue_review'))
    # Duplicating a graph preserves external material references; point utility
    # passes at this shot's material so render-time compilation checks it too.
    node=graph.get_output_node().get_input_pin('integration').get_connected_nodes()[0]
    while not isinstance(node,u.MovieGraphPathTracerRenderPassNode):node=node.get_input_pin('').get_connected_nodes()[0]
    passes=list(node.additional_post_process_materials)
    for p in passes:
        if p.name=='depth_m':p.material=u.load_asset(BASE+'/DepthMetres')
    node.additional_post_process_materials=passes
    for obj in [sequence,graph,cache]:u.EditorAssetLibrary.save_loaded_asset(obj,only_if_is_dirty=False)
    levels.save_current_level()
    (ROOT/'mary-frame-map.json').write_text(json.dumps(mapping,indent=2));(ROOT/'unreal-build.json').write_text(json.dumps({'status':'built','asset':'4D Mary','native_fps':30,'bounds_displacement_first_second_m':math.hypot(*d)/100,'yaw':yaw,'source_frames':source_frames,'body_mesh_offset_cm':[0,0,0]},indent=2))

try:main()
except Exception:
    (ROOT/'unreal-error.txt').write_text(traceback.format_exc());raise
