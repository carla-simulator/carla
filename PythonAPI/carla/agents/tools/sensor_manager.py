#!/usr/bin/env python

# Copyright (c) 2024 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
SensorManager module for managing sensors attached to a vehicle.
Provides configuration presets for Bench2Drive and custom sensor setups.
"""

import weakref
import carla
import queue
import numpy as np


class SensorManager:
    """
    Manages all sensors attached to a vehicle for data collection.
    Supports multiple configuration presets including Bench2Drive format.
    """

    # Bench2Drive sensor configuration preset
    BENCH2DRIVE_CONFIG = {
        'rgb_cameras': [
            {
                'id': 'front',
                'type': 'sensor.camera.rgb',
                'x': 0.80, 'y': 0.0, 'z': 1.60,
                'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                'width': 1600, 'height': 900, 'fov': 70
            },
            {
                'id': 'front_left',
                'type': 'sensor.camera.rgb',
                'x': 0.27, 'y': -0.55, 'z': 1.60,
                'roll': 0.0, 'pitch': 0.0, 'yaw': -55.0,
                'width': 1600, 'height': 900, 'fov': 70
            },
            {
                'id': 'front_right',
                'type': 'sensor.camera.rgb',
                'x': 0.27, 'y': 0.55, 'z': 1.60,
                'roll': 0.0, 'pitch': 0.0, 'yaw': 55.0,
                'width': 1600, 'height': 900, 'fov': 70
            },
            {
                'id': 'back',
                'type': 'sensor.camera.rgb',
                'x': -2.0, 'y': 0.0, 'z': 1.60,
                'roll': 0.0, 'pitch': 0.0, 'yaw': 180.0,
                'width': 1600, 'height': 900, 'fov': 110
            },
            {
                'id': 'back_left',
                'type': 'sensor.camera.rgb',
                'x': -0.32, 'y': -0.55, 'z': 1.60,
                'roll': 0.0, 'pitch': 0.0, 'yaw': -110.0,
                'width': 1600, 'height': 900, 'fov': 70
            },
            {
                'id': 'back_right',
                'type': 'sensor.camera.rgb',
                'x': -0.32, 'y': 0.55, 'z': 1.60,
                'roll': 0.0, 'pitch': 0.0, 'yaw': 110.0,
                'width': 1600, 'height': 900, 'fov': 70
            },
            {
                'id': 'top_down',
                'type': 'sensor.camera.rgb',
                'x': 0.0, 'y': 0.0, 'z': 50.0,
                'roll': 0.0, 'pitch': -90.0, 'yaw': 0.0,
                'width': 1600, 'height': 900, 'fov': 90
            }
        ],
        'lidar': {
            'type': 'sensor.lidar.ray_cast',
            'x': -0.39, 'y': 0.0, 'z': 1.84,
            'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
            'range': 85,
            'rotation_frequency': 10,
            'channels': 64,
            'points_per_second': 600000,
            'dropoff_general_rate': 0.0,
            'dropoff_intensity_limit': 0.0,
            'dropoff_zero_intensity': 0.0
        },
        'imu': {
            'type': 'sensor.other.imu',
            'x': -1.4, 'y': 0.0, 'z': 0.0,
            'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
            'sensor_tick': 0.05
        },
        'gnss': {
            'type': 'sensor.other.gnss',
            'x': 0.0, 'y': 0.0, 'z': 0.0,
            'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0
        }
    }

    def __init__(self, vehicle, config_preset='bench2drive', custom_config=None):
        """
        Initialize the sensor manager.

        Args:
            vehicle: The CARLA vehicle actor to attach sensors to
            config_preset: Preset configuration ('bench2drive' or None for custom)
            custom_config: Custom sensor configuration dict (if config_preset is None)
        """
        self._vehicle = vehicle
        self._world = self._vehicle.get_world()
        self._sensors = {}
        self._sensor_data = {}
        self._queues = {}

        # Load configuration
        if config_preset == 'bench2drive':
            self._config = self.BENCH2DRIVE_CONFIG
        elif custom_config is not None:
            self._config = custom_config
        else:
            raise ValueError("Either config_preset='bench2drive' or custom_config must be provided")

        # Spawn all sensors
        self._spawn_sensors()

    def _spawn_sensors(self):
        """Spawn all sensors according to the configuration."""
        bp_library = self._world.get_blueprint_library()

        # Spawn RGB cameras
        if 'rgb_cameras' in self._config:
            for cam_config in self._config['rgb_cameras']:
                self._spawn_camera(bp_library, cam_config, 'rgb')

                # Spawn corresponding depth, semantic, and instance cameras
                for sensor_type in ['depth', 'semantic_segmentation', 'instance_segmentation']:
                    depth_config = cam_config.copy()
                    depth_config['type'] = f'sensor.camera.{sensor_type}'
                    self._spawn_camera(bp_library, depth_config, sensor_type.split('_')[0])

        # Spawn LiDAR
        if 'lidar' in self._config:
            self._spawn_lidar(bp_library, self._config['lidar'])

        # Spawn IMU
        if 'imu' in self._config:
            self._spawn_imu(bp_library, self._config['imu'])

        # Spawn GNSS
        if 'gnss' in self._config:
            self._spawn_gnss(bp_library, self._config['gnss'])

    def _spawn_camera(self, bp_library, config, sensor_subtype):
        """
        Spawn a camera sensor.

        Args:
            bp_library: CARLA blueprint library
            config: Camera configuration dict
            sensor_subtype: 'rgb', 'depth', 'semantic', or 'instance'
        """
        # Get the appropriate blueprint
        bp = bp_library.find(config['type'])

        # Set attributes if it's a camera
        if 'width' in config:
            bp.set_attribute('image_size_x', str(config['width']))
        if 'height' in config:
            bp.set_attribute('image_size_y', str(config['height']))
        if 'fov' in config:
            bp.set_attribute('fov', str(config['fov']))

        # Create transform
        transform = carla.Transform(
            carla.Location(x=config['x'], y=config['y'], z=config['z']),
            carla.Rotation(pitch=config['pitch'], yaw=config['yaw'], roll=config['roll'])
        )

        # Spawn sensor
        sensor = self._world.spawn_actor(bp, transform, attach_to=self._vehicle)

        # Create sensor ID
        sensor_id = f"{sensor_subtype}_{config['id']}"

        # Store sensor
        self._sensors[sensor_id] = sensor
        self._sensor_data[sensor_id] = None
        self._queues[sensor_id] = queue.Queue()

        # Setup callback
        weak_self = weakref.ref(self)
        sensor.listen(lambda data, s_id=sensor_id: SensorManager._sensor_callback(weak_self, s_id, data))

    def _spawn_lidar(self, bp_library, config):
        """Spawn LiDAR sensor."""
        bp = bp_library.find(config['type'])

        # Set LiDAR attributes
        bp.set_attribute('range', str(config['range']))
        bp.set_attribute('rotation_frequency', str(config['rotation_frequency']))
        bp.set_attribute('channels', str(config['channels']))
        bp.set_attribute('points_per_second', str(config['points_per_second']))
        bp.set_attribute('dropoff_general_rate', str(config['dropoff_general_rate']))
        bp.set_attribute('dropoff_intensity_limit', str(config['dropoff_intensity_limit']))
        bp.set_attribute('dropoff_zero_intensity', str(config['dropoff_zero_intensity']))

        # Create transform
        transform = carla.Transform(
            carla.Location(x=config['x'], y=config['y'], z=config['z']),
            carla.Rotation(pitch=config['pitch'], yaw=config['yaw'], roll=config['roll'])
        )

        # Spawn sensor
        sensor = self._world.spawn_actor(bp, transform, attach_to=self._vehicle)

        sensor_id = 'lidar_top'
        self._sensors[sensor_id] = sensor
        self._sensor_data[sensor_id] = None
        self._queues[sensor_id] = queue.Queue()

        weak_self = weakref.ref(self)
        sensor.listen(lambda data: SensorManager._sensor_callback(weak_self, sensor_id, data))

    def _spawn_imu(self, bp_library, config):
        """Spawn IMU sensor."""
        bp = bp_library.find(config['type'])

        if 'sensor_tick' in config:
            bp.set_attribute('sensor_tick', str(config['sensor_tick']))

        transform = carla.Transform(
            carla.Location(x=config['x'], y=config['y'], z=config['z']),
            carla.Rotation(pitch=config['pitch'], yaw=config['yaw'], roll=config['roll'])
        )

        sensor = self._world.spawn_actor(bp, transform, attach_to=self._vehicle)

        sensor_id = 'imu'
        self._sensors[sensor_id] = sensor
        self._sensor_data[sensor_id] = None
        self._queues[sensor_id] = queue.Queue()

        weak_self = weakref.ref(self)
        sensor.listen(lambda data: SensorManager._sensor_callback(weak_self, sensor_id, data))

    def _spawn_gnss(self, bp_library, config):
        """Spawn GNSS sensor."""
        bp = bp_library.find(config['type'])

        transform = carla.Transform(
            carla.Location(x=config['x'], y=config['y'], z=config['z']),
            carla.Rotation(pitch=config['pitch'], yaw=config['yaw'], roll=config['roll'])
        )

        sensor = self._world.spawn_actor(bp, transform, attach_to=self._vehicle)

        sensor_id = 'gnss'
        self._sensors[sensor_id] = sensor
        self._sensor_data[sensor_id] = None
        self._queues[sensor_id] = queue.Queue()

        weak_self = weakref.ref(self)
        sensor.listen(lambda data: SensorManager._sensor_callback(weak_self, sensor_id, data))

    @staticmethod
    def _sensor_callback(weak_self, sensor_id, data):
        """Callback for sensor data."""
        self = weak_self()
        if not self:
            return

        self._sensor_data[sensor_id] = data
        self._queues[sensor_id].put(data)

    def get_sensor_data(self, sensor_id):
        """
        Get the latest data from a specific sensor.

        Args:
            sensor_id: ID of the sensor

        Returns:
            Latest sensor data or None if no data available
        """
        return self._sensor_data.get(sensor_id)

    def get_all_sensor_data(self):
        """
        Get the latest data from all sensors.

        Returns:
            Dictionary mapping sensor IDs to their latest data
        """
        return self._sensor_data.copy()

    def wait_for_data(self, timeout=1.0):
        """
        Wait for all sensors to have at least one data point.

        Args:
            timeout: Maximum time to wait for each sensor in seconds

        Returns:
            True if all sensors have data, False otherwise
        """
        for sensor_id, q in self._queues.items():
            try:
                data = q.get(timeout=timeout)
                self._sensor_data[sensor_id] = data
            except queue.Empty:
                return False
        return True

    def get_sensor_list(self):
        """Get list of all sensor IDs."""
        return list(self._sensors.keys())

    def get_sensor(self, sensor_id):
        """
        Get a sensor actor by ID.

        Args:
            sensor_id: ID of the sensor

        Returns:
            Sensor actor or None if not found
        """
        return self._sensors.get(sensor_id)

    def destroy(self):
        """Destroy all sensors and clean up."""
        for sensor_id, sensor in self._sensors.items():
            if sensor is not None and sensor.is_alive:
                sensor.stop()
                sensor.destroy()

        self._sensors.clear()
        self._sensor_data.clear()
        self._queues.clear()

    def __del__(self):
        """Destructor to ensure cleanup."""
        self.destroy()
