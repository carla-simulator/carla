"""Fit a local road receiver to high-coverage NuRec depth near the actor path.

This geometric fit does not certify the model's depth weighting or foot contact.
It writes a derivative report; the original CARLA capture is never changed.
"""
import argparse
import json
from pathlib import Path
import numpy as np
from core import read_exr, validate_manifest


def world_points(z, K, camera_to_world):
    yy, xx = np.indices(z.shape)
    rays = np.stack([xx, yy, np.ones_like(xx)], -1) @ np.linalg.inv(K).T
    return (rays * z[..., None]) @ camera_to_world[:3, :3].T + camera_to_world[:3, 3]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('shot', type=Path)
    a = ap.parse_args()
    m = validate_manifest(json.loads((a.shot/'manifest.json').read_text()))
    frames = json.loads((a.shot/'capture.json').read_text())
    frame = frames[0]['frame']
    depth = read_exr(a.shot/f'renders/gs_depth/{frame}.exr')['Z']
    alpha = read_exr(a.shot/f'renders/gs_beauty/{frame}.exr')['A']
    pts = world_points(depth, np.array(m['K']), np.array(frames[0]['camera_to_world_optical']))
    roots = np.array([f['actors'][0]['transform']['location'] for f in frames])
    origin = roots.mean(0)[:2]
    mask = (np.linalg.norm(pts[..., :2]-origin, axis=-1)<3) & (alpha>.97)
    mask &= np.isfinite(pts).all(-1) & (np.abs(pts[..., 2]-m['proxy_road_height_m'])<.4)
    v = pts[mask]
    if len(v)<100: raise ValueError('Insufficient covered road samples')
    A = np.c_[v[:, :2]-origin, np.ones(len(v))]
    coef = np.linalg.lstsq(A, v[:, 2], rcond=None)[0]
    for _ in range(4):
        residual = v[:, 2]-A@coef
        good = np.abs(residual-np.median(residual))<.03
        if good.sum()<100: raise ValueError('Road samples do not support a plane')
        coef = np.linalg.lstsq(A[good], v[good, 2], rcond=None)[0]
    rms = float(np.sqrt(np.mean((v[good, 2]-A[good]@coef)**2)))
    if rms>.03: raise ValueError('Road fit exceeds 3 cm RMS')
    report = {'method': 'robust local plane from NuRec optical depth, high-opacity road samples',
              'origin_xy_m': origin.tolist(), 'slope_x': float(coef[0]), 'slope_y': float(coef[1]),
              'height_m': float(coef[2]), 'inlier_points': int(good.sum()), 'rms_m': rms,
              'native_depth_weighting_verified': False}
    (a.shot/'proxy-fit.json').write_text(json.dumps(report, indent=2))
    print(json.dumps(report, indent=2))


if __name__=='__main__': main()
