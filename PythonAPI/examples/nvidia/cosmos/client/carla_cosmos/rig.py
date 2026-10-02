"""Camera rigs: definitions, YAML presets and mounting on a CARLA ego vehicle.

A :class:`Camera` is defined the NVIDIA way: an FLU pose relative to the
vehicle's rear-axle-on-ground point.  :meth:`Rig.mount_on` turns that into CARLA
attach transforms for a concrete ego vehicle and reports the *actual* poses
(which the calibration table must carry).

Mounting rule ``roofline`` (default)
------------------------------------
NVIDIA's rig positions were measured on a real SUV; applied to a CARLA sedan
the front-wide camera ends up inside the cabin (Phase 0 finding).  Under the
``roofline`` rule every camera whose position lies inside the ego bounding box
(inflated by ``margin``) is lifted vertically to ``roof_top + margin`` and its
x/y are clamped to the bounding-box footprint.  Orientation and FOV are kept.
Cameras already outside the body are left untouched.  ``exact`` disables the
rule.
"""

from __future__ import annotations

import logging
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

import numpy as np
import yaml

import carla

from . import coords
from .contracts import AV_CAMERAS, CameraManifest, FThetaModel, RigManifest, canonical_camera_name

log = logging.getLogger(__name__)

# NVIDIA RDS-HQ rig (multiview_example1 calibration): name -> (t xyz m, rpy deg, hfov deg), FLU, axle origin.
_NVIDIA_AV7: dict[str, tuple[tuple[float, float, float], tuple[float, float, float], float]] = {
    "camera:front:wide:120fov": ((1.697, -0.010, 1.443), (0.13, 0.47, -0.71), 120.0),
    "camera:cross:left:120fov": ((2.473, 0.938, 0.918), (-0.17, 0.44, 66.41), 120.0),
    "camera:cross:right:120fov": ((2.478, -0.955, 0.929), (-0.19, -1.38, -66.81), 120.0),
    "camera:rear:left:70fov": ((2.029, 1.039, 0.885), (0.63, 3.26, 151.40), 70.0),
    "camera:rear:right:70fov": ((2.008, -1.005, 0.896), (-0.28, 0.87, -154.69), 70.0),
    "camera:rear:tele:30fov": ((0.383, 0.300, 1.434), (-0.25, 0.58, 179.19), 30.0),
    "camera:front:tele:30fov": ((1.646, -0.141, 1.449), (-0.33, -0.72, -0.73), 30.0),
}
assert tuple(_NVIDIA_AV7) == AV_CAMERAS

MOUNT_RULES = ("roofline", "exact")


@dataclass
class Camera:
    """A camera in the NVIDIA convention (FLU pose relative to the rear axle on ground)."""

    name: str
    hfov: float
    t: tuple[float, float, float]
    rpy: tuple[float, float, float] = (0.0, 0.0, 0.0)
    width: int = 1280
    height: int = 720
    lens: str = "pinhole"
    ftheta: FThetaModel | None = None
    """Measured lens, when this camera stands for a real one (see :class:`FThetaModel`).

    ``hfov`` stays the pinhole FOV CARLA's own sensors are spawned with; ``ftheta`` is what
    the exported calibration describes.  They differ only when the clip's RGB comes from
    somewhere other than a CARLA pinhole camera."""

    @property
    def canonical(self) -> str:
        """Underscore form used in file names and by the renderer CLI."""
        return canonical_camera_name(self.name)

    def flu_pose(self) -> np.ndarray:
        """4x4 FLU pose relative to the axle origin."""
        return coords.flu_pose_matrix(self.t, self.rpy)


@dataclass
class MountedCamera:
    """A camera resolved against a concrete ego vehicle."""

    camera: Camera
    t_flu: tuple[float, float, float]
    rpy_flu: tuple[float, float, float]
    attach: carla.Transform
    shifted: bool

    def manifest(self) -> CameraManifest:
        """Manifest entry with the actual extrinsics."""
        a = self.attach
        return CameraManifest(
            name=self.camera.name, hfov=self.camera.hfov, width=self.camera.width,
            height=self.camera.height, lens=self.camera.lens, ftheta=self.camera.ftheta,
            t_flu=list(self.t_flu), rpy_flu=list(self.rpy_flu),
            attach_ue={"x": a.location.x, "y": a.location.y, "z": a.location.z,
                       "pitch": a.rotation.pitch, "yaw": a.rotation.yaw, "roll": a.rotation.roll},
            shifted=self.shifted,
        )


