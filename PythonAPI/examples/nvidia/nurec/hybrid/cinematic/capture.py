"""Capture 48 authoritative CARLA samples; no RGB sensor used for final pixels.

Use a dedicated CARLA server: this command replaces its world with the source map.
"""
import argparse,json,sys,zipfile,hashlib,socket
from pathlib import Path
import numpy as np
from scipy.spatial.transform import Rotation
import grpc,carla,msgpack
from core import optical_pose,transform,validate_manifest
sys.path.insert(0,str(Path(__file__).resolve().parents[2]))
from nre.grpc.protos import sensorsim_pb2 as s,sensorsim_pb2_grpc as g,common_pb2 as c
from projection_functions import get_t_rig_enu_from_ecef
from utils import mat_to_carla_transform


def matrix(p):
    t=np.eye(4);t[:3,:3]=Rotation.from_quat([p.quat.x,p.quat.y,p.quat.z,p.quat.w]).as_matrix();t[:3,3]=[p.vec.x,p.vec.y,p.vec.z];return t


def tf_data(t):return {'location':[t.location.x,t.location.y,t.location.z],'rotation':[t.rotation.pitch,t.rotation.yaw,t.rotation.roll]}


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--usdz',type=Path,required=True);ap.add_argument('--out',type=Path,required=True)
    ap.add_argument('--port',type=int,default=4696);ap.add_argument('--nre-port',type=int,default=46439)
    args=ap.parse_args();out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
    if (out/'manifest.json').exists():raise SystemExit('Capture exists; use a new shot directory')
    stub=g.SensorsimServiceStub(grpc.insecure_channel(f'localhost:{args.nre_port}',options=[('grpc.max_receive_message_length',256<<20)]))
    sid=next(x for x in stub.get_available_scenes(c.Empty(),timeout=60).scene_ids if args.usdz.stem in x)
    poses=stub.get_available_trajectories(s.AvailableTrajectoriesRequest(scene_id=sid),timeout=600).available_trajectories[0].trajectory.poses
    cameras=stub.get_available_cameras(s.AvailableCamerasRequest(scene_id=sid),timeout=600).available_cameras
    camera=next(x for x in cameras if x.logical_id=='camera_front_wide_120fov')
    p=poses[len(poses)//3]
    with zipfile.ZipFile(args.usdz) as z:
        xodr=z.read('map.xodr').decode();rig=json.loads(z.read('rig_trajectories.json'))
    scene_to_enu=get_t_rig_enu_from_ecef(np.array(rig['T_world_base']),xodr)
    mirror=np.diag([1,-1,1,1]);scene_to_carla=mirror@scene_to_enu
    optical_to_flu=np.eye(4);optical_to_flu[:3,:3]=[[0,0,1],[-1,0,0],[0,-1,0]]
    camera_scene=matrix(p.pose)@matrix(camera.rig_to_camera)
    camera_tf=mat_to_carla_transform(scene_to_enu@camera_scene@np.linalg.inv(optical_to_flu))
    # Static calibrated camera eliminates novel-view travel while the character crosses.
    client=carla.Client('localhost',args.port);client.set_timeout(120)
    with socket.create_connection(('localhost',args.port),timeout=20) as sock:
        sock.sendall(msgpack.packb([0,1,'console_command',[[False],'carla.OpenDrive.StreetFurniture 0']],use_bin_type=True));sock.recv(1<<16)
    world=client.generate_opendrive_world(xodr,carla.OpendriveGenerationParameters(vertex_distance=2,max_road_length=500,wall_height=0,additional_width=.6,smooth_junctions=True,enable_mesh_visibility=True))
    old=world.get_settings();settings=world.get_settings();settings.synchronous_mode=True;settings.fixed_delta_seconds=1/24;settings.substepping=True;settings.max_substep_delta_time=.01;settings.max_substeps=10
    actor=None
    try:
        world.apply_settings(settings)
        fwd=camera_tf.get_forward_vector();right=camera_tf.get_right_vector()
        loc=camera_tf.location+fwd*9-right*1.4
        wp=world.get_map().get_waypoint(loc,project_to_road=True,lane_type=carla.LaneType.Any)
        loc.z=wp.transform.location.z+1.0
        bp=world.get_blueprint_library().find('walker.pedestrian.german');bp.set_attribute('role_name','cinematic_mvp')
        actor=world.spawn_actor(bp,carla.Transform(loc,carla.Rotation(yaw=camera_tf.rotation.yaw+90)))
        for _ in range(36):world.tick()
        direction=carla.Vector3D(right.x,right.y,0);actor.apply_control(carla.WalkerControl(direction,1.4,False))
        samples=[]
        for i in range(48):
            frame=world.tick();snap=world.get_snapshot();a=actor.get_transform()
            bones=[{'name':b.name,'relative':tf_data(b.relative)} for b in actor.get_bones().bone_transforms]
            samples.append({'frame':1001+i,'simulation_frame':frame,'simulation_time_s':snap.timestamp.elapsed_seconds,
                'timestamp_us':int(p.timestamp_us)+round(i*1e6/24),'camera':tf_data(camera_tf),
                'camera_to_world_optical':optical_pose(**tf_data(camera_tf)).tolist(),
                'actors':[{'id':'hero','blueprint':actor.type_id,'transform':tf_data(a),'bones':bones}],
                'traffic_lights':[{'id':x.id,'state':str(x.get_state())} for x in world.get_actors().filter('traffic.traffic_light')]})
        w,h=960,540;fx=w/(2*np.tan(np.radians(75)/2))
        manifest={'schema':'hybrid.cinematic.v1','shot':'sh010','fps':24,'frame_start':1001,'frame_end':1048,
            'resolution':[w,h],'pixel_aspect':1,'shutter_angle':0,'shutter_note':'Sharp-sample MVP; shared temporal integration deferred',
            'camera_model':'pinhole','distortion_applied_during_render':False,'world_units':'meters','world_axes':'CARLA left-handed X-forward Y-right Z-up',
            'camera_axes':'OpenCV X-right Y-down Z-forward','matrix_direction':'camera_to_world','image_y':'down','pixel_centres':'integer',
            'K':[[fx,0,w/2],[0,fx,h/2],[0,0,1]],'scene_to_carla':scene_to_carla.tolist(),
            'scene_id':sid,'source_usdz':str(args.usdz.resolve()),'camera_logical_id':camera.logical_id,
            'color_working_space':'ACEScg','nurec_color_space':'unverified_native','nurec_depth':'native_ray_distance_weighting_unverified',
            'ue_color_space':'linear_srgb','ue_alpha':'premultiplied','near_m':.1,'far_m':1000,
            'hero_asset':'/Game/Carla/Characters/MetaHumans/German/CinematicReview/German/BP_German',
            'proxy_road_height_m':wp.transform.location.z,'capture_animation':'CARLA per-frame relative bone transforms',
            'gates':{k:'pending' for k in ['camera_render','deterministic_replay','character','integration','temporal','color']}}
        validate_manifest(manifest)
        (out/'capture.json').write_text(json.dumps(samples));(out/'map.xodr').write_text(xodr)
        manifest['capture_sha256']=hashlib.sha256((out/'capture.json').read_bytes()).hexdigest()
        (out/'manifest.json').write_text(json.dumps(manifest,indent=2))
        print(json.dumps({'shot':str(out),'samples':len(samples),'bones':len(bones),'start':samples[0]['actors'][0]['transform'],'end':samples[-1]['actors'][0]['transform']},indent=2))
    finally:
        if actor:actor.destroy()
        world.apply_settings(old)

if __name__=='__main__':main()
