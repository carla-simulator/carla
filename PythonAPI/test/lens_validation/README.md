# lens_validation

Offline-first validation harness for `sensor.camera.rt_lens`, the
path-traced camera with exact lens models being built for CARLA UE5.8.

Contract under test (Blueprint attributes): `camera_model`
(`perspective|stereographic|equidistant|equisolid|orthographic|kannala_brandt|brown_conrady|lut`),
`distortion_coeffs`, `lut`, `fx`, `fy`, `cx`, `cy` (normalized), `theta_max_deg`,
`aperture_fstop`, `focus_distance_m`, `ca_shift_r`, `ca_shift_b`,
`samples_per_pixel`, `image_size_x`/`image_size_y`, `sensor_tick`, `fov`.
`sensor.camera.wide_angle` (cubemap resample) is the exact-distortion
reference used for A/B.

## Files

| File | What it is | Runs now? |
|---|---|---|
| `lens_math_reference.py` | Standalone numpy ground truth for every lens model: forward + inverse projection, `LensModel` class. Everything else is checked against this. | N/A (library) |
| `test_math_roundtrip.py` | Forward/inverse round-trip residuals per model; cross-checks Brown-Conrady against `cv2.projectPoints`/`undistortPoints` and Kannala-Brandt against `cv2.fisheye`. | **Yes** |
| `calibration_scene.py` | CARLA client script: spawns a checkerboard/ChArUco target, captures the rt_lens camera at N known poses. Its offline pattern-generation helpers run now; scene capture needs a server. | Partially |
| `test_calibration_refit.py` | Fits intrinsics/distortion from captured corner detections against `lens_math_reference`-derived ground truth, checks reprojection residual. Has a synthetic offline self-test of the fitting code itself. | Partially |
| `test_ab_wideangle.py` | `sensor.camera.wide_angle` vs `sensor.camera.rt_lens`, same scene/params: distortion field must match; DoF/CA must appear only in rt_lens. | No (server-dependent) |
| `straight_edge_test.py` | World-space straight line imaged through the lens model must match the analytic curve, not a vertex-displacement approximation. Has a synthetic offline self-test of the curve math. | Partially |
| `conftest.py` | Registers the shared `--host`/`--port`/`--capture-dir`/`--tolerance-px`/`--feature-tolerance-px`/`--curve-tolerance-px` pytest CLI options used by the server-dependent tests above. | N/A |

## Running the offline subset now

```bash
cd PythonAPI/test/lens_validation
python3 -m venv .venv && source .venv/bin/activate   # optional, a venv here is fine
pip install numpy opencv-python scipy pytest

pytest -k math -q                 # lens_math_reference round-trip + cv2 cross-checks
pytest -k synthetic -q            # calibration-refit and straight-edge self-tests
pytest . -q                       # everything: offline tests run, server-dependent ones skip cleanly
```

All of the above passes today with no CARLA server: **22 passed, 4 skipped**
(the 4 skips are exactly the tests that need a live rt_lens-capable server).

## Running the full suite later, against a running server

1. Build/start the CARLA server with the rt_lens migration (`sensor.camera.rt_lens`
   blueprint present) and make sure `carla` (the PythonAPI module) is importable
   (`PYTHONPATH` includes `PythonAPI/carla/dist/...` or the built egg/wheel).