@dataclass
class Rig:
    """A named set of cameras plus the mounting rule."""

    name: str
    cameras: list[Camera]
    mount: str = "roofline"
    margin: float = 0.05

    def __post_init__(self) -> None:
        if self.mount not in MOUNT_RULES:
            raise ValueError(f"unknown mount rule '{self.mount}'; use one of {MOUNT_RULES}")
        names = [c.name for c in self.cameras]
        if len(set(names)) != len(names):
            raise ValueError(f"duplicate camera names in rig '{self.name}': {names}")

    # ------------------------------------------------------------------ presets
    @classmethod
    def single(cls, hfov: float = 90.0, width: int = 1280, height: int = 720) -> "Rig":
        """One forward camera in the renderer's ``camera:front:wide:120fov`` slot.

        The slot name is what NVIDIA's ClipGT loader requires; the actual FOV is
        carried by the calibration polynomial and defaults to CARLA's 90 degrees,
        which gives a natural pinhole image for the single-view backends.
        """
        t, _rpy, _ = _NVIDIA_AV7["camera:front:wide:120fov"]
        return cls(name="single_720p", cameras=[Camera("camera:front:wide:120fov", hfov, t, (0.0, 0.0, 0.0), width, height)])

    @classmethod
    def nvidia_av7(cls, width: int = 1280, height: int = 720) -> "Rig":
        """The seven RDS-HQ cameras with NVIDIA's poses and FOVs."""
        cams = [Camera(n, hfov, t, rpy, width, height) for n, (t, rpy, hfov) in _NVIDIA_AV7.items()]
        return cls(name="nvidia_av7", cameras=cams)

    # ------------------------------------------------------------------ yaml
    @classmethod
    def load(cls, path: str | Path) -> "Rig":
        """Load a rig from YAML (see ``client/rigs/*.yaml``)."""
        data = yaml.safe_load(Path(path).read_text())
        cams = [Camera(name=c["name"], hfov=float(c["hfov"]), t=tuple(c["t"]), rpy=tuple(c.get("rpy", (0, 0, 0))),
                       width=int(c.get("width", 1280)), height=int(c.get("height", 720)), lens=c.get("lens", "pinhole"),
                       ftheta=FThetaModel(**c["ftheta"]) if c.get("ftheta") else None)
                for c in data["cameras"]]
        return cls(name=data["name"], cameras=cams, mount=data.get("mount", "roofline"),
                   margin=float(data.get("margin", 0.05)))

    def save(self, path: str | Path) -> None:
        """Write the rig as YAML."""
        data = {"name": self.name, "mount": self.mount, "margin": self.margin,
                "cameras": [_camera_dict(c) for c in self.cameras]}
        Path(path).write_text(yaml.safe_dump(data, sort_keys=False))

    # ------------------------------------------------------------------ mounting
    def mount_on(self, ego: carla.Vehicle, axle_local_ue: np.ndarray | None = None) -> list[MountedCamera]:
        """Resolve attach transforms for ``ego`` and report actual FLU extrinsics."""
        if axle_local_ue is None:
            axle_local_ue = rear_axle_local_ue(ego)
        t_axle = np.eye(4)
        t_axle[:3, 3] = axle_local_ue
        bb = ego.bounding_box
        centre = np.array([bb.location.x, bb.location.y, bb.location.z])
        extent = np.array([bb.extent.x, bb.extent.y, bb.extent.z])
        mounted = []
        for cam in self.cameras:
            m_hero = t_axle @ coords.flu_to_ue(cam.flu_pose())
            pos, shifted = _apply_mount_rule(self.mount, m_hero[:3, 3], centre, extent, self.margin)
            m_hero[:3, 3] = pos
            m_axle_flu = coords.ue_to_flu(np.linalg.inv(t_axle) @ m_hero)
            mc = MountedCamera(
                camera=cam,
                t_flu=tuple(float(v) for v in m_axle_flu[:3, 3]),
                rpy_flu=tuple(coords.flu_rpy_deg(m_axle_flu)),
                attach=coords.ue_transform_from_matrix(m_hero),
                shifted=shifted,
            )
            if shifted:
                log.info("camera %s moved out of the ego body: %s -> %s (FLU, axle frame)",
                         cam.name, np.round(cam.t, 3).tolist(), np.round(mc.t_flu, 3).tolist())
            mounted.append(mc)
        return mounted

    def manifest(self, mounted: Iterable[MountedCamera]) -> RigManifest:
        """Manifest of the rig as mounted."""
        return RigManifest(name=self.name, mount=self.mount, cameras=[m.manifest() for m in mounted])


def _camera_dict(c: Camera) -> dict:
    d = asdict(c)
    d["t"] = [float(v) for v in c.t]
    d["rpy"] = [float(v) for v in c.rpy]
    d["ftheta"] = c.ftheta.model_dump() if c.ftheta is not None else None
    return d


