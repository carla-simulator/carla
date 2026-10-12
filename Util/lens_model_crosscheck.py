#!/usr/bin/env python3
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
"""
Standalone numerical cross-check of the FLensModelDescriptor forward/inverse
lens math shared by:
  - CPU: Carla/Source/Carla/Util/CameraModelUtil.{h,cpp}
  - GPU: Engine/Shaders/Private/LensModels.ush

This script is a from-scratch re-implementation of the same equations
(not a transliteration of the C++/HLSL) used as an independent proof that
forward(inverse(x)) and inverse(forward(x)) round-trip within 1e-5 for every
ECameraModel, including BrownConrady (2D Newton-Raphson inverse) and LUT1D
(binary-search interpolation inverse).

Coordinate convention: +X right, +Y down, +Z forward (matches the CPU/GPU core).

Run with: python3 lens_model_crosscheck.py
"""

import math
import sys

TOLERANCE = 1e-5


# ---------------------------------------------------------------------------
# Radially symmetric R(Theta) / Theta(R).
# ---------------------------------------------------------------------------

def kb_polynomial(theta, coeffs):
    result = 1.0
    theta2 = theta * theta
    theta_n = 1.0
    for k in coeffs:
        theta_n *= theta2
        result += k * theta_n
    return result * theta


def kb_polynomial_derivative(theta, coeffs):
    result = 1.0
    theta2 = theta * theta
    theta_n = theta2
    a = 3.0
    for k in coeffs:
        result += a * k * theta_n
        a += 2.0
        theta_n *= theta2
    return result


def kb_solve_theta(distance, coeffs, iterations=32):
    theta = distance
    for _ in range(iterations):
        n = distance - kb_polynomial(theta, coeffs)
        d = -kb_polynomial_derivative(theta, coeffs)
        theta -= n / d
    return theta


def lut1d_forward(theta, samples, theta_max):
    count = len(samples)
    if count == 0:
        return 0.0
    if count == 1:
        return samples[0]
    clamped = max(0.0, min(theta, theta_max))
    t = (clamped / theta_max) * (count - 1) if theta_max > 0.0 else 0.0
    i0 = max(0, min(int(math.floor(t)), count - 1))
    i1 = min(i0 + 1, count - 1)
    frac = t - i0
    return samples[i0] + (samples[i1] - samples[i0]) * frac


def lut1d_inverse(distance, samples, theta_max):
    count = len(samples)
    if count < 2:
        return 0.0
    if distance <= samples[0]:
        return 0.0
    if distance >= samples[-1]:
        return theta_max
    lo, hi = 0, count - 1
    while hi - lo > 1:
        mid = (lo + hi) // 2
        if samples[mid] <= distance:
            lo = mid
        else:
            hi = mid
    r0, r1 = samples[lo], samples[hi]
    frac = (distance - r0) / (r1 - r0) if r1 > r0 else 0.0
    index_f = lo + frac
    return (index_f / (count - 1)) * theta_max


def radial_forward(model, theta, coeffs, lut, theta_max):
    if model == "Perspective":
        return math.tan(theta)
    if model == "Stereographic":
        return math.tan(theta * 0.5) * 2.0
    if model == "Equidistant":
        return theta
    if model == "Equisolid":
        return math.sin(theta * 0.5) * 2.0
    if model == "Orthographic":
        return math.sin(theta)
    if model == "KannalaBrandt":
        return kb_polynomial(theta, coeffs)
    if model == "LUT1D":
        return lut1d_forward(theta, lut, theta_max)
    raise ValueError(model)


def radial_inverse(model, distance, coeffs, lut, theta_max):
    if model == "Perspective":
        return math.atan(distance)
    if model == "Stereographic":
        return math.atan(distance * 0.5) * 2.0
    if model == "Equidistant":
        return distance
    if model == "Equisolid":
        return math.asin(max(-1.0, min(distance * 0.5, 1.0))) * 2.0
    if model == "Orthographic":
        return math.asin(max(-1.0, min(distance, 1.0)))
    if model == "KannalaBrandt":
        return kb_solve_theta(distance, coeffs)
    if model == "LUT1D":
        return lut1d_inverse(distance, lut, theta_max)
    raise ValueError(model)


