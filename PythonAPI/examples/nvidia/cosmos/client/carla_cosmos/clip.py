"""On-disk clip layout.

::

    <clip_dir>/
      manifest.json                          ClipManifest (contracts.py)
      rgb_<camera>.mp4                       H.264 crf 14
      depth_<camera>.mp4  seg_<camera>.mp4   H.264 4:4:4 lossless-mode controls
      edge_<camera>.mp4                      optional
      semantic_<camera>.mp4                  CityScapes-palette class AOV, written
                                             whenever the "semantic" AOV is captured
                                             (source of --mask-classes, see mask.py;
                                             a seg video captured with
                                             seg_mode="semantic" serves as well)
      scene/                                 ClipGT Parquet package (scene.py)
        <clip_id>.<table>.parquet
        <clip_id>.camera_front_wide_120fov.json
        <clip_id>.traffic_light_states.json

``<camera>`` is the underscore form (``camera_front_wide_120fov``).
"""

from __future__ import annotations

import json
import logging
from dataclasses import dataclass
from pathlib import Path

from .contracts import ClipManifest, canonical_camera_name

log = logging.getLogger(__name__)

MANIFEST_NAME = "manifest.json"
SCENE_DIR = "scene"
VIDEO_KINDS = ("rgb", "depth", "seg", "edge", "semantic")


def video_file_name(kind: str, camera: str) -> str:
    """``rgb_camera_front_wide_120fov.mp4`` and friends."""
    if kind not in VIDEO_KINDS:
        raise ValueError(f"unknown video kind '{kind}' (one of {VIDEO_KINDS})")
    return f"{kind}_{canonical_camera_name(camera)}.mp4"


@dataclass
class Clip:
    """A captured clip on disk."""

    path: Path
    manifest: ClipManifest

    @classmethod
    def load(cls, path: str | Path) -> "Clip":
        """Read ``manifest.json`` from a clip directory."""
        p = Path(path)
        data = json.loads((p / MANIFEST_NAME).read_text())
        return cls(path=p, manifest=ClipManifest.model_validate(data))

    def save_manifest(self) -> Path:
        """Write ``manifest.json``."""
        self.path.mkdir(parents=True, exist_ok=True)
        out = self.path / MANIFEST_NAME
        out.write_text(self.manifest.model_dump_json(indent=2))
        return out

    def video(self, kind: str, camera: str) -> Path:
        """Path of a video; raises ``FileNotFoundError`` if the clip has none."""
        name = self.manifest.video(kind, camera)
        if name is None:
            raise FileNotFoundError(f"clip {self.manifest.clip_id} has no {kind} video for {camera}")
        return self.path / name

    @property
    def scene_dir(self) -> Path | None:
        """Directory of the ClipGT scene package, if exported."""
        return self.path / self.manifest.scene_dir if self.manifest.scene_dir else None

    def validate(self, check_frames: bool = True, *, for_masking: bool = False) -> list[str]:
        """Check that every referenced file exists (and has the right frame count).

        ``for_masking`` additionally requires the class information
        ``--mask-classes`` needs — a ``semantic_<camera>.mp4`` AOV, or a
        CityScapes-coloured ``seg`` video — for every camera, and says what to
        recapture when it is missing (see :mod:`carla_cosmos.mask`).
        """
        errors: list[str] = []
        for key, name in self.manifest.videos.items():
            f = self.path / name
            if not f.is_file():
                errors.append(f"missing video {key}: {name}")
                continue
            if check_frames:
                from .controls import probe_video  # needs ffprobe only

                info = probe_video(f)
                if info["frames"] != self.manifest.frames:
                    errors.append(f"{name} has {info['frames']} frames, manifest says {self.manifest.frames}")
        sd = self.scene_dir
        if sd is not None:
            for table in ("egomotion_estimate", "obstacle", "calibration_estimate"):
                if not (sd / f"{self.manifest.clip_id}.{table}.parquet").is_file():
                    errors.append(f"scene package lacks {table}.parquet")
        if for_masking:
            errors.extend(self._masking_errors())
        return errors

    def _masking_errors(self) -> list[str]:
        """One error per camera without a CityScapes-palette video to build masks from."""
        from .mask import MaskError, semantic_video  # cv2/numpy: capture extras

        errors: list[str] = []
        for camera in self.manifest.camera_names:
            try:
                semantic_video(self, camera)
            except MaskError as exc:
                errors.append(str(exc))
        return errors
