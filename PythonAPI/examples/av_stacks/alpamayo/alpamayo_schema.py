"""Alpamayo request validation and torch conversion."""

from __future__ import annotations

from typing import Any, Mapping

import numpy as np

from alpamayo_rig import CAMERA_INDICES, CAMERA_NAMES


REQUEST_DTYPES = {
    "image_frames": np.dtype(np.uint8),
    "camera_indices": np.dtype(np.int64),
    "ego_history_xyz": np.dtype(np.float32),
    "ego_history_rot": np.dtype(np.float32),
    "relative_timestamps": np.dtype(np.float32),
    "absolute_timestamps": np.dtype(np.int64),
    "ego_t0": np.dtype(np.int64),
    "ego_t0_relative": np.dtype(np.float32),
    "ego_t0_frame_idx": np.dtype(np.int64),
}


def validate_request(metadata: Mapping[str, Any], arrays: Mapping[str, np.ndarray]) -> None:
    missing = REQUEST_DTYPES.keys() - arrays.keys()
    if missing:
        raise ValueError(f"request is missing arrays: {sorted(missing)}")
    for name, dtype in REQUEST_DTYPES.items():
        if arrays[name].dtype != dtype:
            raise ValueError(f"{name} must have dtype {dtype}, got {arrays[name].dtype}")
    if arrays["image_frames"].ndim != 5 or arrays["image_frames"].shape[:3] != (7, 4, 3):
        raise ValueError("image_frames must have shape [7, 4, 3, H, W]")
    if tuple(arrays["camera_indices"].tolist()) != CAMERA_INDICES:
        raise ValueError(f"camera_indices must be {CAMERA_INDICES}")
    if tuple(metadata.get("camera_names", ())) != CAMERA_NAMES:
        raise ValueError("metadata camera_names do not match the canonical camera ring")
    if arrays["ego_history_xyz"].shape != (1, 1, 16, 3):
        raise ValueError("ego_history_xyz must have shape [1, 1, 16, 3]")
    if arrays["ego_history_rot"].shape != (1, 1, 16, 3, 3):
        raise ValueError("ego_history_rot must have shape [1, 1, 16, 3, 3]")
    for name in ("relative_timestamps", "absolute_timestamps"):
        if arrays[name].shape != (7, 4):
            raise ValueError(f"{name} must have shape [7, 4]")
    for name in ("ego_t0", "ego_t0_relative", "ego_t0_frame_idx"):
        if arrays[name].shape != (1,):
            raise ValueError(f"{name} must have shape [1]")
    if not np.isfinite(arrays["ego_history_xyz"]).all():
        raise ValueError("ego history contains non-finite positions")
    if not np.isfinite(arrays["ego_history_rot"]).all():
        raise ValueError("ego history contains non-finite rotations")


def request_to_torch_data(
    metadata: Mapping[str, Any], arrays: Mapping[str, np.ndarray]
) -> dict[str, Any]:
    validate_request(metadata, arrays)
    import torch

    data = {name: torch.from_numpy(np.asarray(value).copy()) for name, value in arrays.items()}
    data["camera_names"] = list(metadata["camera_names"])
    data["camera_tmin"] = int(arrays["absolute_timestamps"].min())
    data["ego_available"] = torch.tensor(True)
    data["prediction_start_offset"] = torch.zeros(1, dtype=torch.float32)
    return data
