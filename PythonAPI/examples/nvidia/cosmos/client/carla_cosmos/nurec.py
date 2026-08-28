"""NuRec scenes as Cosmos clips: a real place, a real rig, a real drive.

A NuRec artifact (``<uuid>.usdz``) is a Gaussian-splat reconstruction of twenty seconds of
real driving.  Everything this module needs is *inside* that zip:

============================  ==========================================================
``rig_trajectories.json``     the rig calibration (``T_sensor_rig`` per camera, f-theta
                              intrinsics) and the ego trajectory (``T_rig_worlds``)
``map.xodr``                  the OpenDRIVE map of the same streets
``data_info.json``            the sequence id the render service keys scenes by
``clipgt/*.parquet``          NVIDIA's own ground truth for the drive (not used here; it is
                              the format this package writes, and a useful cross-check)
``frames/<camera>/<ts>.jpeg`` one real frame per camera
============================  ==========================================================

What this buys over plain CARLA
-------------------------------
Two things, and it is worth being precise about which is which:

* **Photoreal RGB of a real place.**  The neural render is the RGB the derived controls
  (blur / vis / edge) are computed from, so a Transfer-2.5 or Cosmos-3 run conditioned on
  them is anchored to real photometry instead of to CARLA's renderer.
* **The world model of a real drive.**  The map is the real road network and the ego
  trajectory is a real vehicle's, so the ``wsm`` / ``hdmap_bbox`` control describes a place
  that exists.  This half needs no neural rendering at all — it is why ``--fake-nurec``
  is a genuinely useful mode rather than only a test harness.

Frames
------
The NuRec rig frame is right-handed **FLU** and its origin is the rear axle on the ground
(``rig_bbox`` puts the vehicle box at x in [-1.14, 4.07], z in [0, 1.82]) — which is exactly
the origin :mod:`carla_cosmos.scene` anchors the ClipGT world frame at.  ``T_rig_worlds`` is
``T_world_from_rig`` despite the name: the ego's velocity lies along ``R[:, 0]`` to a median
dot product of ``+1.0000`` over the whole trajectory, and it is ``inv(T)`` that does not.
Pose 0 is the identity, so the NuRec world frame is *already* anchored the way ClipGT wants.

Handedness changes go through :func:`flu_matrix_to_carla`, which is
``carla.Transform.from_right_handed`` — the same LibCarla boundary
``nurec/utils.mat_to_carla_transform`` uses, pinned to it by
``tests/test_nurec.py::test_matches_the_nurec_integration_conversion``.  Signs are never
derived from ``get_matrix()``.

Lens
----
The clip's calibration must describe the lens that produced the clip's **pixels**, and the
two available lenses are not the same:

``lens="ftheta"`` (default)
    The clip's RGB is a NuRec render through the sample's own measured f-theta polynomial,
    and the exported calibration carries that polynomial verbatim.  CARLA's depth/seg/semantic
    AOVs are pinhole and therefore *not* pixel-aligned with the RGB — they are still valid
    geometry for the occlusion filter, which is why :meth:`NurecCapture._exporter` hands the
    filter the pinhole manifests while the calibration table gets the f-theta ones.

``lens="pinhole"``
    The NRE is asked for an ``opencv_pinhole`` camera at the FOV derived from the same
    polynomial, so every AOV and the RGB share one camera model.  ``--fake-nurec`` forces
    this, because in that mode the pixels *are* CARLA's pinhole and a calibration claiming
    f-theta would be a lie the preview would faithfully reproduce.

What the NRE does and does not give us
--------------------------------------
``RGBRenderReturn`` has exactly one field, ``image_bytes``.  There is no depth, semantic or
instance buffer in the service, so every AOV other than RGB comes from CARLA's own sensors on
the OpenDRIVE proxy world.  That proxy carries the road mesh plus CARLA's procedural roadside
dressing (verges, trees, street lights) but **no buildings** and none of the real scene's
geometry, so its depth and segmentation describe the road layout and the spawned actors -- not
the place the neural render shows.

The NRE *does* composite dynamic actors, but only tracks the artifact marks ``CONTROLLABLE``
— it re-poses objects the reconstruction already contains.  A freshly spawned CARLA vehicle
has no track and is invisible to it.  So with ``actors="carla"`` the traffic appears in the
ClipGT obstacles and in CARLA's AOVs but **not** in the neural RGB; :func:`capture` warns
about exactly that when it is rendering for real.
"""

from __future__ import annotations

import json
import logging
import math
import zipfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, Iterable, Sequence

import numpy as np
from scipy.spatial.transform import Rotation, Slerp

import carla

from . import coords, scene as scene_mod
from .capture import Capture, _CameraStreams
from .clip import SCENE_DIR, Clip
from .contracts import (AV_CAMERAS, BackendContract, CameraManifest, FThetaModel,
                        canonical_camera_name)
from .rig import _NVIDIA_AV7, Camera, MountedCamera, Rig

log = logging.getLogger(__name__)

USDZ_RIG = "rig_trajectories.json"
USDZ_INFO = "data_info.json"
USDZ_MAP = "map.xodr"
USDZ_FRAMES = "frames"

LENSES = ("ftheta", "pinhole")

#: RDF optical axes (x right, y down, z forward) expressed in the FLU body frame.  A
#: ``T_sensor_rig`` rotation block is ``R_body @ OPTICAL_TO_FLU``; this is the ``A`` matrix of
#: ``nurec/nurec_runner.make_transform_matrix``, and inverting it recovers the body rotation
#: that :func:`carla_cosmos.coords.flu_pose_matrix` composes from roll-pitch-yaw.
OPTICAL_TO_FLU = np.array([[0.0, 0.0, 1.0],
                           [-1.0, 0.0, 0.0],
                           [0.0, -1.0, 0.0]])

