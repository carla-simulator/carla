"""Derive a synchronized native-lens camera rig from an approved cinematic shot."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import numpy as np
from scipy.spatial.transform import Rotation
from core import OPTICAL_TO_UE, optical_pose, validate_manifest

CAMERAS={0:'camera_cross_left_120fov',1:'camera_front_wide_120fov',
         2:'camera_cross_right_120fov',3:'camera_rear_left_70fov',
         5:'camera_rear_right_70fov',6:'camera_front_tele_30fov'}


def pose_matrix(p):
    t=np.eye(4);q=p['quat'];v=p['vec']
    t[:3,:3]=Rotation.from_quat([q.get(k,0.) for k in ('x','y','z','w')]).as_matrix()
    t[:3,3]=[v.get(k,0.) for k in ('x','y','z')]
    return t


def camera_pose(t):
    r=t[:3,:3]@OPTICAL_TO_UE.T
    angles=np.degrees([np.arcsin(np.clip(r[2,0],-1,1)),np.arctan2(r[1,0],r[0,0]),np.arctan2(-r[2,1],r[2,2])])
    p={'location':t[:3,3].tolist(),'rotation':angles.tolist()}
    if not np.allclose(optical_pose(**p),t,atol=1e-8):raise ValueError('Camera Euler conversion failed')
    return p


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('source',type=Path);ap.add_argument('output',type=Path)
    ap.add_argument('--width',type=int,default=960);a=ap.parse_args();out=a.output.resolve();source=a.source.resolve()
    calibrations=json.loads((out/'source-cameras.json').read_text())['available_cameras']
    cams={c['logical_id']:c for c in calibrations};m=json.loads((source/'manifest.json').read_text())
    original=json.loads((source/'capture.json').read_text());front=pose_matrix(cams[m['camera_logical_id']]['rig_to_camera'])
    records=[]
    for index,name in CAMERAS.items():
        calibration=cams[name];short=name.removeprefix('camera_');view=out/short;view.mkdir(exist_ok=True)
        if (view/'manifest.json').exists():raise ValueError('View already exists: '+str(view))
        delta=np.linalg.inv(front)@pose_matrix(calibration['rig_to_camera'])
        frames=copy.deepcopy(original)
        for frame in frames:
            t=np.asarray(frame['camera_to_world_optical'])@delta
            frame['camera_to_world_optical']=t.tolist();frame['camera']=camera_pose(t)
        (view/'capture.json').write_text(json.dumps(frames))
        intr=calibration['intrinsics'];scale=a.width/intr['resolution_w'];height=round(intr['resolution_h']*scale)
        if not np.isclose(height/intr['resolution_h'],scale):raise ValueError('Aspect ratio must be preserved')
        f=copy.deepcopy(intr['ftheta_param']);cde=f.pop('linear_cde',{})
        f['linear_cde']=[cde.get('linear_c',1),cde.get('linear_d',0),cde.get('linear_e',0)]
        f['principal_point_x']*=scale;f['principal_point_y']*=scale
        f['pixeldist_to_angle_poly']=[v/scale**i for i,v in enumerate(f['pixeldist_to_angle_poly'])]
        f['angle_to_pixeldist_poly']=[v*scale for v in f['angle_to_pixeldist_poly']]
        vm=copy.deepcopy(m);fx=1/f['pixeldist_to_angle_poly'][1]
        vm.update(shot='sh040_'+short,resolution=[a.width,height],camera_logical_id=name,camera_index=index,
                  camera_model='ftheta',ftheta=f,distortion_applied_during_render=True,pixel_centres='half_integer',
                  K=[[fx,0,a.width/2],[0,fx,height/2],[0,0,1]],
                  K_note='Nominal pinhole fallback only; native F-theta is authoritative for rays and depth',
                  camera_motion_source='Fixed recorded rig extrinsics composed with approved front-camera trajectory',
                  source_camera_calibration=calibration,world_template='/Game/HybridCinematicMVP/sh031_contactfix/World',
                  allow_empty_foreground=True,revision_of=m['shot'],
                  capture_sha256=hashlib.sha256((view/'capture.json').read_bytes()).hexdigest(),
                  render_scope='Native F-theta six-camera demo; global-shutter sharp samples, not a rolling-shutter sensor replica',
                  shutter_type='GLOBAL',source_shutter_type=intr['shutter_type'],
                  compatibility={'profile':'Alpamayo2-Super trajectory IDs 0,1,2,3,5,6',
                                 'extrinsics':'source log exact','lens':'source polynomial scaled to output',
                                 'cadence':'24fps demo; 10Hz model context export not yet provided',
                                 'rolling_shutter':'not implemented','sensor_noise':'not implemented'})
        validate_manifest(vm);(view/'manifest.json').write_text(json.dumps(vm,indent=2))
        for name_to_link in ('capture','lighting.json','environment.hdr','proxy-fit.json','receiver.fbx','sole-contact.json','contact-surfaces.json'):
            (view/name_to_link).symlink_to((source/name_to_link).resolve())
        # Rig rigidity, clocks, actors and native animation must remain exact.
        for f0,f1 in zip(original,frames):
            assert f0['actors']==f1['actors'] and f0['mary_source_frame']==f1['mary_source_frame']
            assert all(f0[k]==f1[k] for k in ('frame','simulation_time_s','simulation_frame','timestamp_us'))
            np.testing.assert_allclose(np.linalg.inv(f0['camera_to_world_optical'])@np.asarray(f1['camera_to_world_optical']),delta,atol=1e-8)
        records.append({'camera_index':index,'name':name,'view':short,'resolution':vm['resolution'],'frames':len(frames)})
    (out/'rig.json').write_text(json.dumps({'source_shot':str(source),'views':records,'fps':m['fps'],
        'frame_start':m['frame_start'],'frame_end':m['frame_end'],
        'validation':{'rig_rigidity':True,'actor_capture_unchanged':True,'synchronized_clocks':True},
        'profile_source':'https://github.com/NVlabs/alpamayo2/blob/main/src/alpamayo2_super/input_profiles.py',
        'missing_camera':'camera_rear_tele_30fov absent from this log; not required for trajectory profile'},indent=2))


if __name__=='__main__':main()
