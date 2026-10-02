"""carla-cosmos: capture CARLA clips and drive NVIDIA Cosmos generation backends.

The package has two halves:

* the **core** (``contracts``, ``clip``, ``client``, ``cli``, ``serve``) has no
  CARLA or OpenCV dependency and is what the server image and the CLI import;
* the **capture** half (``capture``, ``rig``, ``controls``, ``scene``,
  ``coords``, ``preview``) needs the ``carla`` wheel, OpenCV, pyarrow and scipy
  (``pip install carla-cosmos[capture]``); ``preview`` needs no CARLA server —
  it reads a captured clip back from disk.

Capture symbols are imported lazily so ``import carla_cosmos`` works in both
environments.
"""

from __future__ import annotations

import importlib
from typing import TYPE_CHECKING

from .clip import Clip
from .client import CosmosClient, Job, Result
from .results import IndexEntry, ResultStore, StoredFile, StoredJob, default_results_root
from .contracts import (
    AV_CAMERAS,
    BUILTIN_CONTRACTS,
    COSMOS3_NANO,
    COSMOS3_SUPER,
    TRANSFER25,
    TRANSFER25_AV,
    BackendContract,
    ClipManifest,
    ControlInput,
    JobInfo,
    JobRequest,
    JobSubmission,
    ResultManifest,
    validate_request,
)

if TYPE_CHECKING:  # pragma: no cover
    from .capture import Capture, FrameDesyncError, LiveTicks, ReplayTicks
    from .mask import MaskError, masked_clip_videos, resolve_classes
    from .preview import SceneGT, preview_clip
    from .rig import Camera, MountedCamera, Rig

__version__ = "0.2.0"

_LAZY = {
    "Capture": "capture",
    "FrameDesyncError": "capture",
    "LiveTicks": "capture",
    "ReplayTicks": "capture",
    "Camera": "rig",
    "MountedCamera": "rig",
    "Rig": "rig",
    "SceneGT": "preview",
    "preview_clip": "preview",
    "MaskError": "mask",
    "masked_clip_videos": "mask",
    "resolve_classes": "mask",
}


def __getattr__(name: str):
    module = _LAZY.get(name)
    if module is None:
        raise AttributeError(f"module 'carla_cosmos' has no attribute '{name}'")
    try:
        mod = importlib.import_module(f".{module}", __name__)
    except ImportError as exc:  # carla / cv2 / pyarrow missing
        raise ImportError(
            f"carla_cosmos.{name} needs the capture extras: pip install 'carla-cosmos[capture]' "
            f"(and the CARLA wheel). Original error: {exc}"
        ) from exc
    return getattr(mod, name)


__all__ = [
    "AV_CAMERAS",
    "BUILTIN_CONTRACTS",
    "BackendContract",
    "COSMOS3_NANO",
    "COSMOS3_SUPER",
    "Camera",
    "Capture",
    "Clip",
    "ClipManifest",
    "ControlInput",
    "CosmosClient",
    "FrameDesyncError",
    "IndexEntry",
    "Job",
    "JobInfo",
    "JobRequest",
    "JobSubmission",
    "LiveTicks",
    "MaskError",
    "MountedCamera",
    "ReplayTicks",
    "Result",
    "ResultManifest",
    "ResultStore",
    "Rig",
    "SceneGT",
    "StoredFile",
    "StoredJob",
    "TRANSFER25",
    "TRANSFER25_AV",
    "default_results_root",
    "masked_clip_videos",
    "preview_clip",
    "resolve_classes",
    "validate_request",
]