#: ENU (East, North, Up) -> CARLA (x, -y, z).  The OpenDRIVE world CARLA generates from the
#: sample's ``map.xodr`` is in the map's ENU frame, which is right-handed like the rig frame.
_ENU_TO_CARLA_IS_THE_RH_BOUNDARY = True


# ----------------------------------------------------------------------------- handedness

def flu_matrix_to_carla(m_flu: np.ndarray) -> carla.Transform:
    """A right-handed FLU 4x4 pose -> :class:`carla.Transform`, through LibCarla's boundary.

    Identical to ``nurec/utils.mat_to_carla_transform``; kept here so ``carla_cosmos`` does not
    import a sibling demo package, and pinned to it by the test suite.  The rig frame composes
    ``Rz(yaw) @ Ry(pitch) @ Rx(roll)`` — scipy *extrinsic* ``"xyz"`` — which is precisely
    :class:`carla.RightHandedTransform`; ``from_right_handed`` applies the ``(x, -y, z)`` /
    ``(roll, -pitch, -yaw)`` mapping inside the engine so it is not duplicated here.
    """
    _require_fixed_wheel()
    m = np.asarray(m_flu, dtype=np.float64)
    roll, pitch, yaw = Rotation.from_matrix(m[:3, :3]).as_euler("xyz", degrees=True)
    return carla.Transform.from_right_handed(carla.RightHandedTransform(
        carla.RightHandedVector3D(x=float(m[0, 3]), y=float(m[1, 3]), z=float(m[2, 3])),
        carla.RightHandedRotation(roll=float(roll), pitch=float(pitch), yaw=float(yaw)),
    ))


def _require_fixed_wheel() -> None:
    """Refuse to build poses on a wheel whose geom maths is still mirrored in pitch and roll."""
    if hasattr(carla.Transform, "from_right_handed"):
        return
    probe = np.array(carla.Transform(carla.Location(),
                                     carla.Rotation(pitch=20.0)).get_matrix())[2][0]
    raise RuntimeError(
        "This carla wheel predates the geom pitch/roll fix (carla.Transform.from_right_handed "
        f"is missing; Rotation(pitch=20).get_matrix()[2][0] = {probe:+.3f}, expected +0.342). "
        "Every pitched or rolled NuRec camera would be mounted at twice the angle away from "
        "the truth.  Rebuild the PythonAPI wheel from a branch carrying the geom fix.")


# ----------------------------------------------------------------------------- the rig

def _poly_angle(poly: Sequence[float], radius: float) -> float:
    """Evaluate a ``pixeldistance-to-angle`` polynomial (lowest order first) at ``radius``."""
    return float(np.polyval(np.asarray(poly, dtype=np.float64)[::-1], radius))


@dataclass(frozen=True)
class NurecCamera:
    """One camera of a NuRec rig, as calibrated in ``rig_trajectories.json``."""

    name: str
    """Logical sensor name, underscore form (``camera_front_wide_120fov``)."""
    t_flu: tuple[float, float, float]
    """Mount position in the rig FLU frame, metres from the rear axle on the ground."""
    rpy_flu: tuple[float, float, float]
    """Body roll-pitch-yaw in degrees, FLU, extrinsic ``xyz`` — the optical basis removed."""
    width: int
    height: int
    cx: float
    cy: float
    poly: tuple[float, ...]
    """``pixeldistance-to-angle`` coefficients, lowest order first."""
    max_angle: float
    """Largest angle the lens model is valid to, radians."""
    t_sensor_rig: np.ndarray = field(repr=False)
    """The calibration's own 4x4, verbatim (optical -> rig)."""

    # -------------------------------------------------------------- derived
    @property
    def cosmos_name(self) -> str:
        """Colon form the ClipGT loader and the AV backend key views by."""
        return self.name.replace("_", ":", 3) if self.name.startswith("camera_") else self.name

    def hfov(self) -> float:
        """Pinhole FOV covering the same image width, degrees.

        The angle the lens maps the half-width to, doubled.  On NVIDIA's own rigs this
        reproduces the number in the camera's name — 119.8 for the ``120fov`` cameras, 69.8
        for the ``70fov`` ones, 29.8 for ``30fov`` — which is the check that the polynomial is
        being read in the right units and the right order.
        """
        return 2.0 * math.degrees(_poly_angle(self.poly, self.width / 2.0))

    def flu_pose(self) -> np.ndarray:
        """4x4 FLU body pose of the mount, rear-axle origin."""
        return coords.flu_pose_matrix(self.t_flu, self.rpy_flu)

    def scaled(self, width: int, height: int) -> "NurecCamera":
        """The same lens resampled to ``width`` x ``height``.

        A pixel radius scales by ``s``, so a coefficient of order ``k`` scales by ``s**-k``
        and the principal point by ``s``.  The aspect ratio must be preserved: a different one
        would need a crop, and silently letterboxing a calibration is how boxes end up half a
        car off.
        """
        if abs(width / height - self.width / self.height) > 1e-3:
            raise ValueError(
                f"{self.name}: cannot rescale {self.width}x{self.height} (aspect "
                f"{self.width / self.height:.4f}) to {width}x{height} (aspect "
                f"{width / height:.4f}) without cropping")
        s = width / self.width
        return NurecCamera(
            name=self.name, t_flu=self.t_flu, rpy_flu=self.rpy_flu,
            width=width, height=height, cx=self.cx * s, cy=self.cy * s,
            poly=tuple(c / s ** k for k, c in enumerate(self.poly)),
            max_angle=self.max_angle, t_sensor_rig=self.t_sensor_rig)

    def ftheta_model(self) -> FThetaModel:
        """This lens as the calibration table carries it."""
        return FThetaModel(cx=self.cx, cy=self.cy, poly=list(self.poly),
                           max_angle=self.max_angle)

    def cosmos_camera(self, lens: str = "ftheta") -> Camera:
        """This camera as a :class:`carla_cosmos.rig.Camera`.

        ``hfov`` is always the derived pinhole FOV — it is what CARLA's own AOV sensors are
        spawned with either way.  Under ``lens="ftheta"`` the measured polynomial rides along
        and becomes the exported calibration; under ``lens="pinhole"`` it does not, and the
        calibration is fitted from ``hfov`` exactly as for a native CARLA clip.
        """
        if lens not in LENSES:
            raise ValueError(f"lens must be one of {LENSES}, not '{lens}'")
        return Camera(name=self.cosmos_name, hfov=self.hfov(), t=self.t_flu, rpy=self.rpy_flu,
                      width=self.width, height=self.height, lens=lens,
                      ftheta=self.ftheta_model() if lens == "ftheta" else None)

    def nre_camera_params(self, lens: str = "ftheta") -> dict:
        """Parameters for ``nurec_integration.dict_to_camera_spec``."""
        common = {"logical_id": self.name, "resolution_w": self.width,
                  "resolution_h": self.height, "shutter_type": "ROLLING_TOP_TO_BOTTOM"}
        if lens == "ftheta":
            return {**common, "camera_type": "ftheta", "reference_poly": 1,
                    "principal_point_x": self.cx, "principal_point_y": self.cy,
                    "pixeldist_to_angle_poly": list(self.poly),
                    "angle_to_pixeldist_poly": [], "max_angle": self.max_angle}
        f = self.width / 2.0 / math.tan(math.radians(self.hfov()) / 2.0)
        return {**common, "camera_type": "opencv_pinhole", "focal_length_x": f,
                "focal_length_y": f, "principal_point_x": self.width / 2.0,
                "principal_point_y": self.height / 2.0, "radial_coeffs": []}


