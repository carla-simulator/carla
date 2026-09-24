"""Shot conventions, calibrated projection, float EXR I/O and linear compositing.

Camera: OpenCV optical (+X right,+Y down,+Z forward), integer pixel centres.
World: CARLA metres (+X forward,+Y right,+Z up). Matrices multiply column vectors.
"""
import json
import struct
from pathlib import Path
import numpy as np

# Linear sRGB/D65 -> ACEScg/AP1 D60, including Bradford chromatic adaptation.
SRGB_TO_ACESCG = np.array([[.6130974,.3395231,.0473795],
                          [.0701937,.9163539,.0134524],
                          [.0206156,.1095698,.8698146]], dtype=np.float64)
# Optical coordinates to Unreal local camera coordinates.
OPTICAL_TO_UE = np.array([[0,0,1],[1,0,0],[0,-1,0]], dtype=float)


def transform(location, rotation):
    """Engine-native transform; avoids this CARLA build's get_matrix pitch bug."""
    pitch,yaw,roll=np.radians(rotation)
    cp,sp,cy,sy,cr,sr=np.cos(pitch),np.sin(pitch),np.cos(yaw),np.sin(yaw),np.cos(roll),np.sin(roll)
    t=np.eye(4)
    t[:3,:3]=np.array([[cp*cy,sr*sp*cy-cr*sy,-cr*sp*cy-sr*sy],
                       [cp*sy,sr*sp*sy+cr*cy,cy*sr-cr*sp*sy],
                       [sp,-sr*cp,cr*cp]])
    t[:3,3]=location
    return t


def optical_pose(location, rotation):
    t=transform(location,rotation); t[:3,:3]=t[:3,:3]@OPTICAL_TO_UE
    return t


def project(points, camera_to_world, K):
    p=np.asarray(points,float)
    q=(np.linalg.inv(camera_to_world)@np.c_[p,np.ones(len(p))].T).T[:,:3]
    uv=q@np.asarray(K).T
    return uv[:,:2]/uv[:,2,None],q[:,2]


def ray_to_z(distance,K):
    h,w=distance.shape
    yy,xx=np.mgrid[:h,:w]
    rays=np.stack([xx,yy,np.ones_like(xx)],axis=-1)@np.linalg.inv(K).T
    return distance/np.linalg.norm(rays,axis=-1)


def camera_rays(m):
    """Unit optical rays; F-theta matches CARLA LensModels.ush model 8.

    F-theta coefficients operate in output pixels and use pixel-centre samples.
    Nonidentity CDE is rejected until both backends support the same mapping.
    """
    w,h=m['resolution']; yy,xx=np.mgrid[:h,:w]
    if m['camera_model']=='pinhole':
        rays=np.stack([xx,yy,np.ones_like(xx)],axis=-1)@np.linalg.inv(m['K']).T
        return rays/np.linalg.norm(rays,axis=-1,keepdims=True)
    f=m['ftheta'];x=xx+.5-f['principal_point_x'];y=yy+.5-f['principal_point_y']
    radius=np.hypot(x,y);theta=np.polynomial.polynomial.polyval(radius,f['pixeldist_to_angle_poly'])
    factor=np.divide(np.sin(theta),radius,out=np.ones_like(radius),where=radius>1e-12)
    return np.stack([x*factor,y*factor,np.cos(theta)],axis=-1)


def camera_world_points(z,m,camera_to_world):
    rays=camera_rays(m)
    xyz=rays*np.divide(z,rays[...,2])[...,None]
    t=np.asarray(camera_to_world)
    return xyz@t[:3,:3].T+t[:3,3]


def decode_float_payload(payload):
    if payload[:4]!=b'HYF1':
        raise ValueError('NuRec float transport unavailable: expected HYF1, never upgrade 8-bit HYBR silently')
    h,w=struct.unpack('<II',payload[4:12])
    if not 0<h<=16384 or not 0<w<=16384 or len(payload)!=12+h*w*20:
        raise ValueError('Malformed NuRec float payload')
    values=np.frombuffer(payload, dtype='<f4',offset=12)
    n=h*w
    return values[:3*n].reshape(3,h,w).transpose(1,2,0).copy(),values[3*n:4*n].reshape(h,w).copy(),values[4*n:].reshape(h,w).copy()


def to_acescg(rgb,space):
    rgb=np.asarray(rgb,np.float32)
    if space=='ACEScg':return rgb.copy()
    if space=='srgb_display_assumed':
        rgb=np.where(rgb<=.04045,rgb/12.92,((np.maximum(rgb,0)+.055)/1.055)**2.4)
    elif space!='linear_srgb':
        raise ValueError('Unknown/unverified color space: explicitly declare a conversion before compositing')
    return (rgb@SRGB_TO_ACESCG.T).astype(np.float32)


