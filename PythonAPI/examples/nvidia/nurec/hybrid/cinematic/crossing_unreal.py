"""Unreal helpers for the Scenic traffic crossing cinematic derivative."""
import json
import math
import os
from pathlib import Path
import unreal as u


def animate_camera(binding, frames, start, end):
    section = binding.add_track(u.MovieScene3DTransformTrack).add_section()
    section.set_range(start-32, end)
    for f in frames:
        p = f['camera']; pitch, yaw, roll = p['rotation']
        values = [v*100 for v in p['location']] + [roll, pitch, yaw, 1, 1, 1]
        for channel, value in zip(section.get_all_channels(), values):
            channel.add_key(u.FrameNumber(f['frame']), float(value), interpolation=u.MovieSceneKeyInterpolation.LINEAR)


def animate_wheels(actor, binding, sequence, frames, role, base, start, end):
    movement=actor.get_component_by_class(u.ChaosWheeledVehicleMovementComponent)
    if movement is None: raise RuntimeError('Vehicle has no wheel definitions: '+role)
    mesh=actor.get_component_by_class(u.SkeletalMeshComponent)
    setups=list(movement.get_editor_property('wheel_setups'))
    factory=u.AnimSequenceFactory();factory.target_skeleton=mesh.skeletal_mesh_asset.skeleton
    name='RecordedWheels_'+role
    anim=u.load_asset(base+'/'+name) if u.EditorAssetLibrary.does_asset_exist(base+'/'+name) else u.AssetToolsHelpers.get_asset_tools().create_asset(name,base,u.AnimSequence,factory)
    controller=anim.controller;controller.open_bracket('Bake wheel travel',False)
    controller.remove_all_bone_tracks(False);controller.set_frame_rate(u.FrameRate(120,1),False);controller.set_frame_rate(u.FrameRate(24,1),False);controller.set_number_of_frames(u.FrameNumber(len(frames)),False)
    poses=[next(a for a in f['actors'] if a['id']==role) for f in frames]
    distance=[0.]
    for a,b in zip(poses,poses[1:]):
        p=a['transform']['location'];q=b['transform']['location'];distance.append(distance[-1]+math.hypot(q[0]-p[0],q[1]-p[1])*100)
    report=[]
    for wheel in setups:
        bone=str(wheel.get_editor_property('bone_name'));default=u.get_default_object(wheel.get_editor_property('wheel_class'));radius=float(default.get_editor_property('wheel_radius'))
        reference=mesh.get_ref_pose_transform(mesh.get_bone_index(bone))
        angles=[-math.degrees(d/radius) for d in distance];angles.append(angles[-1])
        rotations=[reference.rotation*u.Rotator(pitch=angle).quaternion() for angle in angles]
        if not controller.add_bone_curve(bone,False):raise RuntimeError('Cannot animate wheel '+bone)
        controller.set_bone_track_keys(bone,[reference.translation]*len(angles),rotations,[reference.scale3d]*len(angles),False)
        report.append({'bone':bone,'radius_cm':radius,'total_rotation_degrees':angles[-1]})
    controller.close_bracket(False);u.EditorAssetLibrary.save_loaded_asset(anim)
    # The CARLA vehicle AnimBP can keep evaluating its physics wheel pose
    # (stationary with simulation disabled) over the recorded cinematic track.
    mesh.set_animation_mode(u.AnimationMode.ANIMATION_SINGLE_NODE)
    mesh.set_animation(anim)
    mesh.set_component_tick_enabled(True);mesh.set_editor_property('visibility_based_anim_tick_option',u.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
    child=sequence.add_possessable(mesh);child.set_parent(binding)
    section=child.add_track(u.MovieSceneSkeletalAnimationTrack).add_section();section.set_range(start,end)
    params=section.params;params.animation=anim;section.params=params
    return report


def spawn_traffic(actors, sequence, frames, base, fit, start, end):
    tools = u.AssetToolsHelpers.get_asset_tools()
    choices = {
        'lead': ('LincolnMKZ', (.28, .30, .33)),
        'queue': ('MiniCooper', (.75, .75, .72)),
        'standing': ('NissanPatrol', (.45, .06, .08)),
    }
    contact_path=Path(os.environ['HYBRID_SHOT'])/'contact-surfaces.json'
    contact={r['frame']:r['actors'] for r in json.loads(contact_path.read_text())['frames']} if contact_path.exists() else {}
    report = []
    for row in frames[0]['actors'][1:]:
        role = row['id']; model, color = choices[role]
        cls = u.EditorAssetLibrary.load_blueprint_class(f'/Game/Carla/Blueprints/Vehicles/{model}/BP_{model}')
        if cls is None:
            raise RuntimeError('Missing vehicle '+model)
        p = row['transform']; yaw = p['rotation'][1]
        actor = actors.spawn_actor_from_class(cls, u.Vector(*[x*100 for x in p['location']]), u.Rotator(yaw=yaw))
        actor.set_actor_label('HeroTraffic_'+role)
        actor.set_actor_tick_enabled(False)
        for comp in actor.get_components_by_class(u.ActorComponent):
            comp.set_component_tick_enabled(False)
            if isinstance(comp, u.PrimitiveComponent):
                comp.set_simulate_physics(False)
        paint = []
        for comp in actor.get_components_by_class(u.MeshComponent):
            for index, slot in enumerate(comp.get_material_slot_names()):
                if not str(slot).startswith('Bodywork_Mat'):
                    continue
                parent = comp.get_material(index)
                name = f'Paint_{role}_{comp.get_name()}_{index}'
                material = u.load_asset(base+'/'+name) if u.EditorAssetLibrary.does_asset_exist(base+'/'+name) else tools.create_asset(name,base,u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
                u.MaterialEditingLibrary.set_material_instance_parent(material,parent)
                # Scenic colors are display RGB; Unreal's parameter is linear.
                linear = [v/12.92 if v<=.04045 else ((v+.055)/1.055)**2.4 for v in color]
                u.MaterialEditingLibrary.set_material_instance_vector_parameter_value(material,'Base Color',u.LinearColor(*linear,1))
                u.EditorAssetLibrary.save_loaded_asset(material)
                comp.set_material(index, material)
                paint.append(str(slot))
        # Actor bounds include CARLA trigger volumes tens of metres tall.
        # Ground against the primary vehicle mesh, never those helper volumes.
        bounds=[]
        for comp in actor.get_components_by_class(u.SkeletalMeshComponent):
            origin,extent,radius=u.SystemLibrary.get_component_bounds(comp)
            if 100<max(extent.x,extent.y)<500 and 30<extent.z<200:
                bounds.append((extent.x*extent.y,origin,extent,comp.get_name()))
        if not bounds:raise RuntimeError('No plausible vehicle mesh bounds for '+role)
        _,origin,extent,mesh_name=max(bounds,key=lambda b:b[0])
        x,y,_ = p['location']
        road = fit['height_m']+fit['slope_x']*(x-fit['origin_xy_m'][0])+fit['slope_y']*(y-fit['origin_xy_m'][1])
        correction = road*100 - (origin.z-extent.z) + .3
        location = actor.get_actor_location(); location.z += correction
        actor.set_actor_location(location,False,False)
        binding = sequence.add_possessable(actor)
        section = binding.add_track(u.MovieScene3DTransformTrack).add_section(); section.set_range(start-32,end)
        # Key every captured vehicle pose: the accepted scenario has a moving
        # lead. XY/yaw remain simulation truth; only ground placement is derived.
        mesh_bottom_offset_cm = (origin.z-extent.z) - p['location'][2]*100
        samples=[]
        for captured in frames:
            pose = next(v['transform'] for v in captured['actors'] if v['id']==role)
            x,y,z = pose['location']; pitch,yaw,roll=pose['rotation']
            road_z = fit['height_m']+fit['slope_x']*(x-fit['origin_xy_m'][0])+fit['slope_y']*(y-fit['origin_xy_m'][1])
            if captured['frame'] in contact: road_z=contact[captured['frame']][role]['height_m']
            height = road_z*100-mesh_bottom_offset_cm+.3
            values=[x*100,y*100,height,roll,pitch,yaw,1,1,1]
            for ch,value in zip(section.get_all_channels(),values):
                ch.add_key(u.FrameNumber(captured['frame']),float(value),interpolation=u.MovieSceneKeyInterpolation.LINEAR)
            samples.append({'frame':captured['frame'],'location_cm':values[:3],'rotation_roll_pitch_yaw':values[3:6]})
        wheels=animate_wheels(actor,binding,sequence,frames,role,base,start,end)
        report.append({'role':role,'model':model,'paint_slots':paint,'height_correction_cm':correction,
                       'grounding_mesh':mesh_name,'bounds_extent_cm':[extent.x,extent.y,extent.z], 'road_height_m':road,'samples':samples,'motion':'per-frame captured XY and rotation; fitted road Z','wheels':wheels})
    (Path(os.environ['HYBRID_SHOT'])/'traffic-build.json').write_text(json.dumps(report,indent=2))