def _camera_from_calibration(calib: dict) -> NurecCamera:
    """Build a :class:`NurecCamera` from one ``camera_calibrations`` entry."""
    model = calib["camera_model"]
    if model["type"] != "ftheta":
        raise ValueError(f"{calib['logical_sensor_name']}: unsupported camera model "
                         f"'{model['type']}' (only ftheta calibrations are shipped)")
    p = model["parameters"]
    t = np.asarray(calib["T_sensor_rig"], dtype=np.float64)
    # The rotation maps OpenCV RDF optical axes into the FLU body frame; strip that basis to
    # recover the body rotation carla_cosmos composes from roll-pitch-yaw.
    body = t[:3, :3] @ OPTICAL_TO_FLU.T
    roll, pitch, yaw = Rotation.from_matrix(body).as_euler("xyz", degrees=True)
    width, height = (int(v) for v in p["resolution"])
    cx, cy = (float(v) for v in p["principal_point"])
    return NurecCamera(
        name=calib["logical_sensor_name"],
        t_flu=(float(t[0, 3]), float(t[1, 3]), float(t[2, 3])),
        rpy_flu=(float(roll), float(pitch), float(yaw)),
        width=width, height=height, cx=cx, cy=cy,
        poly=tuple(float(c) for c in p["pixeldist_to_angle_poly"]),
        max_angle=float(p["max_angle"]), t_sensor_rig=t)


# ----------------------------------------------------------------------------- the sample