def _apply_mount_rule(rule: str, pos: np.ndarray, centre: np.ndarray, extent: np.ndarray,
                      margin: float) -> tuple[np.ndarray, bool]:
    """Return the (possibly moved) camera position in the ego frame and whether it moved."""
    if rule == "exact":
        return pos, False
    lo, hi = centre - extent - margin, centre + extent + margin
    inside = bool(np.all(pos > lo) and np.all(pos < hi))
    if not inside:
        return pos, False
    new = pos.copy()
    new[0] = float(np.clip(pos[0], centre[0] - extent[0], centre[0] + extent[0]))
    new[1] = float(np.clip(pos[1], centre[1] - extent[1], centre[1] + extent[1]))
    new[2] = float(centre[2] + extent[2] + margin)
    return new, True


# ----------------------------------------------------------------------------- ego geometry

def rear_axle_xy_from_bones(bone_names: Iterable[str], bone_world: Iterable[carla.Transform],
                            m_ref: np.ndarray) -> np.ndarray:
    """Mean rear-wheel-bone position (x, y) expressed in the frame ``m_ref`` (a 4x4 UE pose)."""
    inv = np.linalg.inv(m_ref)
    rear = []
    for name, tf in zip(bone_names, bone_world):
        low = name.lower()
        if "wheel" in low and ("rear" in low or "back" in low):
            rear.append((inv @ np.array([tf.location.x, tf.location.y, tf.location.z, 1.0]))[:2])
    if not rear:
        raise RuntimeError(f"no rear wheel bones among {list(bone_names)}")
    return np.mean(rear, axis=0)


def rear_axle_local_ue(vehicle: carla.Vehicle, reference: carla.Transform | None = None) -> np.ndarray:
    """Rear-axle-on-ground point in the vehicle actor frame (UE metres).

    ``WheelPhysicsControl`` carries no wheel position on ue58-dev (Phase 0), so x/y come from the
    skeleton's rear wheel bones and z from the bounding-box bottom.  Raises if no rear wheel bone
    is found.

    ``get_vehicle_bone_world_transforms()`` does not report the skeleton of the tick that just
    finished: except for the very first query on a vehicle (which returns the current tick), every
    later one returns the tick *before* the current one, and it stays that way - the value is
    stable within a tick, extra reads and waiting do not change it.  Expressed in the current pose
    those stale bones drag the axle backwards by exactly one tick of travel: 0.27 m at 8 m/s and
    30 fps, measured live as -1.669 m while driving against -1.397 m parked (wheelbase
    1.470 - (-1.397) = 2.87 m, the Lincoln MKZ's).  That is how the exported rig origin ended up
    near the rear bumper instead of the rear axle.

    ``reference`` is the pose the bones belong to; the default (the current pose) is only right
    for a vehicle standing still or for the first query on that vehicle.  Use
    :func:`measure_rear_axle`, which pins the query down whichever state it starts in.

    The world must have ticked at least once since the vehicle was spawned: until the first
    snapshot containing the actor, ``get_transform()`` returns the identity while the bone
    transforms are already in world space, which would put the axle at world coordinates.  That
    inconsistency is detected and raised.
    """
    tf = vehicle.get_transform() if reference is None else reference
    # not get_inverse_matrix(): see coords.ue_rotation_matrix
    xy = rear_axle_xy_from_bones(vehicle.get_bone_names(), vehicle.get_vehicle_bone_world_transforms(),
                                 coords.ue_matrix(tf))
    bb = vehicle.bounding_box
    span = max(abs(bb.extent.x), abs(bb.extent.y)) + 1.0
    if abs(xy[0]) > span or abs(xy[1]) > span:
        raise RuntimeError(
            f"{vehicle.type_id}: wheel bones are inconsistent with the actor transform "
            f"(axle would be at {xy.round(2).tolist()} in the actor frame); tick the world "
            f"once after spawning before measuring the rear axle")
    return np.array([xy[0], xy[1], bb.location.z - bb.extent.z], dtype=np.float64)


def measure_rear_axle(vehicle: carla.Vehicle, world: carla.World, tick) -> np.ndarray:
    """Rear-axle-on-ground point, measured so the skeleton lag cannot bias it.

    Call right after a tick.  One throw-away bone query moves the server's skeleton query into its
    steady one-tick-late state whatever state it was in before, so after one more tick the bones
    are exactly the ones of the pose read here - the reference and the bones then agree at any
    speed.  Poses come from the world snapshot, not from ``actor.get_transform()``: the actor cache
    is not guaranteed to have caught up with the tick that just returned.

    ``tick`` is the zero-argument callable that advances the world one frame.
    """
    snapshot = world.get_snapshot()
    ego = snapshot.find(vehicle.id)
    if ego is None:
        raise RuntimeError(f"vehicle {vehicle.id} is missing from snapshot frame {snapshot.frame}")
    reference = ego.get_transform()
    vehicle.get_vehicle_bone_world_transforms()  # prime; see rear_axle_local_ue
    tick()
    return rear_axle_local_ue(vehicle, reference=reference)