def merge(plate,coverage,plate_z,fg_premult,fg_alpha,fg_z,tolerance=.1,integration=None,depth_bias=0.):
    """Plate includes its background. Alpha is occluding coverage, NOT plate alpha.

    At partial coverage a nearer splat only blocks its covered fraction. This
    single-depth approximation cannot reconstruct multiple translucent surfaces.
    Integration is a signed ACEScg lighting delta, independently depth-gated by
    the caller; never a black multiply or a second premultiplication of beauty.
    """
    if tolerance<=0:raise ValueError('Depth tolerance must be positive')
    coverage=np.asarray(coverage); fg_alpha=np.asarray(fg_alpha)
    if not np.isfinite(coverage).all() or not np.isfinite(fg_alpha).all():
        raise ValueError('Non-finite coverage')
    valid=np.isfinite(plate_z)&(plate_z>0)
    delta=np.zeros_like(plate_z)
    np.subtract(fg_z,plate_z,out=delta,where=valid)
    x=np.clip(.5+(delta-depth_bias)/(2*tolerance),0,1)
    occluded=np.where(valid,x*x*(3-2*x),0)
    visibility=1-np.clip(coverage,0,1)*occluded
    alpha=np.clip(fg_alpha,0,1)*visibility
    bg=np.asarray(plate)+(0 if integration is None else integration)
    return fg_premult*visibility[...,None]+bg*(1-alpha[...,None])


def transfer_receiver_delta(delta,plate,receiver):
    """Match signed interaction to plate luminance with bounded gain near black."""
    luma=np.array([.2722287,.6740818,.0536895])  # ACEScg/AP1
    gain=np.clip((plate@luma)/np.maximum(receiver@luma,.005),0,8)
    return delta*gain[...,None]


def write_exr(path,channels,half=False,metadata=None):
    import OpenEXR
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    data={k:np.ascontiguousarray(v,dtype=np.float16 if half else np.float32) for k,v in channels.items()}
    if any(not np.isfinite(v).all() for k,v in data.items() if k not in ('Z','distance')):
        raise ValueError('Non-finite color/coverage EXR')
    header={'compression':OpenEXR.ZIP_COMPRESSION,'type':OpenEXR.scanlineimage}
    if metadata:header['hybrid:metadata']=json.dumps(metadata,sort_keys=True)
    with OpenEXR.File(header,data) as f:f.write(str(path))


def read_exr(path):
    import OpenEXR
    with OpenEXR.File(str(path),separate_channels=True) as f:
        return {k:np.asarray(v.pixels,np.float32).copy() for k,v in f.channels().items()}


def validate_manifest(m):
    if m['schema']!='hybrid.cinematic.v1':raise ValueError('Unsupported manifest')
    if m['camera_model'] not in ('pinhole','ftheta'):
        raise ValueError('Unsupported camera model')
    w,h=m['resolution'];K=np.asarray(m['K'],float)
    if w<=0 or h<=0 or K.shape!=(3,3) or not np.isfinite(K).all():raise ValueError('Invalid camera')
    # CineCamera importer currently implements centred square pixels only.
    if m['camera_model']=='pinhole' and (m['distortion_applied_during_render'] or not np.allclose(K,[[K[0,0],0,w/2],[0,K[0,0],h/2],[0,0,1]],atol=1e-6) or K[0,0]<=0):
        raise ValueError('MVP CineCamera requires fx=fy, zero skew, cx=W/2, cy=H/2')
    if m['camera_model']=='ftheta':
        f=m['ftheta'];poly=np.asarray(f['pixeldist_to_angle_poly'])
        if not m['distortion_applied_during_render'] or not 2<=len(poly)<=8 or not np.isfinite(poly).all():
            raise ValueError('Invalid native F-theta camera')
        if not np.allclose(f.get('linear_cde',[1,0,0]),[1,0,0]):
            raise ValueError('Native path tracer does not implement F-theta CDE skew')
        rays=camera_rays(m)
        if not np.isfinite(rays).all() or np.min(rays[...,2])<=0:
            raise ValueError('Cinematic optical-Z compositing requires front-hemisphere lens rays')
    if m['fps']!=24 or m['frame_end']<m['frame_start']:
        raise ValueError('Cinematic shots require a nonempty range at 24 fps')
    if m['shutter_angle']!=0:raise ValueError('MVP validates sharp samples first; motion blur not yet implemented')
    return m
