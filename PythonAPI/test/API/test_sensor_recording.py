# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from __future__ import print_function
import unittest
import glob
import os
import sys

import carla
from carla import ColorConverter as cc
import weakref
import math
from math import isclose
import time
import csv
from enum import Enum

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

class SensorType(Enum):
    Gnss = 1
    IMU = 2
    Radar = 3
    RGB = 4
    Depth_Raw = 5
    Depth_GrayScale = 6
    Depth_LogScale = 7
    SemanticSeg_Raw = 8
    SemanticSeg_CityScapesPalette = 9
    InstanceSeg_Raw = 10
    InstanceSeg_CityScapesPalette = 11
    Lidar = 12
    SemanticLidar = 13
    RGB_Distorted = 14

# ==============================================================================
# -- Scenario ------------------------------------------------------------------
# ==============================================================================
class Scenario():
    def __init__(self, test):
        self.test = test
        client = carla.Client()
        self.world = client.load_world('Town10HD_Opt')
        time.sleep(5)
        
        # sync mode
        self.settings = self.world.get_settings()
        self.settings.synchronous_mode = True
        self.settings.fixed_delta_seconds = 0.05
        self.world.apply_settings(self.settings)
        bp_lib = self.world.get_blueprint_library()
        self.start_time = str(round(time.time()))
        
        # Spawn player actor
        spectator = self.world.get_spectator()
        spectator.set_transform(carla.Transform(carla.Location(39.32, 130.43, 5.84), carla.Rotation(pitch=-29, yaw=180)))
        player_bp_to_spawn = bp_lib.find("static.prop.vendingmachine")
        self.player = self.world.try_spawn_actor(player_bp_to_spawn, carla.Transform(carla.Location(32.338, 130.16, 0.6), carla.Rotation(yaw=180)))
        self.test.assertIsNotNone(self.player)
        self.world.tick()
        self.player_movement = PlayerMovement(self.player)
        self.camera_manager = CameraManager(self.player, self.test, self.start_time)
        self.sensors_array = []
        self.gnss_sensor = None
        self.imu_sensor = None
        self.radar_sensor = None
    
    def spawn_sensor(self, sensor_type):
        if sensor_type == SensorType.Gnss:
            self.gnss_sensor = GnssSensor(self.player, self.start_time)
            self.test.assertIsNotNone(self.gnss_sensor.sensor)
            self.sensors_array.append(self.gnss_sensor.sensor)
        elif sensor_type == SensorType.IMU:
            self.imu_sensor = IMUSensor(self.player, self.start_time)
            self.test.assertIsNotNone(self.imu_sensor.sensor)
            self.sensors_array.append(self.imu_sensor.sensor)
        elif sensor_type == SensorType.Radar:
            self.radar_sensor = RadarSensor(self.player, self.start_time)
            self.test.assertIsNotNone(self.radar_sensor.sensor)
            self.sensors_array.append(self.radar_sensor.sensor)
        else:
            self.camera_manager.spawn_sensor(sensor_type)
            self.test.assertIsNotNone(self.camera_manager)
        
    def game_loop(self):
        while not self.player_movement.movement_finished:
            self.world.tick()
            self.player_movement.move()
            if len(self.camera_manager.spawned_sensors) > 0:
                self.camera_manager.tick() # Waits until all the cameras have the current frame data
            else:
                time.sleep(self.settings.fixed_delta_seconds)
        
    def cleanup(self):
        for sensor in self.sensors_array:
            if sensor is not None:
                sensor.stop()
                sensor.destroy()
                
        if self.camera_manager is not None:
            self.camera_manager.destroy_sensors()
            
        if self.player is not None:
            self.player.destroy()

        self.world.tick()
        sensor_actors = self.world.get_actors().filter('sensor.*')
        self.test.assertEqual(0, sensor_actors.__len__())
        
        settings = self.world.get_settings()
        settings.synchronous_mode = False
        self.world.apply_settings(settings)
        self.world.tick()
    
        