2. Add the calibration-target and straight-edge assets the scripts expect
   (see the module docstrings in `calibration_scene.py` and
   `straight_edge_test.py` for exact blueprint ids and how to generate/import
   the pattern texture -- `calibration_scene.generate_checkerboard_texture()`
   and `generate_charuco_texture()` run offline today and produce the PNG an
   artist needs to turn into that target's material).
3. Capture calibration data:
   ```bash
   python calibration_scene.py --out-dir captures/brown_conrady \
       --camera-model brown_conrady --distortion-coeffs="-0.18,0.045,0.0008,-0.0006,-0.004" \
       --fx 0.9 --fy 0.85 --cx 0.5 --cy 0.5 --num-poses 20
   ```
4. Run everything, pointing at the server and the captures:
   ```bash
   pytest . --host 127.0.0.1 --port 2000 \
       --capture-dir captures/brown_conrady --tolerance-px 0.1 \
       --feature-tolerance-px 1.5 --curve-tolerance-px 0.5
   ```

## Tolerances chosen (and why)

- **Round-trip residual**: `< 1e-6` rad angular error, per the task spec, for
  every closed-form/Kannala-Brandt/Brown-Conrady model (Newton/fixed-point
  inverses easily clear this). The `lut` model uses `< 5e-5` rad since it is
  linear interpolation of a finite sample table, not an analytic inverse --
  tightened further by increasing `lut` sample density if needed.
- **cv2 cross-checks**: `atol=1e-6` on projected coordinates. Two convention
  notes that affect these tests specifically (not the contract itself):
  - `cv2.fisheye` fixes its polynomial's leading coefficient to 1
    (`theta_d = theta*(1+k1 th^2+...)`), while the contract's
    `r(theta) = k1*th + k2*th^3 + k3*th^5 + k4*th^7` leaves `k1` free. Setting
    `k1 = 1.0` recovers an exact analytic equivalence to `cv2.fisheye`'s
    `[k1, k2, k3, k4]` via `D = [k2, k3, k4, 0]`. This also means **`fx` and
    `k1` are not jointly identifiable from reprojection alone** -- the
    calibration refit fixes `k1` and only fits `fx, fy, cx, cy, k2, k3, k4`,
    mirroring how `cv2.fisheye.calibrate` itself works.
  - `cv2.fisheye`'s `theta` is derived as `atan(sqrt(x^2+y^2))` with
    `(x,y) = (X/Z, Y/Z)`, which only ranges over `[0, 90deg)` -- OpenCV's
    fisheye model cannot represent `theta >= 90deg` at all. The cv2
    cross-checks are restricted to `theta_max_deg <= 80` for this reason;
    the >90deg regime is still exercised by `test_math_roundtrip.py`'s
    pure self-consistency round-trip (no cv2 involved).
- **Calibration reprojection**: target **sub-0.1px RMS at 1080p**
  (`--tolerance-px`, default `0.1`), configurable per the task spec.
- **A/B feature-point deviation** (`--feature-tolerance-px`, default `1.5px`):
  looser than the calibration tolerance because it compares two different
  renderers (cubemap resample vs path-traced) sampling the same analytic
  field, not the field against itself.
- **Straight-edge curve deviation** (`--curve-tolerance-px`, default `0.5px`):
  tight enough to catch a vertex-displacement/mesh-warp approximation (which
  would show large, systematic deviations between mesh vertices) while
  tolerating ordinary pixel-detection noise.

## What remains blocked on a running server

- `calibration_scene.py`'s actual scene capture (`CalibrationScene.spawn_target`,
  `spawn_camera`, `capture_view`) -- needs `sensor.camera.rt_lens` to exist on
  the server and a calibration-target blueprint/asset to be added (see its
  docstring).
- `test_calibration_refit.py::test_refit_from_capture` -- needs real captures
  from the above.
- `test_ab_wideangle.py` (both tests) -- needs `sensor.camera.wide_angle` and
  `sensor.camera.rt_lens` both present, and depends on an **unverified
  assumption** that `wide_angle` exposes `fx`/`fy`/`cx`/`cy`/`distortion_coeffs`
  attributes compatible with the rt_lens contract (see `WIDE_ANGLE_ATTR_MAP`
  in that file -- update it once the engine side confirms the real names).
- `straight_edge_test.py::test_straight_edge_matches_analytic_curve` -- needs
  `sensor.camera.rt_lens` and a straight-edge prop asset (see its docstring).

Everything in this list is import-clean and argument-parsed today (verified
with `python3 -c "import <module>"` and `--help`); none of it can actually
execute until the migration build and assets land.
