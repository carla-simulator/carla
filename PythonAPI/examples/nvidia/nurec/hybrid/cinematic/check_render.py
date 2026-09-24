"""Check actual cinematic EXRs; executor success alone is not sufficient."""
import argparse
import json
from pathlib import Path
import numpy as np
from core import read_exr, validate_manifest, camera_world_points
from composite import find_pass
from fit_proxy import world_points


def check(shot, limit=48):
    m=validate_manifest(json.loads((shot/'manifest.json').read_text()))
    if not 1<=limit<=m['frame_end']-m['frame_start']+1:raise ValueError('limit exceeds shot range')
    frames=json.loads((shot/'capture.json').read_text())[:limit]
    fit=json.loads((shot/'proxy-fit.json').read_text()) if (shot/'proxy-fit.json').exists() else None
    attempt=json.loads((shot/'render-attempt.json').read_text()) if (shot/'render-attempt.json').exists() else None
    report={'frames':[], 'errors':[], 'production_ready':False,
            'process_exit_code':attempt.get('returncode') if attempt else None}
    for f in frames:
        frame=f['frame']; layers={}
        try:
            for name,utility in [('beauty',False),('integration',False),('clean',False),('integration',True)]:
                path=find_pass(shot/'renders/ue_review',name,frame,utility)
                if attempt and path.stat().st_mtime<attempt['started_unix']:
                    raise ValueError(f'Stale output: {path.name}')
                data=read_exr(path)
                for channel in 'RGBA':
                    if channel not in data or data[channel].shape!=tuple(m['resolution'][::-1]) or not np.isfinite(data[channel]).all():
                        raise ValueError(f'Invalid {path.name}:{channel}')
                layers['depth' if utility else name]=data
            alpha=layers['beauty']['A']; z=layers['depth']['R']
            if alpha.min()<0 or alpha.max()>1 or alpha.min()>.99 or ((alpha>.01).sum()<16 and not m.get('allow_empty_foreground',False)):
                raise ValueError('Invalid/empty/opaque foreground alpha')
            if not np.array_equal(z,layers['depth']['G']) or not np.array_equal(z,layers['depth']['B']):
                raise ValueError('Depth channels are not scalar data')
            median=float(np.median(z[alpha>.8])) if (alpha>.8).any() else None
            if median is not None and not .5<median<100:raise ValueError('Foreground depth is not plausible metres')
            item={'frame':frame,'actor_pixels':int((alpha>.01).sum()),
                  'soft_alpha_pixels':int(((alpha>.01)&(alpha<.99)).sum()),'median_actor_z_m':median}
            if fit:
                points=camera_world_points(z,m,f['camera_to_world_optical'])
                ground=(points[...,:2]-fit['origin_xy_m'])@np.array([fit['slope_x'],fit['slope_y']])+fit['height_m']
                clearance=points[...,2]-ground
                # Visible opaque surface estimate, not a skeletal/sole-contact certificate.
                feet=clearance[(alpha>.99)&(clearance<.25)]
                if len(feet):
                    item['lowest_visible_surface_above_road_m']=float(np.percentile(feet,1))
                    item['minimum_visible_surface_above_road_m']=float(feet.min())
            report['frames'].append(item)
        except (ValueError,FileNotFoundError) as exc:
            report['errors'].append({'frame':frame,'error':str(exc)})
    report['artifact_passes_valid']=not report['errors'] and len(report['frames'])==len(frames)
    (shot/'ue-pass-validation.json').write_text(json.dumps(report,indent=2))
    return report


if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('shot',type=Path);ap.add_argument('--limit',type=int,default=48)
    args=ap.parse_args();report=check(args.shot,args.limit)
    print(json.dumps(report,indent=2))
    if not report['artifact_passes_valid']:raise SystemExit(1)
