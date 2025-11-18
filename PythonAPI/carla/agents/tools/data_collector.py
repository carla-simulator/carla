#!/usr/bin/env python

# Copyright (c) 2024 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
DataCollector module for collecting and saving sensor data in Bench2Drive format.
"""

import os
import json
import gzip
import math
import numpy as np
from PIL import Image
import carla


class DataCollector:
    """
    Collects sensor data and vehicle telemetry, saving in Bench2Drive format.
    """

    def __init__(self, sensor_manager, vehicle, root_dir, scenario_type='GeneralDriving',
                 town='Town01', route_id=1, weather_id=0, format='bench2drive'):
        """
        Initialize the data collector.

        Args:
            sensor_manager: SensorManager instance managing all sensors
            vehicle: CARLA vehicle actor
            root_dir: Root directory for dataset
            scenario_type: Type of scenario (e.g., 'GeneralDriving')
            town: Town name (e.g., 'Town01')
            route_id: Route number
            weather_id: Weather preset ID
            format: Data format ('bench2drive' supported)
        """
        self._sensor_manager = sensor_manager
        self._vehicle = vehicle
        self._world = self._vehicle.get_world()
        self._root_dir = root_dir
        self._scenario_type = scenario_type
        self._town = town
        self._route_id = route_id
        self._weather_id = weather_id
        self._format = format

        # Create scenario directory name
        self._scenario_name = f"{scenario_type}_{town}_Route{route_id:03d}_Weather{weather_id:02d}"
        self._scenario_dir = os.path.join(root_dir, self._scenario_name)

        # Frame counter
        self._frame_count = 0
        self._recording = False

        # IMU and GNSS data storage
        self._imu_data = None
        self._gnss_data = None

        # Waypoint and navigation info
        self._waypoint_info = None

        # Create directory structure
        self._create_directories()

    def _create_directories(self):
        """Create the Bench2Drive directory structure."""
        # Create main scenario directory
        os.makedirs(self._scenario_dir, exist_ok=True)

        # Create anno directory
        os.makedirs(os.path.join(self._scenario_dir, 'anno'), exist_ok=True)

        # Create camera directories
        camera_dir = os.path.join(self._scenario_dir, 'camera')
        os.makedirs(camera_dir, exist_ok=True)

        # Camera types and positions
        camera_types = ['rgb', 'depth', 'instance', 'semantic']
        positions = ['front', 'front_left', 'front_right', 'back', 'back_left', 'back_right']

        for cam_type in camera_types:
            for pos in positions:
                os.makedirs(os.path.join(camera_dir, f'{cam_type}_{pos}'), exist_ok=True)

        # Add top-down RGB camera directory
        os.makedirs(os.path.join(camera_dir, 'rgb_top_down'), exist_ok=True)

        # Create lidar directory
        os.makedirs(os.path.join(self._scenario_dir, 'lidar'), exist_ok=True)

    def start_recording(self):
        """Start recording data."""
        self._recording = True
        self._frame_count = 0
        print(f"[DataCollector] Started recording to {self._scenario_dir}")

    def stop_recording(self):
        """Stop recording data."""
        self._recording = False
        print(f"[DataCollector] Stopped recording. Total frames: {self._frame_count}")

    def record_frame(self, control=None, waypoint_info=None):
        """
        Record a single frame of data.

        Args:
            control: carla.VehicleControl object with control inputs
            waypoint_info: Dictionary with waypoint and navigation information
        """
        if not self._recording:
            return

        # Get all sensor data
        sensor_data = self._sensor_manager.get_all_sensor_data()

        # Check if we have data from all sensors
        if not all(data is not None for data in sensor_data.values()):
            print(f"[DataCollector] Warning: Not all sensors have data at frame {self._frame_count}")
            return

        # Save images (RGB, depth, semantic, instance)
        self._save_camera_images(sensor_data)

        # Save LiDAR data
        self._save_lidar_data(sensor_data)

        # Collect and save telemetry/annotation data
        self._save_annotation(sensor_data, control, waypoint_info)

        self._frame_count += 1

    def _save_camera_images(self, sensor_data):
        """Save all camera images (RGB, depth, semantic, instance)."""
        frame_str = f"{self._frame_count:05d}"
        camera_dir = os.path.join(self._scenario_dir, 'camera')

        # Save RGB cameras
        for position in ['front', 'front_left', 'front_right', 'back', 'back_left', 'back_right', 'top_down']:
            sensor_id = f'rgb_{position}'
            if sensor_id in sensor_data and sensor_data[sensor_id] is not None:
                img_data = sensor_data[sensor_id]
                self._save_rgb_image(img_data, camera_dir, f'rgb_{position}', frame_str)

            # Save corresponding depth, semantic, and instance
            if position != 'top_down':  # Top-down is RGB only
                for sensor_type in ['depth', 'semantic', 'instance']:
                    sensor_id = f'{sensor_type}_{position}'
                    if sensor_id in sensor_data and sensor_data[sensor_id] is not None:
                        img_data = sensor_data[sensor_id]
                        if sensor_type == 'depth':
                            self._save_depth_image(img_data, camera_dir, sensor_id, frame_str)
                        elif sensor_type == 'semantic':
                            self._save_semantic_image(img_data, camera_dir, sensor_id, frame_str)
                        elif sensor_type == 'instance':
                            self._save_instance_image(img_data, camera_dir, sensor_id, frame_str)

    def _save_rgb_image(self, image_data, camera_dir, sensor_id, frame_str):
        """Save RGB image as JPG."""
        # Convert CARLA image to numpy array
        array = np.frombuffer(image_data.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image_data.height, image_data.width, 4))
        array = array[:, :, :3]  # Remove alpha channel
        array = array[:, :, ::-1]  # BGR to RGB

        # Save as JPG
        img = Image.fromarray(array)
        save_path = os.path.join(camera_dir, sensor_id, f"{frame_str}.jpg")
        img.save(save_path, 'JPEG', quality=95)

    def _save_depth_image(self, image_data, camera_dir, sensor_id, frame_str):
        """Save depth image as PNG."""
        # Convert CARLA depth image to numpy array
        array = np.frombuffer(image_data.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image_data.height, image_data.width, 4))
        array = array[:, :, :3]  # Remove alpha channel

        # Save as PNG (preserving depth encoding)
        img = Image.fromarray(array)
        save_path = os.path.join(camera_dir, sensor_id, f"{frame_str}.png")
        img.save(save_path, 'PNG')

    def _save_semantic_image(self, image_data, camera_dir, sensor_id, frame_str):
        """Save semantic segmentation image as PNG."""
        # Convert to CityScapes palette
        image_data.convert(carla.ColorConverter.CityScapesPalette)

        array = np.frombuffer(image_data.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image_data.height, image_data.width, 4))
        array = array[:, :, :3]  # Remove alpha channel

        # Save as PNG
        img = Image.fromarray(array)
        save_path = os.path.join(camera_dir, sensor_id, f"{frame_str}.png")
        img.save(save_path, 'PNG')

    def _save_instance_image(self, image_data, camera_dir, sensor_id, frame_str):
        """Save instance segmentation image as PNG."""
        # Instance segmentation saves raw data
        array = np.frombuffer(image_data.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image_data.height, image_data.width, 4))
        array = array[:, :, :3]  # Remove alpha channel

        # Save as PNG
        img = Image.fromarray(array)
        save_path = os.path.join(camera_dir, sensor_id, f"{frame_str}.png")
        img.save(save_path, 'PNG')

    def _save_lidar_data(self, sensor_data):
        """Save LiDAR data as PLY (can be converted to LAZ later)."""
        if 'lidar_top' not in sensor_data or sensor_data['lidar_top'] is None:
            return

        frame_str = f"{self._frame_count:05d}"
        lidar_data = sensor_data['lidar_top']

        # Convert to numpy array
        points = np.frombuffer(lidar_data.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 4), 4))

        # Save as PLY for now (can be converted to LAZ offline)
        # Format: x, y, z, intensity
        save_path = os.path.join(self._scenario_dir, 'lidar', f"{frame_str}.ply")
        self._save_ply(points, save_path)

    def _save_ply(self, points, filename):
        """Save point cloud as PLY file."""
        with open(filename, 'w') as f:
            # Write PLY header
            f.write("ply\n")
            f.write("format ascii 1.0\n")
            f.write(f"element vertex {len(points)}\n")
            f.write("property float x\n")
            f.write("property float y\n")
            f.write("property float z\n")
            f.write("property float intensity\n")
            f.write("end_header\n")

            # Write point data
            for point in points:
                f.write(f"{point[0]} {point[1]} {point[2]} {point[3]}\n")

    def _save_annotation(self, sensor_data, control, waypoint_info):
        """Save annotation data as compressed JSON."""
        frame_str = f"{self._frame_count:05d}"

        # Get vehicle transform and velocity
        transform = self._vehicle.get_transform()
        velocity = self._vehicle.get_velocity()
        speed = math.sqrt(velocity.x**2 + velocity.y**2 + velocity.z**2)

        # Get IMU data
        imu_data = sensor_data.get('imu')
        acceleration = [0.0, 0.0, 0.0]
        angular_velocity = [0.0, 0.0, 0.0]

        if imu_data is not None:
            acceleration = [
                imu_data.accelerometer.x,
                imu_data.accelerometer.y,
                imu_data.accelerometer.z
            ]
            angular_velocity = [
                math.degrees(imu_data.gyroscope.x),
                math.degrees(imu_data.gyroscope.y),
                math.degrees(imu_data.gyroscope.z)
            ]

        # Build annotation dictionary
        annotation = {
            # Vehicle position
            'x': transform.location.x,
            'y': transform.location.y,
            'theta': math.radians(transform.rotation.yaw),

            # Vehicle dynamics
            'speed': speed,
            'acceleration': acceleration,
            'angular_velocity': angular_velocity,

            # Control inputs
            'throttle': control.throttle if control else 0.0,
            'steer': control.steer if control else 0.0,
            'brake': control.brake if control else 0.0,
            'reverse': control.reverse if control else False,

            # Weather
            'weather': self._weather_id,

            # Navigation (placeholders for now, will be filled by waypoint_info)
            'x_command_far': 0.0,
            'y_command_far': 0.0,
            'command_far': 0,
            'x_command_near': 0.0,
            'y_command_near': 0.0,
            'command_near': 0,
            'x_target': 0.0,
            'y_target': 0.0,
            'next_command': 0,
            'should_brake': False,
            'only_ap_brake': False,

            # Placeholders for complex data (can be added later)
            'bounding_boxes': {},
            'sensors': {}
        }

        # Update with waypoint info if provided
        if waypoint_info:
            annotation.update(waypoint_info)

        # Save as compressed JSON
        save_path = os.path.join(self._scenario_dir, 'anno', f"{frame_str}.json.gz")
        with gzip.open(save_path, 'wt', encoding='utf-8') as gz_file:
            json.dump(annotation, gz_file, indent=2)

    def get_frame_count(self):
        """Get current frame count."""
        return self._frame_count

    def get_scenario_dir(self):
        """Get scenario directory path."""
        return self._scenario_dir

    def is_recording(self):
        """Check if currently recording."""
        return self._recording