@dataclass
class NurecSample:
    """A NuRec artifact opened for capture: rig, trajectory and map."""

    path: Path
    scene_id: str
    cameras: dict[str, NurecCamera]
    poses: np.ndarray
    """``(N, 4, 4)`` ``T_world_from_rig``, FLU, pose 0 the identity."""
    timestamps_us: np.ndarray
    """``(N,)`` int64, the rig-pose timestamps."""
    xodr: str
    t_world_base: np.ndarray = field(repr=False)
    """``T_world_from_ecef`` of the reconstruction, as shipped."""

    # -------------------------------------------------------------- loading
    @classmethod
    def load(cls, usdz: str | Path) -> "NurecSample":
        """Open ``<uuid>.usdz`` and read the rig, the trajectory and the map out of it."""
        path = Path(usdz)
        if not path.is_file():
            raise FileNotFoundError(f"no NuRec artifact at {path}")
        with zipfile.ZipFile(path) as z:
            names = set(z.namelist())
            missing = {USDZ_RIG, USDZ_INFO, USDZ_MAP} - names
            if missing:
                raise ValueError(f"{path.name} is missing {sorted(missing)} — not a NuRec "
                                 f"clipgt artifact?")
            rig = json.loads(z.read(USDZ_RIG))
            info = json.loads(z.read(USDZ_INFO))
            xodr = z.read(USDZ_MAP).decode()
        cameras = {}
        for calib in rig["camera_calibrations"].values():
            cam = _camera_from_calibration(calib)
            cameras[cam.name] = cam
        traj = rig["rig_trajectories"][0]
        return cls(path=path, scene_id=info["sequence_id"], cameras=cameras,
                   poses=np.asarray(traj["T_rig_worlds"], dtype=np.float64),
                   timestamps_us=np.asarray(traj["T_rig_world_timestamps_us"], dtype=np.int64),
                   xodr=xodr, t_world_base=np.asarray(rig["T_world_base"], dtype=np.float64))

    def recorded_frame(self, camera: str) -> tuple[int, bytes] | None:
        """One real frame for ``camera`` out of the artifact, as ``(timestamp_us, jpeg)``.

        Every shipped sample carries a single frame per camera under ``frames/``.  It is the
        only genuine NuRec-scene image available without the render service, which makes it
        the fixture the real frame-substitution path is tested against.
        """
        with zipfile.ZipFile(self.path) as z:
            entries = sorted(n for n in z.namelist()
                             if n.startswith(f"{USDZ_FRAMES}/{camera}/") and n.endswith(".jpeg"))
            if not entries:
                return None
            return int(Path(entries[0]).stem), z.read(entries[0])

    # -------------------------------------------------------------- trajectory
    @property
    def duration_s(self) -> float:
        """Length of the recorded drive in seconds."""
        return float(self.timestamps_us[-1] - self.timestamps_us[0]) / 1e6

    def pose_at(self, t_us: float) -> np.ndarray:
        """The rig pose at ``t_us``: linear in position, SLERP in rotation, clamped at the ends."""
        return self.poses_at(np.asarray([t_us], dtype=np.float64))[0]

    def poses_at(self, t_us: np.ndarray) -> np.ndarray:
        """Vectorised :meth:`pose_at` for a whole clip's worth of timestamps."""
        ts = np.clip(np.asarray(t_us, dtype=np.float64),
                     self.timestamps_us[0], self.timestamps_us[-1])
        out = np.tile(np.eye(4), (len(ts), 1, 1))
        src = self.timestamps_us.astype(np.float64)
        for axis in range(3):
            out[:, axis, 3] = np.interp(ts, src, self.poses[:, axis, 3])
        slerp = Slerp(src, Rotation.from_matrix(self.poses[:, :3, :3]))
        out[:, :3, :3] = slerp(ts).as_matrix()
        return out

    def clip_timestamps(self, frames: int, fps: int, start_s: float = 0.0) -> np.ndarray:
        """The ``frames`` sample timestamps a clip of ``fps`` starting at ``start_s`` replays."""
        t0 = float(self.timestamps_us[0]) + start_s * 1e6
        step = 1e6 / fps
        end = float(self.timestamps_us[-1])
        last = t0 + (frames - 1) * step
        if last > end + 1e-6:
            raise ValueError(
                f"{self.path.name} is {self.duration_s:.1f}s long; {frames} frames at {fps} fps "
                f"from t+{start_s:.1f}s needs {(frames - 1) / fps + start_s:.1f}s")
        return t0 + np.arange(frames, dtype=np.float64) * step

    # -------------------------------------------------------------- world alignment
    def t_scenario_carla(self) -> np.ndarray:
        """``T_enu_from_nurec``: the reconstruction's world frame in the OpenDRIVE map's ENU.

        Delegates to ``nurec/projection_functions.get_t_rig_enu_from_ecef``, which converts the
        artifact's ECEF anchor into the ENU frame the ``map.xodr`` ``geoReference`` declares —
        the same call ``NurecScenario.__enter__`` makes, so a clip captured here lands where
        the NuRec integration would put it.  Returns the identity when the map carries no
        georeference, in which case the trajectory is replayed in the map's own frame.
        """
        from ._nurec_vendor import get_t_rig_enu_from_ecef
        return np.asarray(get_t_rig_enu_from_ecef(self.t_world_base, self.xodr),
                          dtype=np.float64)

    def ego_transforms(self, t_us: np.ndarray) -> list[carla.Transform]:
        """Rig-origin poses as CARLA transforms, in the OpenDRIVE world's frame."""
        t_enu = self.t_scenario_carla()
        return [flu_matrix_to_carla(t_enu @ m) for m in self.poses_at(t_us)]

    # -------------------------------------------------------------- rig
    def rig(self, width: int = 1280, height: int = 720, cameras: Sequence[str] | None = None,
            lens: str = "ftheta", complete_av7: bool = False) -> Rig:
        """This sample's rig as a :class:`carla_cosmos.rig.Rig`.

        ``mount="exact"``: these are surveyed mounts on a real vehicle expressed against the
        same rear-axle origin the mounting rule exists to protect, so moving them to the CARLA
        ego's roofline would throw the calibration away.

        ``complete_av7`` appends the AV-7 cameras the sample does not have, taken from NVIDIA's
        nominal RDS-HQ table.  The shipped 26.04 samples carry **six** of the seven — every one
        but ``camera:rear:tele:30fov`` — and the AV backend keys its views by name, so a
        seven-view request needs the seventh from somewhere.  Those cameras are nominal, not
        measured: they get no ``ftheta`` model and are reported by :meth:`nominal_cameras`.
        """
        names = list(cameras) if cameras is not None else self.camera_order()
        unknown = [n for n in names if n not in self.cameras]
        if unknown:
            raise KeyError(f"{self.path.name} has no camera {unknown}; available: "
                           f"{sorted(self.cameras)}")
        cams = [self.cameras[n].scaled(width, height).cosmos_camera(lens) for n in names]
        if complete_av7:
            have = {c.name for c in cams}
            for slot in AV_CAMERAS:
                if slot in have:
                    continue
                t, rpy, hfov = _NVIDIA_AV7[slot]
                log.warning("%s has no %s; filling the AV-7 slot from NVIDIA's nominal rig "
                            "(this camera is not calibrated for this scene)",
                            self.path.name, slot)
                cams.append(Camera(slot, hfov, t, rpy, width, height, lens="pinhole"))
            cams.sort(key=lambda c: AV_CAMERAS.index(c.name) if c.name in AV_CAMERAS else 99)
        return Rig(name=f"nurec_{self.scene_id[-12:]}", cameras=cams, mount="exact")

    def camera_order(self) -> list[str]:
        """Sample cameras ordered by their AV-7 slot, so view ``0`` is the front wide."""
        def key(name: str) -> tuple[int, str]:
            colon = name.replace("_", ":", 3)
            return (AV_CAMERAS.index(colon) if colon in AV_CAMERAS else 99, name)
        return sorted(self.cameras, key=key)

    def nominal_cameras(self, rig: Rig) -> list[str]:
        """Cameras of ``rig`` that this sample does not actually calibrate."""
        return [c.name for c in rig.cameras if canonical_camera_name(c.name) not in self.cameras]


# ----------------------------------------------------------------------------- tick source