# ---------------------------------------------------------------------------
# Brown-Conrady: k1,k2,k3 radial + p1,p2 tangential (OpenCV convention),
# operating on normalized pinhole coords N = (X/Z, Y/Z).
# ---------------------------------------------------------------------------

def bc_eval_with_jacobian(xn, yn, coeffs):
    k1, k2, k3, p1, p2 = (list(coeffs) + [0.0] * 5)[:5]
    r2 = xn * xn + yn * yn
    r4 = r2 * r2
    radial = 1.0 + k1 * r2 + k2 * r4 + k3 * r4 * r2
    d_radial_d_r2 = k1 + 2.0 * k2 * r2 + 3.0 * k3 * r4

    xd = xn * radial + 2.0 * p1 * xn * yn + p2 * (r2 + 2.0 * xn * xn)
    yd = yn * radial + p1 * (r2 + 2.0 * yn * yn) + 2.0 * p2 * xn * yn

    d_radial_dxn = 2.0 * xn * d_radial_d_r2
    d_radial_dyn = 2.0 * yn * d_radial_d_r2

    j00 = radial + xn * d_radial_dxn + 2.0 * p1 * yn + 6.0 * p2 * xn
    j01 = xn * d_radial_dyn + 2.0 * p1 * xn + 2.0 * p2 * yn
    j10 = yn * d_radial_dxn + 2.0 * p1 * xn + 2.0 * p2 * yn
    j11 = radial + yn * d_radial_dyn + 6.0 * p1 * yn + 2.0 * p2 * xn

    return xd, yd, (j00, j01, j10, j11)


def bc_distort(xn, yn, coeffs):
    xd, yd, _ = bc_eval_with_jacobian(xn, yn, coeffs)
    return xd, yd


def bc_undistort(xd, yd, coeffs, iterations=10):
    xn, yn = xd, yd
    for _ in range(iterations):
        fx, fy, (j00, j01, j10, j11) = bc_eval_with_jacobian(xn, yn, coeffs)
        fx -= xd
        fy -= yd
        det = j00 * j11 - j01 * j10
        if abs(det) < 1e-12:
            break
        inv_det = 1.0 / det
        dxn = inv_det * (j11 * fx - j01 * fy)
        dyn = inv_det * (-j10 * fx + j00 * fy)
        xn -= dxn
        yn -= dyn
    return xn, yn


# ---------------------------------------------------------------------------
# FLensModelDescriptor-level forward/inverse (direction <-> normalized image point).
# ---------------------------------------------------------------------------

def normalize3(v):
    x, y, z = v
    n = math.sqrt(x * x + y * y + z * z)
    return (x / n, y / n, z / n)


def compute_image_point(desc, direction):
    x, y, z = normalize3(direction)
    if desc["model"] == "BrownConrady":
        zc = max(z, 1e-6)
        xn, yn = bc_distort(x / zc, y / zc, desc["coeffs"])
    else:
        theta = math.acos(max(-1.0, min(z, 1.0)))
        phi = math.atan2(y, x)
        r = radial_forward(desc["model"], theta, desc["coeffs"], desc["lut"], desc["theta_max"])
        xn, yn = r * math.cos(phi), r * math.sin(phi)
    return (desc["cx"] + desc["fx"] * xn, desc["cy"] + desc["fy"] * yn)


