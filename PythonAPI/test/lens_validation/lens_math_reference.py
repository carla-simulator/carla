#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Standalone numpy ground-truth for the sensor.camera.rt_lens lens models.

This module has NO dependency on carla and NO dependency on a running
server. It exists to be the single source of truth that everything else in
lens_validation/ (round-trip tests, calibration refit, A/B vs wide_angle,
the straight-edge test) is checked against.

Camera / ray convention
------------------------
Rays are unit vectors in camera space with +Z as the optical axis (forward),
following CARLA's camera-local convention (x=forward duality is handled by
the caller; here we only care that z is "looking direction"). Each ray is
described by:

    theta = angle from the optical axis (the incidence/field angle), >= 0
    phi   = azimuth around the optical axis, atan2(y, x)

Every lens model maps theta -> r (a unitless "normalized image radius"),
and then

    u = fx * (radial or distorted x) + cx
    v = fy * (radial or distorted y) + cy

which mirrors the Blueprint contract's fx, fy, cx, cy, distortion_coeffs,
lut fields. u, v live in the same normalized image-plane space fx/fy/cx/cy
are defined in (typically ~[0, 1] with cx=cy=0.5 at the principal point);
multiply by image_size_x/image_size_y to get pixels.

Supported camera_model values (matching the Blueprint contract):
    perspective     - pure pinhole, r = tan(theta)
    stereographic   - r = 2 * tan(theta / 2)
    equidistant     - r = theta
    equisolid       - r = 2 * sin(theta / 2)
    orthographic    - r = sin(theta)
    kannala_brandt  - r(theta) = k1*th + k2*th^3 + k3*th^5 + k4*th^7
    brown_conrady   - pinhole (r = tan(theta)) + OpenCV radial/tangential
                       distortion in normalized pinhole coordinates
    lut             - piecewise-linear interpolation of a measured
                       (theta, r) table