# ==============================================================================
# -- Player Movement -----------------------------------------------------------
# ==============================================================================
class PlayerMovement(object):
    def __init__(self, parent_actor):
        self._parent = parent_actor
        self.movement_finished = False
        self.movement_state = 0
        self.speed = 0.30
        self.angular_speed = 1.4
        self.first_target_x = -28.98
        self.second_target_yaw = -90.0
        self.third_target_y = 79.25

    def move(self):
        player_transform = self._parent.get_transform()
        forward_vec = player_transform.get_forward_vector()

        if self.movement_state == 0:
            player_location = player_transform.location + carla.Location(self.speed * forward_vec.x, self.speed * forward_vec.y)
            self._parent.set_location(player_location)
            if isclose(self.first_target_x, player_location.x, abs_tol=self.speed):
                self.movement_state = 1
        elif self.movement_state == 1:
            player_transform.location += carla.Location(self.speed * forward_vec.x, self.speed * forward_vec.y)
            player_transform.rotation.yaw += self.angular_speed
            self._parent.set_transform(player_transform)
            if isclose(self.second_target_yaw, player_transform.rotation.yaw, abs_tol=self.angular_speed):
                self.movement_state = 2
        elif self.movement_state == 2:
            player_location = player_transform.location + carla.Location(self.speed * forward_vec.x, self.speed * forward_vec.y)
            self._parent.set_location(player_location)
            if isclose(self.third_target_y, player_location.y, abs_tol=self.speed):
                self.movement_state = 3
                self.movement_finished = True
       

# ==============================================================================
# -- GnssSensor ----------------------------------------------------------------
# ==============================================================================
class GnssSensor(object):
    def __init__(self, parent_actor, start_time):
        self.sensor = None
        self._parent = parent_actor
        self.file_path= "_out/" + start_time + "/GnssSensor/GnssData.csv"
        self.frame_number = 0
        self.lat = 0.0
        self.lon = 0.0
        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.gnss')
        self.sensor = world.spawn_actor(bp, carla.Transform(carla.Location(x=1.0, z=2.8)), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: GnssSensor._on_gnss_event(weak_self, event))
        
        os.makedirs(os.path.dirname(self.file_path), exist_ok=True)

    @staticmethod
    def _on_gnss_event(weak_self, event):
        self = weak_self()
        if not self:
            return
        
        print("Gnss save")
        self.lat = event.latitude
        self.lon = event.longitude
        GnssSensor.save_to_csv(weak_self)
        self.frame_number += 1
    
    @staticmethod
    def save_to_csv(weak_self):
        self = weak_self()
        if not self:
            return
        
        with open(self.file_path, "a+", newline="") as csv_file:
            fieldnames = ["frame", "latitude", "longitude"]
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
            if csv_file.tell() == 0:
                writer.writeheader()

            writer.writerow({"frame": self.frame_number, "latitude": self.lat, "longitude": self.lon})


# ==============================================================================
# -- IMUSensor -----------------------------------------------------------------
# ==============================================================================
class IMUSensor(object):
    def __init__(self, parent_actor, start_time):
        self.sensor = None
        self._parent = parent_actor
        self.start_time = start_time
        self.file_path= "_out/" + start_time + "/IMUSensor/IMUData.csv"
        self.frame_number = 0
        self.accelerometer = (0.0, 0.0, 0.0)
        self.gyroscope = (0.0, 0.0, 0.0)
        self.compass = 0.0
        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.imu')
        self.sensor = world.spawn_actor(bp, carla.Transform(), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda sensor_data: IMUSensor._IMU_callback(weak_self, sensor_data))
        
        os.makedirs(os.path.dirname(self.file_path), exist_ok=True)
        
    @staticmethod
    def _IMU_callback(weak_self, sensor_data):
        self = weak_self()
        if not self:
            return
        limits = (-99.9, 99.9)
        self.accelerometer = (
            max(limits[0], min(limits[1], sensor_data.accelerometer.x)),
            max(limits[0], min(limits[1], sensor_data.accelerometer.y)),
            max(limits[0], min(limits[1], sensor_data.accelerometer.z)))
        self.gyroscope = (
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.x))),
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.y))),
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.z))))
        self.compass = math.degrees(sensor_data.compass)
        
        IMUSensor.save_to_csv(weak_self)
        self.frame_number += 1
    
    @staticmethod
    def save_to_csv(weak_self):
        self = weak_self()
        if not self:
            return
        
        with open(self.file_path, "a+", newline="") as csv_file:
            fieldnames = ["frame", "accelerometer", "gyroscope", "compass"]
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
            if csv_file.tell() == 0:
                writer.writeheader()

            writer.writerow({"frame": self.frame_number, "accelerometer": self.accelerometer, "gyroscope": self.gyroscope, "compass": self.compass})


