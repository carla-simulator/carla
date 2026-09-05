# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
NUREC Integration Module

This module provides the main integration between NUREC (Neural Reconstruction Engine) 
and CARLA simulation. It contains classes and functions for:

- Loading and managing NUREC scenarios from USDZ files
- Rendering photorealistic images using NUREC's neural reconstruction
- Managing actors and their trajectories in CARLA
- Synchronizing NUREC timeline with CARLA simulation
- Providing camera sensors with NUREC-rendered imagery
- Handling coordinate transformations between NUREC and CARLA coordinate systems

Key Classes:
- NurecScenario: Main class for loading and running NUREC scenarios in CARLA
- NurecRenderer: Handles communication with NUREC rendering service
- NurecSensor: Camera sensor that renders images using NUREC
- NurecActor: Wrapper for CARLA actors with NUREC track data
- TimeKeeper: Abstract interface for time management

The module supports various camera models including F-theta, OpenCV pinhole, and 
OpenCV fisheye cameras, with automatic coordinate system alignment between 
NUREC reconstructions and OpenDRIVE maps.
"""

from abc import abstractmethod
import os
import logging
import numpy as np
from scipy.spatial.transform import Rotation as R
import zipfile
from typing import Dict, List, Any, Optional, Set, Callable, Union, Tuple

import carla
from nurec_render_service import NuRecRenderService
from scenario import Scenario
from blueprint_library import BlueprintLibrary

import grpc
from nre.grpc.protos.common_pb2 import Empty as EmptyRequest
from nre.grpc.protos.sensorsim_pb2_grpc import SensorsimServiceStub
from nre.grpc.protos.sensorsim_pb2 import (
    RGBRenderReturn,
    RGBRenderRequest,
    BatchRGBRenderRequest,
    BatchRGBRenderRequestItem,
    AvailableCamerasRequest,
    AvailableCamerasReturn,
    CameraSpec,
    PosePair,
    DynamicObject,
    ImageFormat,
    OpenCVPinholeCameraParam,
    FthetaCameraParam,
    OpenCVFisheyeCameraParam,
    ShutterType,
    LidarRenderRequest,
    LidarSpec,
    LidarDeviceType,
    LidarRenderFilter,
    EditAssetsRequest,
    ReplaceAssetAction,
    DynamicObjectTrack,
    AvailableDynamicObjectsRequest,
    ExternalAssetObjectsRequest,
)
from track import Track

from constants import (
    EGO_TRACK_ID,
    EGO_LABEL,
    VEHICLE_LABELS,
    MAX_MESSAGE_LENGTH,
    KPH_PER_MPS,
    DEFAULT_NUREC_PORT,
)
from projection_functions import get_t_rig_enu_from_ecef
from simple_trajectory_follower import SimpleTrajectoryFollower
from utils import (
    se3_to_grpc_pose,
    actor_to_grpc_pose,
    carla_transform_to_nurec,
    mat_to_carla_transform,
    xyzeuler_to_carla_transform,
)


logger = logging.getLogger(__name__)

# Minimum recorded travel distance for a vehicle to be handed to the CARLA
# Traffic Manager; below this the recording is a parked/creeping vehicle and
# the TM has no route to follow (it wanders the car into neighbors).
MIN_TM_ROUTE_M = 5.0
MIN_TM_FOLLOW_DISTANCE_M = 10.0
TM_TIME_HEADWAY_S = 2.0


def collect_dynamic_objects(
    actors: carla.WorldSnapshot,
    active_actors: Dict[int, str],
    controllable_tracks: Set[str],
    t_carla_nurec: np.ndarray,
    blueprint_library: Optional[BlueprintLibrary] = None,
    actor_blueprints: Optional[Dict[int, str]] = None,
) -> List[DynamicObject]:
    """Convert the controllable CARLA actors of a snapshot to gRPC DynamicObjects."""
    dynamic_objects = []
    # select all actors that have attributes and have track_id attribute
    for actor in actors:
        if actor.id in active_actors:
            track_id = active_actors[actor.id]
            if not track_id in controllable_tracks:
                continue
            pose = actor_to_grpc_pose(
                actor, t_carla_nurec, blueprint_library, actor_blueprints
            )
            dynamic_objects.append(
                DynamicObject(
                    track_id=track_id,
                    pose_pair=PosePair(
                        start_pose=pose,
                        end_pose=pose,
                    ),
                )
            )
    return dynamic_objects


def generate_request(
    scene_id: str,
    camera_spec: CameraSpec,
    camera_pose: np.ndarray,
    timestamp: int,
    scale: float,
    actors: carla.WorldSnapshot,
    format: ImageFormat,
    active_actors: Dict[int, str],
    controllable_tracks: Set[str],
    t_carla_nurec: np.ndarray,
    blueprint_library: Optional[BlueprintLibrary] = None,
    actor_blueprints: Optional[Dict[int, str]] = None,
) -> RGBRenderRequest:
    """
    Generate a gRPC render request for NUREC rendering service.
    
    Args:
        scene_id: Identifier for the NUREC scene
        camera_spec: Camera specification including intrinsics
        camera_pose: 4x4 camera pose matrix
        timestamp: Timestamp in microseconds
        scale: Resolution scaling factor
        actors: CARLA world snapshot containing actor states
        format: Image format for rendering
        active_actors: Mapping from actor IDs to track IDs
        controllable_tracks: Set of track IDs that can be controlled
        t_carla_nurec: Transformation matrix from CARLA to NUREC coordinates
        blueprint_library: Optional blueprint library for offset calculations
        actor_blueprints: Optional mapping from actor IDs to blueprint IDs
        
    Returns:
        RGBRenderRequest: gRPC request object for rendering
    """
    camera_pose = t_carla_nurec @ camera_pose
    dynamic_objects = collect_dynamic_objects(
        actors, active_actors, controllable_tracks, t_carla_nurec,
        blueprint_library, actor_blueprints,
    )
    return RGBRenderRequest(
        scene_id=scene_id,
        resolution_h=int(camera_spec.resolution_h * scale),
        resolution_w=int(camera_spec.resolution_w * scale),
        camera_intrinsics=camera_spec,
        frame_start_us=timestamp,
        frame_end_us=timestamp + 1,  # important that these are not identical
        sensor_pose=PosePair(
            start_pose=se3_to_grpc_pose(camera_pose),
            end_pose=se3_to_grpc_pose(camera_pose),
        ),
        dynamic_objects=dynamic_objects,
        image_format=format,
        image_quality=95,
    )


def dict_to_camera_spec(params: dict) -> CameraSpec:
    """
    Converts a dictionary of camera parameters to a gRPC CameraSpec.

    Args:
        params: Dictionary containing camera parameters. Supported keys:
                - logical_id: Camera identifier (default: "custom_camera")
                - resolution_h: Image height in pixels (required)
                - resolution_w: Image width in pixels (required)
                - shutter_type: Shutter type (default: ShutterType.GLOBAL). Valid values:
                    * ShutterType.UNKNOWN = 0
                    * ShutterType.ROLLING_TOP_TO_BOTTOM = 1
                    * ShutterType.ROLLING_LEFT_TO_RIGHT = 2
                    * ShutterType.ROLLING_BOTTOM_TO_TOP = 3
                    * ShutterType.ROLLING_RIGHT_TO_LEFT = 4
                    * ShutterType.GLOBAL = 5 (default)

                For OpenCV Pinhole camera:
                - camera_type: "opencv_pinhole"
                - focal_length_x: Focal length in x direction (calculated from resolution if not provided)
                - focal_length_y: Focal length in y direction (defaults to focal_length_x if not provided)
                - principal_point_x: Principal point x coordinate (defaults to resolution_w / 2.0 if not provided)
                - principal_point_y: Principal point y coordinate (defaults to resolution_h / 2.0 if not provided)
                - radial_coeffs: List of radial distortion coefficients (default: [])

                For OpenCV Fisheye camera:
                - camera_type: "opencv_fisheye"
                - focal_length_x: Focal length in x direction (calculated from resolution if not provided)
                - focal_length_y: Focal length in y direction (defaults to focal_length_x if not provided)
                - principal_point_x: Principal point x coordinate (defaults to resolution_w / 2.0 if not provided)
                - principal_point_y: Principal point y coordinate (defaults to resolution_h / 2.0 if not provided)
                - radial_coeffs: List of radial distortion coefficients (default: [])
                - max_angle: Maximum angle for fisheye (default: π)

                For F-theta camera:
                - camera_type: "ftheta" (default)
                - principal_point_x: Principal point x coordinate (defaults to resolution_w / 2.0 if not provided)
                - principal_point_y: Principal point y coordinate (defaults to resolution_h / 2.0 if not provided)
                - reference_poly: Polynomial type (1 for PIXELDIST_TO_ANGLE, 2 for ANGLE_TO_PIXELDIST) (default: 1)
                - pixeldist_to_angle_poly: List of polynomial coefficients (default: [])
                - angle_to_pixeldist_poly: List of polynomial coefficients (default: [])
                - max_angle: Maximum angle (default: π)

                Helper for CARLA-style parameters:
                - image_size_x, image_size_y, fov: Will be converted to pinhole parameters

    Returns:
        CameraSpec: A gRPC CameraSpec object

    Example:
        # F-theta camera with custom parameters
        camera_params = {
            'resolution_w': 640, 
            'resolution_h': 480,
            'camera_type': 'ftheta',
            'principal_point_x': 320.0,
            'principal_point_y': 240.0,
            'max_angle': 3.14159
        }
        spec = dict_to_camera_spec(camera_params)
        
        # CARLA-style camera (automatically converted to f-theta)
        carla_params = {
            'image_size_x': 800,
            'image_size_y': 600, 
            'fov': 90.0
        }
        spec = dict_to_camera_spec(carla_params)
    """
    # Make a copy to avoid modifying the original
    params = params.copy()

    # Set default values
    params.setdefault("logical_id", "custom_camera")
    params.setdefault("shutter_type", ShutterType.GLOBAL)

    # Handle CARLA-style parameters as convenience
    if "image_size_x" in params and "image_size_y" in params and "fov" in params:
        image_size_x = params["image_size_x"]
        image_size_y = params["image_size_y"]
        fov = params["fov"]

        # Set basic parameters
        params.setdefault("camera_type", "ftheta")
        params.setdefault("resolution_w", image_size_x)
        params.setdefault("resolution_h", image_size_y)

        # Only calculate focal length if not provided
        if "focal_length_x" not in params:
            fov_rad = np.deg2rad(fov)
            focal_length_x = (image_size_x / 2.0) / np.tan(fov_rad / 2.0)
            params["focal_length_x"] = focal_length_x

        # Only set focal_length_y if not provided
        params.setdefault("focal_length_y", params["focal_length_x"])

        # Only set principal points if not provided
        params.setdefault("principal_point_x", image_size_x / 2.0)
        params.setdefault("principal_point_y", image_size_y / 2.0)
        params.setdefault("radial_coeffs", [])

    # Get required resolution parameters
    resolution_h = params.get("resolution_h")
    resolution_w = params.get("resolution_w")
    if resolution_h is None or resolution_w is None:
        raise ValueError("resolution_h and resolution_w are required parameters")

    # Determine camera type. CameraSpec is a oneof over three lens models and the server
    # accepts all three; only ftheta used to be built here, so "opencv_pinhole" -- which this
    # docstring has always documented, and which is the only way to render a camera the scene
    # does not calibrate (a nominal AV-7 slot, or a plain CARLA sensor) -- was rejected.
    camera_type = params.get("camera_type", "ftheta")
    supported = ("ftheta", "opencv_pinhole", "opencv_fisheye")
    if camera_type not in supported:
        raise ValueError(f"Unsupported camera_type: {camera_type}, supported types are: "
                         f"{', '.join(supported)}")

    # Set defaults only if parameters are not explicitly provided
    params.setdefault("principal_point_x", resolution_w / 2.0)
    params.setdefault("principal_point_y", resolution_h / 2.0)

    if camera_type == "ftheta":
        params.setdefault("reference_poly", 1)
        params.setdefault("pixeldist_to_angle_poly", [])
        params.setdefault("angle_to_pixeldist_poly", [])
        params.setdefault("max_angle", np.pi)
        spec_kwargs = dict(ftheta_param=FthetaCameraParam(
            principal_point_x=params["principal_point_x"],
            principal_point_y=params["principal_point_y"],
            reference_poly=params["reference_poly"],
            pixeldist_to_angle_poly=params["pixeldist_to_angle_poly"],
            angle_to_pixeldist_poly=params["angle_to_pixeldist_poly"],
            max_angle=params["max_angle"],
        ))
    else:
        # A projective lens needs a focal length; derive it from `fov` when only that is given,
        # which is what a CARLA sensor knows about itself.
        if "focal_length_x" not in params:
            if "fov" not in params:
                raise ValueError(f"{camera_type} needs focal_length_x, or fov to derive it from")
            params["focal_length_x"] = (resolution_w / 2.0) / np.tan(np.deg2rad(params["fov"]) / 2.0)
        params.setdefault("focal_length_y", params["focal_length_x"])
        # ncore's dataclasses assert a fixed shape on every distortion vector, so an
        # undistorted lens is a vector of zeros, not an empty list: the server raises
        # `assert self.<name>_coeffs.shape == (n,)` otherwise.  Pad to the length it wants.
        def coeffs(name: str, n: int) -> list:
            given = list(params.get(name) or [])
            if len(given) > n:
                raise ValueError(f"{camera_type}: at most {n} {name}, got {len(given)}")
            return given + [0.0] * (n - len(given))

        common = dict(principal_point_x=params["principal_point_x"],
                      principal_point_y=params["principal_point_y"],
                      focal_length_x=params["focal_length_x"],
                      focal_length_y=params["focal_length_y"],
                      radial_coeffs=coeffs("radial_coeffs", 6))
        if camera_type == "opencv_pinhole":
            spec_kwargs = dict(opencv_pinhole_param=OpenCVPinholeCameraParam(
                **common,
                tangential_coeffs=coeffs("tangential_coeffs", 2),
                thin_prism_coeffs=coeffs("thin_prism_coeffs", 4),
            ))
        else:
            spec_kwargs = dict(opencv_fisheye_param=OpenCVFisheyeCameraParam(
                **common, max_angle=params.get("max_angle", np.pi)))

    return CameraSpec(
        **spec_kwargs,
        logical_id=params["logical_id"],
        trajectory_idx=params.get("trajectory_idx", 0),
        resolution_h=resolution_h,
        resolution_w=resolution_w,
        shutter_type=params["shutter_type"],
    )


def carla_transform_to_matrix(transform: carla.Transform) -> np.ndarray:
    """
    Converts a CARLA Transform to a 4x4 transformation matrix.

    The result is in CARLA's own left-handed frame (x forward, y right, z up),
    NOT in the NuRec rig frame -- use ``utils.carla_transform_to_nurec`` for
    that.

    Args:
        transform: CARLA Transform object

    Returns:
        4x4 numpy array representing the transformation matrix
    """
    matrix = np.array(transform.get_matrix())
    return matrix.reshape(4, 4)


class TimeKeeper:
    @abstractmethod
    def get_sim_time(self) -> int:
        """
        Returns the current time in the scenario in microseconds.
        """
        pass

    @abstractmethod
    def is_running(self) -> bool:
        pass


class MockWorldSnapshot:
    def __init__(self):
        pass

    def GetTimestamp(self) -> int:
        return 0

    def __iter__(self):
        return self

    def __next__(self):
        raise StopIteration


class NurecRenderer:
    def __init__(
        self,
        scenario: Scenario,
        host=None,
        port=None,
        active_actors: Dict[int, str] = {},
        t_scenario_carla=np.eye(4),
        blueprint_library: Optional[BlueprintLibrary] = None,
        actor_blueprints: Optional[Dict[int, str]] = None,
        image_format: str = "planar",
    ):
        """
        image_format: 'planar' (default) transfers raw RGB_UINT8_PLANAR frames
        over localhost — no encode/decode, no CUDA decoder dependency.
        'jpeg' restores the legacy behavior (requires nvidia-nvimgcodec-cu12).
        """
        self.scenario = scenario
        self.host = host if host is not None else "localhost"
        self.port = port if port is not None else DEFAULT_NUREC_PORT
        self.available_cameras: Dict[str, CameraSpec] = {}
        self.scene_id = self.scenario.metadata["sequence_id"]
        self.start_timestamp = self.scenario.metadata["pose-range"][
            "start-timestamp_us"
        ]
        self.end_timestamp = self.scenario.metadata["pose-range"]["end-timestamp_us"]
        if image_format not in ("planar", "jpeg"):
            raise ValueError(f"Unsupported image_format: {image_format!r}")
        self.image_format = (
            ImageFormat.RGB_UINT8_PLANAR if image_format == "planar" else ImageFormat.JPEG
        )
        self.jpeg_decoder = None
        if image_format == "jpeg":
            import nvidia.nvimgcodec as nvimgcodec
            self.jpeg_decoder = nvimgcodec.Decoder()
        # Whether the server implements batch_render_rgb (NRE >= 26.04);
        # downgraded to False on first UNIMPLEMENTED response.
        self.batch_supported = True
        self.t_carla_nurec = np.linalg.inv(t_scenario_carla)
        self._init_grpc()

        self.active_actors = active_actors
        self.blueprint_library = blueprint_library
        self.actor_blueprints = actor_blueprints

    def _init_grpc(self) -> None:
        self.channel = grpc.insecure_channel(
            f"{self.host}:{self.port}",
            options=[
                ("grpc.max_send_message_length", MAX_MESSAGE_LENGTH),
                ("grpc.max_receive_message_length", MAX_MESSAGE_LENGTH),
            ],
        )
        logger.debug(f"Initializing gRPC channel to {self.host}:{self.port}")
        self.client_service = SensorsimServiceStub(self.channel)
        available_cameras: AvailableCamerasReturn = (
            self.client_service.get_available_cameras(
                AvailableCamerasRequest(scene_id=self.scene_id)
            )
        )
        self.available_cameras = {}
        for available_camera in available_cameras.available_cameras:
            self.available_cameras[available_camera.logical_id] = available_camera

    def _decode(self, response: RGBRenderReturn, request: RGBRenderRequest) -> np.ndarray:
        """Decode a render response to an HxWx3 uint8 RGB array."""
        if self.image_format == ImageFormat.RGB_UINT8_PLANAR:
            h, w = request.resolution_h, request.resolution_w
            image = np.frombuffer(response.image_bytes, dtype=np.uint8)
            return image.reshape(3, h, w).transpose(1, 2, 0)
        image = self.jpeg_decoder.decode(response.image_bytes)
        return np.array(image.cpu()).astype(np.uint8)

    def build_request(
        self,
        world_snapshot: carla.WorldSnapshot,
        camera_spec: CameraSpec,
        pose: np.ndarray,
        resolution_ratio: float = 0.25,
    ) -> RGBRenderRequest:
        timestamp = int(self.scenario.tracks.current_time)

        # bound timestamp to the range of the scenario
        timestamp = min(timestamp, self.end_timestamp - 1)

        return generate_request(
            self.scene_id,
            camera_spec,
            pose,
            timestamp,
            resolution_ratio,
            world_snapshot,
            self.image_format,
            self.active_actors,
            self.scenario.controllable_tracks,
            self.t_carla_nurec,
            self.blueprint_library,
            self.actor_blueprints,
        )

    def render(self, world_snapshot: carla.WorldSnapshot, camera_spec: CameraSpec, pose: np.ndarray, resolution_ratio: float = 0.25) -> np.ndarray:
        request = self.build_request(world_snapshot, camera_spec, pose, resolution_ratio)
        response = self.client_service.render_rgb(request)
        return self._decode(response, request)

    def render_batch(self, requests: List[Tuple[str, RGBRenderRequest]]) -> Dict[str, np.ndarray]:
        """
        Render several cameras in one round trip via batch_render_rgb.
        Transparently falls back to sequential render_rgb against servers
        that predate the batch API. Returns {name: image}; failed items are
        logged and omitted.
        """
        if not requests:
            return {}
        if self.batch_supported:
            try:
                batch = BatchRGBRenderRequest(
                    items=[
                        BatchRGBRenderRequestItem(camera_name=name, request=req)
                        for name, req in requests
                    ]
                )
                response = self.client_service.batch_render_rgb(batch)
                by_name = dict(requests)
                images = {}
                for item in response.items:
                    if not item.success:
                        logger.error(f"Batch render failed for {item.camera_name}: {item.error_message}")
                        continue
                    images[item.camera_name] = self._decode(item.result, by_name[item.camera_name])
                return images
            except grpc.RpcError as e:
                if e.code() == grpc.StatusCode.UNIMPLEMENTED:
                    logger.warning("Server lacks batch_render_rgb; falling back to per-camera render_rgb")
                    self.batch_supported = False
                else:
                    raise
        images = {}
        for name, request in requests:
            response = self.client_service.render_rgb(request)
            images[name] = self._decode(response, request)
        return images

    def get_camera_spec(self, camera_logical_id: str) -> CameraSpec:
        return self.available_cameras[camera_logical_id].intrinsics

    def render_lidar(
        self,
        world_snapshot: carla.WorldSnapshot,
        pose: np.ndarray,
        lidar_type: str = "PANDAR128",
        render_filter: Optional[LidarRenderFilter] = None,
    ) -> Tuple[np.ndarray, np.ndarray]:
        """
        Render a lidar sweep at the given pose (NuRec frame). Returns
        (points Nx3 float32, intensities N float32). Requires NRE >= 26.04.
        """
        timestamp = int(self.scenario.tracks.current_time)
        timestamp = min(timestamp, self.end_timestamp - 1)
        nurec_pose = self.t_carla_nurec @ pose
        request = LidarRenderRequest(
            scene_id=self.scene_id,
            lidar_config=LidarSpec(lidar_type=LidarDeviceType.Value(lidar_type)),
            frame_start_us=timestamp,
            frame_end_us=timestamp + 1,
            sensor_pose=PosePair(
                start_pose=se3_to_grpc_pose(nurec_pose),
                end_pose=se3_to_grpc_pose(nurec_pose),
            ),
            dynamic_objects=collect_dynamic_objects(
                world_snapshot, self.active_actors,
                self.scenario.controllable_tracks, self.t_carla_nurec,
                self.blueprint_library, self.actor_blueprints,
            ),
        )
        if render_filter is not None:
            request.render_filter.CopyFrom(render_filter)
        response = self.client_service.render_lidar(request)
        if response.point_xyzs_buffer:
            points = np.frombuffer(response.point_xyzs_buffer, dtype=np.float32).reshape(-1, 3)
            intensities = np.frombuffer(response.point_intensities_buffer, dtype=np.float32)
        else:
            points = np.array(response.point_xyzs, dtype=np.float32).reshape(-1, 3)
            intensities = np.array(response.point_intensities, dtype=np.float32)
        return points, intensities

    def get_dynamic_objects(self) -> List[DynamicObjectTrack]:
        """Server-side track data for the scene (id, semantic class, trajectory, size)."""
        response = self.client_service.get_dynamic_objects(
            AvailableDynamicObjectsRequest(scene_id=self.scene_id)
        )
        return list(response.dynamic_objects)

    def get_external_asset_objects(self) -> List[str]:
        """Track ids that are backed by external (swappable) assets."""
        response = self.client_service.get_external_asset_objects(
            ExternalAssetObjectsRequest(scene_id=self.scene_id)
        )
        return list(response.track_ids)

    def edit_assets(
        self,
        replace: Optional[List[ReplaceAssetAction]] = None,
        insert: Optional[List[DynamicObjectTrack]] = None,
    ) -> None:
        """
        Replace or insert rendered assets in the scene (scenario variation).
        The server must run with --enable-editing-actors
        (NurecScenario(enable_asset_editing=True)).
        """
        response = self.client_service.edit_assets(
            EditAssetsRequest(
                scene_id=self.scene_id,
                replace=replace or [],
                insert=insert or [],
            )
        )
        if not response.success:
            raise RuntimeError(f"edit_assets failed: {response.message}")


class NurecActor:
    def __init__(self, actor_inst: carla.Actor, track: Track, physics: bool = False, blueprint_id: int = None):
        self.actor_inst = actor_inst
        self.track = track
        self.physics = physics
        # Final waypoint of the TM route (set_follow_path) and whether the
        # vehicle has been held there; a TM vehicle that consumes its route
        # early otherwise keeps driving past the scenario's endpoint.
        self.tm_route_end: Optional[carla.Location] = None
        self.tm_route_done = False
        self.tm_follow_distance = 0.0
        self.tm_controlled = False
        self.alive = True
        self.blueprint_id = blueprint_id

    def destroy(self) -> None:
        self.alive = False
        if self.actor_inst.is_alive:
            self.actor_inst.destroy()

    def set_physics(
        self,
        physics: bool,
        current_time: int,
        seed_recorded_velocity: bool = True,
    ) -> None:
        if self.physics == physics:
            return
        self.physics = physics
        self.actor_inst.set_simulate_physics(physics)
        if not physics:
            self.tm_route_end = None
            self.tm_route_done = False
        if physics and not seed_recorded_velocity:
            self.actor_inst.set_target_velocity(carla.Vector3D())
            self.actor_inst.set_target_angular_velocity(carla.Vector3D())
            return
        min_time = self.track.start_time()
        before_time = max(min_time, current_time - 100_000)
        pose_before = self.track.interpolate_pose_matrix(before_time)
        current_pose = self.track.interpolate_pose_matrix(current_time)
        dt_us = current_time - before_time
        if current_pose is None:
            return
        if dt_us > 0 and pose_before is not None:
            position_delta = current_pose[:3, 3] - pose_before[:3, 3]
        else:
            # At the first scenario timestamp there is no earlier pose. Use a
            # one-sided forward difference so a physics-controlled ego starts
            # at the dataset's recorded speed instead of zero. Starting at
            # zero puts Alpamayo outside its moving-history distribution and
            # creates a receding-horizon stop/hold deadlock.
            after_time = min(self.track.end_time(), current_time + 100_000)
            pose_after = self.track.interpolate_pose_matrix(after_time)
            dt_us = after_time - current_time
            if dt_us <= 0 or pose_after is None:
                return
            position_delta = pose_after[:3, 3] - current_pose[:3, 3]
        velocity_vector = position_delta / dt_us * 1_000_000
        velocity_vector[1] = -velocity_vector[1]
        if not np.all(np.isfinite(velocity_vector)):
            logger.warning(f"Skipping non-finite velocity seed for {self.track.track_id}")
            return
        self.actor_inst.set_target_velocity(
            carla.Vector3D(velocity_vector[0], velocity_vector[1], velocity_vector[2])
        )


class NurecSensor:
    def __init__(
        self,
        parent_actor: NurecActor,
        transform: Optional[np.ndarray],
        renderer: NurecRenderer,
        callback: Callable[[np.ndarray], None],
        camera_spec: Union[Dict[str, Any], str],
        time_keeper: TimeKeeper,
        framerate=2,
        resolution_ratio=0.25,
    ):
        self.parent_actor = parent_actor
        self.transform = np.array(transform)
        self.renderer = renderer
        self.callback = callback
        self.time_keeper = time_keeper
        self.zero = None
        self.zero_count = 0
        self.framerate = framerate
        self.last_timestamp: float = 0.0  # Change to float to match timestamp
        self.rotation = R.from_matrix(self.transform[:3, :3])
        self.translation = self.transform[:3, 3]
        self.resolution_ratio = resolution_ratio
        self.camera_spec = camera_spec

    def _should_render(self) -> bool:
        if not self.time_keeper.is_running():
            return False
        timestamp_us = self.time_keeper.get_sim_time()
        timestamp = timestamp_us / 1_000_000
        if timestamp - self.last_timestamp < 1 / self.framerate:
            return False
        self.last_timestamp += 1 / self.framerate
        if timestamp - self.last_timestamp > 1 / self.framerate:
            self.last_timestamp = timestamp
        return True

    def build_tick_request(self, world: carla.WorldSnapshot) -> Optional[RGBRenderRequest]:
        """
        Build this camera's render request for the current tick, or None if
        the camera is not due (framerate) or its parent actor is gone.
        """
        if not self._should_render():
            return None

        actor = world.find(self.parent_actor.actor_inst.id)
        if actor is None:
            logger.warning(f"Parent actor {self.parent_actor} not found in world")
            return None

        # CARLA (left-handed, y right) -> NuRec rig frame (right-handed FLU)
        # through LibCarla's explicit handedness boundary, then the camera's
        # own T_sensor_rig, which is already in the rig frame.
        camera_transform = (
            carla_transform_to_nurec(actor.get_transform()) @ self.transform
        )
        return self.renderer.build_request(
            world,
            self.camera_spec,
            camera_transform,
            self.resolution_ratio,
        )

    def dispatch(self, image: np.ndarray) -> None:
        try:
            self.callback(image)
        except Exception as e:
            logger.error(f"Error in callback for camera {self.camera_spec.logical_id}: {e}")
            raise e

    def on_world_tick(self, world: carla.WorldSnapshot) -> None:
        """Single-camera path; NurecScenario.render batches instead."""
        request = self.build_tick_request(world)
        if request is None:
            return
        response = self.renderer.client_service.render_rgb(request)
        self.dispatch(self.renderer._decode(response, request))

class NurecLidarSensor:
    """
    Virtual lidar rendered by NuRec (render_lidar RPC, NRE >= 26.04). No CARLA
    actor is spawned; the sweep is generated from the neural reconstruction at
    the sensor pose. The callback receives (points Nx3, intensities N).
    """

    def __init__(
        self,
        parent_actor: NurecActor,
        transform: np.ndarray,
        renderer: NurecRenderer,
        callback: Callable[[np.ndarray, np.ndarray], None],
        time_keeper: TimeKeeper,
        lidar_type: str = "PANDAR128",
        framerate: int = 10,
        render_filter: Optional[LidarRenderFilter] = None,
    ):
        self.parent_actor = parent_actor
        self.transform = np.array(transform)
        self.renderer = renderer
        self.callback = callback
        self.time_keeper = time_keeper
        self.lidar_type = lidar_type
        self.framerate = framerate
        self.render_filter = render_filter
        self.last_timestamp: float = 0.0

    def _should_render(self) -> bool:
        if not self.time_keeper.is_running():
            return False
        timestamp = self.time_keeper.get_sim_time() / 1_000_000
        if timestamp - self.last_timestamp < 1 / self.framerate:
            return False
        self.last_timestamp += 1 / self.framerate
        if timestamp - self.last_timestamp > 1 / self.framerate:
            self.last_timestamp = timestamp
        return True

    def on_world_tick(self, world: carla.WorldSnapshot) -> None:
        if not self._should_render():
            return
        actor = world.find(self.parent_actor.actor_inst.id)
        if actor is None:
            logger.warning(f"Parent actor {self.parent_actor} not found in world")
            return
        sensor_transform = (
            carla_transform_to_nurec(actor.get_transform()) @ self.transform
        )
        points, intensities = self.renderer.render_lidar(
            world, sensor_transform, self.lidar_type, self.render_filter
        )
        try:
            self.callback(points, intensities)
        except Exception:
            # Never let a callback exception escape into libcarla's on_tick
            # dispatch thread: that silently kills event delivery and the next
            # world.tick() times out with an opaque std::exception.
            logger.exception("Error in lidar callback")


"""
A class to load a nurec reconstruction form a file. Spawns the actors in the carla scene and creates the sensors present in the recording.
"""


class NurecScenario(TimeKeeper, NuRecRenderService):
    def __init__(
        self,
        client: carla.Client,
        usdz_path: str,
        port: Optional[int] = None,
        move_spectator: bool = False,
        fps: int = 10,
        image=None,
        reuse_container: bool = True,
        renderer_backend: Optional[str] = None,
        image_format: str = "planar",
        enable_asset_editing: bool = False,
        harmonizer: bool = False,
        extra_server_args: Optional[List[str]] = None,
        cuda_visible_devices: Optional[str] = None,
        traffic_manager_port: int = 8000,
    ):
        # port=None lets the render service pick a free one (never CARLA's 2000).
        # --enable-editing-actors is always passed by the render service (pose
        # streaming needs it); enable_asset_editing is kept for API clarity.
        extra_args = list(extra_server_args or [])
        if harmonizer:
            extra_args.append("--enable-harmonizer")
        NuRecRenderService.__init__(self, usdz_path, port, image, reuse_container,
                                    renderer=renderer_backend,
                                    extra_server_args=extra_args,
                                    cuda_visible_devices=cuda_visible_devices)
        self.image_format = image_format
        self.client = client
        self.scenario: Optional[Scenario] = None
        self.renderer: Optional[NurecRenderer] = None
        self.cameras: List[NurecSensor] = []
        self.lidars: List[NurecLidarSensor] = []
        # Built in __enter__ by probing the live server: static blueprint
        # dimension tables go stale across CARLA versions (UE5 renamed the
        # whole vehicle catalog).
        self.blueprint_library: Optional[BlueprintLibrary] = None
        self.last_time = 0
        self.actors_to_disable_physics: List[NurecActor] = []
        self.move_spectator = move_spectator
        self.running = False
        self.active_actors: Dict[int, str] = {}
        self.actor_mapping: Dict[str, NurecActor] = {}
        self.t_scenario_carla = np.eye(4)
        self.fps = fps
        self.actor_blueprints: Dict[int, str] = {}
        self.usdz_path = usdz_path
        self.synchronous_mode = False
        self.path_spacing = 1
        self.traffic_manager: Optional[carla.TrafficManager] = None
        self.traffic_manager_port = traffic_manager_port
        self.ego_speeds: List[float] = []
        self.ego_path_start_time = -1
        self.ego_following_path = False
        self.ego_trajectory_follower: Optional[SimpleTrajectoryFollower] = None  # Simple trajectory follower for ego
        self.default_follow_path = False
        # When True, vehicles that enter the scene later are also handed to
        # the Traffic Manager (see set_all_actors_carla_controlled).
        self.carla_controls_new_actors = False
        self.freeze_new_actors = False
        # Alpamayo needs background traffic to react to its physics-driven
        # ego instead of blindly preserving the recording.  Keep this opt-in
        # so the standalone NuRec replay can still reproduce the source take.
        self.carla_traffic_collision_aware = False
        # Track ids held collision-free until the ego has opened a safe rear
        # gap. They remain visible at their CARLA pose in NRE when the artifact
        # exposes them as controllable assets.
        self.pending_tm_handoffs: Set[str] = set()
        # Traffic Manager registration calls into native CARLA code and must
        # not run from a world.on_tick callback. New replay actors are found
        # by _update_actors in that callback, then handed over immediately
        # after world.tick() returns on the owning thread.
        self.deferred_carla_handoffs: Set[str] = set()
        self.last_render_frame = -1
        self.last_render_timestamp_us = 0
        self._on_tick_ids: List[int] = []

    def add_ego(self, ego_bp: str = EGO_LABEL, enable_physics: bool = False, move_spectator: bool = True) -> carla.Actor:
        if self.scenario is None:
            raise RuntimeError("Scenario not initialized. Call __enter__ first.")
            
        world = self.client.get_world()

        bp_library = world.get_blueprint_library()
        ego_bp_obj = bp_library.find(ego_bp)

        ego_poses = self.scenario.ego_poses.poses
        if ego_poses is None or len(ego_poses) == 0:
            raise RuntimeError("No ego poses available in scenario")
        ego_spawn = self.t_scenario_carla @ ego_poses[0]
        carla_ego_pose = mat_to_carla_transform(ego_spawn)

        ego_instance = world.try_spawn_actor(ego_bp_obj, carla_ego_pose)

        # even if the spawn fails we want to spawn the ego actor somewhere...
        if ego_instance is None:
            logger.debug(
                f"Failed to spawn ego actor {ego_bp} at {carla_ego_pose}. Trying at an arbitrary location."
            )
            carla_ego_pose = carla.Transform(
                carla.Location(x=0, y=0, z=1000), carla.Rotation(pitch=0, yaw=0, roll=0)
            )
            ego_instance = world.spawn_actor(ego_bp_obj, carla_ego_pose)

        self.actor_mapping[EGO_TRACK_ID] = NurecActor(
            ego_instance, self.scenario.ego_poses, enable_physics
        )
        self.scenario.ego_poses.set_ignore_out_of_bounds(True)
        self.scenario.ego_poses.set_transform(self.t_scenario_carla)

        self.actor_blueprints[ego_instance.id] = ego_bp_obj.id

        if not enable_physics:
            self.actors_to_disable_physics.append(self.actor_mapping[EGO_TRACK_ID])

        if move_spectator:
            spectator = self.client.get_world().get_spectator()
            ego = self.actor_mapping[EGO_TRACK_ID].actor_inst

            spectator_transform = np.eye(4)
            spectator_transform[:3, 3] = [-5, 0, 3]
            ego_transform = ego_spawn @ spectator_transform
            spectator.set_transform(mat_to_carla_transform(ego_transform))

        return ego_instance

    def _add_actors(self, tracks) -> List[NurecActor]:
        world = self.client.get_world()
        bp_library = world.get_blueprint_library()

        new_actors = []
        for track in tracks:
            if not (track.label in VEHICLE_LABELS or track.label == "person"):
                continue

            best_fit_blueprint = self.blueprint_library.get_best_fit_blueprint(
                track.dims, track.label != "person"
            )
            actor_bp = bp_library.find(best_fit_blueprint.id)
            blueprint_id = best_fit_blueprint.id

            spawn_pose_array = track.interpolate_pose_matrix(track.start_time())
            spawn_pose_matrix = self.blueprint_library.apply_offset_to_pose(
                spawn_pose_array, blueprint_id, inverse=True
            )
            spawn_pose = mat_to_carla_transform(spawn_pose_matrix)

            actor_inst = world.try_spawn_actor(actor_bp, spawn_pose)
            if actor_inst is None:
                # try again 10 meters up
                spawn_pose.location.z += 10
                actor_inst = world.try_spawn_actor(actor_bp, spawn_pose)
                if actor_inst is None:
                    logger.warning(
                        f"Failed to spawn actor {track.track_id} ({blueprint_id}) at {spawn_pose}"
                    )
                    continue
            
            actor_inst.set_transform(mat_to_carla_transform(spawn_pose_matrix))

            new_actors.append(
                NurecActor(actor_inst, track, physics=False, blueprint_id=blueprint_id)
            )
            self.actor_blueprints[actor_inst.id] = actor_bp.id

        for actor in new_actors:
            self.actors_to_disable_physics.append(actor)
        return new_actors

    def _read_xodr_from_nurec(self, nurec_file: str) -> str:
        """
        Reads the map.xodr file from a NUREC zip file.

        Args:
            nurec_file: Path to the NUREC zip file containing map.xodr

        Returns:
            str: Content of the map.xodr file

        Raises:
            FileNotFoundError: If the NUREC file doesn't exist
            KeyError: If map.xodr is not found in the zip file
            zipfile.BadZipFile: If the file is not a valid zip file
        """
        try:
            with zipfile.ZipFile(nurec_file, "r") as zip_ref:
                # Check if map.xodr exists in the zip file
                if "map.xodr" not in zip_ref.namelist():
                    available_files = zip_ref.namelist()
                    raise KeyError(
                        f"map.xodr not found in {nurec_file}. Available files: {available_files}"
                    )

                # Read the map.xodr file content
                with zip_ref.open("map.xodr") as xodr_file:
                    data = xodr_file.read().decode("utf-8")

                filename = os.path.basename(nurec_file)
                logger.debug(f"Successfully loaded map.xodr from {filename}")
                return data

        except zipfile.BadZipFile:
            logger.error(f"Invalid zip file: {nurec_file}")
            raise
        except FileNotFoundError:
            logger.error(f"NUREC file not found: {nurec_file}")
            raise
        except Exception as e:
            logger.error(f"Error reading XODR from NUREC file {nurec_file}: {e}")
            raise

    def __enter__(self):
        super().__enter__()
        data = self._read_xodr_from_nurec(self.usdz_path)

        world = self.client.generate_opendrive_world(
            data,
            carla.OpendriveGenerationParameters(
                vertex_distance=2.0,
                max_road_length=500.0,
                wall_height=0.0,
                # additional_width widens every junction driving lane by this
                # much PER SIDE. The imported example shipped 7.6, which turns
                # 3.5 m lanes into ~19 m ribbons that stack over neighbouring
                # roads ("roads on top of roads"). CARLA's default is 0.6.
                additional_width=0.6,
                smooth_junctions=True,
                enable_mesh_visibility=True,
            ),
        )

        self.scenario = Scenario(self.usdz_path)

        # Measure blueprint dimensions against the running server (cached per
        # CARLA version) instead of trusting checked-in JSONs.
        self.blueprint_library = BlueprintLibrary(
            world=world, cache_key=self.client.get_server_version()
        )

        t_world_base = self.scenario.t_world_base
        self.t_scenario_carla = get_t_rig_enu_from_ecef(t_world_base, data)

        self.scenario.tracks.set_view_transform(self.t_scenario_carla)
        self.scenario.tracks.set_mininmum_lifetime(1 / 10)

        self.add_ego()

        self.renderer = NurecRenderer(
            self.scenario,
            "localhost",
            # final_port is what the render service actually bound (it may
            # have been auto-picked); self.port can be None.
            self.final_port,
            self.active_actors,
            self.t_scenario_carla,
            self.blueprint_library,
            self.actor_blueprints,
            image_format=self.image_format,
        )

        self._warm_cache()

        world = self.client.get_world()
        self._on_tick_ids = [
            world.on_tick(lambda snapshot: self.render(snapshot)),
            world.on_tick(lambda snapshot: self.update(snapshot)),
        ]

        self.tick()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        # Best-effort teardown: a Traffic Manager left registered in
        # synchronous mode by a dead client wedges the server (every later
        # apply_settings/tick fails with std::exception), so always
        # unregister it and restore asynchronous mode before leaving.
        self.running = False
        try:
            world = self.client.get_world()
            for callback_id in self._on_tick_ids:
                world.remove_on_tick(callback_id)
            self._on_tick_ids.clear()
        except Exception:
            logger.exception("Could not unregister NuRec world callbacks")
        try:
            if self.traffic_manager is not None:
                self.traffic_manager.set_synchronous_mode(False)
        except Exception:
            logger.exception("Could not disable traffic manager sync mode")
        try:
            world = self.client.get_world()
            settings = world.get_settings()
            if settings.synchronous_mode:
                settings.synchronous_mode = False
                settings.fixed_delta_seconds = None
                world.apply_settings(settings)
        except Exception:
            logger.exception("Could not restore asynchronous mode")
        return super().__exit__(exc_type, exc_val, exc_tb)

    def _warm_cache(self) -> None:
        """
        Renders an initial image at the ego's starting position before the scenario starts.
        This helps initialize the rendering pipeline.
        """
        logger.info("Warming renderer cache...")
        if not self.renderer or self.scenario is None:
            logger.warning(
                "Cannot warm cache: renderer or scenario not initialized"
            )
            return

        world = self.client.get_world()
        snapshot = world.get_snapshot()

        camera_logical_id = list(self.scenario.camera_calibrations.values())[0].logical_sensor_name
        camera_spec = self.renderer.get_camera_spec(camera_logical_id)

        self.renderer.render(snapshot, camera_spec, np.eye(4))

        logger.debug("Cache warmed")

    def start_replay(self, synchronous_mode: bool = True) -> None:
        """
        Starts the scenario replay.
        """

        self.running = False
        self.synchronous_mode = synchronous_mode
        if synchronous_mode:
            world = self.client.get_world()
            world.apply_settings(
                carla.WorldSettings(
                    synchronous_mode=True, fixed_delta_seconds=1 / (self.fps * 2)
                )
            )
            self.last_time = world.get_snapshot().timestamp.elapsed_seconds
        else:
            self.last_time = 0
        self.running = True

    def is_done(self) -> bool:
        return not self.running

    def pause(self) -> None:
        self.running = False

    def resume(self) -> None:
        self.running = True

    def update(self, snapshot: carla.WorldSnapshot) -> None:
        if not self.running:
            self._update(0)
            return

        delta_t = snapshot.timestamp.elapsed_seconds - self.last_time
        if delta_t < 1 / self.fps and not self.synchronous_mode:
            return

        if self.last_time == 0:
            self.last_time = snapshot.timestamp.elapsed_seconds
            return

        self.last_time = snapshot.timestamp.elapsed_seconds

        if self.synchronous_mode:
            expected_delta_t = 1 / (self.fps * 2)
            if abs(delta_t - expected_delta_t) > 0.00001:
                logger.debug("Delta t is not what we expected, skipping update")
                return

        self._update(delta_t * 1_000_000)

    def _set_physics_on_actors(self) -> None:
        if self.scenario is None:
            return
        actors_done_count = 0
        for i, actor in enumerate(self.actors_to_disable_physics):
            actors_done_count = i + 1
            # Guard each actor separately: an actor destroyed between being
            # queued and processed (replay teardown) throws on every RPC, and
            # a shared try aborted the remaining queue and re-logged forever.
            try:
                if not actor.alive:
                    continue
                if not actor.physics:
                    actor.actor_inst.set_simulate_physics(False)
                else:
                    if self.default_follow_path:
                        self.set_follow_path(actor.track.track_id)
                        actor.set_physics(
                            True, self.scenario.tracks.current_time + 100_000
                        )

                next_pose = actor.track.interpolate_pose_matrix(
                    self.scenario.tracks.current_time
                )
                if next_pose is None:
                    # Track already ended (e.g. replay teardown); keep the
                    # actor where it is instead of matmul-ing a None pose.
                    continue
                next_pose = self.blueprint_library.apply_offset_to_pose(
                    next_pose, actor.blueprint_id, inverse=True
                )
                next_pose = mat_to_carla_transform(next_pose)

                actor.actor_inst.set_transform(next_pose)
            except Exception as e:
                logger.error(
                    f"Error disabling physics for actor {actor.track.track_id}: {e}"
                )
        self.actors_to_disable_physics = self.actors_to_disable_physics[
            actors_done_count:
        ]

    def _update_actors(self, time_step: float) -> None:
        if self.scenario is None:
            return
        new_actor_tracks, removed_actor_tracks = self.scenario.tracks.update(time_step)
        for removed_actor_track in removed_actor_tracks:
            if not removed_actor_track.track_id in self.actor_mapping:
                continue
            self.pending_tm_handoffs.discard(removed_actor_track.track_id)
            self.deferred_carla_handoffs.discard(removed_actor_track.track_id)
            actor_to_delete = self.actor_mapping[removed_actor_track.track_id]
            del self.active_actors[actor_to_delete.actor_inst.id]
            if actor_to_delete.tm_controlled:
                # Unregister from the Traffic Manager before destroying:
                # destroying a TM-registered vehicle in synchronous mode
                # crashes the tick.
                try:
                    actor_to_delete.actor_inst.set_autopilot(
                        False, self.traffic_manager_port
                    )
                except Exception:
                    logger.exception(
                        f"Could not unregister actor {removed_actor_track.track_id} from autopilot"
                    )
            actor_to_delete.destroy()
            del self.actor_mapping[removed_actor_track.track_id]

        new_actors = self._add_actors(new_actor_tracks)
        for actor in new_actors:
            self.actor_mapping[actor.track.track_id] = actor
            self.active_actors[actor.actor_inst.id] = actor.track.track_id
            if (self.carla_controls_new_actors
                    and actor.track.label in VEHICLE_LABELS):
                self.deferred_carla_handoffs.add(actor.track.track_id)

    def _move_dynamic_actors(self) -> None:
        if self.scenario is None:
            return
        current_time = self.get_sim_time()
        next_ego_pose = None

        for actor in self.actor_mapping.values():
            # Hold TM vehicles at the end of their recorded route: once the
            # final waypoint is consumed the TM would otherwise keep driving
            # them down the road, diverging from the scenario.
            if (actor.physics and actor.alive and not actor.tm_route_done
                    and actor.tm_route_end is not None
                    and self.traffic_manager is not None):
                try:
                    if not actor.actor_inst.is_alive:
                        actor.alive = False
                        actor.tm_route_done = True
                        continue
                    if actor.actor_inst.get_location().distance(actor.tm_route_end) < 4.0:
                        actor.tm_route_done = True
                        self.traffic_manager.set_desired_speed(actor.actor_inst, 0.0)
                except Exception as exc:
                    # A track can disappear between the client snapshot and
                    # this query. Disable this optional hold after one failure
                    # instead of retrying a stale actor on every tick.
                    actor.tm_route_done = True
                    logger.warning(
                        f"Disabling route-end hold for actor "
                        f"{actor.track.track_id}: {exc}"
                    )
            if not actor.physics and actor.alive and actor.track.dynamic:

                # Apply offset for vehicle actors
                if (
                    actor.track.label in VEHICLE_LABELS
                    and actor.blueprint_id is not None
                ):
                    next_pose = actor.track.interpolate_pose_matrix(current_time)
                    if next_pose is None:
                        continue
                    # Apply offset using helper method
                    next_pose_matrix = self.blueprint_library.apply_offset_to_pose(
                        next_pose, actor.blueprint_id, inverse=True
                    )
                    carla_transform = mat_to_carla_transform(next_pose_matrix)

                    if actor.track.ego:
                        next_ego_pose = next_pose_matrix
                else:
                    next_pose = actor.track.interpolate_pose_matrix(current_time)
                    if next_pose is None:
                        continue
                    carla_transform = mat_to_carla_transform(next_pose)
                    if actor.track.ego:
                        next_ego_pose = actor.track.interpolate_pose_matrix(
                            current_time
                        )

                actor.actor_inst.set_transform(carla_transform)

        if self.move_spectator and self.running:
            spectator = self.client.get_world().get_spectator()
            ego = self.actor_mapping[EGO_TRACK_ID].actor_inst

            # we want to move the spectator to the ego's most up to date pose,
            # the previous call to set_transform won't be reflected in the
            # call to get_transform
            if next_ego_pose is not None:
                ego_transform = next_ego_pose
            else:
                ego_transform = carla_transform_to_nurec(ego.get_transform())
            spectator_transform = np.eye(4)
            spectator_transform[:3, 3] = [-5, 0, 3]
            ego_transform = ego_transform @ spectator_transform
            spectator.set_transform(mat_to_carla_transform(ego_transform))

    def _check_done(self) -> None:
        if self.scenario is None:
            return
        if (
            self.scenario.tracks.current_time
            >= self.scenario.metadata["pose-range"]["end-timestamp_us"]
        ):
            self.running = False

    def _update(self, time_step: float) -> None:
        self._set_physics_on_actors()
        self._update_actors(time_step)
        self._move_dynamic_actors()
        self._update_ego_trajectory_follower()
        self._check_done()

    def get_available_cameras(self) -> List[str]:
        """
        Get the names of all cameras available in the NUREC reconstruction.
        
        These are the logical sensor names from the camera calibrations in the scenario,
        which can be used to create cameras that match the original recording setup.
        
        Returns:
            List[str]: List of logical camera names available in the reconstruction
            
        Raises:
            RuntimeError: If scenario is not initialized (call __enter__ first)
        """
        if self.scenario is None:
            raise RuntimeError("Scenario not initialized. Call __enter__ first.")
        return [
            calibration.logical_sensor_name
            for calibration in self.scenario.camera_calibrations.values()
        ]

    def add_camera(
        self,
        camera_spec: Union[Dict[str, Any], str],
        callback: Callable[[np.ndarray], None],
        transform: Optional[np.ndarray] = None,
        framerate: int = 10,
        resolution_ratio: float = 1,
        translation: np.ndarray = np.zeros(3),
    ) -> None:
        """
        Add a camera sensor to the NUREC scenario that renders images using neural reconstruction.

        Args:
            camera_spec: Either a string (logical camera name from reconstruction) or 
                        dictionary of camera parameters (see dict_to_camera_spec for details)
            callback: Function to call when new images are available, receives np.ndarray image (RGB, HxWx3)
            transform: 4x4 transformation matrix relative to ego vehicle coordinate frame.
                      Required if camera_spec is a dictionary, optional if using existing camera name
            framerate: Camera framerate in Hz (default: 10)
            resolution_ratio: Resolution scaling factor (default: 1.0)
            translation: Additional translation offset [x, y, z] in meters (default: [0, 0, 0])

        Raises:
            RuntimeError: If scenario is not initialized
            ValueError: If transform is required but not provided
            Exception: If camera name is not found in available cameras

        Example:
            # Use existing camera from reconstruction
            scenario.add_camera("front_wide_120fov", image_callback)
            
            # Create custom F-theta camera
            custom_params = {
                'resolution_w': 640, 
                'resolution_h': 480,
                'camera_type': 'ftheta',
                'logical_id': 'front_wide_120fov'  # Must match existing camera
            }
            transform = np.eye(4)  # Identity transform (ego vehicle position)
            scenario.add_camera(custom_params, image_callback, transform)
            
            # CARLA-style camera (automatically converted)
            carla_params = {'image_size_x': 800, 'image_size_y': 600, 'fov': 90.0}
            scenario.add_camera(carla_params, image_callback, transform)
        """
        if self.scenario is None or self.renderer is None:
            raise RuntimeError("Scenario not initialized. Call __enter__ first.")
            
        # Handle different camera_spec types
        if isinstance(camera_spec, str):
            transform_found = None
            for camera_id in self.scenario.camera_calibrations:
                camera = self.scenario.camera_calibrations[camera_id]
                if camera.logical_sensor_name == camera_spec:
                    transform_found = np.array(camera.T_sensor_rig)
                    break
            if transform_found is None:
                available_cameras = [
                    camera.logical_sensor_name
                    for camera in self.scenario.camera_calibrations.values()
                ]
                raise Exception(
                    f"Camera {camera_spec} not found, available cameras: {available_cameras}"
                )
            transform = transform_found
            camera_spec = self.renderer.get_camera_spec(camera_spec)
        elif transform is None:
            raise ValueError(
                "transform is required if custom camera parameters are provided."
            )
        else:
            camera_spec = dict_to_camera_spec(camera_spec)

        if camera_spec.logical_id not in self.get_available_cameras():
            raise Exception(
                f"Camera {camera_spec.logical_id} not found, available cameras: {self.get_available_cameras()}. Logical ID needs to be one of the available cameras to act as reference for unspecified intrinsics."
            )

        # move camera
        t_translation = np.eye(4)
        t_translation[:3, 3] = translation
        transform = t_translation @ transform

        ego = self.actor_mapping[EGO_TRACK_ID]
        self.cameras.append(
            NurecSensor(
                ego,
                transform,
                self.renderer,
                callback,
                camera_spec,
                self,
                framerate,
                resolution_ratio,
            )
        )

    def add_lidar(
        self,
        callback: Callable[[np.ndarray, np.ndarray], None],
        transform: Optional[np.ndarray] = None,
        lidar_type: str = "PANDAR128",
        framerate: int = 10,
        render_filter: Optional[LidarRenderFilter] = None,
    ) -> None:
        """
        Add a neural-rendered lidar attached to the ego vehicle (NRE >= 26.04).

        Args:
            callback: Receives (points Nx3 float32, intensities N float32) per sweep.
            transform: 4x4 sensor-to-ego matrix (default: identity, i.e. rig origin).
            lidar_type: 'PANDAR128' or 'AT128'.
            framerate: Sweep rate in Hz.
            render_filter: Optional LidarRenderFilter (raydrop/opacity/distance).
        """
        if self.scenario is None or self.renderer is None:
            raise RuntimeError("Scenario not initialized. Call __enter__ first.")
        if transform is None:
            transform = np.eye(4)
        ego = self.actor_mapping[EGO_TRACK_ID]
        self.lidars.append(
            NurecLidarSensor(
                ego,
                transform,
                self.renderer,
                callback,
                self,
                lidar_type,
                framerate,
                render_filter,
            )
        )

    def tick(self) -> int:
        world = self.client.get_world()
        frame = int(world.tick())
        self._process_deferred_traffic_handoffs()
        return frame

    def render(self, snapshot: carla.WorldSnapshot) -> None:
        self.last_render_frame = int(snapshot.frame)
        self.last_render_timestamp_us = self.get_sim_time()
        if self.seconds_since_start() == 0:
            return
        if self.renderer is None:
            return
        # Gather every camera due this tick and render them in ONE gRPC round
        # trip (batch_render_rgb); NurecRenderer falls back to sequential
        # render_rgb on servers without the batch API.
        # This method runs inside libcarla's on_tick dispatch thread: an
        # exception escaping here stops event delivery entirely and the next
        # world.tick() times out with an opaque std::exception. Log instead.
        try:
            due: List[Tuple[str, NurecSensor, RGBRenderRequest]] = []
            for i, camera in enumerate(self.cameras):
                request = camera.build_tick_request(snapshot)
                if request is not None:
                    due.append((f"cam{i}", camera, request))
            if due:
                images = self.renderer.render_batch([(name, req) for name, _, req in due])
                for name, camera, _ in due:
                    if name in images:
                        camera.dispatch(images[name])
        except Exception:
            logger.exception("Camera render failed this tick")

        # Lidar has no batch RPC; render due sweeps sequentially.
        for lidar in self.lidars:
            try:
                lidar.on_world_tick(snapshot)
            except Exception:
                logger.exception("Lidar render failed this tick")

    def get_sim_time(self) -> int:
        """
        Returns the current time in the scenario in microseconds.
        """
        if self.scenario is None:
            return 0
        return self.scenario.tracks.current_time

    def seconds_since_start(self) -> float:
        if self.scenario is None:
            return 0.0
        return self.scenario.tracks.get_current_time_seconds()

    def is_running(self) -> bool:
        return self.running

    def apply_ego_control(self, control: carla.VehicleControl) -> None:
        self.actor_mapping[EGO_TRACK_ID].set_physics(True, self.get_sim_time())
        self.actor_mapping[EGO_TRACK_ID].actor_inst.apply_control(control)

    def set_ego_autopilot(self, autopilot: bool) -> None:
        self.actor_mapping[EGO_TRACK_ID].set_physics(autopilot, self.get_sim_time())
        self.actor_mapping[EGO_TRACK_ID].actor_inst.set_autopilot(
            autopilot, self.traffic_manager_port
        )

    def set_ego_follow_path(self, path: Optional[List] = None, spacing: int = 1000000) -> None:
        self.set_follow_path(EGO_TRACK_ID, path, spacing)

    def _has_usable_tm_route(self, actor: "NurecActor") -> bool:
        """
        A vehicle is only handed to the Traffic Manager when its recording
        actually travels somewhere. Parked or creeping vehicles (recorded
        travel below MIN_TM_ROUTE_M) give the TM a degenerate route and it
        wanders them into neighboring geometry; CARLA-control mode holds them
        under physics instead.
        """
        dist_m, _ = self._track_travel_stats(actor.track, self.get_sim_time())
        if dist_m < MIN_TM_ROUTE_M:
            logger.info(
                f"Actor {actor.track.track_id} has no usable TM route "
                f"(recorded travel {dist_m:.1f} m < {MIN_TM_ROUTE_M} m)"
            )
            return False
        return True

    def _track_travel_stats(self, track, from_time_us: float) -> Tuple[float, float]:
        """
        (distance_m, duration_s) of the recorded track from from_time_us on.
        Used to gate TM handoff (parked cars have no route to follow) and to
        seed the TM desired speed from the recording.
        """
        path = track.get_path(1_000_000, start_time=int(from_time_us))
        if len(path) < 2:
            return 0.0, 0.0
        dist = sum(path[i].distance(path[i + 1]) for i in range(len(path) - 1))
        duration = (track.end_time() - max(from_time_us, track.start_time())) / 1e6
        return dist, max(duration, 0.0)

    def set_follow_path(
        self,
        track_id: str,
        path: Optional[List[carla.Location]] = None,
        spacing: int = 1000000,
        collision_aware: bool = False,
    ) -> None:
        """
        Follows a path at a given spacing. If no path is provided, the path is generated from the track.
        Each waypoint should be spacing microseconds apart.
        The TM desired speed is seeded from the recording's average speed so
        handed vehicles keep pace with the scenario instead of the TM default.
        """
        self._enable_traffic_manager()
        if self.traffic_manager is None:
            raise RuntimeError("Traffic manager not initialized")

        actor = self.actor_mapping[track_id]
        dist_m, duration_s = self._track_travel_stats(actor.track, self.get_sim_time())
        speed_mps = dist_m / duration_s if duration_s > 0.5 else 0.0
        follow_distance = max(
            MIN_TM_FOLLOW_DISTANCE_M, speed_mps * TM_TIME_HEADWAY_S
        )

        if collision_aware:
            # A newly materialized recorded actor may be immediately behind a
            # diverged/slower ego. Never inject its recorded velocity before
            # TM has had a chance to calculate a collision response.
            actor.actor_inst.set_collisions(False)
            actor.set_physics(
                True, self.get_sim_time(), seed_recorded_velocity=False
            )
            actor.actor_inst.apply_control(
                carla.VehicleControl(throttle=0.0, brake=1.0, hand_brake=True)
            )
        else:
            actor.set_physics(True, self.get_sim_time())

        if path is None:
            path = actor.track.get_path(spacing, start_time=int(self.get_sim_time()))
        actor.actor_inst.set_autopilot(True, self.traffic_manager_port)
        actor.tm_controlled = True
        self.traffic_manager.set_path(actor.actor_inst, path)
        actor.tm_route_end = path[-1] if path else None
        actor.tm_route_done = False
        if duration_s > 0.5 and dist_m > 1.0:
            # Mean recorded speed, small margin because stops are included.
            speed_kmh = 3.6 * dist_m / duration_s * 1.05
            self.traffic_manager.set_desired_speed(actor.actor_inst, speed_kmh)
        self.traffic_manager.update_vehicle_lights(actor.actor_inst, True)
        self.traffic_manager.random_left_lanechange_percentage(actor.actor_inst, 0)
        self.traffic_manager.random_right_lanechange_percentage(actor.actor_inst, 0)
        self.traffic_manager.auto_lane_change(actor.actor_inst, False)
        # Dataset-faithful replay historically ignored traffic so actors kept
        # their timing.  That is unsafe around a separately controlled ego:
        # a background vehicle will simply drive through the Alpamayo car.
        # Collision-aware mode preserves the route but lets TM brake/yield.
        self.traffic_manager.distance_to_leading_vehicle(
            actor.actor_inst, follow_distance if collision_aware else 0.0
        )
        actor.tm_follow_distance = follow_distance if collision_aware else 0.0
        self.traffic_manager.ignore_lights_percentage(actor.actor_inst, 100)
        self.traffic_manager.ignore_vehicles_percentage(
            actor.actor_inst, 0 if collision_aware else 100
        )
        if collision_aware:
            actor.actor_inst.set_collisions(True)

    def _rear_handoff_status(
        self, actor: "NurecActor"
    ) -> Tuple[bool, float, float]:
        """Return (safe, bumper gap, required gap) for a same-lane rear actor."""
        ego = self.get_ego_actor()
        if ego is None:
            return True, float("inf"), MIN_TM_FOLLOW_DISTANCE_M

        ego_inst = ego.actor_inst
        ego_tf = ego_inst.get_transform()
        ego_location = ego_tf.location
        actor_location = actor.actor_inst.get_location()
        delta = actor_location - ego_location
        forward = ego_tf.get_forward_vector()
        right = ego_tf.get_right_vector()
        longitudinal = delta.x * forward.x + delta.y * forward.y + delta.z * forward.z
        lateral = abs(delta.x * right.x + delta.y * right.y + delta.z * right.z)
        lane_envelope = (
            float(ego_inst.bounding_box.extent.y)
            + float(actor.actor_inst.bounding_box.extent.y)
            + 1.0
        )

        dist_m, duration_s = self._track_travel_stats(actor.track, self.get_sim_time())
        recorded_speed_mps = dist_m / duration_s if duration_s > 0.5 else 0.0
        required_gap = max(
            MIN_TM_FOLLOW_DISTANCE_M,
            recorded_speed_mps * TM_TIME_HEADWAY_S,
        )
        # Actors ahead of the ego or outside its lane do not present the
        # immediate rear-end activation hazard addressed by this guard.
        if longitudinal >= 0.0 or lateral > lane_envelope:
            return True, float("inf"), required_gap

        bumper_gap = (
            -longitudinal
            - float(ego_inst.bounding_box.extent.x)
            - float(actor.actor_inst.bounding_box.extent.x)
        )
        return bumper_gap >= required_gap, bumper_gap, required_gap

    def _hold_actor_with_physics(
        self, actor: "NurecActor", collisions: bool = True
    ) -> None:
        """Stop a route-less vehicle without returning it to rigid replay."""
        actor.set_physics(
            True, self.get_sim_time(), seed_recorded_velocity=False
        )
        # Chaos can recreate the physics body when simulation is enabled,
        # restoring its default collision state. Apply the requested state to
        # the final body, not the pre-physics actor shell.
        actor.actor_inst.set_collisions(collisions)
        actor.tm_route_end = None
        actor.tm_route_done = True
        actor.tm_controlled = False
        actor.actor_inst.apply_control(
            carla.VehicleControl(throttle=0.0, brake=1.0, hand_brake=True)
        )

    def _handoff_background_vehicle(self, actor: "NurecActor") -> None:
        """Put one non-ego vehicle under CARLA physics, never rigid replay."""
        track_id = actor.track.track_id
        if (self.scenario is not None
                and track_id not in self.scenario.controllable_tracks):
            # NRE ignores pose overrides for these tracks. Keep the visual
            # recording, but make its CARLA proxy non-colliding so an invisible
            # or spatially divergent proxy cannot affect the ego.
            actor.actor_inst.set_collisions(False)
            logger.info(
                f"Actor {track_id} is neural-replay-only; disabled CARLA collisions"
            )
            return
        if self._has_usable_tm_route(actor):
            if self.carla_traffic_collision_aware:
                safe, gap_m, required_m = self._rear_handoff_status(actor)
                if not safe:
                    self._hold_actor_with_physics(actor, collisions=False)
                    self.pending_tm_handoffs.add(track_id)
                    logger.info(
                        f"Delayed TM actor {track_id}: rear gap {gap_m:.1f} m "
                        f"< required {required_m:.1f} m"
                    )
                    return
            try:
                self.set_follow_path(
                    track_id,
                    collision_aware=self.carla_traffic_collision_aware,
                )
                logger.info(
                    f"Handed actor {track_id} to collision-aware CARLA TM"
                )
                return
            except Exception as exc:
                logger.warning(
                    f"TM handoff failed for actor {track_id}; holding it with "
                    f"CARLA physics instead: {exc}"
                )
        self._hold_actor_with_physics(actor)
        logger.info(f"Holding route-less actor {track_id} with CARLA physics")

    def _activate_safe_pending_vehicles(self) -> None:
        """Activate delayed rear traffic only after the ego opens a safe gap."""
        for track_id in list(self.pending_tm_handoffs):
            actor = self.actor_mapping.get(track_id)
            if actor is None or not actor.alive:
                self.pending_tm_handoffs.discard(track_id)
                continue
            safe, gap_m, required_m = self._rear_handoff_status(actor)
            if not safe:
                continue
            try:
                self.set_follow_path(track_id, collision_aware=True)
                self.pending_tm_handoffs.discard(track_id)
                logger.info(
                    f"Activated delayed TM actor {track_id}: rear gap "
                    f"{gap_m:.1f} m >= required {required_m:.1f} m"
                )
            except Exception as exc:
                logger.warning(
                    f"Could not activate delayed TM actor {track_id}: {exc}"
                )

    def _process_deferred_traffic_handoffs(self) -> None:
        """Perform native TM operations outside the world-tick callback."""
        for track_id in list(self.deferred_carla_handoffs):
            self.deferred_carla_handoffs.discard(track_id)
            actor = self.actor_mapping.get(track_id)
            if actor is None or not actor.alive:
                continue
            try:
                if self.freeze_new_actors:
                    self._hold_actor_with_physics(actor, collisions=False)
                    logger.info(
                        f"Held actor {track_id} collision-free; traffic disabled"
                    )
                else:
                    self._handoff_background_vehicle(actor)
            except Exception as exc:
                logger.warning(
                    f"Could not hand new actor {track_id} to CARLA control: {exc}"
                )
        self._activate_safe_pending_vehicles()

    def disable_all_traffic(self, include_future_actors: bool = True) -> int:
        """Stop non-ego vehicles without replay, collisions, or Traffic Manager."""
        self.carla_controls_new_actors = include_future_actors
        self.freeze_new_actors = include_future_actors
        disabled = 0
        for track_id, actor in list(self.actor_mapping.items()):
            if track_id == EGO_TRACK_ID or not actor.alive or actor.physics:
                continue
            if actor.track.label not in VEHICLE_LABELS:
                continue
            self._hold_actor_with_physics(actor, collisions=False)
            disabled += 1
        logger.info(f"Held {disabled} actors collision-free; traffic disabled")
        return disabled

    def set_all_actors_carla_controlled(
        self,
        include_future_actors: bool = True,
        collision_aware: bool = False,
    ) -> int:
        """
        Hand every currently spawned vehicle (ego excluded) to CARLA physics.
        Moving vehicles with usable routes go to Traffic Manager, seeded with
        their recorded route; route-less vehicles are held stationary. Their
        poses are streamed to NRE every tick, so supported neural assets follow
        CARLA's simulation instead of the recording. Combined with the regular
        CARLA API this is the basis for authoring custom NuRec scenarios.

        Notes:
        - Only tracks the artifact marks controllable can diverge visually in
          the neural render (the server ignores pose updates for the rest).
        - Walkers stay replay-driven; no non-ego vehicle does.

        Args:
            include_future_actors: Also hand over vehicles that enter the
                scene later (default: True).
            collision_aware: Make Traffic Manager respect other vehicles and
                keep a four-metre following distance. Use this when another
                controller, such as Alpamayo, drives the ego.

        Returns:
            Number of actors handed over now.
        """
        # Initialize TM outside the world-tick callback even when no traffic
        # is active yet. Some NuRec takes introduce every non-ego actor later;
        # lazily creating synchronous TM from that callback is re-entrant and
        # CARLA rejects it with std::exception.
        if include_future_actors:
            self._enable_traffic_manager()

        self.carla_controls_new_actors = include_future_actors
        self.freeze_new_actors = False
        self.carla_traffic_collision_aware = collision_aware
        handed_over = 0
        for track_id, actor in list(self.actor_mapping.items()):
            if track_id == EGO_TRACK_ID or not actor.alive or actor.physics:
                continue
            if actor.track.label not in VEHICLE_LABELS:
                continue
            try:
                self._handoff_background_vehicle(actor)
                handed_over += 1
            except Exception as e:
                logger.warning(
                    f"Could not hand actor {track_id} to CARLA control: {e}"
                )
        logger.info(f"Handed {handed_over} actors to CARLA physics control")
        return handed_over

    def _update_ego_trajectory_follower(self) -> None:
        """Update the ego trajectory follower if active."""
        if (
            self.ego_trajectory_follower
            and not self.ego_trajectory_follower.is_complete()
        ):
            current_world_time = self.seconds_since_start()
            control = self.ego_trajectory_follower.update(current_world_time)

            # Apply control to ego vehicle
            ego_actor = self.get_ego_actor()
            if ego_actor:
                ego_actor.actor_inst.apply_control(control)

    def _enable_traffic_manager(self) -> None:
        if self.traffic_manager is None:
            logger.info(
                f"Enabling traffic manager on port {self.traffic_manager_port}"
            )
            self.traffic_manager = self.client.get_trafficmanager(
                self.traffic_manager_port
            )
            self.traffic_manager.set_synchronous_mode(True)

    def get_world(self) -> carla.World:
        """Get the CARLA world instance."""
        return self.client.get_world()

    def get_ego_actor(self) -> Optional[NurecActor]:
        """Get the ego NurecActor instance."""
        return self.actor_mapping.get(EGO_TRACK_ID)

    def get_scenario_time_range(self) -> Tuple[int, int]:
        """Get the scenario start and end times in microseconds."""
        if self.scenario is None:
            raise RuntimeError("Scenario not initialized. Call __enter__ first.")
        return (
            self.scenario.metadata["pose-range"]["start-timestamp_us"],
            self.scenario.metadata["pose-range"]["end-timestamp_us"],
        )

    def set_ego_simple_trajectory_following(self, time_spacing: int = 25000) -> SimpleTrajectoryFollower:
        """
        Set up simple trajectory following for the ego vehicle starting from current time.
        
        Args:
            time_spacing: Time spacing between trajectory points in microseconds (default: 25ms)

        Returns:
            SimpleTrajectoryFollower: The created trajectory follower instance
        """
        if self.scenario is None:
            raise RuntimeError("Scenario not initialized. Call __enter__ first.")

        # Get ego actor
        ego_actor = self.get_ego_actor()
        if ego_actor is None:
            raise RuntimeError("Ego actor not found. Call add_ego() first.")
        ego_actor.set_physics(True, self.get_sim_time())

        # Create trajectory follower
        trajectory_follower = SimpleTrajectoryFollower(
            nurec_actor=ego_actor, world=self.get_world()
        )

        # Set trajectory from current time to end of scenario
        current_time = self.get_sim_time()
        _, end_time = self.get_scenario_time_range()

        # Use current scenario time as start, or scenario start if we haven't started yet
        start_time = max(
            current_time, self.scenario.metadata["pose-range"]["start-timestamp_us"]
        )

        trajectory_follower.set_trajectory_from_track(
            start_time, end_time, time_spacing=time_spacing
        )

        # Start following at current world time
        world_start_time = self.seconds_since_start()
        trajectory_follower.start_following(world_start_time)

        # Store the trajectory follower for automatic updates
        self.ego_trajectory_follower = trajectory_follower

        logger.debug(f"Simple trajectory follower initialized:")
        logger.debug(
            f"  - Trajectory points: {len(trajectory_follower.trajectory_points)}"
        )
        logger.debug(f"  - Time range: {start_time} to {end_time} microseconds")
        logger.debug(f"  - Point spacing: {time_spacing} microseconds")
        logger.debug(f"  - Speed calculated as distance/time between waypoints")

        return trajectory_follower
