"""Validate rendered lens markers and optical-Z depth against known geometry."""
import argparse
import json
from pathlib import Path
import numpy as np
from scipy.ndimage import label,center_of_mass
from core import read_exr


def check(root):
    files=root/'diagnostics/camera'
    a=read_exr(next(files.glob('*.beauty.*.exr')))['A']
    z=read_exr(next(files.glob('*.integration.depth_m.*.exr')))['R']
    labels,n=label(a>.5);centres=center_of_mass(a,labels,range(1,n+1))
    records=[];matched=[]
    for marker in json.loads((root/'camera-markers.json').read_text()):
        expected=np.array(marker['expected_image_index'])
        if not centres:raise ValueError('No calibration markers rendered')
        j=int(np.argmin([np.linalg.norm(expected-np.array(c)[::-1]) for c in centres]));matched.append(j)
        c=centres[j];x,y=np.rint(c[::-1]).astype(int)
        surface=marker['expected_optical_z_m']*(1-.015/2)
        records.append({'expected':expected.tolist(),'rendered':list(c[::-1]),
                        'error_px':float(np.linalg.norm(expected-np.array(c)[::-1])),
                        'surface_depth_m':float(z[y,x]),'expected_surface_depth_m':surface,
                        'depth_error_m':abs(float(z[y,x])-surface)})
    passed=(n==5 and len(set(matched))==5 and max(r['error_px'] for r in records)<1
            and max(r['depth_error_m'] for r in records)<.01)
    report={'passed':passed,'markers':records,'scope':'Five rendered 3D spheres across view, native F-theta primary rays and optical-Z utility pass; 1px/1cm thresholds'}
    (root/'camera-render-gate.json').write_text(json.dumps(report,indent=2))
    return report


if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('shot',type=Path);a=ap.parse_args()
    report=check(a.shot);print(json.dumps(report,indent=2))
    if not report['passed']:raise SystemExit(1)
