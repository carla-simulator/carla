# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
NUREC Scenario Management Module

This module provides classes and functions for loading and managing NUREC scenarios
from USDZ files. It handles the extraction and processing of trajectory data,
camera calibrations, and metadata from NUREC reconstructions.

Key Classes:
- Scenario: Main class representing a complete NUREC scenario with tracks and metadata
- Tracks: Collection manager for handling track activation/deactivation over time
- CameraCalibration: Dataclass for camera calibration parameters
- CameraModel: Dataclass for camera model specifications
- CameraModelParameters: Dataclass for detailed camera parameters

Key Functions:
- extract_json_from_usdz: Extract JSON data from USDZ archives
- extract_poses_from_json: Parse track data from JSON
- get_best_camera: Select optimal camera from available options
- get_spectator: Create spectator track from ego and camera data
- get_camera_calibrations: Parse camera calibration data

The module supports:
- Loading NUREC scenarios from USDZ files
- Managing object tracks with temporal activation/deactivation
- Camera calibration data with multiple camera models (F-theta, OpenCV pinhole/fisheye)
- Coordinate system transformations between world and local frames
- Ego vehicle and spectator camera management
- Track filtering based on minimum lifetime requirements

NUREC scenarios contain reconstructed 3D environments with tracked objects,
camera trajectories, and calibration data that can be replayed in CARLA
with photorealistic neural rendering.
"""

import zipfile
import json
import numpy as np
import logging
from typing import Dict, List, Any, Tuple, Optional
from dataclasses import dataclass
from track import Track, PoseType
from constants import (
    EGO_TRACK_ID,
    EGO_LABEL,
    EGO_DIMS,
    EGO_FLAG,
    SPECTATOR_TRACK_ID,
    SPECTATOR_LABEL,
    SPECTATOR_FLAG,
    DYNAMIC_FLAG,
)

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def extract_json_from_usdz(usdz_file: str, json_files: List[str]) -> Dict[str, Any]:
    """
    Extract JSON files from a USDZ archive.

    Args:
        usdz_file (str): Path to the USDZ file to extract from.
        json_files (list): List of JSON filenames to extract from the archive.

    Returns:
        dict: A dictionary where keys are filenames and values are the parsed JSON content.
              Only files specified in json_files that exist in the archive will be included.
    """
    results = {}

    with zipfile.ZipFile(usdz_file, "r") as zip_ref:
        for file_info in zip_ref.infolist():
            if file_info.filename in json_files:
                with zip_ref.open(file_info.filename) as source_file:
                    json_data = json.load(source_file)
                    results[file_info.filename] = json_data

    return results


def extract_poses_from_json(json_array: Dict[str, Any], filter_vertical_poses: bool = False) -> List[Track]:
    """
    Extracts track data from JSON, including track IDs, poses, timestamps, dimensions,
    labels, and flags.
    
    Args:
        json_array (dict): Dictionary containing parsed JSON data with track information
        filter_vertical_poses (bool): If True, filter out poses where the z-axis is vertical
    Returns:
        list: List of Track objects created from the extracted data, sorted by start time
    """
    track_data = []
    track_ids = json_array["sequence_tracks.json"]["dummy_chunk_id"]["tracks_data"][
        "tracks_id"
    ]

    for i, track_id in enumerate(track_ids):
        track_data.append(
            Track(
                track_id,
                json_array["sequence_tracks.json"]["dummy_chunk_id"]["tracks_data"][
                    "tracks_poses"
                ][i],
                json_array["sequence_tracks.json"]["dummy_chunk_id"]["tracks_data"][
                    "tracks_timestamps_us"
                ][i],
                json_array["sequence_tracks.json"]["dummy_chunk_id"][
                    "cuboidtracks_data"
                ]["cuboids_dims"][i],
                json_array["sequence_tracks.json"]["dummy_chunk_id"]["tracks_data"][
                    "tracks_label_class"
                ][i],
                json_array["sequence_tracks.json"]["dummy_chunk_id"]["tracks_data"][
                    "tracks_flags"
                ][i],
                filter_vertical_poses=filter_vertical_poses,
            ),
        )

    track_data.sort(key=lambda x: x.start_time())
    return track_data


class Tracks:
    """
    Manages a collection of track objects, handling their activation and deactivation over time.
    Provides methods to update track states based on time progression.
    """
    def __init__(self, track_data, zero_time):
        """
        Initialize the Tracks collection.
        
        Args:
            track_data (list): List of Track objects to manage
            zero_time (float): Reference time in microseconds to use as the starting point
        """
        track_data.sort(key=lambda x: x.start_time())
        self.track_data = track_data
        self.zero_time = zero_time
        self.current_time = self.zero_time
        self.active_tracks = []
        self.track_index = 0
        self.min_lifetime = 1 / 10

    def reset(self) -> None:
        """
        Reset the tracks collection to its initial state, setting current time to zero time
        and clearing active tracks.
        """
        self.current_time = self.zero_time
        self.active_tracks = []
        self.track_index = 0

    def update(self, time_step: float) -> Tuple[List[Track], List[Track]]:
        """
        Update the track collection based on the given time step.
        
        Args:
            time_step (float): Time increment in microseconds
            
        Returns:
            tuple: (new_tracks, tracks_to_remove) - Lists of Track objects that became active
                  and inactive during this update
        """
        self.current_time += time_step

        tracks_to_remove = []
        for track in self.active_tracks:
            if self.current_time > track.end_time():
                tracks_to_remove.append(track)

        for track in tracks_to_remove:
            self.active_tracks.remove(track)

        new_tracks = []

        while (
            self.track_index < len(self.track_data)
            and self.track_data[self.track_index].start_time() <= self.current_time
        ):
            next_track = self.track_data[self.track_index]
            lifetime = next_track.end_time() - next_track.start_time()
            if lifetime > self.min_lifetime:
                self.active_tracks.append(next_track)
                new_tracks.append(next_track)
            self.track_index += 1

        return new_tracks, tracks_to_remove

    def get_current_time_seconds(self) -> float:
        """
        Return the current time in seconds relative to zero time
        
        Returns:
            float: Current time in seconds
        """
        return (self.current_time - self.zero_time) / 1e6

    def set_mininmum_lifetime(self, min_lifetime: float) -> None:
        """
        Set the minimum lifetime for tracks to be considered active
        
        Args:
            min_lifetime (float): Minimum lifetime in seconds
        """
        self.min_lifetime = int(min_lifetime * 1e6)

    def get_all_possible_tracks(self) -> List[Track]:
        """
        Get all possible tracks in the scenario that are longer than the minimum lifetime.
        """
        return [track for track in self.track_data if track.end_time() - track.start_time() > self.min_lifetime]

    def set_view_transform(self, transform: np.ndarray) -> None:
        """
        Apply a view transform to all tracks in the collection
        
        Args:
            transform (numpy.ndarray): Transformation matrix to apply
        """
        for track in self.track_data:
            track.set_transform(transform)


def get_best_camera(rig_trajectories: Dict[str, Any]) -> Dict[str, Any]:
    """
    Select the best camera from the rig trajectories based on predefined search patterns.
    
    Args:
        rig_trajectories (dict): Dictionary containing camera calibration information
        
    Returns:
        dict: Data for the selected best camera
    """
    cameras = rig_trajectories["camera_calibrations"]
    camera_names = []
    for _, camera_data in cameras.items():
        camera_names.append((camera_data["logical_sensor_name"], camera_data))

    best_camera = None
    search_patterns = ["front_wide_120fov", "front_wide", "front", "wide", "120"]

    for pattern in search_patterns:
        for camera_name, camera_data in camera_names:
            if pattern in camera_name:
                best_camera = camera_data
                break
        else:
            continue
        break

    if best_camera is None:
        logger.warning("No suitable camera found, using first one")
        best_camera = camera_names[0][1]

    return best_camera


def get_spectator(rig_trajectories: Dict[str, Any], ego_poses: Track) -> Track:
    """
    Create a spectator track from the best camera and ego poses
    
    Args:
        rig_trajectories (dict): Dictionary containing camera calibration information
        ego_poses (Track): Track object containing ego vehicle poses
        
    Returns:
        Track: A spectator track object with transformed poses from the best camera
    """
    best_camera = get_best_camera(rig_trajectories)
    logger.debug(f"Using {best_camera['logical_sensor_name']} as spectator")
    transfomed_poses = np.array(ego_poses.poses) @ np.array(best_camera["T_sensor_rig"])

    return Track(
        SPECTATOR_TRACK_ID,
        transfomed_poses,
        ego_poses.timestamps,
        None,
        SPECTATOR_LABEL,
        [SPECTATOR_FLAG],
        PoseType.TRANSFORM_MATRIX,
    )

@dataclass
class CameraModelParameters:
    resolution: List[int]
    shutter_type: str
    external_distortion_parameters: Optional[Any]
    principal_point: List[float]
    reference_poly: str
    pixeldist_to_angle_poly: List[float]
    angle_to_pixeldist_poly: List[float]
    max_angle: float
    linear_cde: List[float]

@dataclass
class CameraModel:
    type: str
    parameters: CameraModelParameters

@dataclass
class CameraCalibration:
    sequence_id: str
    logical_sensor_name: str
    unique_sensor_idx: int
    T_sensor_rig: List[List[float]]
    camera_model: CameraModel

def get_camera_calibrations(json_array: Dict[str, Any]) -> Dict[str, CameraCalibration]:
    camera_calibrations = json_array["rig_trajectories.json"][
        "camera_calibrations"
    ]
    result: Dict[str, CameraCalibration] = {}
    for id, calibration in camera_calibrations.items():
        camera_model = CameraModel(
            type=calibration["camera_model"]["type"],
            parameters=CameraModelParameters(
                resolution=calibration["camera_model"]["parameters"]["resolution"],
                shutter_type=calibration["camera_model"]["parameters"]["shutter_type"],
                external_distortion_parameters=calibration["camera_model"]["parameters"]["external_distortion_parameters"],
                principal_point=calibration["camera_model"]["parameters"]["principal_point"],
                reference_poly=calibration["camera_model"]["parameters"]["reference_poly"],
                pixeldist_to_angle_poly=calibration["camera_model"]["parameters"]["pixeldist_to_angle_poly"],
                angle_to_pixeldist_poly=calibration["camera_model"]["parameters"]["angle_to_pixeldist_poly"],
                max_angle=calibration["camera_model"]["parameters"]["max_angle"],
                linear_cde=calibration["camera_model"]["parameters"]["linear_cde"],
            )
        )
        result[id] = CameraCalibration(
            sequence_id=calibration["sequence_id"],
            logical_sensor_name=calibration["logical_sensor_name"],
            unique_sensor_idx=calibration["unique_sensor_idx"],
            T_sensor_rig=calibration["T_sensor_rig"],
            camera_model=camera_model,
        )
    return result

class Scenario:
    """
    Represents a NUREC scenario loaded from a USDZ file, containing track data,
    ego vehicle poses, and spectator information.
    """
    def __init__(self, usdz_file: str) -> None:
        """
        Initialize a scenario from a USDZ file.
        
        Args:
            usdz_file (str): Path to the USDZ file containing scenario data
        """
        json_array = extract_json_from_usdz(
            usdz_file,
            ["rig_trajectories.json", "sequence_tracks.json", "data_info.json"],
        )
        track_data = extract_poses_from_json(json_array, filter_vertical_poses=True)
        self.metadata = json_array["data_info.json"]
        self.camera_calibrations : Dict[str, CameraCalibration] = get_camera_calibrations(json_array)
        self.t_world_base = np.array(
            json_array["rig_trajectories.json"]["T_world_base"]
        )
        self.ego_poses = Track(
            EGO_TRACK_ID,
            json_array["rig_trajectories.json"]["rig_trajectories"][0]["T_rig_worlds"],
            json_array["rig_trajectories.json"]["rig_trajectories"][0][
                "T_rig_world_timestamps_us"
            ],
            EGO_DIMS,
            EGO_LABEL,
            [EGO_FLAG, DYNAMIC_FLAG],
            PoseType.TRANSFORM_MATRIX,
        )

        self.spectator = get_spectator(
            json_array["rig_trajectories.json"], self.ego_poses
        )
        self.controllable_tracks = set()
        for track in track_data:
            if track.controllable:
                self.controllable_tracks.add(track.track_id)

        self.tracks = Tracks(
            track_data, self.metadata["pose-range"]["start-timestamp_us"]
        )

if __name__ == "__main__":
    scenario = Scenario("maps/clipgt-3000f11a-cfcb-460d-a3e9-2565d9f289fe.usdz")
    print(scenario.tracks)