"""Environment maps for CARLA's sky light (`world.set_sky_light_map`).

CARLA panorama convention (what the server expects)
---------------------------------------------------
Equirectangular, CARLA world frame (Unreal: X forward, Y right, Z up, left-handed), float32 linear RGB,
shape (H, W, 3), row 0 = zenith:

    column u:  azimuth   phi   = 2*pi * (u + 0.5) / W    measured from +X toward +Y (same sense as a CARLA yaw)
    row    v:  elevation theta = pi/2 - pi * (v + 0.5) / H
    direction d = (cos(theta) cos(phi), cos(theta) sin(phi), sin(theta))

So a bright patch at column W/4 sits at azimuth 90 deg = +Y = to the RIGHT of an ego with yaw 0, and a
patch at column 3W/4 (azimuth 270) sits on its LEFT.

illum_probe.py panorama convention (the input)
----------------------------------------------
Equirectangular in the rig FLU frame (x forward, y LEFT, z up), row 0 = zenith:

    column c:  az = (0.5 - (c + 0.5) / PW) * 2*pi     centre column = forward, left of centre = +az = toward +y (left)
    row    r:  el = (0.5 - (r + 0.5) / PH) * pi
    d_rig = (cos(el) cos(az), cos(el) sin(az), sin(el))

Rotating one into the other
---------------------------
Given the ego's CARLA transform (yaw/pitch/roll in degrees) the rig axes in CARLA world are the rows of
Unreal's FRotationMatrix (body x forward, y right, z up):

    fwd   = ( cp*cy,               cp*sy,               sp   )
    right = ( sr*sp*cy - cr*sy,    sr*sp*sy + cr*cy,   -sr*cp)
    up    = (-(cr*sp*cy + sr*sy),  cy*sr - cr*sp*sy,    cr*cp)

and a rig FLU direction (x, yL, z) is the body direction (x, -yL, z), hence
d_carla = x*fwd - yL*right + z*up. This is built here from the angles on purpose: the ue58
carla.Transform.get_matrix mirrors pitch and roll relative to the engine (PR #9751).

Alternatively, with the probe's `rig_pose` (rig -> scene, 4x4, scene frame right-handed with z up as
recorded by the NuRec clip) and the scene->CARLA mapping used by the hybrid pipeline (y negated):
d_scene = R_pose @ d_rig; d_carla = (d_scene.x, -d_scene.y, d_scene.z).

Radiometry
----------
The probe .jpg is 8-bit display-referred output of the engine. It is linearised (sRGB curve) and scaled
by `scale`. The sky light treats cubemap values as scene radiance in the project's photometric units (the
sun is ~100 klux at noon, a clear sky is ~5-10 kcd/m^2), so a linear 0..1 map needs an intensity in the
thousands to light anything under CARLA's fixed daylight exposure. `DEFAULT_INTENSITY` is a starting
point calibrated so that the highway probe's sky lands near the atmosphere capture it replaces; measure
against a reference frame if it matters.
"""
import json
import math

import numpy as np
from scipy.ndimage import map_coordinates

# Measured on the OpenDRIVE proxy world (sun az 275 / alt 39.2, manual exposure 0.44): a shadowed road
# patch reads 31 (default atmosphere capture), 2.3 at intensity 0, 15.5 at 12000, 85 at 100000 with the
# highway probe .jpg (linear mean 0.118). Ambient is linear in intensity, so ~26000 matches the default
# atmosphere capture. That default was calibrated for the raster ambient term only: under the exposure that
# matches the sunlit road to the neural road, the neural sky sits at ~6x the road's luminance while the sky
# light at 26000 sits at ~1x, so path-traced car paint (a mirror of the sky) came out 3-6x too dark. At
# 160000 (6.15x) the sky matches (2026-09-08, highway scene 7c2cf6cd: a silver car's side goes from 0.04 to
# 0.21 of the road luminance at its median, 0.54 at p90; the cast shadows keep their depth, the exposure
# solve moves by -0.8 EV). Sweep with `hybrid_run.py --skymap-intensity`.
DEFAULT_INTENSITY = 160000.0


def srgb_to_linear(x):
    x = np.asarray(x, np.float32)
    return np.where(x <= 0.04045, x / 12.92, ((x + 0.055) / 1.055) ** 2.4).astype(np.float32)


def carla_rotation_rows(yaw_deg, pitch_deg=0.0, roll_deg=0.0):
    """Body axes (forward, right, up) in CARLA world coordinates, Unreal's FRotationMatrix rows."""
    cy, sy = math.cos(math.radians(yaw_deg)), math.sin(math.radians(yaw_deg))
    cp, sp = math.cos(math.radians(pitch_deg)), math.sin(math.radians(pitch_deg))
    cr, sr = math.cos(math.radians(roll_deg)), math.sin(math.radians(roll_deg))
    fwd = np.array([cp * cy, cp * sy, sp])
    right = np.array([sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp])
    up = np.array([-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp])
    return fwd, right, up