class TrajectoryTicks:
    """Tick the world with the ego teleported along a recorded trajectory.

    The ego carries no physics: each clip frame it is placed so that its **rear axle** sits on
    the recorded rig pose, which is what makes the exported ClipGT world frame coincide with
    the NuRec world frame (:mod:`carla_cosmos.scene` anchors at
    ``ego_transform @ rear_axle_offset``).  :class:`~carla_cosmos.capture.Capture` calls
    :meth:`prepare_frame` before each clip frame and never during the pre-roll, so the warm-up
    ticks happen at the start pose instead of eating the first second of the drive.
    """

    def __init__(self, world: carla.World, ego: carla.Vehicle,
                 rig_poses: Sequence[carla.Transform], axle_local_ue: np.ndarray | None = None,
                 z_offset: float = 0.0) -> None:
        self.world = world
        self.ego = ego
        self.rig_poses = list(rig_poses)
        self.z_offset = z_offset
        self.axle_local_ue = np.zeros(3) if axle_local_ue is None else np.asarray(axle_local_ue)
        self.commanded: list[np.ndarray] = []
        """``T_world_from_rig`` (UE, metres) actually commanded, one per clip frame."""

    def __enter__(self) -> "TrajectoryTicks":
        return self

    def __exit__(self, *exc) -> None:
        return None

    def set_axle(self, axle_local_ue: np.ndarray) -> None:
        """Adopt the rear-axle offset measured by the capture."""
        self.axle_local_ue = np.asarray(axle_local_ue, dtype=np.float64)

    def _actor_matrix(self, rig_pose: carla.Transform) -> np.ndarray:
        m_rig = coords.ue_matrix(rig_pose)
        m_rig[2, 3] += self.z_offset
        t_axle = np.eye(4)
        t_axle[:3, 3] = self.axle_local_ue
        return m_rig @ np.linalg.inv(t_axle)

    def place(self, index: int) -> None:
        """Teleport the ego so its rear axle sits on rig pose ``index``."""
        pose = self.rig_poses[min(index, len(self.rig_poses) - 1)]
        m_actor = self._actor_matrix(pose)
        self.ego.set_transform(coords.ue_transform_from_matrix(m_actor))

    def prepare_frame(self, index: int) -> None:
        """Place the ego for clip frame ``index`` (called by :class:`Capture` before ticking)."""
        self.place(index)
        m_rig = self._actor_matrix(self.rig_poses[min(index, len(self.rig_poses) - 1)])
        t_axle = np.eye(4)
        t_axle[:3, 3] = self.axle_local_ue
        self.commanded.append(m_rig @ t_axle)

    def tick(self) -> int:
        """Advance one frame; returns the frame id."""
        return self.world.tick()


# ----------------------------------------------------------------------------- rendering

class NreRenderer:
    """Neural renders from a NuRec Render Engine, over gRPC.

    The endpoint is a plain ``host:port`` — the NRE serves insecure gRPC and its container runs
    with ``--net=host``, so there is no scheme and no TLS.  ``NurecScenario`` in the NuRec
    integration hardcodes ``localhost`` and launches Docker itself; this class deliberately does
    neither, so the engine can live on a GPU node while CARLA runs here.

    The NuRec package is imported lazily and only in this class: ``--fake-nurec`` must work with
    no gRPC stubs, no artifact loaded into an engine and no NVIDIA container anywhere.
    """

    def __init__(self, sample: NurecSample, endpoint: str, rig: Rig, lens: str = "ftheta",
                 timeout: float = 120.0) -> None:
        from ._nurec_vendor import camera_spec_from_dict, render_stub

        self.sample = sample
        self.endpoint = endpoint
        self.lens = lens
        self.timeout = timeout
        self.t_carla_nurec = np.linalg.inv(sample.t_scenario_carla())
        self.stub, self._channel = render_stub(endpoint)
        self.specs = {}
        for cam in rig.cameras:
            key = canonical_camera_name(cam.name)
            nurec_cam = sample.cameras.get(key)
            if nurec_cam is None:
                raise KeyError(
                    f"camera '{cam.name}' is not calibrated in {sample.path.name}; a nominal "
                    f"AV-7 slot cannot be neurally rendered — drop it or use --fake-nurec")
            scaled = nurec_cam.scaled(cam.width, cam.height)
            self.specs[cam.name] = camera_spec_from_dict(scaled.nre_camera_params(lens))

    def close(self) -> None:
        """Close the gRPC channel."""
        self._channel.close()

    def render(self, camera: str, world_pose_ue: np.ndarray, timestamp_us: int) -> np.ndarray:
        """One neural frame: ``(H, W, 3)`` uint8 RGB.

        ``world_pose_ue`` is the camera's pose in the CARLA world as a 4x4 UE matrix — taken
        from the sensor CARLA actually spawned, so the neural render and the AOVs cannot drift
        apart no matter how the mounting resolved.
        """
        from ._nurec_vendor import build_render_request, planar_format

        spec = self.specs[camera]
        pose_nurec = self.t_carla_nurec @ coords.ue_to_flu(np.asarray(world_pose_ue))
        request = build_render_request(
            scene_id=self.sample.scene_id, camera_spec=spec, camera_pose=pose_nurec,
            timestamp=int(timestamp_us), image_format=planar_format())
        response = self.stub.render_rgb(request, timeout=self.timeout)
        h, w = request.resolution_h, request.resolution_w
        return np.frombuffer(response.image_bytes, dtype=np.uint8).reshape(3, h, w).transpose(1, 2, 0).copy()


RenderFn = Callable[[str, int, np.ndarray, int], np.ndarray]
"""``(camera_name, frame_index, camera_world_pose_ue, timestamp_us) -> (H, W, 3) uint8 RGB``."""


# ----------------------------------------------------------------------------- capture

