"""Validate durable capture/plate artifacts, including after an interrupted run."""
import argparse,hashlib,json,zipfile
from pathlib import Path
import numpy as np
from core import validate_manifest,read_exr


def validate(shot):
    m=validate_manifest(json.loads((shot/'manifest.json').read_text()))
    raw=(shot/'capture.json').read_bytes()
    if hashlib.sha256(raw).hexdigest()!=m['capture_sha256']:raise ValueError('Capture checksum mismatch')
    frames=json.loads(raw);expected=list(range(m['frame_start'],m['frame_end']+1))
    if [x['frame'] for x in frames]!=expected:raise ValueError('Missing/duplicate/out-of-order capture frame')
    sim=np.array([f['simulation_frame'] for f in frames]);times=np.array([f['simulation_time_s'] for f in frames])
    if not np.all(np.diff(sim)==1):raise ValueError('Simulation frames are not consecutive')
    if not np.allclose(np.diff(times),1/m['fps'],atol=1e-6,rtol=0):raise ValueError('Simulation timing drift')
    timestamps=np.array([f['timestamp_us'] for f in frames])
    if m.get('nurec_time_policy')=='nearest_recorded_rig_pose':
        with zipfile.ZipFile(m['source_usdz']) as z:
            rig=json.loads(z.read('rig_trajectories.json'))['rig_trajectories'][0]
        recorded=np.array(rig['T_rig_world_timestamps_us'],dtype=np.int64)
        if not np.isin(timestamps,recorded).all() or (np.diff(timestamps)<0).any():
            raise ValueError('NuRec source times must advance through recorded samples')
    elif not np.array_equal(timestamps-timestamps[0],np.rint(np.arange(len(frames))*1e6/m['fps']).astype(int)):
        raise ValueError('NuRec timing drift')
    w,h=m['resolution'];decoded={};missing={}
    for kind,channels in [('gs_beauty',('R','G','B','A')),('gs_depth',('Z','distance'))]:
        decoded[kind]=0;missing[kind]=[]
        for frame in expected:
            path=shot/f'renders/{kind}/{frame}.exr'
            if not path.exists():missing[kind].append(frame);continue
            data=read_exr(path)
            for ch in channels:
                if ch not in data or data[ch].shape!=(h,w) or not np.isfinite(data[ch]).all():
                    raise ValueError(f'Invalid {path} channel {ch}')
            decoded[kind]+=1
    result={'capture_sha256_verified':True,'capture_frames':len(frames),'timing_passed':True,'decoded_exrs':decoded,'missing_exrs':missing,
        'production_gates':m['gates'],'production_ready':False}
    # This artifact check must not certify visual, radiometric or renderer gates.
    (shot/'artifact-validation.json').write_text(json.dumps(result,indent=2))
    return result

if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('shot',type=Path);a=ap.parse_args()
    result=validate(a.shot);print(json.dumps(result,indent=2))
    if any(result['missing_exrs'].values()):raise SystemExit(1)
