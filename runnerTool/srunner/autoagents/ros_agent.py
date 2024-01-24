#!/usr/bin/env python
#
# Copyright (c) 2019 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
#

"""
This module provides a ROS autonomous agent interface to control the ego vehicle via a ROS stack
"""

import math
import os
import subprocess
import signal
import threading
import time

import numpy

import carla

import rospy
from cv_bridge import CvBridge
from geometry_msgs.msg import PoseStamped
from nav_msgs.msg import Odometry, Path
from rosgraph_msgs.msg import Clock
from sensor_msgs.msg import Image, PointCloud2, NavSatFix, NavSatStatus, CameraInfo
from sensor_msgs.point_cloud2 import create_cloud_xyz32
from std_msgs.msg import Header, String
import tf
# pylint: disable=line-too-long
from carla_msgs.msg import CarlaEgoVehicleStatus, CarlaEgoVehicleInfo, CarlaEgoVehicleInfoWheel, CarlaEgoVehicleControl, CarlaWorldInfo
# pylint: enable=line-too-long

from srunner.autoagents.autonomous_agent import AutonomousAgent


class RosAgent(AutonomousAgent):

    """
    Base class for ROS-based stacks.

    Derive from it and implement the sensors() method.

    Please define TEAM_CODE_ROOT in your environment.
    The stack is started by executing $TEAM_CODE_ROOT/start.sh

    The sensor data is published on similar topics as with the carla-ros-bridge. You can find details about
    the utilized datatypes there.

    This agent expects a roscore to be running.
    """

    speed = None
    current_control = None
    stack_process = None
    timestamp = None
    current_map_name = None
    step_mode_possible = None
    vehicle_info_publisher = None
    global_plan_published = None

    def setup(self, path_to_conf_file):
        """
        setup agent
        """
        self.stack_thread = None

        # get start_script from environment
        team_code_path = os.environ['TEAM_CODE_ROOT']
        if not team_code_path or not os.path.exists(team_code_path):
            raise IOError("Path '{}' defined by TEAM_CODE_ROOT invalid".format(team_code_path))
        start_script = "{}/start.sh".format(team_code_path)
        if not os.path.exists(start_script):
            raise IOError("File '{}' defined by TEAM_CODE_ROOT invalid".format(start_script))

        # set use_sim_time via commandline before init-node
        process = subprocess.Popen(
            "rosparam set use_sim_time true", shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
        process.wait()
        if process.returncode:
            raise RuntimeError("Could not set use_sim_time")

        # initialize ros node
        rospy.init_node('ros_agent', anonymous=True)

        # publish first clock value '0'
        self.clock_publisher = rospy.Publisher('clock', Clock, queue_size=10, latch=True)
        self.clock_publisher.publish(Clock(rospy.Time.from_sec(0)))

        # execute script that starts the ad stack (remains running)
        rospy.loginfo("Executing stack...")
        self.stack_process = subprocess.Popen(start_script, shell=True, preexec_fn=os.setpgrp)

        self.vehicle_control_event = threading.Event()
        self.timestamp = None
        self.speed = 0
        self.global_plan_published = False

        self.vehicle_info_publisher = None
        self.vehicle_status_publisher = None
        self.odometry_publisher = None
        self.world_info_publisher = None
        self.map_file_publisher = None
        self.current_map_name = None
        self.tf_broadcaster = None
        self.step_mode_possible = False

        self.vehicle_control_subscriber = rospy.Subscriber(
            '/carla/ego_vehicle/vehicle_control_cmd', CarlaEgoVehicleControl, self.on_vehicle_control)

        self.current_control = carla.VehicleControl()

        self.waypoint_publisher = rospy.Publisher(
            '/carla/ego_vehicle/waypoints', Path, queue_size=1, latch=True)

        self.publisher_map = {}
        self.id_to_sensor_type_map = {}
        self.id_to_camera_info_map = {}
        self.cv_bridge = CvBridge()

        # setup ros publishers for sensors
        # pylint: disable=line-too-long
        for sensor in self.sensors():
            self.id_to_sensor_type_map[sensor['id']] = sensor['type']
            if sensor['type'] == 'sensor.camera.rgb':
                self.publisher_map[sensor['id']] = rospy.Publisher(
                    '/carla/ego_vehicle/camera/rgb/' + sensor['id'] + "/image_color", Image, queue_size=1, latch=True)
                self.id_to_camera_info_map[sensor['id']] = self.build_camera_info(sensor)
                self.publisher_map[sensor['id'] + '_info'] = rospy.Publisher(
                    '/carla/ego_vehicle/camera/rgb/' + sensor['id'] + "/camera_info", CameraInfo, queue_size=1, latch=True)
            elif sensor['type'] == 'sensor.lidar.ray_cast':
                self.publisher_map[sensor['id']] = rospy.Publisher(
                    '/carla/ego_vehicle/lidar/' + sensor['id'] + "/point_cloud", PointCloud2, queue_size=1, latch=True)
            elif sensor['type'] == 'sensor.other.gnss':
                self.publisher_map[sensor['id']] = rospy.Publisher(
                    '/carla/ego_vehicle/gnss/' + sensor['id'] + "/fix", NavSatFix, queue_size=1, latch=True)
            elif sensor['type'] == 'sensor.can_bus':
                if not self.vehicle_info_publisher:
                    self.vehicle_info_publisher = rospy.Publisher(
                        '/carla/ego_vehicle/vehicle_info', CarlaEgoVehicleInfo, queue_size=1, latch=True)
                if not self.vehicle_status_publisher:
                    self.vehicle_status_publisher = rospy.Publisher(
                        '/carla/ego_vehicle/vehicle_status', CarlaEgoVehicleStatus, queue_size=1, latch=True)
            elif sensor['type'] == 'sensor.hd_map':
                if not self.odometry_publisher:
                    self.odometry_publisher = rospy.Publisher(
                        '/carla/ego_vehicle/odometry', Odometry, queue_size=1, latch=True)
                if not self.world_info_publisher:
                    self.world_info_publisher = rospy.Publisher(
                        '/carla/world_info', CarlaWorldInfo, queue_size=1, latch=True)
                if not self.map_file_publisher:
                    self.map_file_publisher = rospy.Publisher('/carla/map_file', String, queue_size=1, latch=True)
                if not self.tf_broadcaster:
                    self.tf_broadcaster = tf.TransformBroadcaster()
            else:
                raise TypeError("Invalid sensor type: {}".format(sensor['type']))
        # pylint: enable=line-too-long

    def destroy(self):
        """
        Cleanup of all ROS publishers
        """
        if self.stack_process and self.stack_process.poll() is None:
            rospy.loginfo("Sending SIGTERM to stack...")
            os.killpg(os.getpgid(self.stack_process.pid), signal.SIGTERM)
            rospy.loginfo("Waiting for termination of stack...")
            self.stack_process.wait()
            time.sleep(5)
            rospy.loginfo("Terminated stack.")

        rospy.loginfo("Stack is no longer running")
        self.world_info_publisher.unregister()
        self.map_file_publisher.unregister()
        self.vehicle_status_publisher.unregister()
        self.vehicle_info_publisher.unregister()
        self.waypoint_publisher.unregister()
        self.stack_process = None
        rospy.loginfo("Cleanup finished")

    def on_vehicle_control(self, data):
        """
        callback if a new vehicle control command is received
        """
        cmd = carla.VehicleControl()
        cmd.throttle = data.throttle
        cmd.steer = data.steer
        cmd.brake = data.brake
        cmd.hand_brake = data.hand_brake
        cmd.reverse = data.reverse
        cmd.gear = data.gear
        cmd.manual_gear_shift = data.manual_gear_shift
        self.current_control = cmd
        if not self.vehicle_control_event.is_set():
            self.vehicle_control_event.set()
        # After the first vehicle control is sent out, it is possible to use the stepping mode
        self.step_mode_possible = True

    def build_camera_info(self, attributes):  # pylint: disable=no-self-use
        """
        Private function to compute camera info

        camera info doesn't change over time
        """
        camera_info = CameraInfo()
        # store info without header
        camera_info.header = None
        camera_info.width = int(attributes['width'])
        camera_info.height = int(attributes['height'])
        camera_info.distortion_model = 'plumb_bob'
        cx = camera_info.width / 2.0
        cy = camera_info.height / 2.0
        fx = camera_info.width / (
            2.0 * math.tan(float(attributes['fov']) * math.pi / 360.0))
        fy = fx
        camera_info.K = [fx, 0, cx, 0, fy, cy, 0, 0, 1]
        camera_info.D = [0, 0, 0, 0, 0]
        camera_info.R = [1.0, 0, 0, 0, 1.0, 0, 0, 0, 1.0]
        camera_info.P = [fx, 0, cx, 0, 0, fy, cy, 0, 0, 0, 1.0, 0]
        return camera_info

    def publish_plan(self):
        """
        publish the global plan
        """
        msg = Path()
        msg.header.frame_id = "/map"
        msg.header.stamp = rospy.Time.now()
        for wp in self._global_plan_world_coord:
            pose = PoseStamped()
            pose.pose.position.x = wp[0].location.x
            pose.pose.position.y = -wp[0].location.y
            pose.pose.position.z = wp[0].location.z
            quaternion = tf.transformations.quaternion_from_euler(
                0, 0, -math.radians(wp[0].rotation.yaw))
            pose.pose.orientation.x = quaternion[0]
            pose.pose.orientation.y = quaternion[1]
            pose.pose.orientation.z = quaternion[2]
            pose.pose.orientation.w = quaternion[3]
            msg.poses.append(pose)

        rospy.loginfo("Publishing Plan...")
        self.waypoint_publisher.publish(msg)

    def sensors(self):
        """
        Define the sensor suite required by the agent

        :return: a list containing the required sensors
        """
        raise NotImplementedError(
            "This function has to be implemented by the derived classes")

    def get_header(self):
        """
        Returns ROS message header
        """
        header = Header()
        header.stamp = rospy.Time.from_sec(self.timestamp)
        return header

    def publish_lidar(self, sensor_id, data):
        """
        Function to publish lidar data
        """
        header = self.get_header()
        header.frame_id = 'ego_vehicle/lidar/{}'.format(sensor_id)

        lidar_data = numpy.frombuffer(
            data, dtype=numpy.float32)
        lidar_data = numpy.reshape(
            lidar_data, (int(lidar_data.shape[0] / 3), 3))
        # we take the oposite of y axis
        # (as lidar point are express in left handed coordinate system, and ros need right handed)
        # we need a copy here, because the data are read only in carla numpy
        # array
        lidar_data = -1.0 * lidar_data
        # we also need to permute x and y
        lidar_data = lidar_data[..., [1, 0, 2]]
        msg = create_cloud_xyz32(header, lidar_data)
        self.publisher_map[sensor_id].publish(msg)

    def publish_gnss(self, sensor_id, data):
        """
        Function to publish gnss data
        """
        msg = NavSatFix()
        msg.header = self.get_header()
        msg.header.frame_id = 'gps'
        msg.latitude = data[0]
        msg.longitude = data[1]
        msg.altitude = data[2]
        msg.status.status = NavSatStatus.STATUS_SBAS_FIX
        # pylint: disable=line-too-long
        msg.status.service = NavSatStatus.SERVICE_GPS | NavSatStatus.SERVICE_GLONASS | NavSatStatus.SERVICE_COMPASS | NavSatStatus.SERVICE_GALILEO
        # pylint: enable=line-too-long
        self.publisher_map[sensor_id].publish(msg)

    def publish_camera(self, sensor_id, data):
        """
        Function to publish camera data
        """
        msg = self.cv_bridge.cv2_to_imgmsg(data, encoding='bgra8')
        # the camera data is in respect to the camera's own frame
        msg.header = self.get_header()
        msg.header.frame_id = 'ego_vehicle/camera/rgb/{}'.format(sensor_id)

        cam_info = self.id_to_camera_info_map[sensor_id]
        cam_info.header = msg.header
        self.publisher_map[sensor_id + '_info'].publish(cam_info)
        self.publisher_map[sensor_id].publish(msg)

    def publish_can(self, sensor_id, data):
        """
        publish can data
        """
        if not self.vehicle_info_publisher:
            self.vehicle_info_publisher = rospy.Publisher(
                '/carla/ego_vehicle/vehicle_info', CarlaEgoVehicleInfo, queue_size=1, latch=True)
            info_msg = CarlaEgoVehicleInfo()
            for wheel in data['wheels']:
                wheel_info = CarlaEgoVehicleInfoWheel()
                wheel_info.tire_friction = wheel['tire_friction']
                wheel_info.damping_rate = wheel['damping_rate']
                wheel_info.steer_angle = wheel['steer_angle']
                wheel_info.disable_steering = wheel['disable_steering']
                info_msg.wheels.append(wheel_info)
            info_msg.max_rpm = data['max_rpm']
            info_msg.moi = data['moi']
            info_msg.damping_rate_full_throttle = data['damping_rate_full_throttle']
            info_msg.damping_rate_zero_throttle_clutch_disengaged = data['damping_rate_zero_throttle_clutch_disengaged']
            info_msg.use_gear_autobox = data['use_gear_autobox']
            info_msg.clutch_strength = data['clutch_strength']
            info_msg.mass = data['mass']
            info_msg.drag_coefficient = data['drag_coefficient']
            info_msg.center_of_mass.x = data['center_of_mass']['x']
            info_msg.center_of_mass.y = data['center_of_mass']['y']
            info_msg.center_of_mass.z = data['center_of_mass']['z']
            self.vehicle_info_publisher.publish(info_msg)
        msg = CarlaEgoVehicleStatus()
        msg.header = self.get_header()
        msg.velocity = data['speed']
        self.speed = data['speed']
        # todo msg.acceleration
        msg.control.throttle = self.current_control.throttle
        msg.control.steer = self.current_control.steer
        msg.control.brake = self.current_control.brake
        msg.control.hand_brake = self.current_control.hand_brake
        msg.control.reverse = self.current_control.reverse
        msg.control.gear = self.current_control.gear
        msg.control.manual_gear_shift = self.current_control.manual_gear_shift

        self.vehicle_status_publisher.publish(msg)

    def publish_hd_map(self, sensor_id, data):
        """
        publish hd map data
        """
        roll = -math.radians(data['transform']['roll'])
        pitch = -math.radians(data['transform']['pitch'])
        yaw = -math.radians(data['transform']['yaw'])
        quat = tf.transformations.quaternion_from_euler(roll, pitch, yaw)
        x = data['transform']['x']
        y = -data['transform']['y']
        z = data['transform']['z']

        if self.odometry_publisher:

            odometry = Odometry()
            odometry.header.frame_id = 'map'
            odometry.header.stamp = rospy.Time.from_sec(self.timestamp)
            odometry.child_frame_id = 'base_link'
            odometry.pose.pose.position.x = x
            odometry.pose.pose.position.y = y
            odometry.pose.pose.position.z = z

            odometry.pose.pose.orientation.x = quat[0]
            odometry.pose.pose.orientation.y = quat[1]
            odometry.pose.pose.orientation.z = quat[2]
            odometry.pose.pose.orientation.w = quat[3]

            odometry.twist.twist.linear.x = self.speed
            odometry.twist.twist.linear.y = 0
            odometry.twist.twist.linear.z = 0

            self.odometry_publisher.publish(odometry)

        if self.world_info_publisher:
            # extract map name
            map_name = os.path.basename(data['map_file'])[:-4]
            if self.current_map_name != map_name:
                self.current_map_name = map_name
                world_info = CarlaWorldInfo()
                world_info.map_name = self.current_map_name
                world_info.opendrive = data['opendrive']
                self.world_info_publisher.publish(world_info)
        if self.map_file_publisher:
            self.map_file_publisher.publish(data['map_file'])

    def use_stepping_mode(self):  # pylint: disable=no-self-use
        """
        Overload this function to use stepping mode!
        """
        return False

    def run_step(self, input_data, timestamp):
        """
        Execute one step of navigation.
        """
        self.vehicle_control_event.clear()
        self.timestamp = timestamp
        self.clock_publisher.publish(Clock(rospy.Time.from_sec(timestamp)))

        # check if stack is still running
        if self.stack_process and self.stack_process.poll() is not None:
            raise RuntimeError("Stack exited with: {} {}".format(
                self.stack_process.returncode, self.stack_process.communicate()[0]))

        # publish global plan to ROS once
        if self._global_plan_world_coord and not self.global_plan_published:
            self.global_plan_published = True
            self.publish_plan()

        new_data_available = False

        # publish data of all sensors
        for key, val in input_data.items():
            new_data_available = True
            sensor_type = self.id_to_sensor_type_map[key]
            if sensor_type == 'sensor.camera.rgb':
                self.publish_camera(key, val[1])
            elif sensor_type == 'sensor.lidar.ray_cast':
                self.publish_lidar(key, val[1])
            elif sensor_type == 'sensor.other.gnss':
                self.publish_gnss(key, val[1])
            elif sensor_type == 'sensor.can_bus':
                self.publish_can(key, val[1])
            elif sensor_type == 'sensor.hd_map':
                self.publish_hd_map(key, val[1])
            else:
                raise TypeError("Invalid sensor type: {}".format(sensor_type))

        if self.use_stepping_mode():
            if self.step_mode_possible and new_data_available:
                self.vehicle_control_event.wait()
        # if the stepping mode is not used or active, there is no need to wait here

        return self.current_control
