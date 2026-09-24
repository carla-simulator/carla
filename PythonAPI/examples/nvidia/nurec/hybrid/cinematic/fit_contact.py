"""Sample local NuRec ground along captured actor paths; cinematic Z only."""
import argparse,json
from pathlib import Path
import numpy as np
from scipy.signal import savgol_filter
from core import read_exr
from fit_proxy import world_points

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('shot',type=Path);a=ap.parse_args();root=a.shot
    m=json.loads((root/'manifest.json').read_text());frames=json.loads((root/'capture.json').read_text());fit=json.loads((root/'proxy-fit.json').read_text());K=np.array(m['K'])
    result=[];previous={}
    for f in frames:
        depth=read_exr(root/f'renders/gs_depth/{f["frame"]}.exr')['Z'];alpha=read_exr(root/f'renders/gs_beauty/{f["frame"]}.exr')['A']
        pts=world_points(depth,K,np.array(f['camera_to_world_optical']));actors={}
        for actor in f['actors']:
            xy=np.array(actor['transform']['location'][:2]);pred=fit['height_m']+np.dot([fit['slope_x'],fit['slope_y']],xy-fit['origin_xy_m']);distance=np.linalg.norm(pts[...,:2]-xy,axis=-1)
            mask=(distance<1.3)&(alpha>.97)&np.isfinite(pts).all(-1)&(np.abs(pts[...,2]-pred)<.4);v=pts[mask];near=v[np.linalg.norm(v[:,:2]-xy,axis=-1)<.4]
            if len(v)<20 or len(near)<3:raise ValueError(f'Insufficient ground at {f["frame"]} {actor["id"]}')
            A=np.c_[v[:,:2]-xy,np.ones(len(v))];coef=np.array([fit['slope_x'],fit['slope_y'],np.median(near[:,2])]);good=np.ones(len(v),bool)
            if actor['id'] in previous:
                old_xy,old_coef=previous[actor['id']]
                coef=old_coef.copy();coef[2]+=np.dot(coef[:2],xy-old_xy)
            for _ in range(5):
                good=np.abs(v[:,2]-A@coef)<.04
                if good.sum()<12:raise ValueError('No consistent local ground surface')
                coef=np.linalg.lstsq(A[good],v[good,2],rcond=None)[0]
            previous[actor['id']]=(xy,coef.copy())
            actors[actor['id']]={'height_m':float(coef[2]),'raw_height_m':float(coef[2]),'slope_x':float(coef[0]),'slope_y':float(coef[1]),'inliers':int(good.sum()),'rms_m':float(np.sqrt(np.mean((v[good,2]-A[good]@coef)**2)))}
        result.append({'frame':f['frame'],'actors':actors})
    for role in result[0]['actors']:
        heights=[r['actors'][role]['height_m'] for r in result];smooth=savgol_filter(heights,9,2)
        for r,h in zip(result,smooth):r['actors'][role]['height_m']=float(h)
    (root/'contact-surfaces.json').write_text(json.dumps({'method':'temporally tracked local high-coverage NuRec ground fits, 9-frame height smoothing; XY/timing unchanged','frames':result},indent=2))
    print('Ground samples complete:',len(result))
if __name__=='__main__':main()
