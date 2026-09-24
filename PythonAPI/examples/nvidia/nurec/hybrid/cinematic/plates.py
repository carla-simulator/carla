"""Render unclamped native NuRec floats to EXR, preserving semantic uncertainty."""
import argparse,json,sys
from pathlib import Path
import numpy as np
import grpc
from scipy.spatial.transform import Rotation
from core import decode_float_payload,write_exr,validate_manifest,ray_to_z,camera_rays
sys.path.insert(0,str(Path(__file__).resolve().parents[2]))
from nre.grpc.protos import sensorsim_pb2 as s,sensorsim_pb2_grpc as g,common_pb2 as c


def pose(t):
    q=Rotation.from_matrix(t[:3,:3]).as_quat()
    return c.Pose(vec=c.Vec3(x=t[0,3],y=t[1,3],z=t[2,3]),quat=c.Quat(x=q[0],y=q[1],z=q[2],w=q[3]))


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('shot',type=Path);ap.add_argument('--port',type=int,default=46439);ap.add_argument('--limit',type=int,default=48)
    a=ap.parse_args();m=validate_manifest(json.loads((a.shot/'manifest.json').read_text()));frames=json.loads((a.shot/'capture.json').read_text())
    stub=g.SensorsimServiceStub(grpc.insecure_channel(f'localhost:{a.port}',options=[('grpc.max_receive_message_length',256<<20)]))
    w,h=m['resolution'];K=np.array(m['K']);inv=np.linalg.inv(m['scene_to_carla'])
    spec=s.CameraSpec(logical_id=m['camera_logical_id'],resolution_w=w,resolution_h=h,shutter_type=s.GLOBAL,
        opencv_pinhole_param=s.OpenCVPinholeCameraParam(principal_point_x=K[0,2],principal_point_y=K[1,2],focal_length_x=K[0,0],focal_length_y=K[1,1],radial_coeffs=[0]*6,tangential_coeffs=[0]*2,thin_prism_coeffs=[0]*4))
    if m['camera_model']=='ftheta':
        from google.protobuf.json_format import ParseDict
        spec=s.CameraSpec(logical_id=m['camera_logical_id'],resolution_w=w,resolution_h=h,shutter_type=s.GLOBAL)
        ft=dict(m['ftheta']);ft.pop('linear_cde',None)
        ParseDict(ft,spec.ftheta_param)
        spec.ftheta_param.linear_cde.linear_c=1
    rays=camera_rays(m)
    objects=stub.get_dynamic_objects(s.AvailableDynamicObjectsRequest(scene_id=m['scene_id']),timeout=120).dynamic_objects
    hidden=[]
    for obj in objects:
        if not obj.trajectory.poses:continue
        p=c.Pose();p.CopyFrom(obj.trajectory.poses[0].pose);p.vec.z-=200
        hidden.append(s.DynamicObject(track_id=obj.id,pose_pair=s.PosePair(start_pose=p,end_pose=p)))
    report=[]
    for f in frames[:a.limit]:
        p=pose(inv@np.asarray(f['camera_to_world_optical']))
        req=s.RGBRenderRequest(scene_id=m['scene_id'],resolution_h=h,resolution_w=w,camera_intrinsics=spec,
            frame_start_us=f['timestamp_us'],frame_end_us=f['timestamp_us']+1,sensor_pose=s.PosePair(start_pose=p,end_pose=p),dynamic_objects=hidden,image_format=s.RGB_UINT8_PLANAR,image_quality=-8)
        rgb,distance,alpha=decode_float_payload(stub.render_rgb(req,timeout=600).image_bytes)
        if rgb.shape!=(h,w,3):raise ValueError('Resolution mismatch')
        metadata={'color_space':m['nurec_color_space'],'depth':m['nurec_depth'],'timestamp_us':f['timestamp_us'],'frame':f['frame'],'backend':'NuRec 26.04.01','transport':'HYF1','color_clamped_by_exporter':False}
        write_exr(a.shot/f"renders/gs_beauty/{f['frame']}.exr",dict(zip('RGB',rgb.transpose(2,0,1)))|{'A':alpha},half=True,metadata=metadata)
        # This is ray-distance converted to optical Z, NOT relabelled expected depth.
        write_exr(a.shot/f"renders/gs_depth/{f['frame']}.exr",{'Z':distance*rays[...,2],'distance':distance},metadata=metadata)
        report.append({'frame':f['frame'],'rgb_min':float(rgb.min()),'rgb_max':float(rgb.max()),'opacity_min':float(alpha.min()),'opacity_max':float(alpha.max())})
        print(f"NuRec frame {f['frame']} {report[-1]}",flush=True)
    (a.shot/'nurec-render-report.json').write_text(json.dumps(report,indent=2))

if __name__=='__main__':main()
