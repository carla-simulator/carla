"""Shared between the AV socket worker and the torchrun rank loop."""

from __future__ import annotations

from ..common.ranks import READY_FILE, SHUTDOWN_FILE  # noqa: F401 - re-exported for callers

CAMERA_KEYS: dict[str, str] = {
    "camera:front:wide:120fov": "front_wide",
    "camera:cross:left:120fov": "cross_left",
    "camera:cross:right:120fov": "cross_right",
    "camera:rear:left:70fov": "rear_left",
    "camera:rear:right:70fov": "rear_right",
    "camera:rear:tele:30fov": "rear",
    "camera:front:tele:30fov": "front_tele",
}
KEY_TO_CAMERA = {v: k for k, v in CAMERA_KEYS.items()}
CHUNK_FRAMES = 29   # frames per view per chunk at model fps (state_t 8 -> 29 pixel frames)
CHUNK_OVERLAP = 1