# ==============================================================================
# -- RadarSensor ---------------------------------------------------------------
# ==============================================================================
class RadarSensor(object):
    def __init__(self, parent_actor, start_time):
        self.sensor = None
        self._parent = parent_actor
        self.start_time = start_time
        self.file_path= "_out/" + start_time + "/RadarSensor/"
        self.frame_number = 0
        bound_x = 0.5 + self._parent.bounding_box.extent.x
        bound_y = 0.5 + self._parent.bounding_box.extent.y
        bound_z = 0.5 + self._parent.bounding_box.extent.z

        self.velocity_range = 7.5 # m/s
        world = self._parent.get_world()
        self.draw_radar = False # Set to True if we want to show Radar point cloud in camera sensors.
        self.debug = world.debug
        bp = world.get_blueprint_library().find('sensor.other.radar')
        bp.set_attribute('horizontal_fov', str(35))
        bp.set_attribute('vertical_fov', str(20))
        self.sensor = world.spawn_actor(bp, carla.Transform(carla.Location(x=bound_x + 0.05, z=bound_z+0.05), carla.Rotation(pitch=5)), attach_to=self._parent)
        # We need a weak reference to self to avoid circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda radar_data: RadarSensor._Radar_callback(weak_self, radar_data))
        
        os.makedirs(os.path.dirname(self.file_path), exist_ok=True)

    @staticmethod
    def _Radar_callback(weak_self, radar_data):
        self = weak_self()
        if not self:
            return
        
        RadarSensor.save_to_csv(weak_self, radar_data)
        if self.draw_radar:
            RadarSensor.draw_debug(weak_self, radar_data)
        self.frame_number += 1
            
    @staticmethod
    def save_to_csv(weak_self, radar_data):
        self = weak_self()
        if not self:
            return
        
        final_path = self.file_path + "frame_" + str(self.frame_number) + ".csv" 
        with open(final_path, "a+", newline="") as csv_file:
            fieldnames = ["Altitude", "Azimuth", "Depth", "Velocity"]
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
            writer.writeheader()

            for point in radar_data:
                writer.writerow({"Altitude": point.altitude, "Azimuth": point.azimuth, "Depth": point.depth, "Velocity": point.velocity})
        
    @staticmethod
    def draw_debug(weak_self, radar_data): # method disabled by default as it shows artifacts in the other cameras
        self = weak_self()
        if not self:
            return

        current_rot = radar_data.transform.rotation
        for detect in radar_data:
            azi = math.degrees(detect.azimuth)
            alt = math.degrees(detect.altitude)
            # The 0.25 adjusts a bit the distance so the dots can be properly seen
            fw_vec = carla.Vector3D(x=detect.depth - 0.25)
            carla.Transform(carla.Location(), carla.Rotation(pitch=current_rot.pitch + alt, yaw=current_rot.yaw + azi, roll=current_rot.roll)).transform(fw_vec)

            def clamp(min_v, max_v, value):
                return max(min_v, min(value, max_v))

            norm_velocity = detect.velocity / self.velocity_range # range [-1, 1]
            r = int(clamp(0.0, 1.0, 1.0 - norm_velocity) * 255.0)
            g = int(clamp(0.0, 1.0, 1.0 - abs(norm_velocity)) * 255.0)
            b = int(abs(clamp(- 1.0, 0.0, - 1.0 - norm_velocity)) * 255.0)
            self.debug.draw_point(radar_data.transform.location + fw_vec, size=0.075, life_time=0.06, persistent_lines=False, color=carla.Color(r, g, b))
            
            