kannala_brandt and brown_conrady both still apply fx/fy/cx/cy on top of
their own model output, exactly like the undistorted models -- this keeps
a single, uniform project()/unproject() code path for every camera_model.
"""

import numpy as np


# ---------------------------------------------------------------------------
# Ray <-> (theta, phi) helpers
# ---------------------------------------------------------------------------

def ray_to_theta_phi(rays):
    """rays: (..., 3) array of (not necessarily unit) camera-space vectors
    with +Z forward. Returns (theta, phi), each shape (...)."""
    rays = np.asarray(rays, dtype=np.float64)
    x, y, z = rays[..., 0], rays[..., 1], rays[..., 2]
    r_xy = np.hypot(x, y)
    theta = np.arctan2(r_xy, z)
    phi = np.arctan2(y, x)
    return theta, phi


def theta_phi_to_ray(theta, phi):
    """Inverse of ray_to_theta_phi. Returns unit vectors, shape (..., 3)."""
    theta = np.asarray(theta, dtype=np.float64)
    phi = np.asarray(phi, dtype=np.float64)
    x = np.sin(theta) * np.cos(phi)
    y = np.sin(theta) * np.sin(phi)
    z = np.cos(theta)
    return np.stack([x, y, z], axis=-1)


# ---------------------------------------------------------------------------
# Closed-form radial models: theta -> r and r -> theta
# ---------------------------------------------------------------------------

def _perspective_fwd(theta):
    return np.tan(theta)


def _perspective_inv(r):
    return np.arctan(r)


def _stereographic_fwd(theta):
    return 2.0 * np.tan(theta / 2.0)


def _stereographic_inv(r):
    return 2.0 * np.arctan(r / 2.0)


def _equidistant_fwd(theta):
    return np.asarray(theta, dtype=np.float64).copy()


def _equidistant_inv(r):
    return np.asarray(r, dtype=np.float64).copy()


def _equisolid_fwd(theta):
    return 2.0 * np.sin(theta / 2.0)


def _equisolid_inv(r):
    return 2.0 * np.arcsin(np.clip(np.asarray(r, dtype=np.float64) / 2.0, -1.0, 1.0))


def _orthographic_fwd(theta):
    return np.sin(theta)


def _orthographic_inv(r):
    return np.arcsin(np.clip(np.asarray(r, dtype=np.float64), -1.0, 1.0))


RADIAL_FORWARD = {
    'perspective': _perspective_fwd,
    'stereographic': _stereographic_fwd,
    'equidistant': _equidistant_fwd,
    'equisolid': _equisolid_fwd,
    'orthographic': _orthographic_fwd,
}

RADIAL_INVERSE = {
    'perspective': _perspective_inv,
    'stereographic': _stereographic_inv,
    'equidistant': _equidistant_inv,
    'equisolid': _equisolid_inv,
    'orthographic': _orthographic_inv,
}

# Models whose r(theta) is bounded and therefore have a finite valid FOV.
RADIAL_MAX_THETA_RAD = {
    'equisolid': np.pi,        # r = 2 sin(theta/2), valid up to theta = pi
    'orthographic': np.pi / 2,  # r = sin(theta), valid up to theta = pi/2
}


# ---------------------------------------------------------------------------
# Kannala-Brandt generic polynomial fisheye model
#   r(theta) = k1*theta + k2*theta^3 + k3*theta^5 + k4*theta^7
# ---------------------------------------------------------------------------

def kannala_brandt_forward(theta, coeffs):
    theta = np.asarray(theta, dtype=np.float64)
    k1, k2, k3, k4 = coeffs
    return k1 * theta + k2 * theta**3 + k3 * theta**5 + k4 * theta**7


def kannala_brandt_forward_deriv(theta, coeffs):
    theta = np.asarray(theta, dtype=np.float64)
    k1, k2, k3, k4 = coeffs
    return k1 + 3.0 * k2 * theta**2 + 5.0 * k3 * theta**4 + 7.0 * k4 * theta**6


def kannala_brandt_inverse(r, coeffs, iters=64, tol=1e-14):
    """Newton-Raphson inverse of the KB polynomial. r and the return value
    can be scalars or arrays."""
    r = np.asarray(r, dtype=np.float64)
    k1 = coeffs[0]
    if abs(k1) > 1e-12:
        theta = (r / k1).copy()
    else:
        theta = r.copy()
    for _ in range(iters):
        f = kannala_brandt_forward(theta, coeffs) - r
        fp = kannala_brandt_forward_deriv(theta, coeffs)
        fp = np.where(np.abs(fp) < 1e-12, 1e-12, fp)
        step = f / fp
        theta = theta - step
        if np.max(np.abs(step)) < tol:
            break
    return theta


# ---------------------------------------------------------------------------
# Brown-Conrady (OpenCV) radial + tangential distortion, applied to
# normalized pinhole coordinates (x, y) = (tan(theta) cos(phi), tan(theta) sin(phi))
# coeffs order matches OpenCV: [k1, k2, p1, p2, k3]
# ---------------------------------------------------------------------------

def brown_conrady_distort(x, y, coeffs):
    x = np.asarray(x, dtype=np.float64)
    y = np.asarray(y, dtype=np.float64)
    k1, k2, p1, p2, k3 = coeffs
    r2 = x * x + y * y
    radial = 1.0 + k1 * r2 + k2 * r2**2 + k3 * r2**3
    xd = x * radial + 2.0 * p1 * x * y + p2 * (r2 + 2.0 * x * x)
    yd = y * radial + p1 * (r2 + 2.0 * y * y) + 2.0 * p2 * x * y
    return xd, yd


def brown_conrady_undistort(xd, yd, coeffs, iters=32):
    """Iterative fixed-point inverse, matching OpenCV's cvUndistortPoints
    algorithm (same recurrence cv2.undistortPoints uses internally)."""
    xd = np.asarray(xd, dtype=np.float64)
    yd = np.asarray(yd, dtype=np.float64)
    k1, k2, p1, p2, k3 = coeffs
    x0, y0 = xd.copy(), yd.copy()
    x, y = xd.copy(), yd.copy()
    for _ in range(iters):
        r2 = x * x + y * y
        icdist = 1.0 / (1.0 + k1 * r2 + k2 * r2**2 + k3 * r2**3)
        delta_x = 2.0 * p1 * x * y + p2 * (r2 + 2.0 * x * x)
        delta_y = p1 * (r2 + 2.0 * y * y) + 2.0 * p2 * x * y
        x = (x0 - delta_x) * icdist
        y = (y0 - delta_y) * icdist
    return x, y


# ---------------------------------------------------------------------------
# LUT model: piecewise-linear interpolation of a measured (theta, r) table.
# `lut` is stored flattened as [theta0, r0, theta1, r1, ...] to match the
# Blueprint's csv-floats attribute.
# ---------------------------------------------------------------------------

def lut_pack(theta_samples, r_samples):
    theta_samples = np.asarray(theta_samples, dtype=np.float64)
    r_samples = np.asarray(r_samples, dtype=np.float64)
    return np.stack([theta_samples, r_samples], axis=-1).ravel().tolist()


def lut_unpack(lut):
    flat = np.asarray(lut, dtype=np.float64)
    if flat.size % 2 != 0:
        raise ValueError('lut must contain an even number of floats (theta, r pairs)')
    pairs = flat.reshape(-1, 2)
    theta_samples = pairs[:, 0]
    r_samples = pairs[:, 1]
    return theta_samples, r_samples


def lut_forward(theta, lut):
    theta_samples, r_samples = lut_unpack(lut)
    order = np.argsort(theta_samples)
    return np.interp(theta, theta_samples[order], r_samples[order])


def lut_inverse(r, lut):
    theta_samples, r_samples = lut_unpack(lut)
    order = np.argsort(r_samples)
    r_sorted = r_samples[order]
    theta_sorted = theta_samples[order]
    if np.any(np.diff(r_sorted) <= 0):
        raise ValueError('lut r-samples must be strictly monotonic to be invertible')
    return np.interp(r, r_sorted, theta_sorted)


# ---------------------------------------------------------------------------
# Unified lens model: project() / unproject()
# ---------------------------------------------------------------------------

RADIAL_MODELS = set(RADIAL_FORWARD.keys())
ALL_MODELS = RADIAL_MODELS | {'kannala_brandt', 'brown_conrady', 'lut'}


class LensModel:
    """Ground-truth forward/inverse projection for one camera_model + params.

    Parameters mirror the Blueprint contract: camera_model, fx, fy, cx, cy
    (normalized), distortion_coeffs, lut, theta_max_deg.
    """

    def __init__(self, camera_model, fx, fy, cx, cy, distortion_coeffs=None,
                 lut=None, theta_max_deg=90.0):
        if camera_model not in ALL_MODELS:
            raise ValueError('unknown camera_model %r, expected one of %s' %
                              (camera_model, sorted(ALL_MODELS)))
        self.camera_model = camera_model
        self.fx = float(fx)
        self.fy = float(fy)
        self.cx = float(cx)
        self.cy = float(cy)
        self.distortion_coeffs = list(distortion_coeffs or [])
        self.lut = list(lut or [])
        self.theta_max_deg = float(theta_max_deg)

        if camera_model == 'kannala_brandt' and len(self.distortion_coeffs) != 4:
            raise ValueError('kannala_brandt requires 4 distortion_coeffs [k1, k2, k3, k4]')
        if camera_model == 'brown_conrady' and len(self.distortion_coeffs) != 5:
            raise ValueError('brown_conrady requires 5 distortion_coeffs [k1, k2, p1, p2, k3]')
        if camera_model == 'lut' and len(self.lut) < 4:
            raise ValueError('lut requires at least 2 (theta, r) sample pairs')

    @property
    def theta_max_rad(self):
        return np.deg2rad(self.theta_max_deg)

    def project(self, rays):
        """rays: (..., 3) camera-space vectors, +Z forward. Returns (u, v)."""
        theta, phi = ray_to_theta_phi(rays)

        if self.camera_model == 'brown_conrady':
            x = np.tan(theta) * np.cos(phi)
            y = np.tan(theta) * np.sin(phi)
            xd, yd = brown_conrady_distort(x, y, self.distortion_coeffs)
            u = self.fx * xd + self.cx
            v = self.fy * yd + self.cy
            return u, v

        if self.camera_model == 'kannala_brandt':
            r = kannala_brandt_forward(theta, self.distortion_coeffs)
        elif self.camera_model == 'lut':
            r = lut_forward(theta, self.lut)
        else:
            r = RADIAL_FORWARD[self.camera_model](theta)

        u = self.fx * r * np.cos(phi) + self.cx
        v = self.fy * r * np.sin(phi) + self.cy
        return u, v

    def unproject(self, u, v):
        """Inverse of project(). Returns unit camera-space rays, (..., 3)."""
        u = np.asarray(u, dtype=np.float64)
        v = np.asarray(v, dtype=np.float64)
        x = (u - self.cx) / self.fx
        y = (v - self.cy) / self.fy

        if self.camera_model == 'brown_conrady':
            xp, yp = brown_conrady_undistort(x, y, self.distortion_coeffs)
            theta = np.arctan(np.hypot(xp, yp))
            phi = np.arctan2(yp, xp)
            return theta_phi_to_ray(theta, phi)

        r = np.hypot(x, y)
        phi = np.arctan2(y, x)
        if self.camera_model == 'kannala_brandt':
            theta = kannala_brandt_inverse(r, self.distortion_coeffs)
        elif self.camera_model == 'lut':
            theta = lut_inverse(r, self.lut)
        else:
            theta = RADIAL_INVERSE[self.camera_model](r)
        return theta_phi_to_ray(theta, phi)
