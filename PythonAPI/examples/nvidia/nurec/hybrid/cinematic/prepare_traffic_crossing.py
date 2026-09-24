"""Derive a cinematic shot from validated Scenic actor telemetry.

Uses the original NuRec camera calibration and a stabilized ego-mounted camera.
CARLA XY/yaw trajectories remain authoritative; cinematic ground alignment is
a separate derivative. Original behavior telemetry is never overwritten.
"""
import argparse
import hashlib
import json
import shutil
import sys
from pathlib import Path

import grpc
import numpy as np
from scipy.spatial.transform import Rotation
from core import OPTICAL_TO_UE, transform
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from nre.grpc.protos import sensorsim_pb2 as s, sensorsim_pb2_grpc as g, common_pb2 as c


def matrix(p):
    t = np.eye(4)
    t[:3, :3] = Rotation.from_quat([p.quat.x, p.quat.y, p.quat.z, p.quat.w]).as_matrix()
    t[:3, 3] = [p.vec.x, p.vec.y, p.vec.z]
    return t


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('behavior', type=Path)
    ap.add_argument('template', type=Path)
    ap.add_argument('shot', type=Path)
    a = ap.parse_args()
    out = a.shot.resolve()
    if (out/'manifest.json').exists():
        raise SystemExit('Use a new shot directory; capture already exists')
    out.mkdir(parents=True, exist_ok=True)
    m = json.loads((a.template/'manifest.json').read_text())
    behavior = json.loads(a.behavior.read_text())
    rows = [{x['role']: x for x in f['actors']} for f in behavior['frames']]
    moving = [i for i, r in enumerate(rows) if r['mary_proxy']['speed'] > .2]
    start, end = moving[0], moving[-1]
    stub = g.SensorsimServiceStub(grpc.insecure_channel('localhost:46439'))
    poses = stub.get_available_trajectories(s.AvailableTrajectoriesRequest(scene_id=m['scene_id']), timeout=600).available_trajectories[0].trajectory.poses
    cam = next(x for x in stub.get_available_cameras(s.AvailableCamerasRequest(scene_id=m['scene_id']), timeout=600).available_cameras if x.logical_id == m['camera_logical_id'])
    scene_to_carla = np.array(m['scene_to_carla'])
    rigs = [scene_to_carla @ matrix(p.pose) for p in poses]
    camera0 = rigs[0] @ matrix(cam.rig_to_camera)
    ego0 = rows[0]['hero']
    frames = []
    source_frames = []
    for i, (f, r) in enumerate(zip(behavior['frames'], rows)):
        ego = r['hero']
        delta = transform([0, 0, 0], [0, ego['yaw']-ego0['yaw'], 0])
        camera = delta @ camera0
        camera[:3, 3] = camera0[:3, 3] + np.array([ego[k]-ego0[k] for k in ('x', 'y', 'z')])
        engine_rotation = camera[:3, :3] @ OPTICAL_TO_UE.T
        pitch = np.arcsin(np.clip(engine_rotation[2, 0], -1, 1))
        yaw = np.arctan2(engine_rotation[1, 0], engine_rotation[0, 0])
        roll = np.arctan2(-engine_rotation[2, 1], engine_rotation[2, 2])
        nearest = int(np.argmin([np.linalg.norm(camera[:2, 3]-p[:2, 3]) for p in rigs]))
        actors = []
        for role in ('mary_proxy', 'lead', 'queue', 'standing'):
            if role not in r: continue
            actor = r[role]
            actors.append({'id': role, 'blueprint': actor['blueprint'],
                           'transform': {'location': [actor[k] for k in ('x', 'y', 'z')],
                                         'rotation': [actor.get('pitch', 0), actor['yaw'], actor.get('roll', 0)]}})
        # Hold the first pose before crossing and last pose after it; during
        # crossing, map output time to the nearest 30 fps performance sample.
        n = 1 + (max(0, min(i, end)-start)*30+12)//24
        source_frames.append(n)
        frames.append({'frame': 1001+i, 'simulation_frame': f['frame'], 'simulation_time_s': f['t'],
                       'timestamp_us': int(poses[nearest].timestamp_us),
                       'camera': {'location': camera[:3, 3].tolist(), 'rotation': np.degrees([pitch, yaw, roll]).tolist()},
                       'camera_to_world_optical': camera.tolist(), 'actors': actors,
                       'mary_source_frame': n})
    m.update(shot=out.name, frame_end=1000+len(frames), mary_export_end=max(source_frames)+2,
             scenic_crossing=True, actor_motion_source='Validated Scenic/CARLA telemetry; Mary root follows recorded pedestrian XY/yaw',
             capture_animation='Mary native geometry cache, held before/after crossing; nearest source texture',
             camera_motion_source='Original NuRec camera at source start, translated with ego and rotated by ego yaw change',
             nurec_time_policy='nearest_recorded_rig_pose',
             source_behavior=str(a.behavior.resolve()),
             source_behavior_sha256=hashlib.sha256(a.behavior.read_bytes()).hexdigest(),
             hero_asset=f'/Game/HybridCinematicMVP/{out.name}/MaryPerformance',
             crossing_start_frame=1001+start, crossing_end_frame=1001+end)
    (out/'capture.json').write_text(json.dumps(frames))
    m['capture_sha256'] = hashlib.sha256((out/'capture.json').read_bytes()).hexdigest()
    (out/'manifest.json').write_text(json.dumps(m, indent=2))
    for name in ('lighting.json', 'proxy-fit.json'):
        shutil.copy2(a.template/name, out/name)
    (out/'environment.hdr').symlink_to((a.template/'environment.hdr').resolve())
    print(json.dumps({'shot': str(out), 'frames': len(frames), 'source_end': m['mary_export_end'],
                      'crossing_start_frame': m['crossing_start_frame'], 'crossing_end_frame': m['crossing_end_frame']}, indent=2))


if __name__ == '__main__':
    main()