# ==============================================================================
# -- CameraManager -------------------------------------------------------------
# ==============================================================================
class CameraManager(object):
    def __init__(self, parent_actor, unit_test, start_time):
        self._parent = parent_actor
        self._unit_test = unit_test
        self.spawned_sensors = []
        self.spawned_sensors_frames = [] # used for sync mode to wait until last sensor is updated.
        self.start_time = start_time
        self.frame = None
        self.data_frame = None
        bound_x = 0.5 + self._parent.bounding_box.extent.x
        bound_y = 0.5 + self._parent.bounding_box.extent.y
        bound_z = 0.5 + self._parent.bounding_box.extent.z
        Attachment = carla.AttachmentType

        self._camera_transforms = [
            (carla.Transform(carla.Location(x=-2.0*bound_x, y=+0.0*bound_y, z=2.0*bound_z), carla.Rotation(pitch=8.0)), Attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=+0.8*bound_x, y=+0.0*bound_y, z=1.3*bound_z)), Attachment.Rigid),
            (carla.Transform(carla.Location(x=+1.9*bound_x, y=+1.0*bound_y, z=1.2*bound_z)), Attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=-2.8*bound_x, y=+0.0*bound_y, z=4.6*bound_z), carla.Rotation(pitch=6.0)), Attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=-1.0, y=-1.0*bound_y, z=0.4*bound_z)), Attachment.Rigid)
        ]
        self.transform_index = 1
        
        self.sensors = [
            [SensorType.RGB, 'sensor.camera.rgb', cc.Raw, 'CameraRGB', {}],
            [SensorType.Depth_Raw, 'sensor.camera.depth', cc.Raw, 'CameraDepth (Raw)', {}],
            [SensorType.Depth_GrayScale, 'sensor.camera.depth', cc.Depth, 'CameraDepth (GrayScale)', {}],
            [SensorType.Depth_LogScale, 'sensor.camera.depth', cc.LogarithmicDepth, 'CameraDepth (LogarithmicGrayScale)', {}],
            [SensorType.SemanticSeg_Raw, 'sensor.camera.semantic_segmentation', cc.Raw, 'CameraSemanticSegmentation (Raw)', {}],
            [SensorType.SemanticSeg_CityScapesPalette, 'sensor.camera.semantic_segmentation', cc.CityScapesPalette, 'CameraSemanticSegmentation (CityScapesPalette)', {}],
            [SensorType.InstanceSeg_Raw, 'sensor.camera.instance_segmentation', cc.Raw, 'CameraInstanceSegmentation (Raw)', {}],
            [SensorType.InstanceSeg_CityScapesPalette, 'sensor.camera.instance_segmentation', cc.CityScapesPalette, 'CameraInstanceSegmentation (CityScapesPalette)', {}],
            [SensorType.Lidar, 'sensor.lidar.ray_cast', None, 'Lidar (Ray-Cast)', {'range': '50'}],
            [SensorType.SemanticLidar, 'sensor.lidar.ray_cast_semantic', None, 'SemanticLidar (Ray-Cast)', {'range': '50'}],
            [SensorType.RGB_Distorted, 'sensor.camera.rgb', cc.Raw, 'Camera RGB Distorted', {'lens_circle_multiplier': '3.0', 'lens_circle_falloff': '3.0', 'chromatic_aberration_intensity': '0.5', 'chromatic_aberration_offset': '0'}],
        ]
        
        self.configure_sensors()

    def configure_sensors(self):
        world = self._parent.get_world()
        bp_library = world.get_blueprint_library()
        for item in self.sensors:
            bp = bp_library.find(item[1])
            if item[1].startswith('sensor.camera'):
                bp.set_attribute('image_size_x', '1280')
                bp.set_attribute('image_size_y', '720')
                if bp.has_attribute('gamma'):
                    bp.set_attribute('gamma', str(2.2))
                for attr_name, attr_value in item[4].items():
                    bp.set_attribute(attr_name, attr_value)
            elif item[1].startswith('sensor.lidar'):
                for attr_name, attr_value in item[4].items():
                    bp.set_attribute(attr_name, attr_value)

            item.append(bp)
        
    def spawn_sensor(self, sensor_type):
        for sensor_item in self.sensors:
            if sensor_type == sensor_item[0]:
                spawned_sensor = self._parent.get_world().spawn_actor(
                    sensor_item[-1],
                    self._camera_transforms[self.transform_index][0],
                    attach_to=self._parent,
                    attachment_type=self._camera_transforms[self.transform_index][1])
                
                self._unit_test.assertIsNotNone(spawned_sensor)
                self.spawned_sensors.append(spawned_sensor)
                # We need to pass the lambda a weak reference to self to avoid circular reference.
                weak_self = weakref.ref(self)
                sensor_name = sensor_item[3]
                sensor_color = sensor_item[2]
                self.spawned_sensors_frames.append([sensor_name, 0])
                spawned_sensor.listen(lambda image, sensor_name=sensor_name, sensor_color=sensor_color: CameraManager._parse_image(weak_self, image, sensor_name, sensor_color))
                break

    def tick(self):
        world = self._parent.get_world()
        self.frame = world.tick()
        # Check if all spawned camera sensors are updated
        while True:
            sensors_updated = True
            for sensor_data in self.spawned_sensors_frames:
                if sensor_data[1] < self.frame:
                    sensors_updated = False
                    break
            
            if sensors_updated == True:
                return
        

    @staticmethod
    def _parse_image(weak_self, image, sensor_name, sensor_color):
        self = weak_self()
        if not self:
            return
        
        if sensor_color == cc.Raw or sensor_color == None:
            image.save_to_disk('_out/' + self.start_time + '/' + sensor_name + '/%08d' % image.frame)
        else:
            image.save_to_disk('_out/' + self.start_time + '/' + sensor_name + '/%08d' % image.frame, sensor_color)
        
        # update frame of the sensor
        for sensor_data in self.spawned_sensors_frames:
            if sensor_data[0] == sensor_name:
                sensor_data[1] = image.frame
        
    def destroy_sensors(self):
        for sensor in self.spawned_sensors:
            if sensor is not None:
                sensor.stop()
                sensor.destroy()
                
                
