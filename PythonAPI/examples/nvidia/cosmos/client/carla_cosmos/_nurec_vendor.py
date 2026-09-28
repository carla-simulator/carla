"""Lazy access to the NuRec integration package that lives beside this one.

``PythonAPI/examples/nvidia/nurec/`` is a demo project with its own dependency set (gRPC stubs,
Docker orchestration, an NVIDIA container).  ``carla_cosmos`` is an installed library and must
not depend on it: everything in :mod:`carla_cosmos.nurec` except talking to a live render engine
works from the ``.usdz`` alone, and ``--fake-nurec`` has to run on a machine with no NuRec
install at all.

So the import happens here, once, behind functions that are only called when a render engine is
actually in play — with the single exception of :func:`get_t_rig_enu_from_ecef`, which is pure
numpy and XML and is needed to place the trajectory on the map.

``CARLA_NUREC_DIR`` overrides the location; the default is the sibling directory.
"""

from __future__ import annotations

import functools
import os
import sys
from pathlib import Path
from typing import Any

DEFAULT_NUREC_DIR = Path(__file__).resolve().parents[3] / "nurec"


def nurec_dir() -> Path:
    """Where the NuRec integration package lives."""
    return Path(os.environ.get("CARLA_NUREC_DIR", DEFAULT_NUREC_DIR))


def _import(module: str) -> Any:
    """Import ``module`` from the NuRec package, putting it on ``sys.path`` once."""
    directory = nurec_dir()
    if not (directory / "nurec_integration.py").is_file():
        raise ModuleNotFoundError(
            f"the NuRec integration is not at {directory}. It ships in the CARLA tree at "
            f"PythonAPI/examples/nvidia/nurec; set CARLA_NUREC_DIR to point at it, or use "
            f"--fake-nurec, which needs none of it.")
    path = str(directory)
    if path not in sys.path:
        sys.path.insert(0, path)
    return __import__(module, fromlist=["*"])


@functools.lru_cache(maxsize=1)
def _projection() -> Any:
    return _import("projection_functions")


def get_t_rig_enu_from_ecef(t_rig_ecef, xodr_data: str):
    """``T_enu_from_nurec`` for a reconstruction anchored at ``t_rig_ecef``.

    The one call made even in ``--fake-nurec`` mode: it converts the artifact's ECEF anchor into
    the ENU frame the OpenDRIVE ``geoReference`` declares, which is what puts the replayed
    trajectory on the map's actual roads.  Pure numpy and ``xml.etree``; it imports neither
    gRPC nor carla.  Returns the identity when the map has no parseable georeference.
    """
    return _projection().get_t_rig_enu_from_ecef(t_rig_ecef, xodr_data)


@functools.lru_cache(maxsize=1)
def _protos() -> tuple[Any, Any, Any]:
    return (_import("nre.grpc.protos.sensorsim_pb2"),
            _import("nre.grpc.protos.common_pb2"),
            _import("nre.grpc.protos.sensorsim_pb2_grpc"))


@functools.lru_cache(maxsize=1)
def _integration() -> Any:
    return _import("nurec_integration")


@functools.lru_cache(maxsize=1)
def _utils() -> Any:
    return _import("utils")


def camera_spec_from_dict(params: dict) -> Any:
    """A gRPC ``CameraSpec`` from NuRec camera parameters (``dict_to_camera_spec``)."""
    return _integration().dict_to_camera_spec(params)


def planar_format() -> Any:
    """``ImageFormat.RGB_UINT8_PLANAR``: raw CHW bytes, no codec, no decoder dependency.

    A function rather than a constant so that importing this module needs no gRPC stubs.
    """
    sensorsim, _common, _grpc = _protos()
    return sensorsim.ImageFormat.RGB_UINT8_PLANAR


def render_stub(endpoint: str) -> tuple[Any, Any]:
    """``(stub, channel)`` for an NRE at ``endpoint`` (``host:port``).

    Insecure gRPC, because that is all the engine serves.  The message size limit matches the
    integration's: an uncompressed 4K planar frame is about 25 MB.
    """
    import grpc

    sensorsim, _common, sensorsim_grpc = _protos()
    options = [("grpc.max_send_message_length", 64_000_000),
               ("grpc.max_receive_message_length", 64_000_000)]
    channel = grpc.insecure_channel(endpoint, options=options)
    return sensorsim_grpc.SensorsimServiceStub(channel), channel


def server_scenes(endpoint: str, timeout: float = 10.0) -> tuple[str, list[str]]:
    """``(version, scene_ids)`` from a live NRE — the readiness probe the runbook uses."""
    sensorsim, common, _grpc = _protos()
    stub, channel = render_stub(endpoint)
    try:
        version = stub.get_version(common.Empty(), timeout=timeout)
        scenes = stub.get_available_scenes(common.Empty(), timeout=timeout)
        return str(version.version_id), list(scenes.scene_ids)
    finally:
        channel.close()


def build_render_request(scene_id: str, camera_spec: Any, camera_pose, timestamp: int,
                         image_format: Any, dynamic_objects: list | None = None) -> Any:
    """An ``RGBRenderRequest`` for one camera at one instant.

    ``camera_pose`` is 4x4 in the reconstruction's own frame.  ``dynamic_objects`` is empty by
    default and that is the right default for trajectory replay: with no pose overrides the
    engine renders the scene's own recorded actors at their recorded poses for ``timestamp``,
    which is exactly the drive being replayed.  Overrides only matter when CARLA is driving a
    reconstructed actor, and the engine ignores them for tracks the artifact does not mark
    controllable.
    """
    sensorsim, _common, _grpc = _protos()
    pose = _utils().se3_to_grpc_pose(camera_pose)
    return sensorsim.RGBRenderRequest(
        scene_id=scene_id,
        resolution_h=int(camera_spec.resolution_h),
        resolution_w=int(camera_spec.resolution_w),
        camera_intrinsics=camera_spec,
        frame_start_us=int(timestamp),
        # The engine requires a non-empty interval; the integration uses the same +1 us.
        frame_end_us=int(timestamp) + 1,
        sensor_pose=sensorsim.PosePair(start_pose=pose, end_pose=pose),
        dynamic_objects=list(dynamic_objects or []),
        image_format=image_format,
        image_quality=95,
    )