def rig_to_carla_matrix(ego_transform=None, rig_pose=None):
    """3x3 matrix M with d_carla = M @ d_rig (rig FLU). One of the two inputs must be given."""
    if ego_transform is not None:
        r = ego_transform.rotation
        fwd, right, up = carla_rotation_rows(r.yaw, r.pitch, r.roll)
        # columns: image of rig x (fwd), rig y (left = -right), rig z (up)
        return np.stack([fwd, -right, up], axis=1)
    if rig_pose is not None:
        R = np.asarray(rig_pose, float)[:3, :3]
        return np.diag([1.0, -1.0, 1.0]) @ R
    raise ValueError("ego_transform or rig_pose required")


def carla_equirect_dirs(W, H):
    """Unit directions (H, W, 3) in CARLA world for every pixel of a W x H CARLA panorama."""
    phi = 2 * np.pi * (np.arange(W) + 0.5) / W
    theta = np.pi / 2 - np.pi * (np.arange(H) + 0.5) / H
    PH, TH = np.meshgrid(phi, theta)
    return np.stack([np.cos(TH) * np.cos(PH), np.cos(TH) * np.sin(PH), np.sin(TH)], axis=-1)


def rig_pano_to_carla(pano_rig, M, W=None, H=None):
    """Resample an illum_probe panorama (rig FLU, (PH, PW, C) float) into the CARLA convention.

    M: 3x3 with d_carla = M @ d_rig (see rig_to_carla_matrix). Output (H, W, C), defaults to the input size.
    """
    PH, PW = pano_rig.shape[:2]
    W = W or PW
    H = H or PH
    d = carla_equirect_dirs(W, H).reshape(-1, 3)
    d_rig = d @ M            # M^T applied to each row: d_rig = M^T d_carla
    az = np.arctan2(d_rig[:, 1], d_rig[:, 0])
    el = np.arcsin(np.clip(d_rig[:, 2], -1, 1))
    c = (0.5 - az / (2 * np.pi)) * PW - 0.5
    r = (0.5 - el / np.pi) * PH - 0.5
    out = np.empty((H * W, pano_rig.shape[2]), np.float32)
    for ch in range(pano_rig.shape[2]):
        out[:, ch] = map_coordinates(pano_rig[..., ch], [np.clip(r, 0, PH - 1), c % PW], order=1, mode="wrap")
    return out.reshape(H, W, -1)


def load_probe(jpg_path, json_path=None, scale=1.0):
    """Linear float (PH, PW, 3) rig-frame panorama from illum_probe output, plus its json (or None)."""
    from PIL import Image
    img = np.asarray(Image.open(jpg_path).convert("RGB"), np.float32) / 255.0
    meta = json.load(open(json_path)) if json_path else None
    if meta and meta.get("sun") and not meta.get("jpg_clean"):
        # probes before 2026-09-08 drew a magenta crosshair (5 px row + column bands) through the sun on the .jpg
        # (now on OUT_vis.jpg, `jpg_clean` in the json); it would show up as a magenta ring in every reflection.
        # Fill the bands from their neighbours.
        PH, PW = img.shape[:2]
        cu = int((0.5 - math.radians(meta["sun"]["az_deg"]) / (2 * np.pi)) * PW)
        cv = int((0.5 - math.radians(meta["sun"]["el_deg"]) / np.pi) * PH)
        r0, r1 = max(cv - 4, 0), min(cv + 5, PH)
        c0, c1 = max(cu - 4, 0), min(cu + 5, PW)
        img[r0:r1] = 0.5 * (img[max(r0 - 1, 0)] + img[min(r1, PH - 1)])[None]
        img[:, c0:c1] = 0.5 * (img[:, max(c0 - 1, 0)] + img[:, min(c1, PW - 1)])[:, None]
    return srgb_to_linear(img) * np.float32(scale), meta


def synthetic_test_map(W=512, H=256, side="left", patch=20.0):
    """Blue upper half, orange lower half, a bright white patch on the ego's LEFT (azimuth 270, elevation
    10..60 deg) or RIGHT (azimuth 90) for an ego with yaw 0. Linear RGB, sky/ground in 0..1."""
    m = np.zeros((H, W, 3), np.float32)
    m[: H // 2] = (0.05, 0.15, 0.9)
    m[H // 2:] = (0.9, 0.45, 0.05)
    u0 = int(W * (0.75 if side == "left" else 0.25))
    m[int(H * 0.17): int(H * 0.44), u0 - W // 16: u0 + W // 16] = (patch, patch, patch)
    return m


def apply(world, pano_carla, intensity=DEFAULT_INTENSITY, face_size=512):
    """Send a (H, W, 3) float32 linear CARLA-convention panorama to the server."""
    pano_carla = np.ascontiguousarray(pano_carla, dtype=np.float32)
    H, W = pano_carla.shape[:2]
    world.set_sky_light_map(int(W), int(H), pano_carla.tobytes(), float(intensity), int(face_size))