class NurecCapture(Capture):
    """A :class:`~carla_cosmos.capture.Capture` whose RGB comes from a NuRec render.

    Everything else — the AOVs, the ClipGT export, the occlusion filter, the clip package — is
    the base class unchanged.  Two things differ:

    * :meth:`_rgb_frame` substitutes the neural render (still consuming CARLA's own image, so
      the frame-exact queue matching keeps working and ``--fake-nurec`` is one branch away);
    * :meth:`_exporter` gives the occlusion filter the **pinhole** camera manifests, because
      that is the lens that produced the depth AOV it z-tests against, while the calibration
      table written by ``exporter.write`` keeps the measured f-theta.
    """

    def __init__(self, *args, render: RenderFn | None = None,
                 timestamps_us: Sequence[int] | None = None, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.render = render
        self.timestamps_us = list(timestamps_us) if timestamps_us is not None else []
        self.rendered = 0

    def _exporter(self, clip_id: str, axle: np.ndarray,
                  mounted: list[MountedCamera]) -> scene_mod.SceneExporter:
        return scene_mod.SceneExporter(
            self.world, clip_id, self.ego, axle,
            cameras=[_pinhole_manifest(m) for m in mounted], visibility=self.visibility)

    def _rgb_frame(self, s: _CameraStreams, index: int, fid: int, first: bool) -> np.ndarray:
        # Always drain CARLA's image: the queues are matched frame by frame and skipping one
        # would desync every frame after it, whether or not the pixels get used.
        carla_rgb = super()._rgb_frame(s, index, fid, first)
        if self.render is None:
            return carla_rgb
        cam = s.mounted.camera
        sensor = s.sensors["rgb"]
        pose = coords.ue_matrix(sensor.get_transform())
        ts = self.timestamps_us[index] if index < len(self.timestamps_us) else index
        frame = np.asarray(self.render(cam.name, index, pose, int(ts)))
        if frame.shape != (cam.height, cam.width, 3) or frame.dtype != np.uint8:
            raise ValueError(
                f"{cam.name} frame {index}: render returned {frame.shape} {frame.dtype}, "
                f"expected ({cam.height}, {cam.width}, 3) uint8")
        self.rendered += 1
        return frame


def _pinhole_manifest(m: MountedCamera) -> CameraManifest:
    """``m`` as a manifest describing CARLA's pinhole sensor rather than the measured lens."""
    return m.manifest().model_copy(update={"ftheta": None})


# ----------------------------------------------------------------------------- alignment

@dataclass
class Alignment:
    """How far the exported ClipGT ego track sits from the trajectory that was replayed."""

    frames: int
    max_translation_m: float
    max_rotation_deg: float
    timestamps_us: list[int]

    @property
    def ok(self) -> bool:
        """Whether the two agree to within the tolerance :func:`check_alignment` enforces."""
        return self.max_translation_m <= TRANSLATION_TOL_M and self.max_rotation_deg <= ROTATION_TOL_DEG

    def as_dict(self) -> dict:
        """JSON form for the clip's sidecar."""
        return {"frames": self.frames,
                "max_translation_m": self.max_translation_m,
                "max_rotation_deg": self.max_rotation_deg,
                "translation_tol_m": TRANSLATION_TOL_M,
                "rotation_tol_deg": ROTATION_TOL_DEG,
                "ok": self.ok}


TRANSLATION_TOL_M = 0.02
ROTATION_TOL_DEG = 0.05


def check_alignment(clip: Clip, commanded_ue: Sequence[np.ndarray],
                    timestamps_us: Sequence[int]) -> Alignment:
    """Assert the ClipGT ego track *is* the replayed trajectory, frame for frame.

    The neural render is posed from the CARLA sensor transforms and the ClipGT egomotion is
    exported from the CARLA ego pose, so the two can only disagree if the ego did not go where
    it was told — teleporting fights nothing here, but physics, a collision or an off-by-one in
    the frame indexing all would, and every one of them would put the boxes on the wrong pixels
    while leaving both files individually plausible.  This is the check that catches it.

    Compares in the ClipGT world frame: both sides are anchored at their own first pose, so a
    constant offset cancels and only genuine frame-to-frame drift is measured.
    """
    import pyarrow.parquet as pq

    scene_dir = clip.scene_dir
    if scene_dir is None:
        raise ValueError(f"{clip.path} has no scene package to check")
    rows = pq.read_table(scene_dir / f"{clip.manifest.clip_id}.egomotion_estimate.parquet").to_pylist()
    rows.sort(key=lambda r: r["key"]["timestamp_micros"])
    if len(rows) != len(commanded_ue):
        raise AssertionError(f"egomotion has {len(rows)} rows, {len(commanded_ue)} frames were "
                             f"commanded — the export and the replay disagree on frame count")
    exported = np.tile(np.eye(4), (len(rows), 1, 1))
    for i, row in enumerate(rows):
        e = row["egomotion_estimate"]
        q, loc = e["orientation"], e["location"]
        exported[i, :3, :3] = Rotation.from_quat([q["x"], q["y"], q["z"], q["w"]]).as_matrix()
        exported[i, :3, 3] = [loc["x"], loc["y"], loc["z"]]
    commanded = np.asarray([coords.ue_to_flu(np.asarray(m)) for m in commanded_ue])
    ref_c, ref_e = np.linalg.inv(commanded[0]), np.linalg.inv(exported[0])
    dt = dr = 0.0
    for i in range(len(rows)):
        delta = np.linalg.inv(ref_e @ exported[i]) @ (ref_c @ commanded[i])
        dt = max(dt, float(np.linalg.norm(delta[:3, 3])))
        dr = max(dr, float(Rotation.from_matrix(delta[:3, :3]).magnitude() * 180.0 / math.pi))
    return Alignment(frames=len(rows), max_translation_m=dt, max_rotation_deg=dr,
                     timestamps_us=[int(t) for t in timestamps_us])


# ----------------------------------------------------------------------------- world setup

OPENDRIVE_PARAMS = dict(vertex_distance=2.0, max_road_length=500.0, wall_height=0.0,
                        additional_width=0.6, smooth_junctions=True, enable_mesh_visibility=True)
"""Mesh generation for the sample's ``map.xodr``, matching ``NurecScenario.__enter__``."""

EGO_BLUEPRINT = "vehicle.lincoln.mkz"


@dataclass
class WorldGuard:
    """Restores the map and the world settings a capture borrowed.

    The local server is long-lived and shared, so loading a NuRec map is a *borrow*: the map it
    was on and its synchronous-mode settings come back on the way out, whether the capture
    succeeded or raised.
    """

    client: carla.Client
    original_map: str
    original_settings: carla.WorldSettings
    actors: list[carla.Actor] = field(default_factory=list)

    def restore(self, reload_map: bool = True) -> None:
        """Destroy everything spawned, put the original map back and restore async mode."""
        for actor in self.actors:
            try:
                actor.destroy()
            except RuntimeError as exc:
                log.warning("actor cleanup failed: %s", exc)
        self.actors.clear()
        if reload_map:
            log.info("restoring map %s", self.original_map)
            world = self.client.load_world(self.original_map)
        else:
            world = self.client.get_world()
        settings = world.get_settings()
        settings.synchronous_mode = self.original_settings.synchronous_mode
        settings.fixed_delta_seconds = self.original_settings.fixed_delta_seconds
        world.apply_settings(settings)
        log.info("world settings restored (sync=%s)", settings.synchronous_mode)


def load_map(client: carla.Client, sample: NurecSample,
             weather: str = "ClearNoon") -> tuple[carla.World, WorldGuard]:
    """Generate the sample's OpenDRIVE world, remembering what to put back.

    An OpenDRIVE world is the road network plus CARLA's procedural roadside dressing (verges,
    trees, street lights); there are no buildings and nothing of the real scene's own geometry.
    So CARLA's depth and segmentation describe the road layout and the actors on it, which is
    the honest ceiling on what the CARLA side contributes -- and why the neural RGB is the
    interesting half.

    The generated world comes up with a low sun (altitude 10 deg), which is dark enough to
    matter for anything derived from CARLA's pixels, so ``weather`` is applied explicitly.
    """
    world = client.get_world()
    guard = WorldGuard(client=client, original_map=world.get_map().name.split("/")[-1],
                       original_settings=world.get_settings())
    log.info("generating the OpenDRIVE world of %s (leaving %s)", sample.scene_id, guard.original_map)
    world = client.generate_opendrive_world(
        sample.xodr, carla.OpendriveGenerationParameters(**OPENDRIVE_PARAMS))
    if weather:
        preset = getattr(carla.WeatherParameters, weather, None)
        if preset is None:
            raise ValueError(f"unknown weather preset '{weather}'")
        world.set_weather(preset)
        log.info("weather set to %s (sun altitude %.0f deg)", weather, preset.sun_altitude_angle)
    return world, guard


def spawn_ego(world: carla.World, first_pose: carla.Transform,
              blueprint: str = EGO_BLUEPRINT) -> carla.Vehicle:
    """The replay ego: a vehicle with physics off, to be teleported frame by frame.

    Physics would fight the teleport (and the OpenDRIVE mesh has no collision geometry worth
    landing on), and the trajectory being replayed already is a real vehicle's, so simulating
    one on top of it would only add error.
    """
    bp = world.get_blueprint_library().find(blueprint)
    bp.set_attribute("role_name", "hero")
    spawn = carla.Transform(first_pose.location + carla.Location(z=0.5), first_pose.rotation)
    ego = world.try_spawn_actor(bp, spawn)
    if ego is None:
        raise RuntimeError(f"could not spawn {blueprint} at {spawn.location} — is the "
                           f"trajectory inside the generated map?")
    ego.set_simulate_physics(False)
    return ego


def spawn_traffic(world: carla.World, client: carla.Client, count: int, seed: int,
                  tm_port: int = 8000) -> tuple[list[carla.Actor], carla.TrafficManager | None]:
    """``count`` autopilot vehicles on the generated map, as many as it has room for."""
    if count <= 0:
        return [], None
    import random

    tm = client.get_trafficmanager(tm_port)
    tm.set_synchronous_mode(True)
    tm.set_random_device_seed(seed)
    rng = random.Random(seed)
    points = world.get_map().get_spawn_points()
    rng.shuffle(points)
    bps = [b for b in world.get_blueprint_library().filter("vehicle.*")
           if int(b.get_attribute("number_of_wheels")) == 4]
    spawned: list[carla.Actor] = []
    for point in points[:count]:
        actor = world.try_spawn_actor(rng.choice(bps), point)
        if actor is not None:
            actor.set_autopilot(True, tm_port)
            spawned.append(actor)
    if len(spawned) < count:
        log.warning("asked for %d vehicles, the map had room for %d", count, len(spawned))
    return spawned, tm


# ----------------------------------------------------------------------------- the driver

def capture(client: carla.Client, sample: NurecSample, out_dir: str | Path, clip_id: str, *,
            frames: int, fps: int = 30, lens: str = "ftheta", cameras: Sequence[str] | None = None,
            complete_av7: bool = False, width: int = 1280, height: int = 720,
            nre_endpoint: str | None = None, fake_nurec: bool = False, start_s: float = 0.0,
            vehicles: int = 0, seed: int = 7, z_offset: float = 0.0, edge: bool = True,
            aovs: tuple[str, ...] = ("rgb", "depth", "semantic", "instance"),
            visibility: str = "depth", warmup: int = 5, tm_port: int = 8000,
            weather: str = "ClearNoon",
            contract: "BackendContract | None" = None,
            progress: Callable[[int, int], None] | None = None,
            restore_map: bool = True) -> tuple[Clip, Alignment]:
    """Turn a NuRec sample into a Cosmos clip package.

    Loads the sample's own OpenDRIVE map, replays its ego trajectory in synchronous mode with
    the rig mounted at the sample's real extrinsics, writes the neural RGB (or CARLA's, under
    ``fake_nurec``) alongside CARLA's depth / segmentation / semantic / edge AOVs, exports the
    ClipGT scene package with the occlusion filter, and asserts frame by frame that the ego
    track in that package is the trajectory that was replayed.

    The map and the world settings are always put back (see :class:`WorldGuard`) and every actor
    spawned here is destroyed, including when this raises.
    """
    if lens not in LENSES:
        raise ValueError(f"lens must be one of {LENSES}, not '{lens}'")
    if fake_nurec and lens != "pinhole":
        raise ValueError(
            "--fake-nurec substitutes CARLA's own pinhole RGB, so the clip must be captured "
            "with lens='pinhole'; an f-theta calibration over pinhole pixels would put the "
            "preview's boxes in the wrong place and look like a geometry bug")
    if not fake_nurec and not nre_endpoint:
        raise ValueError("no NuRec render engine: pass nre_endpoint='host:port', or "
                         "fake_nurec=True to substitute CARLA RGB and exercise everything else")
    rig = sample.rig(width=width, height=height, cameras=cameras, lens=lens,
                     complete_av7=complete_av7)
    timestamps = sample.clip_timestamps(frames, fps, start_s)
    ego_poses = sample.ego_transforms(timestamps)

    world, guard = load_map(client, sample, weather=weather)
    renderer: NreRenderer | None = None
    try:
        ego = spawn_ego(world, ego_poses[0])
        guard.actors.append(ego)
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 1.0 / fps
        world.apply_settings(settings)
        world.tick()

        traffic, _tm = spawn_traffic(world, client, vehicles, seed, tm_port)
        guard.actors.extend(traffic)
        if traffic and not fake_nurec:
            log.warning("%d CARLA vehicles are in the scene package and in the AOVs but will "
                        "NOT appear in the neural RGB: the engine only re-poses tracks the "
                        "artifact marks controllable, and a freshly spawned actor has none",
                        len(traffic))
        for _ in range(max(0, 2 * fps if traffic else 0)):
            world.tick()

        render: RenderFn | None = None
        if not fake_nurec:
            renderer = NreRenderer(sample, str(nre_endpoint), rig, lens=lens)
            def render(camera: str, index: int, pose_ue: np.ndarray, ts: int) -> np.ndarray:
                return renderer.render(camera, pose_ue, ts)

        ticks = TrajectoryTicks(world, ego, ego_poses, z_offset=z_offset)
        cap = NurecCapture(world, ego, rig, contract,
                           frames=frames, fps=fps, aovs=aovs, edge=edge,
                           ticks=ticks, visibility=visibility, warmup=warmup,
                           render=render, timestamps_us=[int(t) for t in timestamps])
        clip = cap.run(out_dir, clip_id, seed=seed,
                       carla_version=client.get_server_version(), progress=progress)

        alignment = check_alignment(clip, ticks.commanded, [int(t) for t in timestamps])
        write_sidecar(clip, sample, rig, alignment, lens=lens, fake_nurec=fake_nurec,
                      nre_endpoint=nre_endpoint, frames_rendered=cap.rendered)
        if not alignment.ok:
            raise AssertionError(
                f"the exported ego track drifts from the replayed trajectory by "
                f"{alignment.max_translation_m * 100:.1f} cm / "
                f"{alignment.max_rotation_deg:.3f} deg (tolerance "
                f"{TRANSLATION_TOL_M * 100:.0f} cm / {ROTATION_TOL_DEG} deg) — the neural "
                f"renders and the scene package describe different drives")
        log.info("alignment ok: %.1f mm / %.4f deg over %d frames",
                 alignment.max_translation_m * 1000, alignment.max_rotation_deg, alignment.frames)
        return clip, alignment
    finally:
        if renderer is not None:
            renderer.close()
        if tm_port and vehicles:
            try:
                client.get_trafficmanager(tm_port).set_synchronous_mode(False)
            except RuntimeError:
                pass
        guard.restore(reload_map=restore_map)


def write_sidecar(clip: Clip, sample: NurecSample, rig: Rig, alignment: Alignment, *,
                  lens: str, fake_nurec: bool, nre_endpoint: str | None,
                  frames_rendered: int) -> Path:
    """``scene/<clip_id>.nurec.json``: where these pixels came from and how well they line up.

    Kept out of ``manifest.json`` on purpose — that file is the clip contract every backend
    reads, and a NuRec provenance block has no meaning to any of them.  This one is for the
    reader who has to decide whether to trust the clip.
    """
    scene_dir = clip.path / SCENE_DIR
    scene_dir.mkdir(parents=True, exist_ok=True)
    path = scene_dir / f"{clip.manifest.clip_id}.nurec.json"
    payload = {
        "artifact": str(sample.path),
        "scene_id": sample.scene_id,
        "lens": lens,
        "rgb_source": "carla (fake-nurec)" if fake_nurec else f"nurec-render-engine @ {nre_endpoint}",
        "frames_rendered": frames_rendered,
        "cameras": [{"name": c.name,
                     "calibrated": canonical_camera_name(c.name) in sample.cameras,
                     "hfov": c.hfov, "t_flu": list(c.t), "rpy_flu": list(c.rpy)}
                    for c in rig.cameras],
        "nominal_cameras": sample.nominal_cameras(rig),
        "trajectory": {"timestamps_us": alignment.timestamps_us,
                       "duration_s": sample.duration_s},
        "alignment": alignment.as_dict(),
    }
    path.write_text(json.dumps(payload, indent=2))
    return path
