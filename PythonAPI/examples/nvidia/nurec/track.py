# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Track and Pose Interpolation Module

This module provides classes for managing and interpolating object trajectories (tracks)
in NUREC scenarios. It handles different pose representation formats and provides
smooth interpolation between poses at different timestamps.

Key Classes:
- PoseType: Enum defining supported pose formats (transformation matrices, xyz+quaternion)
- InterpolatedPoses: Base class for pose interpolation with support for multiple formats
- Track: Represents a single object's trajectory with metadata (dimensions, labels, flags)

The module supports:
- Conversion between pose formats (4x4 matrices, xyz+quaternion, euler angles)
- Smooth interpolation using linear interpolation for translation and SLERP for rotation
- Coordinate system transformations
- Path generation for CARLA waypoint following
- Track metadata management (ego vehicle, dynamic objects, controllable actors)

Pose interpolation uses spherical linear interpolation (SLERP) for rotations to ensure
smooth orientation changes, while translations use standard linear interpolation.
"""

from enum import Enum
from constants import EGO_FLAG, SPECTATOR_FLAG, DYNAMIC_FLAG, CONTROLLABLE_FLAG
import numpy as np
import logging
from scipy.spatial.transform import Rotation
from scipy.signal import butter, filtfilt
from scipy.interpolate import interp1d
from utils import mat_to_carla_transform
from typing import List, Union, Optional, Tuple
import carla

logger = logging.getLogger(__name__)


def lowpass_filter_vertical_component(
    poses: List[np.ndarray],
    threshold: float = 0.01,
    max_slope: float = 0.25,
    window_size: int = 5,
    cutoff_freq: float = 0.05,
) -> List[np.ndarray]:
    """
    Filter vertical component based on grade (slope) filtering for realistic road characteristics.

    Args:
        poses: List of 4x4 transformation matrices
        threshold: Minimum horizontal movement to calculate grade (m)
        max_slope: Maximum allowed road grade (default 0.25 = 25%)
        window_size: Window size for moving average filter
        cutoff_freq: Cutoff frequency for lowpass filter (normalized, 0-1)

    Returns:
        List of filtered 4x4 transformation matrices
    """
    if len(poses) < 3:
        logger.warning("Not enough poses for filtering, returning original poses")
        return poses

    poses_array = np.array(poses)
    n_poses = len(poses_array)

    # Step 1: Extract positions (assume z-axis is up)
    logger.debug(
        f"Filtering vertical component using grade-based approach for {n_poses} poses"
    )
    positions = poses_array[:, :3, 3]  # Extract x, y, z positions

    # Step 2: Calculate horizontal distances and vertical displacements
    horizontal_distances_list = []
    grades_list = []

    for i in range(n_poses - 1):
        dx = positions[i + 1, 0] - positions[i, 0]
        dy = positions[i + 1, 1] - positions[i, 1]
        dz = positions[i + 1, 2] - positions[i, 2]

        # Calculate horizontal distance moved
        horizontal_distance = np.sqrt(dx**2 + dy**2)
        horizontal_distances_list.append(horizontal_distance)

        # Calculate grade (slope)
        if horizontal_distance > threshold:
            grade = dz / horizontal_distance
        else:
            grade = 0.0  # No grade when not moving horizontally

        grades_list.append(grade)

    horizontal_distances = np.array(horizontal_distances_list)
    grades = np.array(grades_list)

    logger.debug(
        f"Original grade statistics: mean={np.mean(grades):.4f}, std={np.std(grades):.4f}, max={np.max(np.abs(grades)):.4f}"
    )

    # Step 3: Apply grade constraints (cap unrealistic grades)
    constrained_grades = np.copy(grades)

    for i in range(len(grades)):
        if abs(constrained_grades[i]) > max_slope:
            # Cap the grade to maximum allowed, preserving sign
            constrained_grades[i] = np.sign(constrained_grades[i]) * max_slope
            logger.debug(
                f"Capped grade at index {i}: {grades[i]:.4f} -> {constrained_grades[i]:.4f}"
            )

    # Step 4: Apply spatial frequency filtering to grades
    filtered_grades = constrained_grades.copy()

    if len(constrained_grades) > 10:
        # Calculate cumulative distance for each pose
        cumulative_distances = np.cumsum(np.concatenate([[0], horizontal_distances]))

        # Apply moving average filter first for basic smoothing
        if window_size > 1:
            kernel = np.ones(window_size) / window_size
            filtered_grades = np.convolve(constrained_grades, kernel, mode="same")
            logger.debug(
                f"Applied moving average filter to grades with window size {window_size}"
            )

        # Apply spatial frequency filtering using interpolation and uniform FFT
        if (
            len(filtered_grades) > 20
        ):  # Need enough points for meaningful frequency analysis
            try:
                # Check for repeated values in cumulative_distances that would break interp1d
                distances_for_interp = cumulative_distances[1:]  # Skip first zero
                unique_distances, unique_indices = np.unique(
                    distances_for_interp, return_index=True
                )

                if len(unique_distances) < len(distances_for_interp):
                    # We have repeated distances (vehicle stationary between poses)
                    num_duplicates = len(distances_for_interp) - len(unique_distances)
                    logger.debug(
                        f"Found {num_duplicates} repeated cumulative distances, adding epsilon values"
                    )

                    # Add small incremental epsilon to make distances unique
                    epsilon = 1e-6  # 1 micrometer
                    fixed_distances = distances_for_interp.copy()

                    for i in range(1, len(fixed_distances)):
                        if fixed_distances[i] <= fixed_distances[i - 1]:
                            fixed_distances[i] = fixed_distances[i - 1] + epsilon
                            epsilon += 1e-6  # Increment epsilon for each fix

                    distances_for_interp = fixed_distances

                # Create uniform distance sampling for FFT
                total_distance = distances_for_interp[-1] - distances_for_interp[0]
                n_uniform = len(filtered_grades)
                uniform_distances = np.linspace(
                    distances_for_interp[0], distances_for_interp[-1], n_uniform
                )

                # Interpolate grades to uniform spacing
                interp_func = interp1d(
                    distances_for_interp,
                    filtered_grades,
                    kind="linear",
                    bounds_error=False,
                    fill_value="extrapolate",
                )
                uniform_grades = interp_func(uniform_distances)

                # Apply FFT-based lowpass filter
                fft_data = np.fft.fft(uniform_grades)
                freqs = np.fft.fftfreq(
                    len(uniform_grades), d=total_distance / n_uniform
                )

                # Define spatial frequency cutoff for grade changes
                # Preserve grade changes over distances longer than 40m
                spatial_cutoff_freq = (
                    1.0 / 40.0
                )  # 0.025 cycles per meter (40m wavelength)

                # Create lowpass filter mask
                filter_mask = np.abs(freqs) <= spatial_cutoff_freq
                fft_filtered = fft_data * filter_mask

                # Transform back to spatial domain
                uniform_filtered = np.real(np.fft.ifft(fft_filtered))

                # Interpolate back to original non-uniform spacing
                interp_back = interp1d(
                    uniform_distances,
                    uniform_filtered,
                    kind="linear",
                    bounds_error=False,
                    fill_value="extrapolate",
                )
                filtered_grades = interp_back(distances_for_interp)

                logger.debug(
                    f"Applied spatial frequency filter to grades with cutoff {spatial_cutoff_freq:.4f} cycles/m (wavelength ≥ {1/spatial_cutoff_freq:.1f}m)"
                )

            except Exception as e:
                logger.warning(
                    f"Failed to apply spatial frequency filter to grades: {e}, using moving average only"
                )

        # Additional Butterworth filter as backup/supplement if specified
        elif cutoff_freq > 0 and cutoff_freq < 0.5 and len(filtered_grades) > 10:
            try:
                b, a = butter(2, cutoff_freq, btype="low")
                filtered_grades = filtfilt(b, a, filtered_grades)
                logger.debug(
                    f"Applied additional Butterworth lowpass filter to grades with cutoff {cutoff_freq}"
                )
            except Exception as e:
                logger.warning(f"Failed to apply Butterworth filter to grades: {e}")

    logger.debug(
        f"Filtered grade statistics: mean={np.mean(filtered_grades):.4f}, std={np.std(filtered_grades):.4f}, max={np.max(np.abs(filtered_grades)):.4f}"
    )

    # Step 5: Reconstruct vertical positions from filtered grades
    logger.debug(
        "Reconstructing poses from filtered grades using local median z-position"
    )

    filtered_poses = []

    # Find middle pose index
    middle_idx = len(poses_array) // 2

    # Calculate median z-position from middle 5 values of the sequence
    original_z_positions = poses_array[:, 2, 3]
    start_idx = max(0, middle_idx - 2)
    end_idx = min(len(original_z_positions), middle_idx + 3)
    middle_z_values = original_z_positions[start_idx:end_idx]
    local_median_z = np.median(middle_z_values)

    logger.debug(
        f"Using local median z-position: {local_median_z:.4f}m from middle 5 values at pose index: {middle_idx}"
    )

    # Create array to store all filtered z-positions
    filtered_z_positions = np.zeros(len(poses_array))
    filtered_z_positions[middle_idx] = local_median_z

    # Integrate grades forward from middle to end
    for i in range(middle_idx, len(filtered_grades)):
        horizontal_dist = horizontal_distances[i]
        if horizontal_dist > threshold:
            vertical_displacement = filtered_grades[i] * horizontal_dist
        else:
            vertical_displacement = 0.0

        filtered_z_positions[i + 1] = filtered_z_positions[i] + vertical_displacement

    # Integrate grades backward from middle to start
    for i in range(middle_idx - 1, -1, -1):
        horizontal_dist = horizontal_distances[i]
        if horizontal_dist > threshold:
            vertical_displacement = filtered_grades[i] * horizontal_dist
        else:
            vertical_displacement = 0.0

        # Going backward, so subtract the displacement
        filtered_z_positions[i] = filtered_z_positions[i + 1] - vertical_displacement

    # Create filtered poses with new z-positions
    for i in range(len(poses_array)):
        new_pose = poses_array[i].copy()
        new_pose[2, 3] = filtered_z_positions[i]
        filtered_poses.append(new_pose)

    # Step 6: Prepare data for plotting and calculate statistics
    original_positions = poses_array[:, :3, 3]
    filtered_positions = np.array([pose[:3, 3] for pose in filtered_poses])

    original_z_std = np.std(original_positions[:, 2])
    filtered_z_std = np.std(filtered_positions[:, 2])
    max_grade_correction = np.max(np.abs(filtered_grades - grades))

    # Log statistics
    logger.debug(f"Original Z-position std: {original_z_std:.4f}m")
    logger.debug(f"Filtered Z-position std: {filtered_z_std:.4f}m")
    logger.debug(f"Maximum grade correction: {max_grade_correction:.4f}")
    logger.debug(f"Poses with capped grades: {np.sum(np.abs(grades) > max_slope)}")

    return filtered_poses


class PoseType(Enum):
    """
    Enum defining the supported pose representation formats.

    TRANSFORM_MATRIX: 4x4 transformation matrix
    XYZ_QUAT: 7-element array [x, y, z, qx, qy, qz, qw] with position and quaternion
    """

    TRANSFORM_MATRIX = "transform_matrix"
    XYZ_QUAT = "xyz_quat"


class InterpolatedPoses:
    """
    Class that handles interpolation between poses at different timestamps.
    Supports conversion between different pose representation formats and
    linear/spherical interpolation of positions and rotations.
    """

    def __init__(
        self,
        poses: List[Union[np.ndarray, List[float]]],
        timestamps: List[float],
        pose_type: PoseType = PoseType.TRANSFORM_MATRIX,
        filter_vertical_poses: bool = False,
    ):
        """
        Initialize the interpolated poses.

        Args:
            poses: List of poses in either transform_matrix or xyz_quat format
            timestamps: List of timestamps corresponding to each pose
            pose_type: Type of poses provided (TRANSFORM_MATRIX or XYZ_QUAT)
            filter_vertical_poses: If True, filter out poses where the z-axis is vertical
        """
        # Convert all poses to 4x4 matrices regardless of input format
        self.poses: List[np.ndarray] = []
        self._convert_poses_to_mats(poses, pose_type, filter_vertical_poses)

        self.timestamps = timestamps
        self.ignore_out_of_bounds = False
        self.transform = np.eye(4)

    def _convert_poses_to_mats(
        self,
        poses: List[Union[np.ndarray, List[float]]],
        pose_type: PoseType,
        filter_vertical_poses: bool = False,
    ) -> None:
        """
        Convert input poses to 4x4 transformation matrices.

        Args:
            poses: List of poses in the specified format
            pose_type: Format of the input poses (TRANSFORM_MATRIX or XYZ_QUAT)
            filter_vertical_poses: If True, filter out poses where the z-axis is vertical
        Raises:
            ValueError: If poses have invalid format or unsupported pose type
        """
        for pose in poses:
            if pose_type == PoseType.TRANSFORM_MATRIX:
                # If already a matrix, ensure it's a proper 4x4
                matrix = np.array(pose)
                if matrix.shape != (4, 4):
                    raise ValueError(
                        f"Expected 4x4 matrix but got shape {matrix.shape}"
                    )
                self.poses.append(matrix)
            elif pose_type == PoseType.XYZ_QUAT:
                # Convert from [x, y, z, qx, qy, qz, qw] to 4x4 matrix
                pose_array = np.array(pose)

                # Extract translation and quaternion
                translation = pose_array[:3]
                quaternion = pose_array[3:7]  # [qx, qy, qz, qw]

                # Create rotation matrix from quaternion
                rot_matrix = Rotation.from_quat(quaternion).as_matrix()

                # Build 4x4 transformation matrix
                transform = np.eye(4)
                transform[:3, :3] = rot_matrix
                transform[:3, 3] = translation

                self.poses.append(transform)
            else:
                raise ValueError(f"Unsupported pose type: {pose_type}")

        # Apply vertical filtering after all poses are converted
        if filter_vertical_poses:
            self.poses = lowpass_filter_vertical_component(self.poses)

    def set_ignore_out_of_bounds(self, ignore_out_of_bounds: bool) -> None:
        """
        Set whether to ignore timestamps outside the available range.

        Args:
            ignore_out_of_bounds (bool): If True, use the last pose for timestamps beyond the end
        """
        self.ignore_out_of_bounds = ignore_out_of_bounds

    def set_transform(self, transform: np.ndarray) -> None:
        """
        Set a transformation matrix to apply to all interpolated poses.

        Args:
            transform (numpy.ndarray): 4x4 transformation matrix
        """
        self.transform = transform

    def _get_interpolation_params(
        self, timestamp: float
    ) -> Tuple[Optional[np.ndarray], Optional[np.ndarray], float]:
        """
        Common function to determine interpolation parameters.

        Args:
            timestamp (float): The timestamp to interpolate for

        Returns:
            tuple: (prev_pose, next_pose, t_factor) where t_factor is interpolation factor
                  Returns (None, None, None) if timestamp is outside the track's time range.
        """
        # Check if the timestamp is within the track's time range
        if self.ignore_out_of_bounds and timestamp > self.timestamps[-1]:
            return self.poses[-2], self.poses[-1], 1.0
        elif timestamp < self.timestamps[0] or timestamp > self.timestamps[-1]:
            logger.error(
                f"Timestamp {timestamp} is outside the track's time range {self.timestamps[0]} - {self.timestamps[-1]}"
            )
            return None, None, 0.0

        # Find the index of the previous timestamp
        prev_timestamp_idx = 0
        for i in range(len(self.timestamps)):
            if self.timestamps[i] > timestamp:
                break
            prev_timestamp_idx = i

        next_timestamp_idx = prev_timestamp_idx + 1

        # If at last timestamp, return the last pose
        if next_timestamp_idx >= len(self.timestamps):
            logger.warning(
                f"Timestamp {timestamp} is outside the track's time range {self.timestamps[0]} - {self.timestamps[-1]}"
            )
            return self.poses[prev_timestamp_idx], None, 0.0

        # Calculate interpolation factor (t) between 0 and 1
        t = (timestamp - self.timestamps[prev_timestamp_idx]) / (
            self.timestamps[next_timestamp_idx] - self.timestamps[prev_timestamp_idx]
        )

        # Get the poses to interpolate between
        start_pose = self.poses[prev_timestamp_idx]
        end_pose = self.poses[next_timestamp_idx]

        return start_pose, end_pose, t

    def interpolate_pose_matrix(self, timestamp: float) -> Optional[np.ndarray]:
        """
        Interpolate a pose and return as a 4x4 transformation matrix.

        Args:
            timestamp: The timestamp to interpolate for

        Returns:
            4x4 numpy array transformation matrix, or None if timestamp is out of range
        """
        # Get interpolation parameters
        start_pose, end_pose, t = self._get_interpolation_params(timestamp)

        if start_pose is None:
            logger.warning(
                f"Start pose is None for timestamp {timestamp}, likly out of bounds"
            )
            return None

        if end_pose is None:
            return start_pose

        # Extract rotation matrices and translations
        start_rotation_matrix = start_pose[:3, :3]
        start_translation = start_pose[:3, 3]

        end_rotation_matrix = end_pose[:3, :3]
        end_translation = end_pose[:3, 3]

        # Linear interpolation of translation
        interp_translation = start_translation + t * (
            end_translation - start_translation
        )

        # Convert matrices to Rotation objects
        start_rotation = Rotation.from_matrix(start_rotation_matrix)
        end_rotation = Rotation.from_matrix(end_rotation_matrix)

        # Create a Slerp object
        # key_rotations = Rotation.concatenate([start_rotation, end_rotation])
        # key_times = np.array([0, 1])
        # slerp = Slerp(key_times, key_rotations)
        rotvec = (start_rotation.inv() * end_rotation).as_rotvec()
        slerp_result = start_rotation * Rotation.from_rotvec(rotvec * t)

        # Interpolate rotation
        interp_rotation_matrix = slerp_result.as_matrix()

        # Create the interpolated transformation matrix
        result = np.eye(4)
        result[:3, :3] = interp_rotation_matrix
        result[:3, 3] = interp_translation

        result = self.transform @ result

        return result

    def interpolate_pose_xyzquat(self, timestamp: float) -> Optional[List[float]]:
        """
        Interpolate a pose and return as a translation and quaternion (xyzquat).

        Args:
            timestamp: The timestamp to interpolate for

        Returns:
            List [x, y, z, qx, qy, qz, qw], or None if timestamp is out of range
        """
        # Get the interpolated 4x4 matrix
        matrix = self.interpolate_pose_matrix(timestamp)

        if matrix is None:
            return None

        # Extract translation
        translation = matrix[:3, 3]

        # Extract rotation as quaternion
        rotation = Rotation.from_matrix(matrix[:3, :3])
        quaternion = rotation.as_quat()  # This returns [x, y, z, w]

        # Return as [x, y, z, qx, qy, qz, qw]
        return np.concatenate([translation, quaternion]).tolist()

    def interpolate_pose_euler(self, timestamp: float) -> Optional[List[float]]:
        """
        Interpolate a pose and return as a translation and Euler angles.

        Args:
            timestamp: The timestamp to interpolate for

        Returns:
            List [x, y, z, roll, pitch, yaw] where:
              - x, y, z: Position coordinates
              - roll: Rotation around X-axis (in radians)
              - pitch: Rotation around Y-axis (in radians)
              - yaw: Rotation around Z-axis (in radians)

            Returns None if timestamp is out of range.

        Note:
            Euler angles use the 'zyx' convention, which means the rotations are applied
            in the order: first yaw (z), then pitch (y), then roll (x).
        """
        # Get the interpolated 4x4 matrix
        matrix = self.interpolate_pose_matrix(timestamp)

        if matrix is None:
            return None

        # Extract translation
        translation = matrix[:3, 3]

        # Extract rotation as Euler angles (in radians)
        rotation = Rotation.from_matrix(matrix[:3, :3])
        euler_angles = rotation.as_euler("zyx")

        # Return as [x, y, z, roll, pitch, yaw]
        return np.concatenate([translation, euler_angles]).tolist()

    def interpolate_pose(self, timestamp: float) -> Optional[List[float]]:
        """
        Default interpolation method that returns xyzquat format for backward compatibility

        Args:
            timestamp (float): The timestamp to interpolate for

        Returns:
            List [x, y, z, qx, qy, qz, qw], or None if timestamp is out of range
        """
        return self.interpolate_pose_xyzquat(timestamp)

    def get_path(
        self, spacing: float = 1e6, start_time: Optional[float] = None
    ) -> List[carla.Location]:
        """
        Get the path of the track as a list of poses as a carla.Transform.
        Args:
            spacing (int): The spacing between poses in the path in microseconds.
            start_time (int): The start time of the path in microseconds.

        Returns:
            List of carla.Transform objects.
        """
        if start_time is None:
            start_time = self.timestamps[0]
        path = []
        for i in range(int(start_time), int(self.timestamps[-1]), int(spacing)):
            mat = self.interpolate_pose_matrix(i)
            if mat is not None:
                path.append(mat_to_carla_transform(mat).location)
        return path


class Track(InterpolatedPoses):
    """
    Represents a track (trajectory) of an object with additional metadata
    such as dimensions, label, and flags indicating properties like dynamic/static,
    controllable, ego vehicle, or spectator.
    """

    def __init__(
        self,
        track_id: str,
        poses: List[Union[np.ndarray, List[float]]],
        timestamps: List[float],
        dims: Optional[List[float]],
        label: str,
        flags: List[str],
        pose_type: PoseType = PoseType.XYZ_QUAT,
        filter_vertical_poses: bool = False,
    ):
        """
        Initialize a track with poses and metadata.

        Args:
            track_id: Unique identifier for the track, probably a string
            poses: List of poses in the specified format
            timestamps: List of timestamps corresponding to each pose
            dims: Dimensions of the object [length, width, height]
            label: Class label of the object (e.g., "automobile", "person")
            flags: List of flags indicating special properties
            pose_type: Format of the input poses (default: XYZ_QUAT)
            filter_vertical_poses: If True, filter out poses where the z-axis is vertical
        """
        super().__init__(poses, timestamps, pose_type, filter_vertical_poses)
        self.track_id = track_id
        self.dims = dims
        self.label = label
        self.dynamic = DYNAMIC_FLAG in flags
        self.controllable = CONTROLLABLE_FLAG in flags
        self.ego = EGO_FLAG in flags
        self.spectator = SPECTATOR_FLAG in flags

    def start_time(self) -> float:
        """
        Get the start time of the track in microseconds.

        Returns:
            float: First timestamp of the track
        """
        return self.timestamps[0]

    def end_time(self) -> float:
        """
        Get the end time of the track in microseconds.

        Returns:
            float: Last timestamp of the track
        """
        return self.timestamps[-1]

    def get_class(self) -> str:
        """
        Get the class label of the track.

        Returns:
            str: Class label
        """
        return self.label

    def is_dynamic(self) -> bool:
        """
        Check if the track is dynamic (moving).

        Returns:
            bool: True if the track is dynamic
        """
        return self.dynamic

    def is_controllable(self) -> bool:
        """
        Check if the track is controllable.

        Returns:
            bool: True if the track is controllable
        """
        return self.controllable