def compute_ray_direction(desc, image_point):
    xn = (image_point[0] - desc["cx"]) / desc["fx"]
    yn = (image_point[1] - desc["cy"]) / desc["fy"]
    if desc["model"] == "BrownConrady":
        px, py = bc_undistort(xn, yn, desc["coeffs"])
        return normalize3((px, py, 1.0))
    r = math.hypot(xn, yn)
    phi = math.atan2(yn, xn)
    theta = radial_inverse(desc["model"], r, desc["coeffs"], desc["lut"], desc["theta_max"])
    s = math.sin(theta)
    return (s * math.cos(phi), s * math.sin(phi), math.cos(theta))


# ---------------------------------------------------------------------------
# Convention adapters: lens camera space (+X right, +Y down, +Z forward) <->
# UE camera space (+X forward, +Y right, +Z up). Mirrors
# CameraModelUtil::LensDirToUE / UEDirToLens and LensModels.ush's
# LensDirToUE / UEDirToLens.
# ---------------------------------------------------------------------------

def lens_dir_to_ue(v):
    x, y, z = v
    return (z, x, -y)


def ue_dir_to_lens(v):
    x, y, z = v
    return (y, -z, x)


def vec_close(a, b, tol=TOLERANCE):
    return all(abs(ai - bi) < tol for ai, bi in zip(a, b))


def run_adapter_tests():
    failures = []

    # Basis-vector sanity: physical direction should be preserved (forward ->
    # forward, right -> right, down -> down/-up) across the convention swap.
    basis_checks = [
        ("lens forward -> UE forward", (0.0, 0.0, 1.0), (1.0, 0.0, 0.0)),
        ("lens right -> UE right", (1.0, 0.0, 0.0), (0.0, 1.0, 0.0)),
        ("lens down -> UE down (-Z)", (0.0, 1.0, 0.0), (0.0, 0.0, -1.0)),
    ]
    for label, lens_dir, expected_ue in basis_checks:
        ue_dir = lens_dir_to_ue(lens_dir)
        if not vec_close(ue_dir, expected_ue):
            failures.append(f"{label}: got {ue_dir}, expected {expected_ue}")

    # Inverse-of-each-other over both directional test sets.
    max_err_lens = 0.0
    max_err_ue = 0.0
    for lens_dir in make_test_directions():
        round_tripped = ue_dir_to_lens(lens_dir_to_ue(lens_dir))
        err = math.dist(round_tripped, lens_dir)
        max_err_lens = max(max_err_lens, err)
        if err >= TOLERANCE:
            failures.append(f"LensDirToUE/UEDirToLens not inverse for {lens_dir}: err={err:.3e}")

    # Reuse the same directions reinterpreted as UE-space test vectors.
    for ue_dir in make_test_directions():
        round_tripped = lens_dir_to_ue(ue_dir_to_lens(ue_dir))
        err = math.dist(round_tripped, ue_dir)
        max_err_ue = max(max_err_ue, err)
        if err >= TOLERANCE:
            failures.append(f"UEDirToLens/LensDirToUE not inverse for {ue_dir}: err={err:.3e}")

    return max(max_err_lens, max_err_ue), failures


# ---------------------------------------------------------------------------
# Round-trip tests.
# ---------------------------------------------------------------------------

def make_test_directions():
    dirs = []
    for theta_deg in (0.0, 5.0, 15.0, 30.0, 45.0, 60.0, 75.0):
        theta = math.radians(theta_deg)
        for phi_deg in (0.0, 37.0, 90.0, 143.0, 200.0, 271.0):
            phi = math.radians(phi_deg)
            s = math.sin(theta)
            dirs.append((s * math.cos(phi), s * math.sin(phi), math.cos(theta)))
    return dirs


def run_model(desc, max_theta_deg):
    failures = []
    max_err = 0.0
    for direction in make_test_directions():
        theta = math.acos(max(-1.0, min(direction[2], 1.0)))
        if math.degrees(theta) > max_theta_deg:
            continue

        image_point = compute_image_point(desc, direction)
        round_tripped_dir = compute_ray_direction(desc, image_point)
        round_tripped_point = compute_image_point(desc, round_tripped_dir)

        err = math.hypot(round_tripped_point[0] - image_point[0], round_tripped_point[1] - image_point[1])
        max_err = max(max_err, err)
        if err >= TOLERANCE:
            failures.append((direction, image_point, round_tripped_dir, err))

    return max_err, failures