# ==============================================================================
# -- Tests ---------------------------------------------------------------------
# ==============================================================================
class TestSensorRecording(unittest.TestCase):  
    def test_gnss(self):
        print("TestSensorRecording.test_gnss")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.Gnss)
        scenario.game_loop()
        scenario.cleanup()
        
    def test_imu(self):
        print("TestSensorRecording.test_imu")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.IMU)
        scenario.game_loop()
        scenario.cleanup()
    
    def test_radar(self):
        print("TestSensorRecording.test_radar")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.Radar)
        scenario.game_loop()
        scenario.cleanup()
        
    def test_rgb(self):
        print("TestSensorRecording.test_rgb")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.RGB)
        scenario.spawn_sensor(SensorType.RGB_Distorted)
        scenario.game_loop()
        scenario.cleanup()
        
    def test_depth(self):
        print("TestSensorRecording.test_depth")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.Depth_Raw)
        scenario.spawn_sensor(SensorType.Depth_GrayScale)
        scenario.spawn_sensor(SensorType.Depth_LogScale)
        scenario.game_loop()
        scenario.cleanup()
        
    def test_instance_segmentation(self):
        print("TestSensorRecording.test_instance_segmentation")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.InstanceSeg_Raw)
        scenario.spawn_sensor(SensorType.InstanceSeg_CityScapesPalette)
        scenario.game_loop()
        scenario.cleanup()
        
    def test_semantic_segmentation(self):
        print("TestSensorRecording.test_semantic_segmentation")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.SemanticSeg_CityScapesPalette)
        scenario.spawn_sensor(SensorType.SemanticSeg_Raw)
        scenario.game_loop()
        scenario.cleanup()
        
    def test_lidar(self):
        print("TestSensorRecording.test_lidar")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.Lidar)
        scenario.game_loop()
        scenario.cleanup()
        
    def test_semantic_lidar(self):
        print("TestSensorRecording.test_semantic_lidar")
        scenario = Scenario(self)
        scenario.spawn_sensor(SensorType.SemanticLidar)
        scenario.game_loop()
        scenario.cleanup()
        
