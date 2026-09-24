"""Build a static NuRec receiver and ground the actual Mary sole samples on it.

Run sample_mary_soles.py in Blender first. Geometry and contact share the same
triangulated height field; captured horizontal motion and sample timing stay fixed.
"""
import argparse,json
from pathlib import Path
import numpy as np
from scipy.spatial import cKDTree
from scipy.ndimage import gaussian_filter
from core import read_exr
from fit_proxy import world_points


def sample_heightfield(xy, xs, ys, height):
    q=np.asarray(xy);sx=xs[1]-xs[0];sy=ys[1]-ys[0]
    px=(q[...,0]-xs[0])/sx;py=(q[...,1]-ys[0])/sy
    ix=np.clip(np.floor(px).astype(int),0,len(xs)-2);iy=np.clip(np.floor(py).astype(int),0,len(ys)-2)
    x=np.clip(px-ix,0,1);y=np.clip(py-iy,0,1)
    a=height[iy,ix];b=height[iy,ix+1];c=height[iy+1,ix+1];d=height[iy+1,ix]
    return np.where(x>=y,a+(b-a)*x+(c-b)*y,a+(c-d)*x+(d-a)*y)


def main():
    ap=argparse.ArgumentParser();ap.add_argument('shot',type=Path);a=ap.parse_args();r=a.shot
    m=json.loads((r/'manifest.json').read_text());frames=json.loads((r/'capture.json').read_text());fit=json.loads((r/'proxy-fit.json').read_text());contact={f['frame']:f['actors'] for f in json.loads((r/'contact-surfaces.json').read_text())['frames']}
    rows=[(np.array(a['transform']['location'][:2]),contact[f['frame']][a['id']]) for f in frames for a in f['actors']]
    centres=np.array([p for p,c in rows]);coefs=np.array([[c['slope_x'],c['slope_y'],c['height_m']] for p,c in rows]);tree=cKDTree(centres)
    def prior(q):
        dist,ids=tree.query(q,k=1);return coefs[ids,2]+np.sum((q-centres[ids])*coefs[ids,:2],axis=-1),dist
    clouds=[]
    for f in frames[::8]:
        z=read_exr(r/f'renders/gs_depth/{f["frame"]}.exr')['Z'];alpha=read_exr(r/f'renders/gs_beauty/{f["frame"]}.exr')['A'];pts=world_points(z,np.array(m['K']),np.array(f['camera_to_world_optical']))[::4,::4];ok=(alpha[::4,::4]>.97)&np.isfinite(pts).all(-1);p=pts[ok];expected,d=prior(p[:,:2]);p=p[(d<4)&(np.abs(p[:,2]-expected)<.07)];clouds.append(p)
    cloud=np.concatenate(clouds);low=centres.min(0)-7;high=centres.max(0)+7;xs=np.arange(low[0],high[0]+.2,.2);ys=np.arange(low[1],high[1]+.2,.2);xx,yy=np.meshgrid(xs,ys);xy=np.c_[xx.ravel(),yy.ravel()];base,_=prior(xy)
    ds,ids=cKDTree(cloud[:,:2]).query(xy,k=8);w=1/np.maximum(ds,.025)**2;observed=np.sum(w*cloud[ids,2],axis=1)/w.sum(1);blend=np.clip((1.5-ds[:,0])/1.2,0,1);heights=(observed*blend+base*(1-blend)).reshape(xx.shape);heights=gaussian_filter(heights,.6)
    # Receiver surface is the reconstructed physical ground, not a shared moving plane.
    np.savez(r/'receiver-heightfield.npz',xs=xs,ys=ys,height=heights)
    yaw=json.loads((r.parent/m['revision_of']/'unreal-build.json').read_text())['yaw'];th=np.radians(yaw);rot=np.array([[np.cos(th),-np.sin(th)],[np.sin(th),np.cos(th)]]);bl={s['source_frame']:s for s in json.loads((r/'capture/mary/blender-samples.json').read_text())};report=[]
    for f in frames:
        n=f['mary_source_frame'];v=np.load(r/f'sole-samples/{n:04d}.npy');root=np.array(bl[n]['matrix_world'])[:3,3];local=v-root;local[:,1]*=-1;target=np.array(f['actors'][0]['transform']['location'][:2]);xy=local[:,:2]@rot.T+target
        h=sample_heightfield(xy,xs,ys,heights);z=float(np.max(h-local[:,2])+.004);clearance=local[:,2]+z-h;support=int(np.argmin(clearance))
        report.append({'frame':f['frame'],'source_frame':n,'root_height_m':z,'min_sole_clearance_m':float(clearance.min()),'sole_vertices':len(v),'support_world_xy_m':xy[support].tolist(),'root_delta_from_previous_fit_m':z-contact[f['frame']]['mary_proxy']['height_m'],'max_foot_surface_slope_height_range_m':float(np.ptp(h))})
    origin=np.array([*fit['origin_xy_m'],fit['height_m']]);verts=np.c_[xx.ravel(),yy.ravel(),heights.ravel()]-origin
    np.savez(r/'receiver-mesh.npz',vertices=verts,shape=np.array(xx.shape),origin=origin)
    (r/'sole-contact.json').write_text(json.dumps({'method':'exact native evaluated foot vertices grounded to static triangulated receiver; 4mm minimum clearance','yaw_degrees':yaw,'frames':report,'ground_points':len(cloud),'receiver_grid_shape':list(xx.shape)},indent=2))
    print('contact',len(report),'height delta range',min(v['root_delta_from_previous_fit_m'] for v in report),max(v['root_delta_from_previous_fit_m'] for v in report),flush=True)
if __name__=='__main__':main()