def main():
    models = []

    models.append(("Perspective", {"model": "Perspective", "fx": 1.0, "fy": 1.0, "cx": 0.5, "cy": 0.5,
                                    "coeffs": [], "lut": [], "theta_max": math.pi / 2}, 75.0))
    models.append(("Stereographic", {"model": "Stereographic", "fx": 0.5, "fy": 0.5, "cx": 0.5, "cy": 0.5,
                                      "coeffs": [], "lut": [], "theta_max": math.pi / 2}, 89.0))
    models.append(("Equidistant", {"model": "Equidistant", "fx": 1.0, "fy": 1.0, "cx": 0.5, "cy": 0.5,
                                    "coeffs": [], "lut": [], "theta_max": math.pi / 2}, 89.0))
    models.append(("Equisolid", {"model": "Equisolid", "fx": 1.0, "fy": 1.0, "cx": 0.5, "cy": 0.5,
                                  "coeffs": [], "lut": [], "theta_max": math.pi / 2}, 89.0))
    models.append(("Orthographic", {"model": "Orthographic", "fx": 1.0, "fy": 1.0, "cx": 0.5, "cy": 0.5,
                                     "coeffs": [], "lut": [], "theta_max": math.pi / 2}, 75.0))
    models.append(("KannalaBrandt", {"model": "KannalaBrandt", "fx": 1.0, "fy": 1.0, "cx": 0.5, "cy": 0.5,
                                      "coeffs": [-0.01, 0.002, -0.0005, 0.0001], "lut": [], "theta_max": math.pi / 2}, 89.0))
    models.append(("BrownConrady", {"model": "BrownConrady", "fx": 0.8, "fy": 0.8, "cx": 0.5, "cy": 0.5,
                                     "coeffs": [-0.18, 0.03, -0.002, 0.004, -0.002], "lut": [], "theta_max": math.pi / 2}, 70.0))

    # LUT1D sampling a mild fisheye-like R(Theta) curve on [0, 90deg], 128 samples.
    lut_theta_max = math.radians(90.0)
    lut_samples = [math.tan(min(t, math.radians(85.0)) * 0.5) * 2.0
                   for t in [i / 127.0 * lut_theta_max for i in range(128)]]
    models.append(("LUT1D", {"model": "LUT1D", "fx": 0.5, "fy": 0.5, "cx": 0.5, "cy": 0.5,
                              "coeffs": [], "lut": lut_samples, "theta_max": lut_theta_max}, 85.0))

    overall_ok = True
    for name, desc, max_theta_deg in models:
        max_err, failures = run_model(desc, max_theta_deg)
        status = "OK" if not failures else "FAIL"
        if failures:
            overall_ok = False
        print(f"{name:15s} max round-trip error = {max_err:.3e}  [{status}]")
        for direction, image_point, round_tripped_dir, err in failures[:3]:
            print(f"    direction={direction} image_point={image_point} "
                  f"round_tripped_dir={round_tripped_dir} err={err:.3e}")

    adapter_max_err, adapter_failures = run_adapter_tests()
    adapter_status = "OK" if not adapter_failures else "FAIL"
    if adapter_failures:
        overall_ok = False
    print(f"{'ConventionAdapters':15s} max round-trip error = {adapter_max_err:.3e}  [{adapter_status}]")
    for failure in adapter_failures[:5]:
        print(f"    {failure}")

    print()
    if overall_ok:
        print(f"ALL MODELS PASS round-trip tolerance {TOLERANCE:.1e}")
        return 0
    else:
        print(f"FAILURES: one or more models exceeded round-trip tolerance {TOLERANCE:.1e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
